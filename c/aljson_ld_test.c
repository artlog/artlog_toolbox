#include "aljson_ld.h"
#include <stdio.h>
#include "al_options.h"

static const char * GET_KEYWORD_INDEXSTR0="get_keyword_index";
static const char * LIST_KEYWORDSSTR0="list_keywords";

void usage()
{
  printf("usage:\n");
  printf("get key index : %s=/keyword/\n", GET_KEYWORD_INDEXSTR0);
  printf("list keywords : %s=/keyword/\n", LIST_KEYWORDSSTR0);
}

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);
  al_options_set_debug(options,0);

  struct alhash_datablock * opt1=al_option_get(options,GET_KEYWORD_INDEXSTR0);
  struct alhash_datablock * opt2=al_option_get(options,LIST_KEYWORDSSTR0);

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

  al_options_release(options);
  
}
