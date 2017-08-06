#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "c_parser.h"
#include "todo.h"
#include "json_import_internal.h"

void usage()
{
  printf("intention is to parse a c file ...");
}

void reset_tokenizer_buffer(struct json_ctx * tokenizer)
{
  // reset
  tokenizer->bufpos=0;
}

int c_grow_word_buffer(struct c_parser_ctx * parser)
{
  // should allocate a new buffer
  // should read all entries then re-add them in new dict to free the previous one.
  todo("[FATAL] grow word buffer . not implemented");
  exit(1);
}

/* return an entry pointer in global dict_index table
 */
void * c_cut_token_string(struct c_parser_ctx * parser)
{
  struct json_ctx * tokenizer = parser->tokenizer;
  char * buffer = tokenizer->buf;
  int length = tokenizer->bufpos;

  buffer[length]=0;
  printf("%s",buffer);

  struct alhash_datablock key;
  struct alhash_datablock * value;

  // todo("create an entry in dict...")
  key.length=length;
  key.data=&parser->word_buffer[parser->word_buffer_pos];
  if ( parser->word_buffer_pos + length < parser->word_buffer_length )
    {
      memcpy(key.data,buffer,length);
    }
  else
    {
      fprintf(stderr,"[WARNING] internal buffer for words full %i+%i>%i",parser->word_buffer_pos,length,parser->word_buffer_length);
      c_grow_word_buffer(parser);
    }
  
  struct alhash_entry * entry = alhash_get_entry(&parser->dict, &key);
  if ( entry == NULL )
    {
      ++parser->words;
      parser->word_buffer_pos+=length;
      value=&key;
      entry = alhash_put(&parser->dict, &key, value);
    }
  else
    {
      // printf("SAME TOKEN SEEN\n");
    }

  reset_tokenizer_buffer(tokenizer);
  return entry->value.data;
}

void print_c_token(enum c_word_token c_token)
{
  switch(c_token)
    {
    case TOKEN_C_IF_ID:
      printf("if");
      break;
    case TOKEN_C_WHILE_ID:
      printf("while");
      break;
    case TOKEN_C_ELSE_ID:
      printf("else ");
      break;
    case TOKEN_C_RETURN_ID:
      printf("return ");
      break;
    case TOKEN_C_STRUCT_ID:
      printf("struct ");
      // either a declaration or a definition;      
      break;
    case TOKEN_C_UNION_ID:
      printf("union ");
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
	  if ((buffer[0]=='u')&&(buffer[1]=='n')&&(buffer[2]=='i')&&(buffer[3]=='o')&&(buffer[4]=='n'))
	    {
	      return TOKEN_C_UNION_ID;
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

void c_parse_any(struct c_parser_ctx * parser,   struct al_token* token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
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
    case JSON_TOKEN_COMPARE_EQUAL_ID:
      printf("==");
      break;
    case JSON_TOKEN_COMPARE_DIFFERENT_ID:
      printf("!=");
      break;
    case JSON_TOKEN_COMMA_ID:
      puts(",");
      break;
    case JSON_TOKEN_AMPERSAND_ID:
      printf("&");
      break;
    case JSON_TOKEN_LOGICAL_AND_ID:
      printf("&&");
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
    case JSON_TOKEN_EXCLAMATION_ID:
      printf("!");
      break;
    case JSON_TOKEN_PIPE_ID:
      printf("|");
      break;
    case JSON_TOKEN_LOGICAL_OR_ID:
      printf("||");
      break;

    case JSON_TOKEN_NUMBER_ID:
      c_cut_token_string(parser);
      break;
    case JSON_TOKEN_DQUOTE_ID:
      printf("\"");
      c_cut_token_string(parser);
      printf("\"");
      break;
    case JSON_TOKEN_SQUOTE_ID:
      printf("'");
      c_cut_token_string(parser);
      printf("'");
      break;
    case JSON_TOKEN_WORD_ID:
      {
	c_cut_token_string(parser);
	break;
      }
    case JSON_TOKEN_COMMENT_ID:
      // disregards comments
      reset_tokenizer_buffer(tokenizer);
      break;
    case JSON_TOKEN_PRAGMA_ID:
      printf("#");
      c_cut_token_string(parser);
      printf("\n");
      break;
    case JSON_TOKEN_EOF_ID:
      printf("EOF id reached\n");
      break;
    default:
      printf("\n<%i>",token->token);
      // lazzy cut_string...
      c_cut_token_string(parser);
      printf("</%i>",token->token);
    }  

}

struct al_token * c_parse_variable(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  if ( token->token != JSON_TOKEN_WORD_ID )
    {
      return token;
    }
  int c_token = get_word_token(tokenizer->buf,tokenizer->bufpos);
  if ( c_token == TOKEN_C_NOMATCH_ID )
    {
      c_parse_any(parser,token);
      return NULL;
    }
  else
    {
      printf("c_token %i\n",c_token);
      return token;
    }
}

struct al_token * c_parse_left_type(struct c_parser_ctx * parser, struct al_token * token )
{
  struct json_ctx* tokenizer = parser->tokenizer;
  int c_type = -1;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }      
  while ( token != NULL )
    {
      switch(token->token)
	{
	case JSON_TOKEN_AMPERSAND_ID:
	case JSON_TOKEN_STAR_ID:
	case JSON_TOKEN_COMMENT_ID:
	  c_parse_any(parser,token);
	  break;
	case JSON_TOKEN_WORD_ID:
	  {
	    int c_token = get_word_token(tokenizer->buf,tokenizer->bufpos);
	    switch(c_token)
	      {
	      case TOKEN_C_STRUCT_ID:
	      case TOKEN_C_UNION_ID:
	      case TOKEN_C_ENUM_ID:
	      case TOKEN_C_CHAR_ID:
	      case TOKEN_C_INT_ID:
	      case TOKEN_C_FLOAT_ID:
	      case TOKEN_C_LONG_ID:
	      case TOKEN_C_VOID_ID:
		if ( c_type == -1 )
		  {
		    // recognized a type.
		    c_parse_any(parser,token);
		    printf(" ");
		    c_type = c_token;
		    parser->last_type=c_type;
		  }
		else
		  {
		    printf("[ERROR] conflcting types %i != %i", c_type, c_token);
		    return token;
		  }
		break;
	      case TOKEN_C_NOMATCH_ID:
		// struct and enum types are named.
		if ( ( c_type == TOKEN_C_STRUCT_ID ) || ( c_type == TOKEN_C_ENUM_ID ))
		  {
		    token=c_parse_variable(parser,token);
		    if ( token != NULL )
		      {
			printf(" non varname %i ",token->token);
			return token;
		      }
		  }
		else
		  {
		    return token;
		  }
		break;
	      default:
		return token;
	      }
	  }
	  break;
	default:
	  return token;
	}
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  return token;
}

struct al_token * c_parse_struct_member(struct c_parser_ctx * parser, struct c_struct_info * struct_info)
{
  return c_parse_statement(parser);
}

struct al_token * c_parse_rhs(struct c_parser_ctx * parser, struct al_token * token);
struct al_token * c_parse_call_parameters(struct c_parser_ctx * parser, struct al_token * token);
  
struct al_token * c_parse_lhs(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  while ( token != NULL )
    {
      token = c_parse_left_type(parser,token);
      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}
      if (c_parse_variable(parser,token) == NULL)
	{
	  if ( (token = c_tokenizer(tokenizer,parser->tokenizer_data)) != NULL )
	    {
	      if ( token == NULL )
		{
		  token = c_tokenizer(tokenizer,parser->tokenizer_data);
		}
	      if  ( token->token == JSON_TOKEN_RIGHT_ARROW_ID )
		{
		  c_parse_any(parser,token);
		}
	      else if ( token->token == JSON_TOKEN_OPEN_BRACKET_ID )
		{
		  c_parse_any(parser,token);
		  if ((token = c_tokenizer(tokenizer,parser->tokenizer_data)) != NULL )
		    {
		      token=c_parse_rhs(parser,token);
		      if ( token == NULL )
			{
			  token = c_tokenizer(tokenizer,parser->tokenizer_data);
			}
		      if ( token != NULL )
			{
			  if ( token->token == JSON_TOKEN_CLOSE_BRACKET_ID )
			    {
			      c_parse_any(parser,token);
			      return NULL;
			    }
			  else
			    {
			      return token;
			    }
			}
		    }
		}
	      else
		{
		  return token;
		}
	    }
	}
      else
	{
	  return token;
	}
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  return token;
}

struct al_token * c_parse_litteral(struct c_parser_ctx * parser,struct al_token * token)
{
  // litterals
  if  ( token->token == JSON_TOKEN_DQUOTE_ID )
    {
      c_parse_any(parser,token);
      return NULL;
    }
  else if  ( token->token == JSON_TOKEN_SQUOTE_ID )
    {
      c_parse_any(parser,token);
      return NULL;
    }
  else if (token->token == JSON_TOKEN_NUMBER_ID )
    {
      c_parse_any(parser,token);
      return NULL;
    }
  else return token;
}

struct al_token * c_parse_rhs(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  // printf("//start rhs\n");
  while ( token != NULL )
    {
      if (( token=c_parse_litteral(parser,token)) == NULL)
	{
	  break;
	}
      else {
	token=c_parse_lhs(parser,token);
	// is it a function call ?
	if ( token != NULL )
	  {
	    if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	      {
		c_parse_any(parser,token);  
		token = c_parse_call_parameters(parser,NULL);
		if ( token == NULL )
		  {
		    token = c_tokenizer(tokenizer,parser->tokenizer_data);
		  }
		if ( token != NULL )
		  {
		    if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		      {
			c_parse_any(parser,token);
			return NULL;
		      }
		  }
		return token;
	      }
	  }
	if ( token != NULL )
	  {
	    break;
	  }
      }
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  return token;
}

struct al_token * c_parse_rhs_semi_colon(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  token = c_parse_rhs(parser,token);

   if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  if ( token != NULL)
    {
      if (token->token != JSON_TOKEN_SEMI_COLON_ID)
	{
	  return token;
	}
      else
	{
	  printf(";// end rhs\n");
	  return NULL;
	}
    }
  return token;
}

// int a,void b,struct * c,d ...
struct al_token * c_parse_call_definition_parameters(struct c_parser_ctx * parser)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  struct al_token * token = NULL;
  int i=0;
  token = c_tokenizer(tokenizer,parser->tokenizer_data);
  while (token != NULL )
    {
      token = c_parse_rhs(parser,token);
      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}
      if ( token !=NULL)
	{
	  if (token->token != JSON_TOKEN_COMMA_ID )
	    {
	      printf("// def param %i end token %i\n",i,token->token);
	      return token;
	    }
	  printf("// def param %i\n",i);
	  ++i;
	  c_parse_any(parser,token);
	}
      else
	{
	  return token;
	}
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  printf("// def param %i end token %i\n",i, token->token);
  return token;
  
}

// a,b,c,d ...
struct al_token * c_parse_call_parameters(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  
  int i=0;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  while (token  != NULL )
    {
      token = c_parse_rhs(parser,token);
      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}
      if ( token !=NULL)
	{
	  if (token->token != JSON_TOKEN_COMMA_ID )
	    {
	      printf("//param wrong token %i\n",token->token);
	      return token;
	    }
	  printf("// call param %i\n",i);
	  ++i;
	  c_parse_any(parser,token);
	}
      else
	{
	  return token;
	}
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  printf("// call param %i end\n",i);
  return token;
  
}

struct al_token * c_parse_simple_boolean_expression(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
    {
      c_parse_any(parser,token);  
      token = c_parse_simple_boolean_expression(parser,NULL);
      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}
      if ( token != NULL )
	{
	  if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
	    {
	      c_parse_any(parser,token);
	      return NULL;
	    }
	}
    }
   else 
    {
      token = c_parse_rhs(parser,token);
      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}
      if ( ( token->token == JSON_TOKEN_COMPARE_EQUAL_ID )
	   || ( token->token == JSON_TOKEN_COMPARE_DIFFERENT_ID ) )
	{
	  c_parse_any(parser,token);  
	  token = c_parse_simple_boolean_expression(parser,NULL);
	}
      return token;
    }
  return token;
  
}

// should be logical expression
struct al_token * c_parse_logical_expression(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  while ( token != NULL )
    {
      token = c_parse_simple_boolean_expression(parser,token);
      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}
      if ( token != NULL )
	{
	  if ( token->token == JSON_TOKEN_EQUAL_ID )
	    {
	      // mixing ... ex ((buffer=1)==2)
	      token=c_parse_simple_boolean_expression(parser,NULL);
	    }
	  if ( ( token->token == JSON_TOKEN_LOGICAL_AND_ID )
	       || ( token->token == JSON_TOKEN_LOGICAL_OR_ID ) )
	    {
	      c_parse_any(parser,token);
	      token = c_tokenizer(tokenizer,parser->tokenizer_data);
	    }
	  else
	    {
	      // printf("invalid logical combination token %i", token->token);
	      return token;
	    }
	}
    }
  return token;
}

struct al_token * c_parse_enum_member(struct c_parser_ctx * parser, struct c_enum_info * enum_info, int index)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  struct al_token * token = NULL;
  token = c_tokenizer(tokenizer,parser->tokenizer_data);
  if ( token == NULL )
    {
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }
  if (token->token == JSON_TOKEN_WORD_ID)
    {
      if (index > 0 )
	{
	  printf(",\n");
	}
      c_parse_any(parser,token);
    }
  else
    {
      return token;
    }
  token = c_tokenizer(tokenizer,parser->tokenizer_data);
  if ( token == NULL )
    {
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }
  if (token->token == JSON_TOKEN_COMMA_ID)
    {
      // c_parse_any(parser,token);
      return NULL;
    }
  else
    {
      return token;
    }
}

struct al_token * c_parse_block(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  if ( token != NULL )
    {
      if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
	{
	  printf("{\n");
	  while ( (token=c_parse_statement(parser)) == NULL );
	  if ( token != NULL )
	    {
	      if (token->token == JSON_TOKEN_CLOSE_BRACE_ID )
		{
		  printf("}\n");
		  return NULL;
		}
	      else {
		printf("[ERROR] expected } to close block  ... {}. got token %i \n",token->token);
		return token;
	      }
	    }
	}
      // FIXME this is an error we expect a } close
      return NULL;
    }
  return NULL;
}


// parse after '(' of int a,char,b){...}
struct al_token * c_parse_function_params(struct c_parser_ctx * parser,   struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  printf("//parse func params\n");
  if ( token != NULL )
    {
      printf("(\n");
    }
  // function call or declaration
  if ( (token = c_parse_call_definition_parameters(parser) ) == NULL)
    {
      token=c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  if (token != NULL )
    {
     if  ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
       {
	 // function call
	 printf(")");
	 if ( (token = c_tokenizer(tokenizer,parser->tokenizer_data)) != NULL)
	   {
	     printf("// %i\n", token->token);
	     if ( token->token == JSON_TOKEN_SEMI_COLON_ID )
	       {
		 // function declaration 
		 printf(";\n");
		 return NULL;
	       }
	     else {
	       // function definition
	       token = c_parse_block(parser,token);
	     }
	     return token;
	   }
	 return NULL;
       }
     else {
       // declaration
       while ( (token = c_parse_statement(parser) ) == NULL);
       if  ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
	 {
	   printf(")");
	   return c_parse_block(parser,NULL);
	 }
       else
	 {
	   printf("[ERROR] a function parameters fails got %i token\n",token->token);
	 }
     }
  }
  
}
// return NULL if parsing is ok and all token at eat
// else returns token that causes non prsing
struct al_token * c_parse_statement(struct c_parser_ctx * parser)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  struct al_token * token = NULL;
  token = c_tokenizer(tokenizer,parser->tokenizer_data);
  if ( token == NULL )
    {
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }

    switch(token->token)
    {
    case JSON_TOKEN_WORD_ID:
      break;

    case JSON_TOKEN_OPEN_PARENTHESIS_ID:
    case JSON_TOKEN_CLOSE_PARENTHESIS_ID:
    case JSON_TOKEN_OPEN_BRACKET_ID:
    case JSON_TOKEN_CLOSE_BRACKET_ID:
    case JSON_TOKEN_OPEN_BRACE_ID:
    case JSON_TOKEN_CLOSE_BRACE_ID:
    case JSON_TOKEN_SEMI_COLON_ID:
    case JSON_TOKEN_COLON_ID:
    case JSON_TOKEN_EQUAL_ID:
    case JSON_TOKEN_COMPARE_EQUAL_ID:
    case JSON_TOKEN_COMMA_ID:
    case JSON_TOKEN_AMPERSAND_ID:
    case JSON_TOKEN_STAR_ID:
    case JSON_TOKEN_DOT_ID:
    case JSON_TOKEN_RIGHT_ARROW_ID:
    case JSON_TOKEN_SUPERIOR_ID:
    case JSON_TOKEN_INFERIOR_ID:
    case JSON_TOKEN_EXCLAMATION_ID:
    case JSON_TOKEN_NUMBER_ID:
    case JSON_TOKEN_DQUOTE_ID:
    case JSON_TOKEN_SQUOTE_ID:
      //printf("// [WARNING] unexpected token %i at start of a statement\n",token->token);
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_parse_any(parser,token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_parse_any(parser,token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf("EOF id reached");
      return token;
      break;
    default:
      printf("\n<%i>",token->token);
      // lazzy cut_string...
      c_cut_token_string(parser);
      printf("</%i>",token->token);
    }  

  if ( token->token != JSON_TOKEN_WORD_ID )
    {
      printf("[ERROR] a new statement should start with a word. got token %i \n",token->token);
      return token;
    }
  int c_token = get_word_token(tokenizer->buf,tokenizer->bufpos);
  if ( c_token == TOKEN_C_NOMATCH_ID )
    {
      printf("// a variable or function name\n");
      token = c_parse_lhs(parser,token);

      if ( token == NULL )
	{
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	}

      if ( token != NULL )
	{
	  if ( token->token == JSON_TOKEN_EQUAL_ID )
	    {
	      c_parse_any(parser,token);
	      if ( (token = c_tokenizer(tokenizer,parser->tokenizer_data) ) != NULL)
		{
		   token = c_parse_rhs_semi_colon(parser,token);
		}
	    }
	  else
	  if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	    {	      
	      return c_parse_function_params(parser,token);
	    }
	}
      if (token != NULL )
	{
	  printf("[ERROR] a new statement can not start with unrecognized word %i\n", token->token);
	}
      return token;
    }
  else
    {
      reset_tokenizer_buffer(tokenizer);
      // struct definition or declaration
      if ( c_token == TOKEN_C_STRUCT_ID )
	{
	  print_c_token(c_token);
	  parser->state=C_STATE_STRUCT_ID;
	  struct al_token * token;
	  struct c_struct_info struct_info;
	  while ( (token = c_tokenizer(tokenizer,parser->tokenizer_data)) != NULL )
	    {
	      if ( parser->state==C_STATE_STRUCT_ID )
		{
		  if ( token->token == JSON_TOKEN_WORD_ID )
		    {
		      struct_info.dict_index = c_cut_token_string(parser);
		      parser->state=C_STATE_STRUCT_NAME_ID;
		      printf(" ");
		    }
		  else
		    {
		      printf("[ERROR] struct name expected got token %i \n",token->token);
		    }
		}
	      else if ( parser->state==C_STATE_STRUCT_NAME_ID)
		{
		  if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
		    {
		      printf("{\n");
		      parser->state=C_STATE_STRUCT_DEFINITION_ID;
		      while ( (token=c_parse_struct_member(parser,&struct_info)) == NULL );
		      if ( token != NULL )
			{
			  if (token->token == JSON_TOKEN_CLOSE_BRACE_ID )
			    {
			      token = c_tokenizer(tokenizer,parser->tokenizer_data);
			      if ( (token != NULL) && (token->token != JSON_TOKEN_SEMI_COLON_ID ) )
				{
				  printf("[ERROR] expected ; to close struct  ... {} name expected got token %i \n",token->token);
				};
			      printf("};\n");
			    }
			}
		      break;
		    }
		  else if ( token->token == JSON_TOKEN_STAR_ID )
		    {
		      c_parse_any(parser,token);
		      parser->state=C_STATE_DECLARE_TYPE_ID;
		    }
		  else if ( token->token == JSON_TOKEN_WORD_ID )
		    {
		      void * dict_index = c_cut_token_string(parser);
		      parser->state=C_STATE_STRUCT_DECLARATION_ID;
		    }
		  else
		    {
		      printf("[ERROR] struct definition or declaration expected, got token %i\n", token->token);
		    }
		}
	      else if ( parser->state==C_STATE_DECLARE_TYPE_ID)
		{
		  if ( token->token == JSON_TOKEN_STAR_ID )
		    {
		      c_parse_any(parser,token);
		      parser->state=C_STATE_DECLARE_TYPE_ID;
		    }
		  else if ( token->token == JSON_TOKEN_WORD_ID )
		    {
		      void * dict_index = c_cut_token_string(parser);
		      parser->state=C_STATE_STRUCT_DECLARATION_ID;
		    }
		  else
		    {
		      printf("[ERROR] struct declaration unexpected token %i expected\n",token->token);
		    }       
		}
	      else {
		if ( token->token == JSON_TOKEN_SEMI_COLON_ID )
		  {
		    printf(";\n");
		    break;
		  }
		if ( token->token == JSON_TOKEN_EQUAL_ID )
		  {
		    token=c_parse_rhs(parser,NULL);
		  }
		if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
		  {
		    return c_parse_function_params(parser,token);
		  }
		if ( token->token == JSON_TOKEN_COMMA_ID )
		  {
		    printf(",\n");
		    break;
		  }
		else
		  {
		    printf("[ERROR] unrecognized struct usage token %i\n", token->token);
		    break;
		  }
	      }
	    }
	}
      else if ( c_token == TOKEN_C_ENUM_ID )
	{
	  print_c_token(c_token);
	  parser->state=C_STATE_ENUM_ID;
	  struct al_token * token;
	  struct c_enum_info enum_info;
	  while ( (token = c_tokenizer(tokenizer,parser->tokenizer_data)) != NULL )
	    {
	      if ( parser->state==C_STATE_ENUM_ID )
		{
		  if ( token->token == JSON_TOKEN_WORD_ID )
		    {
		      enum_info.dict_index = c_cut_token_string(parser);
		      parser->state=C_STATE_ENUM_NAME_ID;
		      printf(" ");
		    }
		  else
		    {
		      printf("[ERROR] struct name expected got token %i \n",token->token);
		    }
		}
	      else if ( parser->state==C_STATE_ENUM_NAME_ID)
		{
		  if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
		    {
		      printf("{\n");
		      parser->state=C_STATE_ENUM_DEFINITION_ID;
		      int index = 0;
		      token=c_parse_enum_member(parser,&enum_info,index);
		      while ( token == NULL )
			{
			  ++index;
			  token=c_parse_enum_member(parser,&enum_info,index);
			}
		      if ( token != NULL )
			{
			  if (token->token == JSON_TOKEN_CLOSE_BRACE_ID )
			    {
			      token = c_tokenizer(tokenizer,parser->tokenizer_data);
			      if ( (token != NULL) && (token->token != JSON_TOKEN_SEMI_COLON_ID ) )
				{
				  printf("[ERROR] expected ; to close struct  ... {} name expected got token %i \n",token->token);
				};
			      printf("};\n");
			    }
			}
		      break;
		    }
		  else if ( token->token == JSON_TOKEN_STAR_ID )
		    {
		      c_parse_any(parser,token);
		      parser->state=C_STATE_DECLARE_TYPE_ID;
		    }
		  else if ( token->token == JSON_TOKEN_WORD_ID )
		    {
		      void * dict_index = c_cut_token_string(parser);
		      parser->state=C_STATE_ENUM_DECLARATION_ID;
		    }
		  else
		    {
		      printf("[ERROR] enum definition or declaration expected, got token %i\n", token->token);
		    }
		}
	      else if ( parser->state==C_STATE_DECLARE_TYPE_ID)
		{
		  if ( token->token == JSON_TOKEN_STAR_ID )
		    {
		      parser->state=C_STATE_DECLARE_TYPE_ID;
		    }
		  else if ( token->token == JSON_TOKEN_WORD_ID )
		    {
		      void * dict_index = c_cut_token_string(parser);
		      parser->state=C_STATE_ENUM_DECLARATION_ID;
		    }
		  else
		    {
		      printf("[ERROR] struct declaration unexpected token %i expected\n",token->token);
		    }       
		}
	      else {
		if ( token->token == JSON_TOKEN_SEMI_COLON_ID )
		  {
		    printf(";\n");
		    break;
		  }
		if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
		  {
		    return c_parse_function_params(parser,token);
		  }
		else
		  {
		    printf("[ERROR] unrecognized enum usage token %i\n", token->token);
		    break;
		  }
	      }
	    }
	}
      else if ( c_token == TOKEN_C_IF_ID )
	{
	  printf("IF START\n");
	  print_c_token(c_token);
	  printf(" ");
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
		{
		  c_parse_any(parser,token);		  
		  token = c_parse_logical_expression(parser,NULL);
		  if ( token == NULL )
		    {
		      token = c_tokenizer(tokenizer,parser->tokenizer_data);
		    }
		  if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		    {
		      c_parse_any(parser,token);
		      token = c_parse_block(parser,NULL);
		    }
		}
	    }
	  printf("IF END\n");
	  return token;
	}
      else if ( c_token == TOKEN_C_WHILE_ID )
	{
	  print_c_token(c_token);
	  printf(" ");
	  token = c_tokenizer(tokenizer,parser->tokenizer_data);
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
		{
		  c_parse_any(parser,token);		  
		  token = c_parse_logical_expression(parser,NULL);
		  if ( token == NULL )
		    {
		      token = c_tokenizer(tokenizer,parser->tokenizer_data);
		    }
		  if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		    {
		      c_parse_any(parser,token);
		      token = c_parse_block(parser,NULL);
		    }
		}
	    }
	  return token;
	}
      else	
	{
	  // non struct,enum,if or while
	  token = c_parse_left_type(parser,token);
	  if ( token == NULL )
	    {
	      token = c_tokenizer(tokenizer,parser->tokenizer_data);
	    }
	  printf("\n// wild parsing start\n");
	  print_c_token(c_token);
	  while ( token != NULL )
	    {
	      if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
		{
		  c_parse_any(parser,token);
		  printf("\n// wild parsing statements start\n");
		  token = c_parse_statement(parser);
		  while (token == NULL)
		    {
		      token = c_parse_statement(parser);
		    }
		  printf("\n// wild parsing statements end\n");
		  if (token->token == JSON_TOKEN_CLOSE_BRACE_ID)
		    {
		      printf("}\n");
		      break;
		    }
		  else
		    {
		      printf("unmatched { got token %i\n",token->token);
		    }
		}
	      if (token->token == JSON_TOKEN_EQUAL_ID)
		{
		  c_parse_any(parser,token);
		  if ( (token = c_tokenizer(tokenizer,parser->tokenizer_data)) != NULL )
		    {
		      token=c_parse_rhs_semi_colon(parser,token);
		    }
		  break;
		}
	      if (token->token == JSON_TOKEN_SEMI_COLON_ID)
		{
		  c_parse_any(parser,token);
		  break;
		}
	      if (token->token == JSON_TOKEN_EOF_ID)
		{
		  printf("[ERROR] EOF hit prematurly in statement\n");
		  break;
		}
	      c_parse_any(parser,token);
	      token = c_tokenizer(tokenizer,parser->tokenizer_data);
	    }
	  printf("\n// wild parsing end\n");
	}
    }
  return NULL;
}


int init_c_parser(  struct c_parser_ctx* parser, struct json_ctx* tokenizer, void * data)
{
  parser->state=C_STATE_START_ID;
  parser->tokenizer=tokenizer;
  parser->tokenizer_data=data;
  parser->last_type=-1;
  parser->last_word=-1;
  // length in number of entries [ at least ALHASH_BUCKET_SIZE will be used ]
  // long (*alhash_func) (void * value, int length));
  alhash_init(&parser->dict, 0, NULL);
  
  parser->word_buffer_length=4096;
  parser->word_buffer_pos=0;
  parser->word_buffer=malloc(parser->word_buffer_length);
  return 1;
}

int main(int argc,char ** argv)
{
  struct c_parser_ctx parser;
  struct json_import_context_data importer;
  struct json_ctx tokenizer;
  struct inputstream inputstream;
  FILE * file = NULL;

  bzero(&tokenizer,sizeof(tokenizer));
  bzero(&importer,sizeof(importer));
      
  init_c_parser(&parser,&tokenizer,&importer);
  
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

      struct al_token * token = NULL;
      while ( ( token = c_parse_statement(&parser)) == NULL );

      if (token->token != JSON_TOKEN_EOF_ID )
	{
	  printf("expected eof, got token %i.\n", token->token);
	}
      
      fflush(stdout);
      todo("implement parser ... !");
    }
  else
    {
      usage();
    }
}

