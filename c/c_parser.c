#include <stdio.h>
#include "c_parser.h"
#include "todo.h"
#include "json_import_internal.h"

void usage()
{
  printf("intention is to parse a c file ...");
}

int main(int argc,char ** argv)
{

  struct json_import_context_data importer;
  struct json_ctx tokenizer;
  struct inputstream inputstream;
  FILE * file = NULL;

  if ( argc > 1 )
    {
      file = fopen(argv[1],"r");
    }
  if ( file != NULL )
    {
      json_import_context_initialize(&tokenizer);
      inputstream_init(&inputstream,fileno(file));
      importer.inputstream=&inputstream;
      //json_ctx_set_debug(&tokenizer,TOKENIZER_DEBUG_ADD);
      //json_ctx_set_debug(&tokenizer,TOKENIZER_DEBUG_PUSHBACK);
      json_ctx_set_debug(&tokenizer,0);
  
      usage();

      struct al_token * token;
  
      while ( (token = c_tokenizer(&tokenizer,&importer)) != NULL )
	{
	  printf("\n<%i>",token->token);
	  switch(token->token)
	    {
	    case JSON_TOKEN_NUMBER_ID:
	    case JSON_TOKEN_WORD_ID:
	      {
		// lazzy cut_string...
		tokenizer.buf[tokenizer.bufpos]=0;
		printf("%s",tokenizer.buf);
		break;
	      }
	    }  
	  printf("</%i>",token->token);
	  if ( token->token == JSON_TOKEN_EOF_ID )
	    {
	      printf("EOF id reached");
	      break;
	    }
	  // reset
	  tokenizer.bufpos=0;
	}

      fflush(stdout);
      todo("implement parser ... !");
    }
}
