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


char * json_token_to_char(enum json_token_id token_id)
{
  // ==> need a generator :-)
}

void reset_tokenizer_buffer(struct json_ctx * tokenizer)
{
  // reset when word is parsed and recognized as either a reserved word or stored in variable dict with cut_string.
  // printf("//reset token buffer\n");
  tokenizer->token_buf.bufpos=0;
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
  struct token_char_buffer * tb = &tokenizer->token_buf;
  char * buffer = tb->buf;
  int length = tb->bufpos;

  if (buffer == NULL )
    {
      todo("[FATAL] corrupted parser token char buffer NULL");
      return NULL;
    }

  if (length == 0 )
    {
      printf("[FATAL] corrupted parser empty char buffer\n");
      return NULL;
    }
  if(length>=0)
    {
      // thanks to this format ... print non NULL terminated string
      printf("%.*s",length,buffer);
    }
  else
    {
      fprintf(stderr,"[FATAL] corrupted parser token char buffer length %i <=0\n",length);
      return NULL;
    }

  struct alhash_datablock key;
  struct alhash_datablock * value;

  // todo("create an entry in dict...")
  key.length=length;
  key.data=&parser->word_buffer[parser->word_buffer_pos];
  if ( (parser->word_buffer_pos + length) < parser->word_buffer_length )
    {
      memcpy(key.data,buffer,length);
    }
  else
    {
      fprintf(stderr,"[WARNING] internal char buffer for words full %i+%i>%i",parser->word_buffer_pos,length,parser->word_buffer_length);
      c_grow_word_buffer(parser);
    }
  
  struct alhash_entry * entry = alhash_get_entry(&parser->dict, &key);
  if ( entry == NULL )
    {
      ++parser->words;
      parser->word_buffer_pos+=length;
      value=&key;
      entry = alhash_put(&parser->dict, &key, value);
      if ( entry == NULL )
	{
	  fprintf(stderr,"[FATAL] FAIL to insert '%s' into word buffer %p \n", buffer, &parser->dict);
	  exit(1);
	}
    }
  else
    {
      // printf("SAME TOKEN SEEN\n");
    }

  reset_tokenizer_buffer(tokenizer);
  
  parser->last_word=TOKEN_C_DICTENTRY_ID;
  parser->dict_value=&entry->value;
  
  return entry->value.data;
}

void print_c_token(struct c_parser_ctx * parser, enum c_word_token c_token)
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
    case TOKEN_C_TYPEDEF_ID:
      printf("typedef ");
      break;
    default:
      printf("<cword>%i</cword>",c_token);
    }
}

  
enum c_word_token get_word_token(struct token_char_buffer * tb)
{
  char * buffer = tb->buf;
  int length = tb->bufpos;
  enum c_word_token word_token = TOKEN_C_NOMATCH_ID;
  
  if ( ( length == 0 )  || (buffer == NULL))
    {
      printf("[ERROR] empty or null buffer for get_word_token %p %i", buffer, length);
      return TOKEN_C_NOMATCH_ID;
    }

  if ( length == 2)
    {
      if ((buffer[0]=='i')&&(buffer[1]=='f'))
	{
	  word_token = TOKEN_C_IF_ID;
	}
    }
  else  if ( length == 3 )
    {
      if ((buffer[0]=='i')&&(buffer[1]=='n')&&(buffer[2]=='t'))
	{
	  word_token = TOKEN_C_INT_ID;
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
		    word_token = TOKEN_C_ENUM_ID;
		  }
	      }
	    else
	    if ((buffer[1]=='l')&&(buffer[2]=='s')&&(buffer[3]=='e'))
	      {
		word_token = TOKEN_C_ELSE_ID;
	      }
	  }
	else if (buffer[0]=='v')
	  {
	    if ((buffer[1]=='o')&&(buffer[2]=='i')&&(buffer[3]=='d'))
	      {
		word_token = TOKEN_C_VOID_ID;
	      }
	  }
	else if (buffer[0]=='c')
	  {
	    if (buffer[1]=='h')
	      {
		if ((buffer[2]=='a')&&(buffer[3]=='r'))
		  {
		    word_token = TOKEN_C_CHAR_ID;
		  }
	      }
	    else if (buffer[1]=='a')
	      {
		if ((buffer[2]=='s')&&(buffer[3]=='e'))
		  {
		    word_token = TOKEN_C_CASE_ID;
		  }
	      }
	  }
	else if (buffer[0]=='l')
	  {
	    if ((buffer[1]=='h')&&(buffer[2]=='a')&&(buffer[3]=='r'))
	      {
		word_token = TOKEN_C_LONG_ID;
	      }	    
	  }
      }
    else
      if ( length == 5 )
	{
	  if ((buffer[0]=='f')&&(buffer[1]=='l')&&(buffer[2]=='o')&&(buffer[3]=='a')&&(buffer[4]=='t'))
	    {
	      word_token = TOKEN_C_FLOAT_ID;
	    }
	  if ((buffer[0]=='w')&&(buffer[1]=='h')&&(buffer[2]=='i')&&(buffer[3]=='l')&&(buffer[4]=='e'))
	    {
	      word_token = TOKEN_C_WHILE_ID;
	    }
	  if ((buffer[0]=='b')&&(buffer[1]=='r')&&(buffer[2]=='e')&&(buffer[3]=='a')&&(buffer[4]=='k'))
	    {
	      word_token = TOKEN_C_BREAK_ID;
	    }
	  if ((buffer[0]=='u')&&(buffer[1]=='n')&&(buffer[2]=='i')&&(buffer[3]=='o')&&(buffer[4]=='n'))
	    {
	      word_token = TOKEN_C_UNION_ID;
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
			word_token = TOKEN_C_SWITCH_ID;
		      }
		  }
		else if (buffer[1] =='i')
		  {
		    if ((buffer[2]=='g')&&(buffer[3]=='n')&&(buffer[4]=='e')&&(buffer[5]=='d'))
		      {
			word_token = TOKEN_C_SIGNED_ID;
		      }
		  }
		else if (buffer[1] =='t')
		  {
		    if ((buffer[2]=='r')&&(buffer[3]=='u')&&(buffer[4]=='c')&&(buffer[5]=='t'))
		      {
			word_token = TOKEN_C_STRUCT_ID;
		      }
		  }
	      }
	    else if (buffer[0]=='r')
	      {
		if ((buffer[1]=='e')&&(buffer[2]=='t')&&(buffer[3]=='u')&&(buffer[4]=='r')&&(buffer[5]=='n'))
		  {
		    word_token = TOKEN_C_RETURN_ID;
		  }
	      }
	  }
	else if (length == 7)
	  {
	    if ( buffer[0] == 'd' )
	      {
		if ( strncmp(buffer,"default",length) == 0 )
		  {
		    word_token = TOKEN_C_DEFAULT_ID;
		  }
	      }
	    else if ( buffer[0] == 't' )
	      {
		if ( strncmp(buffer,"typedef",length) == 0 )
		  {
		    word_token = TOKEN_C_TYPEDEF_ID;
		  }
	      }
	  }
	else if (length == 8)
	  {
	    if ( strncmp(buffer,"unsigned",length) == 0 )
	      {
		word_token = TOKEN_C_UNSIGNED_ID;
	      }
	    else
	    if ( strncmp(buffer,"continue",length) == 0 )
	      {
		word_token = TOKEN_C_CONTINUE_ID;
	      }
	  }
  return word_token;
}


enum c_word_token c_parse_word_token(struct c_parser_ctx * parser)
{
  enum c_word_token word_token = get_word_token(&parser->tokenizer->token_buf);
  parser->last_word=word_token;
  if ( word_token == TOKEN_C_NOMATCH_ID )
    {
      // will require a c_cut_string
    }
  else
    {
      //      printf("// c_parse_word_token %i", word_token);
      reset_tokenizer_buffer(parser->tokenizer);
    }
  return word_token;
}

void c_print_json_token(struct c_parser_ctx * parser,   struct al_token* token)
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
	if  ( c_cut_token_string(parser) == NULL )
	  {
	    printf("// NON recognized word ...\n");
	  }
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
    case JSON_TOKEN_PLUS_ID:
      printf(" + ");
      break;
    case JSON_TOKEN_INCREMENT_ID:
      printf("++");
      break;
    case JSON_TOKEN_ADD_ID:
      printf("+=");
      break;
    case JSON_TOKEN_SUBTRACT_ID:
      printf("-=");
      break;
    case JSON_TOKEN_DECREMENT_ID:
      printf("--");
      break;
    default:
      printf("\n<%i>",token->token);
      // lazzy cut_string...
      c_cut_token_string(parser);
      printf("</%i>",token->token);
    }
}


struct al_token * c_parse_next(struct c_parser_ctx * parser)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  struct al_token * token;

  token = c_tokenizer(tokenizer,parser->tokenizer_data);

  // ignore any comment
  while ( ( token != NULL ) && ( token->token == JSON_TOKEN_COMMENT_ID ))
    {
      printf("// ignore comment\n");
      reset_tokenizer_buffer(tokenizer);
      token = c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  // allow progress checking 
  ++ parser->token_count;
  
  if ( token != NULL )
    {
      // printf("//next %i %i , %i\n", parser->last_word, parser->last_token, token->token);
      parser->last_token = token->token;
      parser->last_word=TOKEN_C_NOTWORD_ID;
      if ( token->token == JSON_TOKEN_WORD_ID )
	{
	  // will set last_word
	  c_parse_word_token(parser);
	}
      if ( token->token == JSON_TOKEN_EOF_ID)
	{
	  printf("// EOF reached at parse next\n");
	}
    }
  else
    {
      printf("// NULL token creates EOF reached at parse next\n");
      parser->last_token = JSON_TOKEN_EOF_ID;
      parser->last_word=TOKEN_C_NOTWORD_ID;
    }
  return token;
}

// return NULL only if variable was recognized.
struct al_token * c_parse_variable(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_parse_next(parser);
    }
  if ( token->token != JSON_TOKEN_WORD_ID )
    {
      return token;
    }
  if ( parser->last_word == TOKEN_C_NOMATCH_ID )
    {
      c_cut_token_string(parser);
      printf(" // variable\n");
      return NULL;
    }
  else
    {
      printf("reserved word c_token %i at %i\n", parser->last_word,parser->token_count);
      reset_tokenizer_buffer(tokenizer);
      return token;
    }
}

struct al_token * c_parse_call_definition_parameters(struct c_parser_ctx * parser);


int c_is_typedef(struct c_parser_ctx * parser)
{

  struct json_ctx * tokenizer = parser->tokenizer;
  struct token_char_buffer * tb = &tokenizer->token_buf;
  char * buffer = tb->buf;
  int length = tb->bufpos;

  if (buffer == NULL )
    {
      todo("[FATAL] corrupted parser token char buffer NULL");
      return 0;
    }

  if (length == 0 )
    {
      printf("[FATAL] corrupted parser empty char buffer\n");
      return 0;
    }

  struct alhash_datablock key;

  key.length=length;
  key.data=buffer;
  
  struct alhash_entry * entry = alhash_get_entry(&parser->dict, &key);
  if ( entry != NULL )
    {
      printf("SAME TOKEN SEEN. for test assume typedef\n");
      //return 1;
    }

  return 0;
}

/*
*/

struct al_token * c_parse_left_type(struct c_parser_ctx * parser, struct al_token * token, int c_token)
{
  struct json_ctx* tokenizer = parser->tokenizer;  
  int c_type = -1;
  int named = 0;

  printf("// parse left type at %i\n", parser->token_count);
  // set when declaration is done during type ( functions names / typedef ... ).
  parser->lhs_variable_data=NULL;
    
  if ( token == NULL )
    {
      token = c_parse_next(parser);
      c_token= parser->last_word;
    }
  while ( token != NULL )
    {
      // printf("!!%i::",token->token);
      switch(token->token)
	{
	case JSON_TOKEN_AMPERSAND_ID:	  
	case JSON_TOKEN_COMMENT_ID:
	  c_print_json_token(parser,token);
	  break;
	case JSON_TOKEN_STAR_ID:
	  c_print_json_token(parser,token);
	  // return NULL; NOPE could still be a function return value
	  break;
	case JSON_TOKEN_OPEN_PARENTHESIS_ID:
	  c_print_json_token(parser,token);
	  printf("// expects a function definition '(*' function_name ')'\n");
	  token = c_parse_next(parser);
	  c_token= parser->last_word;
	  if ( token != NULL )
	    {
	      if (token->token == JSON_TOKEN_STAR_ID)
		{
		  c_print_json_token(parser,token);
		  token = c_parse_next(parser);
		  c_token= parser->last_word;
		  if ( token != NULL )
		    {
		      if ( token->token == JSON_TOKEN_WORD_ID)
			{
			  if ( c_token == TOKEN_C_NOMATCH_ID )
			    {
			      c_print_json_token(parser,token);
			    }
			  else
			    {			      
			      printf("// [ERROR] reserved word while expecting function name c_type %i\n", c_token);
			      print_c_token(parser,c_token);
			    }
			}
		      token = c_parse_next(parser);
		      c_token= parser->last_word;
		      if ( token != NULL )
			{
			  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
			    {
			      c_print_json_token(parser,token);
			      token = c_parse_next(parser);
			      c_token= parser->last_word;
			      if ( token != NULL )
				{
				  if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
				    {
				      c_print_json_token(parser,token);
				      printf("// function parameters definition \n");
				      token=c_parse_call_definition_parameters(parser);
				      if ( token == NULL )
					{
					  token = c_parse_next(parser);
					  c_token= parser->last_word;
					}
				      if ( token != NULL )
					{					  
					  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
					    {
					      c_print_json_token(parser,token);
					      parser->lhs_variable_data=token;
					      return NULL;
					    }					  
					}					 
				    }
				}
			    }
			  else
			    {
			      printf("// expected ) to close function name/n");
			    }
			}
		    }		  
		}
	    }
	  else
	    {
	      return token;
	    }
	  break;
	case JSON_TOKEN_WORD_ID:
	  {
	    switch(c_token)
	      {
	      case TOKEN_C_STRUCT_ID:
	      case TOKEN_C_UNION_ID:
	      case TOKEN_C_ENUM_ID:
	      case TOKEN_C_INT_ID:
	      case TOKEN_C_LONG_ID:
	      case TOKEN_C_CHAR_ID:
	      case TOKEN_C_FLOAT_ID:
	      case TOKEN_C_VOID_ID:
		if ( c_type == -1 )
		  {
		    // recognized a type.
		    print_c_token(parser,c_token);
		    printf(" ");
		    c_type = c_token;
		    parser->last_type=c_type;
		  }
		else
		  {
		    printf("[ERROR] conflicting types (1) %i != %i\n", c_type, c_token);
		    return token;
		  }
		break;		
	      case TOKEN_C_NOMATCH_ID:		
		if ( ( c_type == TOKEN_C_STRUCT_ID ) || ( c_type == TOKEN_C_ENUM_ID ))
		  {
		    if ( named == 0 )
		      {
			token=c_parse_variable(parser,token);
			printf("// struct and enum types are named.\n");
			if ( token != NULL )
			  {
			    printf("// non varname %i \n",token->token);
			    return token;
			  }
			++named;
		      }
		    else
		      {
			return token;
		      }
		  }
		else
		  {
		    printf("// typedef ???\n");
		    if ( ! c_is_typedef(parser) )
		      {
			return token;
		      }		    
		    token=c_parse_variable(parser,token);
		    return token;
		  }
		break;
	      default:
		return token;
	      }
	  }
	  break;
	default:
	  printf("// unrecognized token for left type %i\n", token->token);
	  return token;
	}
      {
	// printf("!next!");
	token = c_parse_next(parser);
	c_token= parser->last_word;
      }
    }
  // printf("!!");
  return token;
}

struct al_token * c_parse_define_type(struct c_parser_ctx * parser, struct al_token * token, int within_typedef);

struct al_token * c_parse_struct_member(struct c_struct_info * struct_info, struct c_parser_ctx * parser,   struct al_token * token)
{
  if ( token == NULL )
    {
      token = c_parse_next(parser);
    }
  token = c_parse_left_type(parser,token,parser->last_word);
  if ( token == NULL )
    {
      token = c_parse_next(parser);
    }
  if (c_parse_variable(parser,token) == NULL)
    {
      token = c_parse_next(parser);
      if (( token != NULL ) && ( token->token == JSON_TOKEN_SEMI_COLON_ID ))
	{
	  c_print_json_token(parser,token);	  
	  return NULL;
	}
    }
  else
    {
      printf("// variable declaration not found\n");
    }
  return token;
}


struct al_token * eat_json_token(enum json_token_id tokenid, struct c_parser_ctx * parser, struct al_token * token)
{
  if (token == NULL)
    {
      token = c_parse_next(parser);
    }
  if ( token != NULL )
    {
      if ( token->token == tokenid )
	{
	  c_print_json_token(parser,token);
	  return NULL;
	}
    }
  return token;
}

struct al_token * eat_semi_colon(struct c_parser_ctx * parser, struct al_token * token)
{
  return eat_json_token(JSON_TOKEN_SEMI_COLON_ID,parser,token);
}


struct al_token * c_parse_rhs(struct c_parser_ctx * parser, struct al_token * token);
struct al_token * c_parse_call_parameters(struct c_parser_ctx * parser, struct al_token * token);
struct al_token * c_parse_block(struct c_parser_ctx * parser, struct al_token * token, enum c_parser_state state);

struct al_token * c_parse_lhs(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token = c_parse_next(parser);
    }
  while ( token != NULL )
    {
      token = c_parse_left_type(parser,token,parser->last_word);
      if ( token == NULL )
	{
	  token = c_parse_next(parser);
	}
      if (c_parse_variable(parser,token) == NULL)
	{
	  token = c_parse_next(parser);
	  
	  if ( token != NULL )
	    {
	      if ( token == NULL )
		{
		  token = c_parse_next(parser);
		}
	      if  ( token->token == JSON_TOKEN_RIGHT_ARROW_ID )
		{
		  c_print_json_token(parser,token);
		}
	      else if ( token->token == JSON_TOKEN_DOT_ID )
		{
		  c_print_json_token(parser,token);
		}
	      else if ( token->token == JSON_TOKEN_OPEN_BRACKET_ID )
		{
		  c_print_json_token(parser,token);
		  
		  token = c_parse_next(parser);
		  
		  if ( token != NULL )
		    {
		      token=c_parse_rhs(parser,token);
		      if ( token == NULL )
			{
			  token = c_parse_next(parser);
			}
		      if ( token != NULL )
			{
			  if ( token->token == JSON_TOKEN_CLOSE_BRACKET_ID )
			    {
			      c_print_json_token(parser,token);
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
      token = c_parse_next(parser);
    }
  return token;
}


struct al_token * c_parse_litteral(struct c_parser_ctx * parser,struct al_token * token)
{
  // litterals
  if  ( token->token == JSON_TOKEN_DQUOTE_ID )
    {
      c_print_json_token(parser,token);
      return NULL;
    }
  else if  ( token->token == JSON_TOKEN_SQUOTE_ID )
    {
      c_print_json_token(parser,token);
      return NULL;
    }
  else if (token->token == JSON_TOKEN_NUMBER_ID )
    {
      c_print_json_token(parser,token);
      return NULL;
    }
  else return token;
}

struct al_token * c_parse_rhs(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if (token == NULL)
    {     
      token = c_parse_next(parser);
    }
  // printf("//start rhs\n");
  while ( token != NULL )
    {
      if (( token=c_parse_litteral(parser,token)) == NULL)
	{
	  break;
	}
      else {
	// rhs is a '(' ...')'
	if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	  {
	    c_print_json_token(parser,token);  
	    token = c_parse_rhs(parser,NULL);
	    if ( token == NULL )
	      {
		token = c_parse_next(parser);
	      }
	    if ( token != NULL )
	      {
		if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		  {
		    c_print_json_token(parser,token);
		    return NULL;
		  }
	      }
	    return token;		
	  }
	token=c_parse_lhs(parser,token);
	// is it a function call ?
	if ( token != NULL )
	  {
	    if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	      {
		c_print_json_token(parser,token);  
		token = c_parse_call_parameters(parser,NULL);
		if ( token == NULL )
		  {
		    token = c_parse_next(parser);
		  }
		if ( token != NULL )
		  {
		    if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		      {
			c_print_json_token(parser,token);
			return NULL;
		      }
		  }
		return token;
	      }
	    else if (token->token == JSON_TOKEN_PLUS_ID )
	      {
		c_print_json_token(parser,token);
		token=c_parse_lhs(parser,NULL);
		return token;
	      }	      
	  }	  	    
	if ( token != NULL )
	  {
	    break;
	  }
      }
      token = c_parse_next(parser);
    }
  return token;
}

struct al_token * c_parse_rhs_semi_colon(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  token = c_parse_rhs(parser,token);

   if ( token == NULL )
    {
      token = c_parse_next(parser);
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
  token = c_parse_next(parser);  
  while (token != NULL )
    {
      token = c_parse_left_type(parser,token,parser->last_word);
      if ( token == NULL )
	{
	  token = c_parse_next(parser);
	}
      // name of type.
      if ( token != NULL )
	{
	  if (c_parse_variable(parser,token) == NULL)
	    {
	      token = c_parse_next(parser);
	    }
	}
      if ( token !=NULL)
	{
	  if (token->token != JSON_TOKEN_COMMA_ID )
	    {
	      printf("// def param %i end token %i A\n",i,token->token);
	      return token;
	    }
	  printf("// def param %i\n",i);
	  ++i;
	  c_print_json_token(parser,token);
	}
      else
	{
	  return token;
	}
      token = c_parse_next(parser);
    }
  printf("// def param %i end token %i B\n",i, token->token);
  return token;
  
}

// a,b,c,d ...
struct al_token * c_parse_call_parameters(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  
  int i=0;
  if ( token == NULL )
    {
      token=c_parse_next(parser);
    }
  while (token  != NULL )
    {
      token = c_parse_rhs(parser,token);
      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      if ( token !=NULL)
	{
	  if (token->token != JSON_TOKEN_COMMA_ID )
	    {
	      if (token->token != JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		{
		  printf("// call parameters wrong token %i\n",token->token);
		}
	      return token;
	    }
	  printf("// call param %i\n",i);
	  ++i;
	  c_print_json_token(parser,token);
	}
      else
	{
	  return token;
	}
      token=c_parse_next(parser);
    }
  printf("// call param %i end\n",i);
  return token;
  
}

// 0 if not an operator
// else number of elements it operates on
int c_operator_arity(struct c_parser_ctx * parser,struct al_token * token)
{

  if ( token != NULL )
    {
      if ( ( token->token == JSON_TOKEN_COMPARE_EQUAL_ID )
	   || ( token->token == JSON_TOKEN_COMPARE_DIFFERENT_ID )
	   || ( token->token == JSON_TOKEN_SUPERIOR_ID )
	   || ( token->token == JSON_TOKEN_INFERIOR_ID ) )
	{
	  return 2;
	}
      if ( token->token == JSON_TOKEN_EXCLAMATION_ID )
	{
	  return 1;
	}	   
    }
  else
    {
      return 0;
    }
}

// a + b + c + d ... TODO :-)
struct al_token * c_parse_simple_expression(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token=c_parse_next(parser);
    }

  int arity = 0;
  if ( c_operator_arity(parser,token) == 0 )
    {
      // this is not an operator.
    }
  else
    {
      // this is an operator
    }

  if ( token != NULL )
    {
      token = eat_json_token(JSON_TOKEN_EXCLAMATION_ID,parser,token);      
      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      token = c_parse_rhs(parser,token);
      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      if ( ( token->token == JSON_TOKEN_COMPARE_EQUAL_ID )
	   || ( token->token == JSON_TOKEN_COMPARE_DIFFERENT_ID )
	   || ( token->token == JSON_TOKEN_SUPERIOR_ID )
	   || ( token->token == JSON_TOKEN_INFERIOR_ID ) )
	{
	  c_print_json_token(parser,token);  
	  token = NULL;
	}
    }
  return token;
  
}

struct al_token * c_parse_simple_boolean_expression(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token=c_parse_next(parser);
    }

  if ( token != NULL )
    {
      token = eat_json_token(JSON_TOKEN_EXCLAMATION_ID,parser,token);      
      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      token = c_parse_rhs(parser,token);
      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      if ( ( token->token == JSON_TOKEN_COMPARE_EQUAL_ID )
	   || ( token->token == JSON_TOKEN_COMPARE_DIFFERENT_ID )
	   || ( token->token == JSON_TOKEN_SUPERIOR_ID )
	   || ( token->token == JSON_TOKEN_INFERIOR_ID ) )
	{
	  c_print_json_token(parser,token);  
	  token = c_parse_simple_boolean_expression(parser,NULL);
	}
    }
  return token;
  
}

// should be logical expression
struct al_token * c_parse_logical_expression(struct c_parser_ctx * parser,struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;

  printf("// parse logical expression\n");
  if ( token == NULL )
    {
      token=c_parse_next(parser);
    }
  while ( token != NULL )
    {
      if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	{
	  c_print_json_token(parser,token);		  
	  token = c_parse_logical_expression(parser,NULL);
	  token = eat_json_token(JSON_TOKEN_CLOSE_PARENTHESIS_ID,parser,token);
	  if ( token == NULL )
	    {
	      token=c_parse_next(parser);
	    }
	  else
	    {
	      printf("// error logical expression unbalanced parenthesis\n");
	      parser->state=C_STATE_ERROR;
	      return token;
	    }
	}
      else {
	token = c_parse_simple_boolean_expression(parser,token);
	if ( token == NULL )
	  {
	    token=c_parse_next(parser);
	  }
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
	      c_print_json_token(parser,token);
	      token=c_parse_next(parser);
	    }
	  else
	    {
	      if ( token->token != JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		{
		  printf("invalid logical combination token %i at %i", token->token, parser->token_count);
		}
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
  token=c_parse_next(parser);
  if ( token == NULL )
    {
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }
  if (token->token == JSON_TOKEN_WORD_ID)
    {
      if (index > 0 )
	{
	  printf(",\n",index);
	}
      c_print_json_token(parser,token);
      printf(" //=%i\n",index);
    }
  else
    {
      return token;
    }
  token=c_parse_next(parser);
  if ( token == NULL )
    {
		  printf("// empty token in parse enum member\n");
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }
  
  if ( token->token == JSON_TOKEN_EQUAL_ID )
    {
      c_print_json_token(parser,token);
      token=c_parse_next(parser);
      if ( token != NULL )
	{
	  if ( token->token == JSON_TOKEN_NUMBER_ID )
	    {
	      c_print_json_token(parser,token);
	    }
	  else
	    {
	      printf("// ERROR enum value expected number value, got token %i", token->token);
	      return token;
	    }
	}
      else
	{
	  // ... fixme in fact too short missing value
	  return NULL;
	}
      token=c_parse_next(parser);
      if ( token == NULL )
	{
	  // ... fixme in fact too short missing value
	  return NULL;
	}
    }

  if (token->token == JSON_TOKEN_COMMA_ID)
    {
      // c_print_json_token(parser,token);
      return NULL;
    }
  else
    {
      return token;
    }
}


struct al_token * c_parse_define_type(struct c_parser_ctx * parser, struct al_token * token, int within_typedef)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  void * lhs_variable_data = NULL;
  
  token=c_parse_left_type(parser,token,parser->last_word);

  // a full parsing of a function definition was done ...
  lhs_variable_data = parser->lhs_variable_data;
  
  if ( token == NULL )
    {
      token = c_parse_next(parser);
    }

  if (token != NULL )
    {
      // struct or enum definition.
      if (token->token == JSON_TOKEN_OPEN_BRACE_ID )
	{
	  printf("{ // type %i definition.\n", parser->last_type);
	  if (parser->last_type == TOKEN_C_STRUCT_ID )
	    {
	      struct c_struct_info struct_info;
	      int index = 0;
	      printf("// struct definition \n");
	      
	      token = c_parse_next(parser);

	      while ( (token != NULL) && ( token->token != JSON_TOKEN_CLOSE_BRACE_ID) )
		{
		  // ??? c_parse_left_type + variable ?
		  token = c_parse_struct_member(&struct_info,parser,token);
		  printf("// struct member %i", index);
		  ++index;
		  // always move forward ( unless parses same token forever... )
		  // if ( token == NULL )
		    {
		      token = c_parse_next(parser);
		    }
		}
		
	      if ( token != NULL )
		{		  
		  if (token->token == JSON_TOKEN_CLOSE_BRACE_ID )
		    {
		      printf("} // close struct\n");
		      if ( within_typedef == 1  )
			{
			  return NULL;
			}
		      // struct definition always ends with ;
		      token=eat_semi_colon(parser,NULL);
		      return token;
		    }
		  else
		    {
		      return token;
		    }
		}
	    }
	  else if (parser->last_type == TOKEN_C_ENUM_ID )
	    {
	      struct c_enum_info enum_info;
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
		      printf("} // close enum\n");
		      if ( within_typedef == 1  )
			{
			  return NULL;
			}
		      // enum definition always ends with ;
		      token=eat_semi_colon(parser,NULL);
		      return token;
		    }
		  else
		    {
		      return token;
		    }
		}
	    }
	  else {
	    // another type ...
	    printf("// ANOTHER TYPE ???\n");
	  }
	  return NULL;
	}
    }

  // this declare a variable of this type.
  if (c_parse_variable(parser,token) == NULL)
    {

      printf("// declaration (type %i).\n", parser->last_type);
      token = c_parse_next(parser);
	  
      if ( token != NULL )
	{
	  if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	    {	      
	      c_print_json_token(parser,token);
	      printf("// function\n");
	      token=c_parse_call_definition_parameters(parser);
	      token=eat_json_token(JSON_TOKEN_CLOSE_PARENTHESIS_ID,parser,token);
	      if ( token != NULL )
		{
		  return token;
		}
	    }

	  if ( token == NULL )
	    {
	      token = c_parse_next(parser);
	    }

	  // body
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_OPEN_BRACE_ID )
		{
		  // block { and } will be printed by c_parse_block
		  token = c_parse_block(parser,token,C_STATE_FUNCTION_DEFINITION_ID);
		  if ( token != NULL )
		    {
		      printf("// block unfinished\n");
		      return token;
		    }
		  printf("// block finished\n");
		  return NULL;
		}
	      else if ( token->token == JSON_TOKEN_SEMI_COLON_ID )
		{
		  printf("; // no body\n");
		  return NULL;
		}
	    }

	  if ( token == NULL )
	    {
	      token = c_parse_next(parser);
	    }

	  // initialization
	  if ( token->token == JSON_TOKEN_EQUAL_ID )
	    {
	      c_print_json_token(parser,token);
	      token=c_parse_rhs_semi_colon(parser,NULL);
	      return token;
	    }
	}
    }
  else
    {
      if ( token->token == JSON_TOKEN_SEMI_COLON_ID )
	{
	  printf("// forward type declaration\n");
	  return token;
	}
      if ( token->token == JSON_TOKEN_CLOSE_BRACE_ID )
	{
	  return token;
	}
      printf("unrecognized declaration expected variable got %i", token->token);
      return token;
    }
      
  return token;

}

// parse statements *; case => excluded.
struct al_token * c_parse_case(struct c_parser_ctx * parser, struct al_token * token)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  int ca = 0;
  parser->state = C_STATE_START_ID;
  if (token == NULL)
    {
      token=c_parse_next(parser);      
    }
  while ( (token=c_parse_statement(parser,NULL,C_STATE_START_ID)) == NULL )
    {
      ++ca;
      printf("// within case line %i\n", ca);
    }
  if ( token != NULL )
    {
      if (token->token == JSON_TOKEN_WORD_ID )
	{
	  int c_token = parser->last_word;
	  if ( c_token == TOKEN_C_CASE_ID )
	    {
	      print_c_token(parser,c_token);
	      return token;
	    }
	  else
	    {
	      printf("// exit case line %i NOT a case %i \n", ca, c_token);
	    }
	}
    }
  printf("// exit case line %i\n", ca);
  return token;
}

struct al_token * c_parse_case_statement(struct c_parser_ctx * parser, struct al_token * token,  enum c_parser_state level_state);

// parse '{' statements* '}' , returns NULL if block parsing is ok.
// alll element within this block are in state state 
struct al_token * c_parse_block(struct c_parser_ctx * parser, struct al_token * token, enum c_parser_state state)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  printf("//block start at %i\n", parser->token_count);
  token = eat_json_token(JSON_TOKEN_OPEN_BRACE_ID,parser,token);
  if ( token == NULL )
    {
      printf("\n");
      printf("//block line (state %i)\n", state);
      // remark token == NULL at start
      int line = 0;
      int check_token_count=parser->token_count;
      int loop_protection = 2;
      while ( parser->state != C_STATE_ERROR )
	{
	  check_token_count=parser->token_count;
	  if (token == NULL )
	    {
	      token = c_parse_next(parser);
	    }
	  if ( token != NULL )
	    {
	      if (token->token == JSON_TOKEN_CLOSE_BRACE_ID )
		{
		  printf("}\n");
		  return NULL;
		}
	      else if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
		{
		  printf("// block within block...\n");
		  // remark loose state, restart ...
		  token = c_parse_block(parser,token,C_STATE_START_ID);
		}	    
	    }
	  printf("// block line (state %i) line %i token count %i\n", state, line, parser->token_count );
	  if ( state == C_STATE_SWITCH_ID )
	    {
	      token=c_parse_case_statement(parser,token,state);
	    }
	  else
	    {
	      token=c_parse_statement(parser,token,state);
	    }
	  ++line;
	  if ( parser->token_count == check_token_count )
	    {
	      printf("// block loop parsing risk at %i \n", parser->token_count );
	      -- loop_protection;
	      if ( loop_protection <= 0 )
		{
		  printf("// block loop protection at %i \n", parser->token_count );
		  break;
		}
	    }
	}
    }
  else
    {
      // a block should start with a '{'
      return token;
    }
  parser->state = C_STATE_ERROR;
  return token;
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
  // function call or declaration ==> not the same thing...
  if ( (token = c_parse_call_parameters(parser,NULL)) == NULL)
    {
		  printf("// parse function params\n");
      token=c_tokenizer(tokenizer,parser->tokenizer_data);
    }
  if (token != NULL )
    {
     if  ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
       {
	 // function call
	 printf(")");
	 if ( (token=c_parse_next(parser)) != NULL)
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
	       token = c_parse_block(parser,token,C_STATE_FUNCTION_DEFINITION_ID);
	     }
	     return token;
	   }
	 return NULL;
       }
     else {
       // declaration
       while ( (token = c_parse_statement(parser,NULL,C_STATE_START_ID) ) == NULL);
       if  ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
	 {
	   printf(")");
	   return c_parse_block(parser,NULL,C_STATE_FUNCTION_DECLARATION_ID);
	 }
       else
	 {
	   printf("[ERROR] a function parameters fails got %i token\n",token->token);
	 }
     }
  }
  
}

// toplevel statement : statement that is not within a block.
struct al_token * c_parse_toplevel_statement(struct c_parser_ctx * parser, struct al_token * token, int c_token, enum c_parser_state level_state)
{
  struct json_ctx* tokenizer = parser->tokenizer;
  if ( token == NULL )
    {
      token=c_parse_next(parser);
      c_token = parser->last_word;
    }
  
  if ( token == NULL )
    {
		  printf("// empty token in parse toplevel statement\n");
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      parser->state == C_STATE_ERROR;
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
    case JSON_TOKEN_INCREMENT_ID:
    case JSON_TOKEN_DECREMENT_ID:
    case JSON_TOKEN_STAR_ID:
    case JSON_TOKEN_DOT_ID:
    case JSON_TOKEN_RIGHT_ARROW_ID:
    case JSON_TOKEN_SUPERIOR_ID:
    case JSON_TOKEN_INFERIOR_ID:
    case JSON_TOKEN_EXCLAMATION_ID:
    case JSON_TOKEN_NUMBER_ID:
    case JSON_TOKEN_DQUOTE_ID:
    case JSON_TOKEN_SQUOTE_ID:
      printf("// [WARNING] unexpected token %i at start of a toplevel statement\n",token->token);
      parser->state == C_STATE_ERROR;
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_print_json_token(parser,token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_print_json_token(parser,token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf("EOF id reached");
      parser->state == C_STATE_ERROR;
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
      printf("[ERROR] a new toplelvel statement should start with a word. got token %i \n",token->token);
      return token;
    }

  if ( c_token == TOKEN_C_NOMATCH_ID )
    {
      printf("// a toplevel variable or function name at %i \n", parser->token_count);
      token = c_parse_lhs(parser,token);
      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      c_token = parser->last_word;
      if ( token != NULL )
	{
	  if ( ( token->token == JSON_TOKEN_EQUAL_ID )
	       || (token->token == JSON_TOKEN_ADD_ID ) )
	    {
	      c_print_json_token(parser,token);
	      token = c_parse_rhs_semi_colon(parser,NULL);
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
	  parser->state == C_STATE_ERROR;
	}
      return token;
    }
  else
    if ( c_token == TOKEN_C_TYPEDEF_ID )
    {
      // typedef ...
      print_c_token(parser,c_token);
      token = c_parse_define_type(parser,NULL,1);
      
      if ( (token != NULL) && (parser->lhs_variable_data == NULL) )
	{
	    if ( (token = c_parse_variable(parser,token)) == NULL )
	    {
	      token=c_parse_next(parser);
	    }
	    else
	      {
		printf("[ERROR] expected name for toplevel typedef  ... {} name expected got token %i \n",token->token);
		parser->state == C_STATE_ERROR;
		return token;
	      }
	}
      
      if ( (token != NULL) && (token->token != JSON_TOKEN_SEMI_COLON_ID))
	{
	  printf("[ERROR] expected ; to close typedef  ... {} name expected got token %i \n",token->token);
	}
      else
	{
	  printf(";\n");
	}      
    }
  else
    {
      printf("// toplevel default statement\n");
      token = c_parse_define_type(parser,token,0);
      if ( token == NULL )
	{
	  return NULL;
	  //token=c_parse_next(parser);
	}
      else
	{
	  if ( token->token != JSON_TOKEN_SEMI_COLON_ID )
	    {
	      printf("[ERROR] expected ; to close type {}  got token %i \n",token->token);
	      parser->state == C_STATE_ERROR;
	      return token;
	    };
	}
      printf(";\n");
      return NULL;
    }

  return NULL;
}


// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read ahead was needed.
struct al_token * c_parse_if_statement(struct c_parser_ctx * parser, struct al_token * token,  enum c_parser_state level_state)
{
  if  ( parser->last_word == TOKEN_C_IF_ID )
    {	  
      printf("// IF START\n");
      print_c_token(parser,parser->last_word);
      printf(" ");
      token=eat_json_token(JSON_TOKEN_OPEN_PARENTHESIS_ID,parser,NULL);
      if ( token == NULL )
	{
	  token = c_parse_logical_expression(parser,NULL);
	  token = eat_json_token(JSON_TOKEN_CLOSE_PARENTHESIS_ID,parser,token);
	  if ( token == NULL )
	    {
	      token = c_parse_block(parser,NULL,C_STATE_START_ID);
	    }
	  else
	    {
	      printf("// wrong expression for if missing ')' token %i", token->token);
	    }
	}
      else
	{
	  printf("// wrong expression for if missing '(' token %i", token->token);
	}
      printf("// IF END\n");
      parser->state=C_STATE_IF_BLOCK_ID;
      if ( token == NULL )
	{
	  token = c_parse_next(parser);
	}
      if ( parser->last_word == TOKEN_C_ELSE_ID )      
	{
	  if (parser->state == C_STATE_IF_BLOCK_ID )
	    {
	      print_c_token(parser,parser->last_word);
	      parser->state = C_STATE_START_ID;
	      token = c_parse_next(parser);	      
	      if ( ( token != NULL ) && ( parser->last_word == TOKEN_C_IF_ID ) )
		{
		  return c_parse_if_statement(parser,token, C_STATE_IF_BLOCK_ID);
		}			  
	      else
		{
		  token = c_parse_block(parser,token,C_STATE_START_ID);
		}
	    }
	  else
	    {
	      printf("[ERROR] else statement not following a if block (state %i !=  C_STATE_IF_BLOCK_ID %i )",parser->state, C_STATE_IF_BLOCK_ID);
	      parser->state == C_STATE_ERROR;
	    }
	  return token;
	}
      return token;
    }
  parser->state == C_STATE_ERROR;
  return token;
}

// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read aheadwas needed.
struct al_token * c_parse_case_statement(struct c_parser_ctx * parser, struct al_token * token,  enum c_parser_state level_state)
{
  struct json_ctx* tokenizer = parser->tokenizer;

  if ( token == NULL )
    {
      token=c_parse_next(parser);
    }
  
  if ( token == NULL )
    {
		  printf("// empty token in parse case statement\n");
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      parser->state == C_STATE_ERROR;
      return &tokenizer->last_token;
    }

  switch(token->token)
    {
    case JSON_TOKEN_WORD_ID:
    case JSON_TOKEN_INCREMENT_ID:
    case JSON_TOKEN_DECREMENT_ID:
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
      parser->state == C_STATE_ERROR;
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_print_json_token(parser,token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_print_json_token(parser,token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf("EOF id reached");
      parser->state == C_STATE_ERROR;
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
		  if ( ( token->token == JSON_TOKEN_INCREMENT_ID ) || ( token->token == JSON_TOKEN_DECREMENT_ID ) )
	{
	  c_print_json_token(parser,token);
	  token = c_parse_lhs(parser,NULL);
	  if ( token == NULL )
	    {
	      token=c_parse_next(parser);
	    }
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_SEMI_COLON_ID)		
		{
		  c_print_json_token(parser,token);
		  return NULL;
		}
	    }
	  parser->state == C_STATE_ERROR;
	  return token;
	}
      else
	{
	  printf("[ERROR] a new statement should start with a word. got token %i \n",token->token);
	  parser->state == C_STATE_ERROR;
	  return token;
	}
    }

  if ( parser->last_word == TOKEN_C_NOMATCH_ID )
    {
      printf("// a variable or function name at %i\n", parser->token_count);
      token = c_parse_lhs(parser,token);

      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      if ( token != NULL )
	{
	  if ( ( token->token == JSON_TOKEN_EQUAL_ID )
	       || (token->token == JSON_TOKEN_ADD_ID ) )
	    {
	      c_print_json_token(parser,token);
	      token = c_parse_rhs_semi_colon(parser,NULL);
	    }
	  else
	  if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	    {	      
	      return c_parse_function_params(parser,token);
	    }
	}
      if (token != NULL )
	{
	  printf("[ERROR] a new statement can not start with unrecognized word %i at %i\n", token->token, parser->token_count);
	  parser->state == C_STATE_ERROR;
	}
      return token;
    }
  else
    {
	if ( parser->last_word == TOKEN_C_CASE_ID )
	{	  
	  if (level_state == C_STATE_SWITCH_ID )
	    {
	      print_c_token(parser,parser->last_word);
	      token = c_parse_rhs(parser,NULL);
	      if ( token != NULL )
		{
		  if (token->token == JSON_TOKEN_COLON_ID )
		    {
		      c_print_json_token(parser,token);
		      return NULL;
		    }
		  else
		    {
		      printf("wrong token for case %i\n", token-token);
		    }
		}
	      parser->state == C_STATE_ERROR;
	      return token;
	    }
	  else
	    {
	      printf("[ERROR] case statement not within a switch block (state %i %i)",parser->state, level_state);
	    }
	  return token;
	}
      else
	if ( parser->last_word == TOKEN_C_DEFAULT_ID )
	  {	  
	    if (level_state == C_STATE_SWITCH_ID )
	      {
		print_c_token(parser,parser->last_word);
		token=c_parse_next(parser);
		if ( token != NULL )
		  {
		    if (token->token == JSON_TOKEN_COLON_ID )
		      {
			c_print_json_token(parser,token);
			return NULL;
		      }
		    else
		      {
			printf("wrong token for case %i\n", token-token);
		      }
		  }
		return token;
	      }
	    else
	      {
		parser->state == C_STATE_ERROR;
		printf("[ERROR] default statement not within a switch block (state %i;%i)\n", parser->state,level_state);
	      }
	    return token;	    
	  }
      else
      if ( parser->last_word == TOKEN_C_RETURN_ID )
	{
      	  print_c_token(parser,parser->last_word);
	  token=c_parse_rhs_semi_colon(parser,NULL);
	  parser->state = C_STATE_START_ID;
	  return token;
        }
      else
      if ( parser->last_word == TOKEN_C_BREAK_ID )
	{
      	  print_c_token(parser,parser->last_word);
	  token=eat_semi_colon(parser,NULL);
	  if (token != NULL )
	    {
	      printf("// expected ; after break");
	      parser->state == C_STATE_ERROR;
	    }
	  else
	    {
	      parser->state = C_STATE_START_ID;
	    }
	  return token;
        }
      else if ( parser->last_word == TOKEN_C_IF_ID )
	{
	  return c_parse_if_statement(parser,token, C_STATE_IF_BLOCK_ID);
	}
      else if ( parser->last_word == TOKEN_C_WHILE_ID )
	{
	  print_c_token(parser,parser->last_word);
	  printf(" ");
	  token=c_parse_next(parser);
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
		{
		  c_print_json_token(parser,token);		  
		  token = c_parse_logical_expression(parser,NULL);
		  if ( token == NULL )
		    {
		      token=c_parse_next(parser);
		    }
		  if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		    {
		      c_print_json_token(parser,token);
		      printf("// parse while block at %i\n", parser->token_count);
		      token = c_parse_block(parser,NULL,C_STATE_START_ID);
		    }
		}
	    }
	  if ( token != NULL )
	    {
            parser->state == C_STATE_ERROR;
	    }
	  return token;
	}
      else if ( parser->last_word == TOKEN_C_SWITCH_ID )
	{
	  parser->state=C_STATE_ERROR;
	  printf("// [ERROR] switch in switch\n");
	  return token;
	}
      else	
	{
	  // non struct,enum,if,while or switch	  
	  token = c_parse_toplevel_statement(parser,token,parser->last_word,C_STATE_START_ID);
	  return token;
	}
    }
  return NULL;
}

// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read ahead was needed.
struct al_token * c_parse_switch_statement(struct c_parser_ctx * parser, struct al_token * token,  enum c_parser_state level_state)
{
  if ( parser->last_word == TOKEN_C_SWITCH_ID )
    {
      print_c_token(parser,parser->last_word);
      printf(" ");
      token=c_parse_next(parser);
      if ( token != NULL )
	{
	  if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	    {
	      c_print_json_token(parser,token);	  
	      token = c_parse_rhs(parser,NULL);
	      if ( token == NULL )
		{
		  token=c_parse_next(parser);
		}
	      if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		{
		  c_print_json_token(parser,token);
		  token = c_parse_block(parser,NULL,C_STATE_SWITCH_ID);
		}
	    }
	}
      if ( token != NULL )
	{
	  parser->state == C_STATE_ERROR;
	}
      return token;
    }
  else	
    {
      // non struct,enum,if,while or switch	  
      token = c_parse_toplevel_statement(parser,token,parser->last_word,C_STATE_START_ID);
      return token;
    }
}


// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read aheadwas needed.
struct al_token * c_parse_statement(struct c_parser_ctx * parser, struct al_token * token,  enum c_parser_state level_state)
{
  struct json_ctx* tokenizer = parser->tokenizer;

  if ( token == NULL )
    {
      token=c_parse_next(parser);
    }
  
  if ( token == NULL )
    {
		  printf("// empty token in parse statement\n");
      tokenizer->last_token.token=JSON_TOKEN_EOF_ID;
      parser->state == C_STATE_ERROR;
      return &tokenizer->last_token;
    }

  switch(token->token)
    {
    case JSON_TOKEN_WORD_ID:
    case JSON_TOKEN_INCREMENT_ID:
    case JSON_TOKEN_DECREMENT_ID:
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
      parser->state == C_STATE_ERROR;
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_print_json_token(parser,token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_print_json_token(parser,token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf("EOF id reached");
      parser->state == C_STATE_ERROR;
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
		  if ( ( token->token == JSON_TOKEN_INCREMENT_ID ) || ( token->token == JSON_TOKEN_DECREMENT_ID ) )		
	{
	  c_print_json_token(parser,token);
	  token = c_parse_lhs(parser,NULL);
	  if ( token == NULL )
	    {
	      token=c_parse_next(parser);
	    }
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_SEMI_COLON_ID)		
		{
		  c_print_json_token(parser,token);
		  return NULL;
		}
	    }
	  parser->state == C_STATE_ERROR;
	  return token;
	}
      else
	{
	  printf("[ERROR] a new statement should start with a word. got token %i \n",token->token);
	  parser->state == C_STATE_ERROR;
	  return token;
	}
    }

  if ( parser->last_word == TOKEN_C_NOMATCH_ID )
    {
      printf("// a variable or function name at %i\n", parser->token_count);
      token = c_parse_lhs(parser,token);

      if ( token == NULL )
	{
	  token=c_parse_next(parser);
	}
      if ( token != NULL )
	{
	  if ( ( token->token == JSON_TOKEN_EQUAL_ID )
	       || (token->token == JSON_TOKEN_ADD_ID ) )
	    {
	      c_print_json_token(parser,token);
	      token = c_parse_rhs_semi_colon(parser,NULL);
	    }
	  else
	  if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
	    {	      
	      return c_parse_function_params(parser,token);
	    }
	}
      if (token != NULL )
	{
	  printf("[ERROR] a new statement can not start with unrecognized word %i at %i\n", token->token, parser->token_count);
	  parser->state == C_STATE_ERROR;
	}
      return token;
    }
  else
    {
	if ( parser->last_word == TOKEN_C_CASE_ID )
	{	  
	  if (level_state == C_STATE_SWITCH_ID )
	    {
	      print_c_token(parser,parser->last_word);
	      token = c_parse_rhs(parser,NULL);
	      if ( token != NULL )
		{
		  if (token->token == JSON_TOKEN_COLON_ID )
		    {
		      c_print_json_token(parser,token);
		      return NULL;
		    }
		  else
		    {
		      printf("wrong token for case %i\n", token-token);
		    }
		}
	      parser->state == C_STATE_ERROR;
	      return token;
	    }
	  else
	    {
	      printf("[ERROR] case statement not within a switch block (state %i %i)",parser->state, level_state);
	    }
	  return token;
	}
      else
	if ( parser->last_word == TOKEN_C_DEFAULT_ID )
	  {	  
	    if (level_state == C_STATE_SWITCH_ID )
	      {
		print_c_token(parser,parser->last_word);
		token=c_parse_next(parser);
		if ( token != NULL )
		  {
		    if (token->token == JSON_TOKEN_COLON_ID )
		      {
			c_print_json_token(parser,token);
			return NULL;
		      }
		    else
		      {
			printf("wrong token for case %i\n", token-token);
		      }
		  }
		return token;
	      }
	    else
	      {
		parser->state == C_STATE_ERROR;
		printf("[ERROR] default statement not within a switch block (state %i;%i)\n", parser->state,level_state);
	      }
	    return token;	    
	  }
      else
      if ( parser->last_word == TOKEN_C_RETURN_ID )
	{
      	  print_c_token(parser,parser->last_word);
	  token=c_parse_rhs_semi_colon(parser,NULL);
	  parser->state = C_STATE_START_ID;
	  return token;
        }
      else
	if ( ( parser->last_word == TOKEN_C_BREAK_ID ) || ( parser->last_word == TOKEN_C_CONTINUE_ID ) )
	{
      	  print_c_token(parser,parser->last_word);
	  token=eat_semi_colon(parser,NULL);
	  if (token != NULL )
	    {
	      printf("// expected ; after break");
	      parser->state == C_STATE_ERROR;
	    }
	  else
	    {
	      parser->state = C_STATE_START_ID;
	    }
	  return token;
        }
      else if ( parser->last_word == TOKEN_C_IF_ID )
	{
	  return c_parse_if_statement(parser,token, C_STATE_IF_BLOCK_ID);
	}
      else if ( parser->last_word == TOKEN_C_WHILE_ID )
	{
	  print_c_token(parser,parser->last_word);
	  printf(" ");
	  token=c_parse_next(parser);
	  if ( token != NULL )
	    {
	      if ( token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID )
		{
		  c_print_json_token(parser,token);		  
		  token = c_parse_logical_expression(parser,NULL);
		  if ( token == NULL )
		    {
		      token=c_parse_next(parser);
		    }
		  if ( token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID )
		    {
		      c_print_json_token(parser,token);
		      token = c_parse_block(parser,NULL,C_STATE_START_ID);
		    }
		}
	    }
	  if ( token != NULL )
	    {
            parser->state == C_STATE_ERROR;
    }
	  return token;
	}
      else if ( parser->last_word == TOKEN_C_SWITCH_ID )
	{
	  return c_parse_switch_statement(parser,token, C_STATE_START_ID);
	}
      else	
	{
	  // non struct,enum,if,while or switch	  
	  token = c_parse_toplevel_statement(parser,token,parser->last_word,C_STATE_START_ID);
	  return token;
	}
    }
  return NULL;
}


int init_c_parser(  struct c_parser_ctx* parser, struct json_ctx* tokenizer, void * data)
{
  bzero(parser,sizeof(*parser));
  parser->state=C_STATE_START_ID;
  parser->tokenizer=tokenizer;
  parser->tokenizer_data=data;
  parser->last_type=-1;
  parser->last_word=-1;
  // length in number of entries [ at least ALHASH_BUCKET_SIZE will be used ]
  // long (*alhash_func) (void * value, int length));
  todo("support a growable hashtable. here limited to 1024 words");
  alhash_init(&parser->dict, 1024, NULL);
  
  parser->word_buffer_length=10240;
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
      parser.token_count=0;
      int check_token_count=parser.token_count;
      while ( parser.state != C_STATE_ERROR )
	{
	  check_token_count=parser.token_count;
	  token = c_parse_toplevel_statement(&parser,NULL,TOKEN_C_NOMATCH_ID,C_STATE_START_ID);
	  if ( token != NULL )
	    {
	      printf("// non NULL token at toplevel parsing\n");
	      break;
	    }
	  if (parser.token_count == check_token_count)
	    {
	      printf("// risk of parsing loop\n");
	      break;
	    }
	}

      if (token->token != JSON_TOKEN_EOF_ID )
	{
	  printf("[level %i] expected eof, got token %i.\n", parser.nested,token->token);
	}
      
      fflush(stdout);
      todo("implement parser ... !");
    }
  else
    {
      usage();
    }
}

