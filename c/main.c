#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "json.h"

/**
a complicated json stream ( one char ahead ) parser 
**/

/** fully rely on stream buffering */
struct main_context_data {
  char last;
  FILE * f;
  int flags;
};

enum als_flag {
  ALSFLAG_PUSHBACK = 1
};

static int main_debug=0;

char main_next_char(struct json_ctx* ctx, void * data)
{
  struct main_context_data * main_data = (struct main_context_data *) data;
  if ( ( main_data->flags &  ALSFLAG_PUSHBACK ) != 0 )
    {
      main_data->flags ^= ALSFLAG_PUSHBACK;
      if ( main_debug > 0 )
	{
	  printf("main_next_char next_char got pushedback value '%c'",main_data->last);
	}
      return main_data->last;
    }
    
  if ( fread(&main_data->last, 1, 1, main_data->f) == 1 )
    {
      if ( main_data->last ==  0x0a )
	{	  
	  ++ctx->pos_info.line;
	  ctx->pos_info.column=0;
	}
      else if ( main_data->last != 0x0c )
	{
	  ++ctx->pos_info.column;
	}
      ++ctx->pos;
      return main_data->last;
    }
  else
    {
      return 0;
    }
}

void main_pushback_char(struct json_ctx *ctx, void *data, char pushback)
{
  struct main_context_data * main_data = (struct main_context_data *) data;
  // should not pushback something else than previous char. programming error
  assert( (ctx->pos>0) && (pushback == main_data->last ));
  if ( pushback != main_data->last )
    {
      fprintf(stderr,"assert + pushback error %c != %c \n", pushback, main_data->last);
    }
  else if ( main_debug > 0 )
    {
      printf("pushback '%c'\n", pushback);
    }
  
  if ( ( main_data->flags &  ALSFLAG_PUSHBACK ) != 0 )
    {
      // two pushback ... NOT SUPPORTED
      fprintf(stderr,"2 pushbacks line %i column %i\n", ctx->pos_info.line, ctx->pos_info.column);
    }    
  main_data->flags |= ALSFLAG_PUSHBACK;  
  ctx->pos--;
}

void usage()
{
  printf("First argument : filename to open in read only mode to parse in json.\n");
  printf("Second argument : filename to open in read only mode to parse in json for template.\n");
  printf("Output : dump parsed json to standard output.\n");
}

/**
main
First argument : filename to open in read only mode to parse in json.

Output : dump parsed json to standard output
 **/
int main(int argc, char ** argv)
{
  char * json_filename = NULL;
  char * json_template = NULL;
  char * json_path = NULL;
  int debug = 0;
  int path = 0;
  
  struct json_ctx json_context;
  struct print_ctx print_context;

  struct main_context_data data;

  json_context_initialize( &json_context);

  json_context.next_char=main_next_char;
  json_context.pushback_char=main_pushback_char;
  json_context.pos_info.line=0;
  json_context.pos_info.column=0;
  
  /* tabs
  print_context.do_indent = 1;
  print_context.indent = 0;
  print_context.s_indent = "\t";
  */

  // two spaces
  print_context.do_indent = 2;
  print_context.indent = 0;
  print_context.s_indent = " ";
  
  /* flat canonical
  print_context.do_indent = 0;
  print_context.indent = 0;
  print_context.s_indent = NULL;
  */

  struct json_ctx json_template_context;
  struct print_ctx print_template_context;

  struct main_context_data template_data;

  json_context_initialize( &json_template_context);

  json_template_context.next_char=main_next_char;
  json_template_context.pushback_char=main_pushback_char;
  json_template_context.pos_info.line=0;
  json_template_context.pos_info.column=0;
  
  // two spaces
  print_template_context.do_indent = 0;
  print_template_context.indent = 0;
  print_template_context.s_indent = "";
  
  if ( argc > 1 )
    {
      for (int i =1; i< argc ; i++)
	{
	  if ( argv[i][0] != '-' )
	    {
	      if ( path == 1 )
		{
		  if ( json_path == NULL )
		    {
		      json_path = argv[i];
		    }
		  path =  0;
		}
	      else
		{
		  if ( json_filename == NULL )
		    {
		      json_filename = argv[i];
		    }
		  else if ( json_template == NULL )
		    {
		      json_template = argv[i];
		    }
		}
	    }
	  else
	    {
	      switch(argv[i][1])
		{
		case 'd':
		  debug=1;
		  break;
		case 'p':
		  path=1;
		}		  
	    }
	}
    }

  json_set_debug(debug);
  json_ctx_set_debug(&json_context,debug);
  json_ctx_set_debug(&json_template_context,debug);
  main_debug=debug;
  
  if (json_filename != NULL)
    {
      if ( debug > 0)
	{
	  printf("parsing json filename : %s\n", json_filename);
	}
      data.last=0;
      data.flags=0;
      data.f = fopen(json_filename,"r");
      if ( data.f != NULL )
	{
	  struct json_object * root=NULL;
	  root=parse_level(&json_context,&data,root);
	  fclose(data.f);
	  //dump_ctx(&json_context);
	  dump_object(&json_context,root,&print_context);
	  printf("\n");
	  if ( json_path != NULL )
	    {
	      struct json_object * found = json_walk_path(json_path, &json_context,root);
	      if ( found != NULL )
		{
		  printf("=");
		  dump_object(&json_context,found,&print_context);
		}
	      else
		{
		  printf(" NOT FOUND.");
		}
	    }
	  if ( json_template != NULL )
	    {
	      if ( debug > 0 )
		{
		  printf("parsing json template : %s\n", json_template);
		}
	      template_data.last=0;
	      template_data.flags=0;
	      template_data.f = fopen(json_template,"r");
	      if ( template_data.f != NULL )
		{
		  struct json_object * template_root=NULL;
		  template_root=parse_level(&json_template_context,&template_data,template_root);
		  fclose(template_data.f);
		  dump_object(&json_template_context,template_root,&print_template_context);
		  printf("\n");
		  if ( json_unify_object(&json_context, root, &json_template_context, template_root,&print_template_context) )
		    {
		      printf("\n%s and %s json match\n", argv[1], argv[2]);
		      exit(0);
		    }
		  else
		    {
		      printf("\n%s and %s json DOES NOT match\n", argv[1], argv[2]);
		      exit(1);
		    }
		}
	    }
	      
	}
      else
	{
	  exit(1);
	}
      
    }
  else
    {
      usage();
    }
  exit(0);
}

