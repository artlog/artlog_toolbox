#include "aljson_ld.h"
#include <stdio.h>
#include "al_options.h"
#include "alinput_file.h"
// for struct print_ctx * , to check if can be avoided
#include "aljson_import_internal.h"


static const char * GET_KEYWORD_INDEXSTR0="get_keyword_index";
static const char * LIST_KEYWORDSSTR0="list_keywords";
static const char * DRY_RUNSTR0="dry_run";

void usage()
{
  printf("usage:\n");
  printf("get key index : %s=/keyword/\n", GET_KEYWORD_INDEXSTR0);
  printf("list keywords : %s=/keyword/\n", LIST_KEYWORDSSTR0);
  printf("dry_run keywords : %s\n", DRY_RUNSTR0);
}

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);
  al_options_set_debug(options,0);

  struct alhash_datablock * opt1=al_option_get(options,GET_KEYWORD_INDEXSTR0);
  struct alhash_datablock * opt2=al_option_get(options,LIST_KEYWORDSSTR0);
  struct alhash_datablock * dry_run=al_option_get(options,DRY_RUNSTR0);

  aljson_ld();
  
  if (opt1 != NULL)
    {
      char * word = opt1->data.charptr;
      int index = aljson_ld_is_keyword(word);
      printf("%s = %i %s\n", word, index, ( index <0 )  ? "?" : aljson_ld_c_keyword(index));
    }
  
  if ( opt2 != NULL )
    {
      aljson_ld_list_keywords();
    }
   else
    {
      usage();
    }

  if ( dry_run != NULL )
    {
      struct aljson_ld_context* context = NULL;
      struct json_object * root = NULL;

        // intialisation of json content parser  
      struct json_parser_ctx json_context;
      struct json_ctx json_tokenizer;
      struct print_ctx print_context;

      aljson_init(&json_context,&json_tokenizer,&print_context);
      aljson_print_ctx_set_format(&print_context, ALJSON_PRINT_FLAT);

      struct json_import_context_data data;
      struct alinputstream inputstream;

      struct aloutputstream output;
      aloutputstream_fd_init(&output,fileno(stderr));
      json_set_debug(0);
	
      if (dry_run->data.charptr != NULL )
	{
	  printf("-----\n");
	  
	  enum al_global_error_code ec_fopen =  alinput_file_open_init(&inputstream, dry_run->data.charptr);
	  printf("json input file=%s\n",dry_run->data.charptr);
	  if ( ec_fopen == AL_EC_OK )
	    {
	      data.last=0;
	      data.flags=0;
	      data.inputstream=&inputstream;
	      // where the parsing actualy take place
	      root=parse_level(&json_context,&data,root);
	      printf("%p\n",root);
	      aljson_output(root,&print_context);
	      printf("\n");
	      struct aljson_ld_named_graph * test = aljson_ld_build_from_json(context,root);

	    }
	  // TODO close inputstream
	}
    }

  al_options_release(options);
  
}
