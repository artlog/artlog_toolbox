#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "json_import_internal.h"

/**
a complicated json stream ( one char ahead ) parser 
**/

static int main_debug=0;

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

  struct json_import_context_data data;

  json_import_context_initialize( &json_context);
  // force legacy
  json_context.unstack=parse_level_legacy;

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

  struct json_import_context_data template_data;

  json_import_context_initialize( &json_template_context);

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
	  root=parse_level_legacy(&json_context,&data,root);
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

