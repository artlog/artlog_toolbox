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
  
char main_next_char(struct json_ctx* ctx, void * data)
{
  struct main_context_data * main_data = (struct main_context_data *) data;
  if ( ( main_data->flags &  ALSFLAG_PUSHBACK ) != 0 )
    {
      main_data->flags ^= ALSFLAG_PUSHBACK;
      return main_data->last;
    }
    
  if ( fread(&main_data->last, 1, 1, main_data->f) == 1 )
    {
      if ( main_data->last ==  0x0a )
	{	  
	  ++ctx->pos_info.column;
	}
      else if ( main_data->last != 0x0c )
	{
	  ++ctx->pos_info.line;
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
  if ( ( main_data->flags &  ALSFLAG_PUSHBACK ) != 0 )
    {
      // two pushback ... NOT SUPPORTED
      fprintf(stderr,"2 pushbacks line %i column %i\n", ctx->pos_info.line, ctx->pos_info.column);
    }    
  main_data->flags &= ALSFLAG_PUSHBACK;
  ctx->pos--;
}

/**
main
First argument : filename to open in read only mode to parse in json.

Output : dump parsed json to standard output
 **/
int main(int argc, char ** argv)
{
  struct json_ctx json_context;
  struct print_ctx print_context;

  struct main_context_data data;

  json_context_initialize( &json_context);

  json_context.next_char=main_next_char;
  json_context.pushback_char=main_pushback_char;
  
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

  if (argc > 1)
    {
      data.last=0;
      data.flags=0;
      data.f = fopen(argv[1],"r");
      if ( data.f != NULL )
	{
	  struct json_object * root=NULL;
	  root=parse_level(&json_context,&data,root);
	  fclose(data.f);
	  //dump_ctx(&json_context);
	  dump_object(&json_context,root,&print_context);
	}
      else
	{
	  exit(1);
	}
      
    }
  exit(0);
}
