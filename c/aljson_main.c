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

const char * aljson_main_version="0.2";

/**
a complicated json stream ( one char ahead ) parser 

see usage()
**/

static int main_debug=0;

void usage()
{
  aldebug_printf(DBGSTREAM,"Output : dump parsed json to standard output.\n");
  aldebug_printf(DBGSTREAM,"-d debug\n");
  aldebug_printf(DBGSTREAM,"-m non recursive\n");
  aldebug_printf(DBGSTREAM,"-c check only (no print)\n");
  aldebug_printf(DBGSTREAM,"-b bare : no indent\n");
  aldebug_printf(DBGSTREAM,"indent 2space : hierarchical two spaces header\n");
  aldebug_printf(DBGSTREAM,"json_path=<path>\n");
  aldebug_printf(DBGSTREAM,"template= filename to open in read only mode to parse in json for template.\n");
  aldebug_printf(DBGSTREAM,"          template is used for json unification ie extracting fields from a template pattern\n");
  aldebug_printf(DBGSTREAM,"-- to separate options from arguments\n");  
  aldebug_printf(DBGSTREAM,"First argument : filename to open in read only mode to parse in json.\n");

  aldebug_printf(DBGSTREAM,"\naljson_main version %s\n",aljson_main_version);
}

int main(int argc, char ** argv)
{
  char * json_filename = NULL;
  char * json_template = NULL;
  char * json_path = NULL;
  int debug = 0;
  int checkonly = 0;

  FILE * data_file;
  FILE * template_file;

  aldebug_start(NULL);
  
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
  aljson_print_ctx_set_format(&print_context, ALJSON_PRINT_FLAT);
  
  struct al_options * options = al_options_create(argc,argv);

  // don't set debug to options
  al_options_set_debug(options,0);

  debug = (al_option_get(options,"d") == NULL) ? 0 : 1;
  checkonly = (al_option_get(options,"c") == NULL) ? 0 : 1;
  
  struct alhash_datablock * json_path_value = al_option_get(options,"json_path");
  if ( json_path_value != NULL )
    {
      json_path = json_path_value->data.charptr;
    }

  if ( al_option_get(options,"b") != NULL )
    {
      // bare => no indent
      aljson_print_ctx_set_format(&print_context, ALJSON_PRINT_FLAT);
    }

  if ( al_option_get(options,"indent") != NULL )
    {
      // indent, TODO should select flat,2space,tab, default 2space
      aljson_print_ctx_set_format(&print_context, ALJSON_PRINT_2SPACE);
    }


  struct alhash_datablock * json_template_value = al_option_get(options,"template");
  if ( json_template_value != NULL )
    {
      json_template = json_template_value->data.charptr;
    }

  if ( al_option_getargsnumber(options) > 0 )
    {
      json_filename = al_option_getarg(options,0);
    }
  
  if ( debug > 0 )
    {
      struct aloutputstream output;
      aloutputstream_fd_init(&output,fileno(stderr));
      al_option_dump_output(options,&output);
    }
			
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
	  aldebug_printf(DBGSTREAM,"parsing json filename : %s\n", json_filename);
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
	    
	  if ( json_path != NULL )
	    {
	      struct json_object * found = aljson_walk_path(json_path, &json_context,root);
	      if ( found != NULL )
		{
		  aljson_output(found,&print_context);
		}
	      else
		{
		  aldebug_printf(DBGSTREAM," NOT FOUND.");
		}
	    }
	  else {
	    if ( checkonly == 0 )
	      {
		aljson_output(root,&print_context);
	      }
	    else
	      {
		aldebug_printf(DBGSTREAM,"parsing complete\n");
	      }
	  }
	    
	  if ( json_template != NULL )
	    {
	      if ( debug > 0 )
		{
		  aldebug_printf(DBGSTREAM,"parsing json template : %s\n", json_template);
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
		  if ( debug > 0 )
		    {
		      aljson_output(template_root,&print_template_context);
		    }
		  aldebug_printf(DBGSTREAM,"\n");
		  if ( aljson_unify_object(&json_context, root, &json_template_context, template_root,&print_template_context) )
		    {
		      aldebug_printf(DBGSTREAM,"\ntemplate '%s' and '%s' json match\n", json_template, json_filename);
		      exit(0);
		    }
		  else
		    {
		      aldebug_printf(DBGSTREAM,"\ntemplate '%s' and '%s' json DOES NOT match\n", json_template, json_filename);
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

  aldebug_end();
  exit(0);
}

