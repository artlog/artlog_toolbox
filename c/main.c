#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "inputstream.h"
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
Second argument : filename to open in read only mode to parse in json for template

Output : dump parsed json to standard output
 **/
int main(int argc, char ** argv)
{
  char * json_filename = NULL;
  char * json_template = NULL;
  char * json_path = NULL;
  int debug = 0;
  int path = 0;

  FILE * data_file;
  FILE * template_file;

  struct json_parser_ctx json_context;
  struct json_ctx json_tokenizer;
  struct print_ctx print_context;

  struct json_import_context_data data;
  struct inputstream inputstream;
  struct inputstream template_inputstream;

  json_import_context_initialize( &json_tokenizer);
  json_context.tokenizer=&json_tokenizer;

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

  struct json_ctx json_template_tokenizer;
  struct json_parser_ctx json_template_context;
  struct print_ctx print_template_context;

  struct json_import_context_data template_data;

  json_import_context_initialize( &json_template_tokenizer);
  json_template_context.tokenizer=&json_template_tokenizer;

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
  json_ctx_set_debug(&json_tokenizer,debug);
  json_ctx_set_debug(&json_template_tokenizer,debug);
  main_debug=debug;
  
  if (json_filename != NULL)
    {
      if ( debug > 0)
	{
	  printf("parsing json filename : %s\n", json_filename);
	}
      data.last=0;
      data.flags=0;
      data_file = fopen(json_filename,"r");
      if ( data_file != NULL )
	{
	  struct json_object * root=NULL;
	  inputstream_init(&inputstream,fileno(data_file));
	  data.inputstream=&inputstream;
	  root=parse_level(&json_context,&data,root);
	  fclose(data_file);
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
	      template_file = fopen(json_template,"r");
	      if ( template_file != NULL )
		{
		  struct json_object * template_root=NULL;
		  inputstream_init(&template_inputstream,fileno(template_file));
		  template_data.inputstream=&inputstream;
		  template_root=parse_level(&json_template_context,&template_data,template_root);
		  fclose(template_file);
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

