#include <stdio.h>
#include <string.h>

#include "c_parser.h"
#include "todo.h"
#include "json_import_internal.h"

// reserved c words
enum c_word_token {
  TOKEN_C_NOMATCH_ID,
  TOKEN_C_STRUCT_ID,
  TOKEN_C_ENUM_ID,
  TOKEN_C_SWITCH_ID,
  TOKEN_C_CASE_ID,
  TOKEN_C_BREAK_ID,
  TOKEN_C_DEFAULT_ID,
  TOKEN_C_WHILE_ID,
  TOKEN_C_IF_ID,
  TOKEN_C_ELSE_ID,
  TOKEN_C_RETURN_ID,
  TOKEN_C_VOID_ID,
  TOKEN_C_INT_ID,
  TOKEN_C_FLOAT_ID,
  TOKEN_C_LONG_ID,
  TOKEN_C_CHAR_ID,
  TOKEN_C_SIGNED_ID,
  TOKEN_C_UNSIGNED_ID,
};

void cut_token_string(char * buffer, int length)
{
  buffer[length]=0;
  printf("%s",buffer);
}

void print_c_token(enum c_word_token c_token)
{
  switch(c_token)
    {
    case TOKEN_C_IF_ID:
      printf("if");
      break;
    case TOKEN_C_ELSE_ID:
      printf("else ");
      break;
    case TOKEN_C_RETURN_ID:
      printf("return ");
      break;
    case TOKEN_C_STRUCT_ID:
      printf("struct ");
      break;
    case TOKEN_C_ENUM_ID:
      printf("enum ");
      break;
    case TOKEN_C_SWITCH_ID:
      printf("switch");
      break;
    case TOKEN_C_CASE_ID:
      printf("case ");
      break;
    case TOKEN_C_DEFAULT_ID:
      printf("default");
      break;
    case TOKEN_C_BREAK_ID:
      printf("break");
      break;
    case TOKEN_C_CHAR_ID:
      printf("char ");
      break;
    case TOKEN_C_INT_ID:
      printf("int ");
      break;
    case TOKEN_C_FLOAT_ID:
      printf("float ");
      break;
    case TOKEN_C_LONG_ID:
      printf("long ");
      break;
    case TOKEN_C_VOID_ID:
      printf("void ");
      break;
    default:
      printf("<cword>%i</cword>",c_token);
    }
}
enum c_word_token get_word_token(char * buffer, int length)
{
  if ( length == 2)
    {
      if ((buffer[0]=='i')&&(buffer[1]=='f'))
	{
	  return TOKEN_C_IF_ID;
	}
    }
  else  if ( length == 3 )
    {
      if ((buffer[0]=='i')&&(buffer[1]=='n')&&(buffer[2]=='t'))
	{
	  return TOKEN_C_INT_ID;
	}
    }
  else
    if ( length == 4 )
      {
	if ( buffer[0]=='e' )
	  {
	    if (buffer[1]=='n')
	      {
		if ((buffer[2]=='u')&&(buffer[3]=='m'))
		  {
		    return TOKEN_C_ENUM_ID;
		  }
	      }
	    else
	    if ((buffer[1]=='l')&&(buffer[2]=='s')&&(buffer[3]=='e'))
	      {
		return TOKEN_C_ELSE_ID;
	      }
	  }
	else if (buffer[0]=='v')
	  {
	    if ((buffer[1]=='o')&&(buffer[2]=='i')&&(buffer[3]=='d'))
	      {
		return TOKEN_C_VOID_ID;
	      }
	  }
	else if (buffer[0]=='c')
	  {
	    if (buffer[1]=='h')
	      {
		if ((buffer[2]=='a')&&(buffer[3]=='r'))
		  {
		    return TOKEN_C_CHAR_ID;
		  }
	      }
	    else if (buffer[1]=='a')
	      {
		if ((buffer[2]=='s')&&(buffer[3]=='e'))
		  {
		    return TOKEN_C_CASE_ID;
		  }
	      }
	  }
	else if (buffer[0]=='l')
	  {
	    if ((buffer[1]=='h')&&(buffer[2]=='a')&&(buffer[3]=='r'))
	      {
		return TOKEN_C_LONG_ID;
	      }	    
	  }
      }
    else
      if ( length == 5 )
	{
	  if ((buffer[0]=='f')&&(buffer[1]=='l')&&(buffer[2]=='o')&&(buffer[3]=='a')&&(buffer[4]=='t'))
	    {
	      return TOKEN_C_FLOAT_ID;
	    }
	  if ((buffer[0]=='w')&&(buffer[1]=='h')&&(buffer[2]=='i')&&(buffer[3]=='l')&&(buffer[4]=='e'))
	    {
	      return TOKEN_C_WHILE_ID;
	    }
	  if ((buffer[0]=='b')&&(buffer[1]=='r')&&(buffer[2]=='e')&&(buffer[3]=='a')&&(buffer[4]=='k'))
	    {
	      return TOKEN_C_BREAK_ID;
	    }
	}
      else
	if ( length == 6 )
	  {
	    if ( buffer[0]=='s' )
	      {
		if ( buffer[1] == 'w')
		  {
		    if ((buffer[2]=='i')&&(buffer[3]=='t')&&(buffer[4]=='c')&&(buffer[5]=='h'))
		      {
			return TOKEN_C_SWITCH_ID;
		      }
		  }
		else if (buffer[1] =='i')
		  {
		    if ((buffer[2]=='g')&&(buffer[3]=='n')&&(buffer[4]=='e')&&(buffer[5]=='d'))
		      {
			return TOKEN_C_SIGNED_ID;
		      }
		  }
		else if (buffer[1] =='t')
		  {
		    if ((buffer[2]=='r')&&(buffer[3]=='u')&&(buffer[4]=='c')&&(buffer[5]=='t'))
		      {
			return TOKEN_C_STRUCT_ID;
		      }
		  }
	      }
	    else if (buffer[0]=='r')
	      {
		if ((buffer[1]=='e')&&(buffer[2]=='t')&&(buffer[3]=='u')&&(buffer[4]=='r')&&(buffer[5]=='n'))
		  {
		    return TOKEN_C_RETURN_ID;
		  }
	      }
	  }
	else if (length == 7)
	  {
	    if ( strncmp(buffer,"default",length) == 0 )
	      {
		return TOKEN_C_DEFAULT_ID;
	      }
	  }
	else if (length == 8)
	  {
	    if ( strncmp(buffer,"unsigned",length) == 0 )
	      {
		return TOKEN_C_UNSIGNED_ID;
	      }
	  }
  return TOKEN_C_NOMATCH_ID;
}
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

      struct al_token* token;
  
      while ( (token = c_tokenizer(&tokenizer,&importer)) != NULL )
	{
	  switch(token->token)
	    {
	    case JSON_TOKEN_OPEN_PARENTHESIS_ID:
	      printf("(");
	      break;
	    case JSON_TOKEN_CLOSE_PARENTHESIS_ID:
	      printf(")");
	      break;
	    case JSON_TOKEN_OPEN_BRACKET_ID:
	      printf("[");
	      break;
	    case JSON_TOKEN_CLOSE_BRACKET_ID:
	      printf("]");
	      break;
	    case JSON_TOKEN_OPEN_BRACE_ID:
	      puts("{");
	      break;
	    case JSON_TOKEN_CLOSE_BRACE_ID:
	      puts("}");
	      break;
	    case JSON_TOKEN_SEMI_COLON_ID:
	      puts(";");
	      break;
	    case JSON_TOKEN_COLON_ID:
	      puts(":");
	      break;
	    case JSON_TOKEN_EQUAL_ID:
	      printf("=");
	      break;
	    case JSON_TOKEN_COMMA_ID:
	      puts(",");
	      break;
	    case JSON_TOKEN_AMPERSAND_ID:
	      printf("&");
	      break;
	    case JSON_TOKEN_STAR_ID:
	      printf("*");
	      break;
	    case JSON_TOKEN_DOT_ID:
	      printf(".");
	      break;
	    case JSON_TOKEN_RIGHT_ARROW_ID:
	      printf("->");
	      break;
	    case JSON_TOKEN_SUPERIOR_ID:
	      printf(">");
	      break;
	    case JSON_TOKEN_INFERIOR_ID:
	      printf("<");
	      break;
	    case JSON_TOKEN_NUMBER_ID:
	      cut_token_string(tokenizer.buf,tokenizer.bufpos);
	      break;
	    case JSON_TOKEN_DQUOTE_ID:
	      printf("\"");
	      cut_token_string(tokenizer.buf,tokenizer.bufpos);
	      printf("\"");
	      break;
	    case JSON_TOKEN_SQUOTE_ID:
	      printf("'");
	      cut_token_string(tokenizer.buf,tokenizer.bufpos);
	      printf("'");
	      break;
	    case JSON_TOKEN_WORD_ID:
	      {
		int c_token = get_word_token(tokenizer.buf,tokenizer.bufpos);
		if ( c_token == TOKEN_C_NOMATCH_ID )
		  {
		    cut_token_string(tokenizer.buf,tokenizer.bufpos);
		  }
		else
		  {
		    print_c_token(c_token);
		  }
		break;
	      }
	    case JSON_TOKEN_COMMENT_ID:
	      // disregards comments
	      break;
	    case JSON_TOKEN_PRAGMA_ID:
	      printf("#");
	      cut_token_string(tokenizer.buf,tokenizer.bufpos);
	      printf("\n");
	      break;
	    default:
	      printf("\n<%i>",token->token);
	      // lazzy cut_string...
	      cut_token_string(tokenizer.buf,tokenizer.bufpos);
	      printf("</%i>",token->token);
	    }  
	  
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
