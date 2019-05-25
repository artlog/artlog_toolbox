#include <stdlib.h>
#include <assert.h>
#include <strings.h>
#include <stdio.h>

#include "alinput.h"
#include "aljson_import_internal.h"
#include "aldebug.h"
#include "aljson_unify.h"
#include "aljson_walk.h"
#include "aldebug_output.h"
#include "al_options.h"
#include "al_options_output.h"

const char * aljson_main_version="0.1";

/**
a complicated json stream ( one char ahead ) parser 

see usage()
**/

static int main_debug=0;

void usage()
{
  aldebug_printf(NULL,"Output : dump parsed json to standard output.\n");
  aldebug_printf(NULL,"-d debug\n");
  aldebug_printf(NULL,"-m non recursive\n");
  // aldebug_printf(NULL,"-p path\n");
  aldebug_printf(NULL,"path=<path>\n");
  aldebug_printf(NULL,"-c check only (no print)\n");
  aldebug_printf(NULL,"-b bare : no indent");
  aldebug_printf(NULL,"-- to separate options from arguments\n");  
  aldebug_printf(NULL,"First argument : filename to open in read only mode to parse in json.\n");
  aldebug_printf(NULL,"Second argument : filename to open in read only mode to parse in json for template.\n");
  aldebug_printf(NULL,"                  template is used for json unification ie extratcing fields from a template pattern\n");

  aldebug_printf(NULL,"\naljson_main version %s\n",aljson_main_version);
}

int main(int argc, char ** argv)
{
  char * json_filename = NULL;
  char * json_template = NULL;
  char * json_path = NULL;
  int debug = 0;
  int path = 0;
  int checkonly = 0;

  FILE * data_file;
  FILE * template_file;

  // intialisation of json content parser
  
  struct json_parser_ctx json_context;
  struct json_ctx json_tokenizer;
  struct print_ctx print_context;

  aljson_init(&json_context,&json_tokenizer,&print_context);

  // initialisation of json template parser
  struct json_parser_ctx json_template_context;
  struct json_ctx json_template_tokenizer;
  struct print_ctx print_template_context;

  aljson_init(&json_template_context,&json_template_tokenizer,&print_template_context);
  
  // no indent
  print_template_context.do_indent = 0;
  print_template_context.indent = 0;
  print_template_context.s_indent = "";

  struct al_options * options = al_options_create(argc,argv);

  // don't set debug to options
  al_options_set_debug(options,0);

  debug = (al_option_get(options,"d") == NULL) ? 0 : 1;
  checkonly = (al_option_get(options,"c") == NULL) ? 0 : 1;
  path = (al_option_get(options,"p") == NULL) ? 0 : 1;
  
  struct alhash_datablock * json_path_value = al_option_get(options,"json_path");
  if ( json_path_value != NULL )
    {
      json_path = json_path_value->data.charptr;
    }

  if ( al_option_get(options,"b") != NULL )
    {
      // bare => no indent
      print_context.do_indent = 0;
      print_context.indent = 0;
    }

  if ( al_option_getargsnumber(options) > 0 )
    {
      json_filename = al_option_getarg(options,0);
      json_template = al_option_getarg(options,1);
    }
  
  struct aloutputstream output;
  aloutputstream_fd_init(&output,fileno(stderr));
  
  al_option_dump_output(options,&output);
			
  json_set_debug(debug);
  json_ctx_set_debug(&json_tokenizer,debug);
  json_ctx_set_debug(&json_template_tokenizer,debug);
  main_debug=debug;

  if (json_filename != NULL)
    {
      struct json_import_context_data data;
      struct json_import_context_data template_data;
      
      struct alinputstream inputstream;
      struct alinputstream template_inputstream;

      if ( debug > 0)
	{
	  aldebug_printf(NULL,"parsing json filename : %s\n", json_filename);
	}
      data.last=0;
      data.flags=0;
      data_file = fopen(json_filename,"r");
      if ( data_file != NULL )
	{
	  struct json_object * root=NULL;
	  alinputstream_init(&inputstream,fileno(data_file));
	  data.inputstream=&inputstream;
	  // where the parsing actualy take place
	  root=parse_level(&json_context,&data,root);
	  fclose(data_file);
	  if ( checkonly == 0 )
	    {
	      aljson_output(&json_context,root,&print_context);
	    }
	  else
	    {
	      aldebug_printf(NULL,"parsing complete\n");
	    }
	    
	  if ( json_path != NULL )
	    {
	      struct json_object * found = aljson_walk_path(json_path, &json_context,root);
	      if ( found != NULL )
		{
		  aljson_output(&json_context,found,&print_context);
		}
	      else
		{
		  aldebug_printf(NULL," NOT FOUND.");
		}
	    }
	  if ( json_template != NULL )
	    {
	      if ( debug > 0 )
		{
		  aldebug_printf(NULL,"parsing json template : %s\n", json_template);
		}
	      template_data.last=0;
	      template_data.flags=0;
	      template_file = fopen(json_template,"r");
	      if ( template_file != NULL )
		{
		  struct json_object * template_root=NULL;
		  alinputstream_init(&template_inputstream,fileno(template_file));
		  template_data.inputstream=&inputstream;
		  template_root=parse_level(&json_template_context,&template_data,template_root);
		  fclose(template_file);
		  aljson_output(&json_template_context,template_root,&print_template_context);
		  aldebug_printf(NULL,"\n");
		  if ( aljson_unify_object(&json_context, root, &json_template_context, template_root,&print_template_context) )
		    {
		      aldebug_printf(NULL,"\n%s and %s json match\n", argv[1], argv[2]);
		      exit(0);
		    }
		  else
		    {
		      aldebug_printf(NULL,"\n%s and %s json DOES NOT match\n", argv[1], argv[2]);
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

