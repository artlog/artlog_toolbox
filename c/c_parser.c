#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "c_parser.h"
#include "todo.h"
#include "aljson_import_internal.h"
#include "al_options.h"
#include "alcommon.h"


void
usage()
{
  printf("\nUSAGE:\n");
  printf("work in progress: first goal is to generate json stub from c struct definition see json_to_c_stub.c\n");

  printf("ex:./c_parser infile=./input_for_c_parser.h outform=aljson_stub\n");

  printf("more advanced goal is to be a c parser ... \n");
  printf("ex:./c_parser debug=true infile=./c_parser.c\n");

}


char *
json_token_to_char (enum json_token_id token_id)
{
  // ==> need a generator :-)
}

int
c_parser_is_debug (struct c_parser_ctx *parser)
{
  return ALC_FLAG_IS_SET(parser->flags,ALCPARSER_DEBUG);
}

void
c_show_info (struct c_parser_ctx *parser, char *category, char *info)
{
  if (info != NULL)
    {
      struct json_pos_info *pos = &parser->tokenizer->pos_info;
      if (c_parser_is_debug (parser))
	{
	  printf
	    ("// [%s] %s token %i c_token %i, state %i at nested:%i count:%i LC=(%i,%i)\n",
	     category, info, parser->last_token, parser->last_word, parser->state,
	     parser->nested, parser->token_count, pos->line, pos->column);
	}
    }
}

void
c_create_error (struct c_parser_ctx *parser, enum c_parser_state state,
		char *info)
{
  if (info != NULL)
    {
      if (state == C_STATE_ERROR)
	{
	  c_show_info (parser, "ERROR", info);
	}
      else if (state == C_STATE_REJECT)
	{
	  c_show_info (parser, "REJECT", info);
	}
      else
	{
	  c_show_info (parser, "DEBUG", info);
	}
    }
  parser->state = state;
}

void
reset_tokenizer_buffer (struct json_ctx *tokenizer)
{
  // reset when word is parsed and recognized as either a reserved word or stored in variable dict with cut_string.
  // printf("//reset token buffer\n");
  tokenizer->token_buf.bufpos = 0;
}


/* return an entry pointer in global dict_index table
 */
struct alhash_entry *
alparser_dict_add_string (struct alparser_ctx *alparser, char * buffer, int length)
{

  if (buffer == NULL)
    {
      assert(buffer!=NULL);
      todo ("[FATAL] corrupted parser token char buffer NULL");
      return NULL;
    }

  if (length == 0)
    {
      assert(length!=0);
      printf ("[FATAL] corrupted parser empty char buffer\n");
      return NULL;
    }
  if (length >= 0)
    {
      // thanks to this format ... print non NULL terminated string
      printf ("%.*s", length, buffer);
    }
  else
    {
      fprintf (stderr,
	       "[FATAL] corrupted parser token char buffer length %i <=0\n",
	       length);
      return NULL;
    }

  struct alhash_datablock key;
  struct alhash_datablock *valuep;
  
  //  create an entry in dict
  key.type = ALTYPE_OPAQUE;
  key.length = length;
  key.data.ptr = &alparser->word_buffer.buf[alparser->word_buffer.bufpos];
  if ((alparser->word_buffer.bufpos + length) < alparser->word_buffer.bufsize)
    {
      memcpy (key.data.ptr, buffer, length);
    }
  else
    {
      fprintf (stderr,
	       "[WARNING] internal char buffer for words full %i+%i>%i",
	       alparser->word_buffer.bufpos, length, alparser->word_buffer.bufsize);
      // alparser_grow_word_buffer (alparser);
      todo ("[FATAL] grow word buffer . not implemented");
      exit (1);
    }

  struct alhash_entry *entry = alhash_get_entry (&alparser->dict, &key);
  if (entry == NULL)
    {
      // no entry found, create it
      ++alparser->words;
      alparser->word_buffer.bufpos += length;
      valuep = &key;
      entry = alhash_put (&alparser->dict, &key, valuep);
      if (entry == NULL)
	{
	  fprintf (stderr,
		   "[FATAL] FAIL to insert '%s' into word buffer %p \n",
		   buffer, &alparser->dict);
	  exit (1);
	}
    }
  else
    {
      // printf("SAME TOKEN SEEN\n");
    }

  return entry;
}

/* return an entry pointer in global dict_index table

  and set last_word and dict_value.

  parser->last_word = TOKEN_C_DICTENTRY_ID;
  parser->dict_value = (struct alhash_datablock *) value;

  tokenizer internal token_char_buffer will be flushed.
 */
void *
c_cut_token_string (struct c_parser_ctx *parser)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  struct token_char_buffer *tb = &tokenizer->token_buf;
  char *buffer = tb->buf;
  int length = tb->bufpos;

  struct alparser_ctx *alparser = &parser->alparser;

  struct alhash_entry *entry = alparser_dict_add_string(alparser, buffer, length);
    
  if (entry != NULL)
    {
        parser->last_word = TOKEN_C_DICTENTRY_ID;
	parser->dict_value = &entry->value;
	
	reset_tokenizer_buffer (tokenizer);
	
	return entry->value.data.ptr;
    }
  else
    {
      return NULL;
    }

}

char c_backslash[]="0123456a8tnbcnefghijklmdopqrs9uv";

// get char ( to display with \ prefix ) mainly for first 32 ascii characters
char c_getbackslash(char c)
{
  if ( c > ' ' )
    {
      return c;
    }
  else
    {
      return c_backslash[c];
    }
}
/* from internal c string display a formated string
 */
void *
c_cut_c_string (struct c_parser_ctx *parser, char stop)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  struct token_char_buffer *tb = &tokenizer->token_buf;
  char *buffer = tb->buf;
  int length = tb->bufpos;

  if (buffer == NULL)
    {
      todo ("[FATAL] corrupted parser token char buffer NULL");
      return NULL;
    }

  if (length == 0)
    {
      assert(length!=0);
      printf ("[FATAL] corrupted parser empty char buffer\n");
      return NULL;
    }
  if (length < 0)
    {
      fprintf (stderr,
	       "[FATAL] corrupted parser token char buffer length %i <=0\n",
	       length);
      return NULL;
    }
  for (int i=0; i < length; i++)
    {
      char c=buffer[i];
      if ( c >= ' ' )
	{
	  printf("%c",c);
	}
      else
	{
	  printf("\\%c", c_getbackslash(c));
	}
    }
  reset_tokenizer_buffer (tokenizer);
  return NULL;
}

void
print_c_token (struct c_parser_ctx *parser, enum c_word_token c_token)
{
  switch (c_token)
    {
    case TOKEN_C_IF_ID:
      printf ("if");
      break;
    case TOKEN_C_WHILE_ID:
      printf ("while");
      break;
    case TOKEN_C_ELSE_ID:
      printf ("else ");
      break;
    case TOKEN_C_RETURN_ID:
      printf ("return ");
      break;
    case TOKEN_C_STRUCT_ID:
      printf ("struct ");
      // either a declaration or a definition;      
      break;
    case TOKEN_C_UNION_ID:
      printf ("union ");
      break;
    case TOKEN_C_ENUM_ID:
      printf ("enum ");
      break;
    case TOKEN_C_SWITCH_ID:
      printf ("switch");
      break;
    case TOKEN_C_CASE_ID:
      printf ("case ");
      break;
    case TOKEN_C_DEFAULT_ID:
      printf ("default");
      break;
    case TOKEN_C_BREAK_ID:
      printf ("break");
      break;
    case TOKEN_C_CHAR_ID:
      printf ("char ");
      break;
    case TOKEN_C_INT_ID:
      printf ("int ");
      break;
    case TOKEN_C_FLOAT_ID:
      printf ("float ");
      break;
    case TOKEN_C_LONG_ID:
      printf ("long ");
      break;
    case TOKEN_C_VOID_ID:
      printf ("void ");
      break;
    case TOKEN_C_TYPEDEF_ID:
      printf ("typedef ");
      break;
    case TOKEN_C_FOR_ID:
      printf ("for ");
      break;
    default:
      printf ("<cword>%i</cword>", c_token);
    }
}


enum c_word_token
get_word_token (struct token_char_buffer *tb)
{
  char *buffer = tb->buf;
  int length = tb->bufpos;
  enum c_word_token word_token = TOKEN_C_NOMATCH_ID;

  if ((length == 0) || (buffer == NULL))
    {
      printf ("[ERROR] empty or null buffer for get_word_token %p %i", buffer,
	      length);
      return TOKEN_C_NOMATCH_ID;
    }

  if (length == 2)
    {
      if ((buffer[0] == 'i') && (buffer[1] == 'f'))
	{
	  word_token = TOKEN_C_IF_ID;
	}
    }
  else if (length == 3)
    {
      if (buffer[0] == 'i') 
	{
	  if ((buffer[1] == 'n') && (buffer[2] == 't'))
	    {
	      word_token = TOKEN_C_INT_ID;
	    }
	}
      else
      if (buffer[0] == 'f') 
	{
	  if ((buffer[1] == 'o') && (buffer[2] == 'r'))
	    {
	      word_token = TOKEN_C_FOR_ID;
	    }
	}
    }
  else if (length == 4)
    {
      if (buffer[0] == 'e')
	{
	  if (buffer[1] == 'n')
	    {
	      if ((buffer[2] == 'u') && (buffer[3] == 'm'))
		{
		  word_token = TOKEN_C_ENUM_ID;
		}
	    }
	  else
	    if ((buffer[1] == 'l') && (buffer[2] == 's')
		&& (buffer[3] == 'e'))
	    {
	      word_token = TOKEN_C_ELSE_ID;
	    }
	}
      else if (buffer[0] == 'v')
	{
	  if ((buffer[1] == 'o') && (buffer[2] == 'i') && (buffer[3] == 'd'))
	    {
	      word_token = TOKEN_C_VOID_ID;
	    }
	}
      else if (buffer[0] == 'c')
	{
	  if (buffer[1] == 'h')
	    {
	      if ((buffer[2] == 'a') && (buffer[3] == 'r'))
		{
		  word_token = TOKEN_C_CHAR_ID;
		}
	    }
	  else if (buffer[1] == 'a')
	    {
	      if ((buffer[2] == 's') && (buffer[3] == 'e'))
		{
		  word_token = TOKEN_C_CASE_ID;
		}
	    }
	}
      else if (buffer[0] == 'l')
	{
	  if ((buffer[1] == 'h') && (buffer[2] == 'a') && (buffer[3] == 'r'))
	    {
	      word_token = TOKEN_C_LONG_ID;
	    }
	}
    }
  else if (length == 5)
    {
      if ((buffer[0] == 'f') && (buffer[1] == 'l') && (buffer[2] == 'o')
	  && (buffer[3] == 'a') && (buffer[4] == 't'))
	{
	  word_token = TOKEN_C_FLOAT_ID;
	}
      if ((buffer[0] == 'w') && (buffer[1] == 'h') && (buffer[2] == 'i')
	  && (buffer[3] == 'l') && (buffer[4] == 'e'))
	{
	  word_token = TOKEN_C_WHILE_ID;
	}
      if ((buffer[0] == 'b') && (buffer[1] == 'r') && (buffer[2] == 'e')
	  && (buffer[3] == 'a') && (buffer[4] == 'k'))
	{
	  word_token = TOKEN_C_BREAK_ID;
	}
      if ((buffer[0] == 'u') && (buffer[1] == 'n') && (buffer[2] == 'i')
	  && (buffer[3] == 'o') && (buffer[4] == 'n'))
	{
	  word_token = TOKEN_C_UNION_ID;
	}

    }
  else if (length == 6)
    {
      if (buffer[0] == 's')
	{
	  if (buffer[1] == 'w')
	    {
	      if ((buffer[2] == 'i') && (buffer[3] == 't')
		  && (buffer[4] == 'c') && (buffer[5] == 'h'))
		{
		  word_token = TOKEN_C_SWITCH_ID;
		}
	    }
	  else if (buffer[1] == 'i')
	    {
	      if ((buffer[2] == 'g') && (buffer[3] == 'n')
		  && (buffer[4] == 'e') && (buffer[5] == 'd'))
		{
		  word_token = TOKEN_C_SIGNED_ID;
		}
	    }
	  else if (buffer[1] == 't')
	    {
	      if ((buffer[2] == 'r') && (buffer[3] == 'u')
		  && (buffer[4] == 'c') && (buffer[5] == 't'))
		{
		  word_token = TOKEN_C_STRUCT_ID;
		}
	    }
	}
      else if (buffer[0] == 'r')
	{
	  if ((buffer[1] == 'e') && (buffer[2] == 't') && (buffer[3] == 'u')
	      && (buffer[4] == 'r') && (buffer[5] == 'n'))
	    {
	      word_token = TOKEN_C_RETURN_ID;
	    }
	}
    }
  else if (length == 7)
    {
      if (buffer[0] == 'd')
	{
	  if (strncmp (buffer, "default", length) == 0)
	    {
	      word_token = TOKEN_C_DEFAULT_ID;
	    }
	}
      else if (buffer[0] == 't')
	{
	  if (strncmp (buffer, "typedef", length) == 0)
	    {
	      word_token = TOKEN_C_TYPEDEF_ID;
	    }
	}
    }
  else if (length == 8)
    {
      if (strncmp (buffer, "unsigned", length) == 0)
	{
	  word_token = TOKEN_C_UNSIGNED_ID;
	}
      else if (strncmp (buffer, "continue", length) == 0)
	{
	  word_token = TOKEN_C_CONTINUE_ID;
	}
    }
  return word_token;
}


enum c_word_token
c_parse_word_token (struct c_parser_ctx *parser)
{
  enum c_word_token word_token =
    get_word_token (&parser->tokenizer->token_buf);
  parser->last_word = word_token;
  if (word_token == TOKEN_C_NOMATCH_ID)
    {
      // will require a c_cut_token_string
    }
  else
    {
      //      printf("// c_parse_word_token %i", word_token);
      reset_tokenizer_buffer (parser->tokenizer);
    }
  return word_token;
}

void
c_print_json_token (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  switch (token->token)
    {
    case JSON_TOKEN_OPEN_PARENTHESIS_ID:
      printf ("(");
      break;
    case JSON_TOKEN_CLOSE_PARENTHESIS_ID:
      printf (")");
      break;
    case JSON_TOKEN_OPEN_BRACKET_ID:
      printf ("[");
      break;
    case JSON_TOKEN_CLOSE_BRACKET_ID:
      printf ("]");
      break;
    case JSON_TOKEN_OPEN_BRACE_ID:
      puts ("{");
      break;
    case JSON_TOKEN_CLOSE_BRACE_ID:
      puts ("}");
      break;
    case JSON_TOKEN_SEMI_COLON_ID:
      puts (";");
      break;
    case JSON_TOKEN_COLON_ID:
      puts (":");
      break;
    case JSON_TOKEN_EQUAL_ID:
      printf ("=");
      break;
    case JSON_TOKEN_COMPARE_EQUAL_ID:
      printf ("==");
      break;
    case JSON_TOKEN_COMPARE_DIFFERENT_ID:
      printf ("!=");
      break;
    case JSON_TOKEN_COMMA_ID:
      puts (",");
      break;
    case JSON_TOKEN_AMPERSAND_ID:
      printf ("&");
      break;
    case JSON_TOKEN_LOGICAL_AND_ID:
      printf ("&&");
      break;
    case JSON_TOKEN_STAR_ID:
      printf ("*");
      break;
    case JSON_TOKEN_DOT_ID:
      printf (".");
      break;
    case JSON_TOKEN_RIGHT_ARROW_ID:
      printf ("->");
      break;
    case JSON_TOKEN_SUPERIOR_ID:
      printf (">");
      break;
    case JSON_TOKEN_INFERIOR_ID:
      printf ("<");
      break;
    case JSON_TOKEN_SUPERIOR_EQUAL_ID:
      printf (">");
      break;
    case JSON_TOKEN_INFERIOR_EQUAL_ID:
      printf ("<");
      break;
    case JSON_TOKEN_EXCLAMATION_ID:
      printf ("!");
      break;
    case JSON_TOKEN_PIPE_ID:
      printf ("|");
      break;
    case JSON_TOKEN_LOGICAL_OR_ID:
      printf ("||");
      break;

    case JSON_TOKEN_NUMBER_ID:
      c_cut_token_string (parser);
      break;
    case JSON_TOKEN_DQUOTE_ID:
      printf ("\"");
      c_cut_c_string(parser,'"');
      printf ("\"");
      break;
    case JSON_TOKEN_SQUOTE_ID:
      printf ("'");
      c_cut_c_string(parser,'\'');
      printf ("'");
      break;
    case JSON_TOKEN_WORD_ID:
      {
	// somehow should be a variable, litterals are strings or numbers...
	if (c_cut_token_string (parser) == NULL)
	  {
	    c_show_info (parser, "INFO", "NON recognized word");
	  }
	break;
      }
    case JSON_TOKEN_COMMENT_ID:
      // disregards comments
      reset_tokenizer_buffer (tokenizer);
      break;
    case JSON_TOKEN_PRAGMA_ID:
      printf ("#");
      c_cut_token_string (parser);
      printf ("\n");
      break;
    case JSON_TOKEN_EOF_ID:
      printf ("EOF id reached\n");
      break;
    case JSON_TOKEN_PLUS_ID:
      printf (" + ");
      break;
    case JSON_TOKEN_INCREMENT_ID:
      printf ("++");
      break;
    case JSON_TOKEN_ADD_ID:
      printf ("+=");
      break;
    case JSON_TOKEN_SUBTRACT_ID:
      printf ("-=");
      break;
    case JSON_TOKEN_DECREMENT_ID:
      printf ("--");
      break;
    default:
      printf ("\n<%i>", token->token);
      // lazzy cut_string...
      c_cut_token_string (parser);
      printf ("</%i>", token->token);
    }
}

struct al_token *
c_parse_next (struct c_parser_ctx *parser)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  struct al_token *token;

  token = c_tokenizer (tokenizer, parser->tokenizer_data);

  // ignore any comment
  while ((token != NULL) && (token->token == JSON_TOKEN_COMMENT_ID))
    {
      c_show_info (parser, "INFO", "ignore comment");
      reset_tokenizer_buffer (tokenizer);
      token = c_tokenizer (tokenizer, parser->tokenizer_data);
    }
  // allow progress checking 
  ++parser->token_count;

  if (token != NULL)
    {
      // printf("//next %i %i , %i\n", parser->last_word, parser->last_token, token->token);
      parser->last_token = token->token;
      parser->last_word = TOKEN_C_NOTWORD_ID;
      if (token->token == JSON_TOKEN_WORD_ID)
	{
	  // will set last_word
	  c_parse_word_token (parser);
	}
      if (token->token == JSON_TOKEN_EOF_ID)
	{
	  c_show_info (parser, "INFO", "EOF reached at parse next");
	}
    }
  else
    {
      printf ("// NULL token creates EOF reached at parse next\n");
      parser->last_token = JSON_TOKEN_EOF_ID;
      parser->last_word = TOKEN_C_NOTWORD_ID;
    }
  return token;
}

// return NULL only if variable was recognized.
struct al_token *
c_parse_variable (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  if (token->token != JSON_TOKEN_WORD_ID)
    {
      return token;
    }
  if (parser->last_word == TOKEN_C_NOMATCH_ID)
    {
      c_cut_token_string (parser);
      c_show_info (parser, "INFO", "variable");
      return NULL;
    }
  else
    {
      printf ("reserved word c_token %i at %i\n", parser->last_word,
	      parser->token_count);
      reset_tokenizer_buffer (tokenizer);
      return token;
    }
}

struct al_token *c_parse_call_definition_parameters (struct c_parser_ctx
						     *parser);


int
c_is_typedef (struct c_parser_ctx *parser)
{

  struct json_ctx *tokenizer = parser->tokenizer;
  struct token_char_buffer *tb = &tokenizer->token_buf;
  char *buffer = tb->buf;
  int length = tb->bufpos;

  if (buffer == NULL)
    {
      todo ("[FATAL] corrupted parser token char buffer NULL");
      return 0;
    }

  if (length == 0)
    {
      printf ("[FATAL] corrupted parser empty char buffer\n");
      return 0;
    }

  struct alhash_datablock key;

  key.type = ALTYPE_OPAQUE;
  key.length = length;
  key.data.ptr = buffer;

  if (strncmp (buffer, "FILE", 4) == 0)
    {
      c_show_info (parser, "HACK", "accept FILE as typedef");
      return 1;
    }

  struct alhash_entry *entry = alhash_get_entry (&parser->alparser.dict, &key);
  if (entry != NULL)
    {
      c_show_info (parser, "INFO",
		   "SAME TOKEN SEEN. for test assume typedef");
      //return 1;
    }

  return 0;
}

/*
will populate type with parsed type
*/
struct al_token *
c_parse_left_type (struct c_parser_ctx *parser,
		   struct c_full_type * type,
		   struct al_token *token,
		   int c_token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  int c_type = -1;
  int named = 0;

  type->dereference = 0;
  type->dict_index = NULL;
  
  c_show_info (parser, "INFO", "parse left type");
  // set when declaration is done during type ( functions names / typedef ... ).
  parser->lhs_variable_data = NULL;

  if (token == NULL)
    {
      token = c_parse_next (parser);
      c_token = parser->last_word;
    }
  while (token != NULL)
    {
      // printf("!!%i::",token->token);
      switch (token->token)
	{
	case JSON_TOKEN_AMPERSAND_ID:
	case JSON_TOKEN_COMMENT_ID:
	  c_print_json_token (parser, token);
	  break;
	case JSON_TOKEN_STAR_ID:
	  c_print_json_token (parser, token);
	  // this is a pointer to type.
	  type->dereference ++;
	  // return NULL; NOPE could still be a function return value
	  break;
	case JSON_TOKEN_OPEN_PARENTHESIS_ID:
	  c_print_json_token (parser, token);
	  c_show_info (parser, "INFO",
		       "expects a function definition '(*' function_name ')'");
	  token = c_parse_next (parser);
	  c_token = parser->last_word;
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_STAR_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_next (parser);
		  c_token = parser->last_word;
		  if (token != NULL)
		    {
		      if (token->token == JSON_TOKEN_WORD_ID)
			{
			  if (c_token == TOKEN_C_NOMATCH_ID)
			    {
			      c_print_json_token (parser, token);
			    }
			  else
			    {
			      printf
				("// [ERROR] reserved word while expecting function name c_type %i\n",
				 c_token);
			      print_c_token (parser, c_token);
			    }
			}
		      token = c_parse_next (parser);
		      c_token = parser->last_word;
		      if (token != NULL)
			{
			  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
			    {
			      c_print_json_token (parser, token);
			      token = c_parse_next (parser);
			      c_token = parser->last_word;
			      if (token != NULL)
				{
				  if (token->token ==
				      JSON_TOKEN_OPEN_PARENTHESIS_ID)
				    {
				      c_print_json_token (parser, token);
				      c_show_info(parser,"INFO"
						  ,"function parameters definition");
				      token =
					c_parse_call_definition_parameters
					(parser);
				      if (token == NULL)
					{
					  token = c_parse_next (parser);
					  c_token = parser->last_word;
					}
				      if (token != NULL)
					{
					  if (token->token ==
					      JSON_TOKEN_CLOSE_PARENTHESIS_ID)
					    {
					      c_print_json_token (parser,
								  token);
					      parser->lhs_variable_data =
						token;
					      return NULL;
					    }
					}
				    }
				}
			    }
			  else
			    {
			      printf
				("// expected ) to close function name/n");
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
	    switch (c_token)
	      {
	      case TOKEN_C_STRUCT_ID:
	      case TOKEN_C_UNION_ID:
	      case TOKEN_C_ENUM_ID:
	      case TOKEN_C_INT_ID:
	      case TOKEN_C_LONG_ID:
	      case TOKEN_C_CHAR_ID:
	      case TOKEN_C_FLOAT_ID:
	      case TOKEN_C_VOID_ID:
		if (c_type == -1)
		  {
		    // recognized a type.
		    print_c_token (parser, c_token);
		    printf (" ");
		    c_type = c_token;
		    type->word_type = c_type;
		    parser->last_type = c_type;
		  }
		else
		  {
		    printf ("[ERROR] conflicting types (1) %i != %i\n",
			    c_type, c_token);
		    return token;
		  }
		break;
	      case TOKEN_C_NOMATCH_ID:
		if ((c_type == TOKEN_C_STRUCT_ID)
		    || (c_type == TOKEN_C_ENUM_ID))
		  {
		    if (named == 0)
		      {
			token = c_parse_variable (parser, token);
			if (c_parser_is_debug (parser))
			  {
			    printf ("// struct and enum types are named.\n");
			  }
			if (token != NULL)
			  {
			    if (c_parser_is_debug (parser))
			      {
				printf ("// non varname %i \n", token->token);
			      }
			    return token;
			  }
			type->dict_index = parser->dict_value;
			++named;
		      }
		    else
		      {
			return token;
		      }
		  }
		else
		  {
		    c_show_info (parser, "INFO", "typedef ???");
		    if (!c_is_typedef (parser))
		      {
			return token;
		      }
		    token = c_parse_variable (parser, token);
		  }
		break;
	      default:
		return token;
	      }
	  }
	  break;
	default:
	  c_create_error(parser,C_STATE_REJECT,"unrecognized token for left type");
	  return token;
	}
      {
	token = c_parse_next (parser);
	c_token = parser->last_word;
      }
    }
  // printf("!!");
  return token;
}


struct al_token *c_parse_define_type (struct c_parser_ctx *parser,
				      struct al_token *token,
				      int within_typedef);

struct c_struct_info *
c_parser_ctx_allocate_c_struct_info(struct c_parser_ctx *parser)  
{
  struct c_struct_info * allocated = NULL;
  if ( parser != NULL )
    {
      todo("fixme hardcoded 1000 structures");
      int max_alloc = 1000;
      if (
	  (parser->allocated_structures == 0 )
	  && ( parser->used_structures == 0 )
	  && ( parser->structure_array == NULL ) )
	{
	  parser->structure_array = calloc(max_alloc,sizeof(struct c_struct_info));
	  parser->allocated_structures = max_alloc;
	}
      if ( parser->used_structures <  max_alloc )
	{
	  allocated = &parser->structure_array[parser->used_structures];
	  parser->used_structures ++;
	}
    }
  return allocated;
}

struct c_declaration_info_list*
c_parser_ctx_allocate_c_declaration_info_list(struct c_parser_ctx *parser)  
{
  return calloc(1,sizeof(struct c_declaration_info_list));
}

// add a new member in struct info, uses parser general allocation table.
int c_struct_info_add_member(
			     struct c_parser_ctx *parser,
			     struct c_struct_info *struct_info,
			     struct c_full_type * type,
			     void * variable
			     )
{
  if ( struct_info != NULL )
    {
      struct c_declaration_info_list* info = c_parser_ctx_allocate_c_declaration_info_list(parser);
      if (info != NULL )
	{
	  struct c_declaration_info_list* last = struct_info->first;

	  // info->info.first ???  type
	  // copy type.
	  memcpy(&info->info.full_type,type,sizeof(info->info.full_type));
	  info->info.dict_index = variable;
	  
	  // add it after next...
	  if ( last == NULL )
	    {
	      struct_info->first = info;
	    }
	  else
	    {
	      // protect against ... coder - ( linked list corruption / loop ).
	      int max = 1000;
	      struct c_declaration_info_list* next = last;	      
	      while ( ( next != NULL ) && ( max > 0 ) )
		{
		  last = next;
		  next = last->next;
		  max --;
		}
	      if ( max > 0 )
		{
		  last->next = info;
		}
	    }
	}
    }
  // todo("");  
  return 0;
}

struct al_token *
c_parse_struct_member (struct c_struct_info *struct_info,
		       struct c_parser_ctx *parser, struct al_token *token, int index)
{
  struct c_full_type type;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  token = c_parse_left_type (parser,&type,token, parser->last_word);
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  printf(" ");
  if (c_parse_variable (parser, token) == NULL)
    {
      c_struct_info_add_member(parser,struct_info,&type,parser->dict_value);
      printf (" // struct member %i\n", index);
      token = c_parse_next (parser);
      if ((token != NULL) && (token->token == JSON_TOKEN_SEMI_COLON_ID))
	{
	  c_print_json_token (parser, token);
	  return NULL;
	}
    }
  else
    {
      printf ("// variable declaration not found\n");
    }
  return token;
}


struct al_token *
eat_json_token (enum json_token_id tokenid, struct c_parser_ctx *parser,
		struct al_token *token)
{
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  if (token != NULL)
    {
      if (token->token == tokenid)
	{
	  c_print_json_token (parser, token);
	  return NULL;
	}
    }
  return token;
}

struct al_token *
eat_semi_colon (struct c_parser_ctx *parser, struct al_token *token)
{
  return eat_json_token (JSON_TOKEN_SEMI_COLON_ID, parser, token);
}


struct al_token *c_parse_rhs (struct c_parser_ctx *parser,
			      struct al_token *token);
struct al_token *c_parse_call_parameters (struct c_parser_ctx *parser,
					  struct al_token *token);
struct al_token *c_parse_block (struct c_parser_ctx *parser,
				struct al_token *token,
				enum c_parser_state state);

struct al_token *c_parse_simple_expression (struct c_parser_ctx *parser,
					    struct al_token *token);


struct al_token *
c_parse_array_definition(struct c_parser_ctx *parser, struct al_token *token)
{
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  else
    {
      if ( token->token != JSON_TOKEN_OPEN_BRACKET_ID )
	{
	  c_create_error(parser,C_STATE_REJECT,"unrecognized token for array definition");
	  return token;
	}
    }
  c_show_info(parser,"INFO","parse array definition");
  if ( token != NULL )
    {
      c_print_json_token (parser, token);
      token = c_parse_simple_expression(parser, NULL);
      token =
	eat_json_token (JSON_TOKEN_CLOSE_BRACKET_ID, parser, token);
      if ( token == NULL )
	{
	  c_show_info(parser,"INFO","parse array definition end");
	}
    }
  return token;
}

struct al_token *
c_parse_lhs (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  while (token != NULL)
    {
      struct c_full_type type;
      token = c_parse_left_type (parser, &type, token, parser->last_word);
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      if (c_parse_variable (parser, token) == NULL)
	{
	  token = c_parse_next (parser);

	  if (token != NULL)
	    {
	      if (token == NULL)
		{
		  token = c_parse_next (parser);
		}
	      if (token->token == JSON_TOKEN_RIGHT_ARROW_ID)
		{
		  c_print_json_token (parser, token);
		}
	      else if (token->token == JSON_TOKEN_DOT_ID)
		{
		  c_print_json_token (parser, token);
		}
	      else if (token->token == JSON_TOKEN_OPEN_BRACKET_ID)
		{
		  c_print_json_token (parser, token);

		  token = c_parse_next (parser);

		  if (token != NULL)
		    {
		      token = c_parse_rhs (parser, token);
		      if (token == NULL)
			{
			  token = c_parse_next (parser);
			}
		      if (token != NULL)
			{
			  if (token->token == JSON_TOKEN_CLOSE_BRACKET_ID)
			    {
			      c_print_json_token (parser, token);
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
      token = c_parse_next (parser);
    }
  return token;
}


struct al_token *
c_parse_litteral (struct c_parser_ctx *parser, struct al_token *token)
{
  // litterals
  if (token->token == JSON_TOKEN_DQUOTE_ID)
    {
      c_print_json_token (parser, token);
      return NULL;
    }
  else if (token->token == JSON_TOKEN_SQUOTE_ID)
    {
      c_print_json_token (parser, token);
      return NULL;
    }
  else if (token->token == JSON_TOKEN_NUMBER_ID)
    {
      c_print_json_token (parser, token);
      return NULL;
    }
  else
    return token;
}

struct al_token *
c_parse_rhs (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  // printf("//start rhs\n");
  while (token != NULL)
    {
      if ((token = c_parse_litteral (parser, token)) == NULL)
	{
	  break;
	}
      else
	{
	  // rhs is a '(' ...')'
	  if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_rhs (parser, NULL);
	      if (token == NULL)
		{
		  token = c_parse_next (parser);
		}
	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		    {
		      c_print_json_token (parser, token);
		      return NULL;
		    }
		}
	      return token;
	    }
	  token = c_parse_lhs (parser, token);
	  // is it a function call ?
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_call_parameters (parser, NULL);
		  if (token == NULL)
		    {
		      token = c_parse_next (parser);
		    }
		  if (token != NULL)
		    {
		      if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
			{
			  c_print_json_token (parser, token);
			  return NULL;
			}
		    }
		  return token;
		}
	      else if (token->token == JSON_TOKEN_PLUS_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_lhs (parser, NULL);
		  return token;
		}
	    }
	  if (token != NULL)
	    {
	      break;
	    }
	}
      token = c_parse_next (parser);
    }
  return token;
}

struct al_token *
c_parse_rhs_semi_colon (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  // token = c_parse_rhs(parser,token);
  token = c_parse_simple_expression (parser, token);

  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  if (token != NULL)
    {
      if (token->token != JSON_TOKEN_SEMI_COLON_ID)
	{
	  c_create_error (parser, C_STATE_ERROR, NULL);
	  return token;
	}
      else
	{
	  c_print_json_token (parser, token);
	  c_show_info (parser, "INFO", "end rhs with semi colon");
	  parser->state = C_STATE_START_ID;
	  return NULL;
	}
    }
  return token;
}

// int a,void b,struct * c,d ...
struct al_token *
c_parse_call_definition_parameters (struct c_parser_ctx *parser)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  struct al_token *token = NULL;
  int i = 0;
  token = c_parse_next (parser);
  while (token != NULL)
    {
      struct c_full_type type;
      token = c_parse_left_type (parser, &type, token, parser->last_word);
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      // name of type.
      if (token != NULL)
	{
	  if (c_parse_variable (parser, token) == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	}
      if (token != NULL)
	{
	  if (token->token != JSON_TOKEN_COMMA_ID)
	    {
	      if (c_parser_is_debug(parser))
		{
		  printf ("// def param %i end token %i A\n", i, token->token);
		}
	      return token;
	    }
	  if (c_parser_is_debug(parser))
	    {
	      printf ("// def param %i\n", i);
	    }
	  ++i;
	  c_print_json_token (parser, token);
	}
      else
	{
	  return token;
	}
      token = c_parse_next (parser);
    }
  printf ("// def param %i end token %i B\n", i, token->token);
  return token;

}

// a,b,c,d ...
struct al_token *
c_parse_call_parameters (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;

  int i = 0;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  if (token != NULL)
    {
      if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
	{
	  c_create_error (parser, C_STATE_REJECT,
			  "empty/void function parameters");
	  return token;
	}
    }
  while (token != NULL)
    {
      token = c_parse_simple_expression (parser, token);
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      if (token != NULL)
	{
	  if (token->token != JSON_TOKEN_COMMA_ID)
	    {
	      if (token->token != JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		{
		  printf ("// call parameters wrong token %i\n",
			  token->token);
		}
	      return token;
	    }
	  parser->state = C_STATE_START_ID;
	  if (c_parser_is_debug (parser))
	    {
	      printf ("// call param %i\n", i);
	    }
	  ++i;
	  c_print_json_token (parser, token);
	}
      else
	{
	  return token;
	}
      token = c_parse_next (parser);
    }
  printf ("// call param %i end\n", i);
  return token;

}

// 0 if not an operator
// else number of elements it operates on
int
c_operator_arity (struct c_parser_ctx *parser, struct al_token *token)
{

  if (token != NULL)
    {
      if ((token->token == JSON_TOKEN_COMPARE_EQUAL_ID)
	  || (token->token == JSON_TOKEN_COMPARE_DIFFERENT_ID)
	  || (token->token == JSON_TOKEN_SUPERIOR_ID)
	  || (token->token == JSON_TOKEN_INFERIOR_ID)
	  || (token->token == JSON_TOKEN_SUPERIOR_EQUAL_ID)
	  || (token->token == JSON_TOKEN_INFERIOR_EQUAL_ID)
	  || (token->token == JSON_TOKEN_LOGICAL_AND_ID)
	  || (token->token == JSON_TOKEN_LOGICAL_OR_ID))
	{
	  return 2;
	}
      // special operator to access members
      if ((token->token == JSON_TOKEN_RIGHT_ARROW_ID)
	  || (token->token == JSON_TOKEN_DOT_ID))
	{
	  return 2;
	}

      if ((token->token == JSON_TOKEN_AMPERSAND_ID)
	  || (token->token == JSON_TOKEN_STAR_ID))
	{
	  return 1;
	}


      if (token->token == JSON_TOKEN_EQUAL_ID)
	{
	  return 2;
	}
      if ((token->token == JSON_TOKEN_INCREMENT_ID)
	  || (token->token == JSON_TOKEN_DECREMENT_ID))
	{
	  return 1;
	}
      
      if ((token->token == JSON_TOKEN_PLUS_ID)
	  || (token->token == JSON_TOKEN_MINUS_ID))
	{
	  return 2;
	}

      if (token->token == JSON_TOKEN_EXCLAMATION_ID)

	{
	  return 1;
	}
    }
  return 0;
}

// a + b + c + d ... TODO :-)
struct al_token *
c_parse_simple_expression (struct c_parser_ctx *parser,
			   struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }

  int operatorisexpected = 0;
  int operatorexpected = 0;

  while (token != NULL)
    {
      if ((token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
	  || (token->token == JSON_TOKEN_CLOSE_BRACKET_ID))
	{
	  if (operatorisexpected == 0)
	    {
	      c_create_error (parser, C_STATE_REJECT,
			      "invalid short close of simple expression");
	    }
	  return token;
	}

      if (token->token == JSON_TOKEN_COMMA_ID)
	{
	  c_create_error (parser, C_STATE_REJECT,
			  "simple expression reject ,");
	  return token;
	}
      if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	{
	  if (operatorisexpected == 0)
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_simple_expression (parser, NULL);
	      token =
		eat_json_token (JSON_TOKEN_CLOSE_PARENTHESIS_ID, parser,
				token);
	      if (token == NULL)
		{
		  operatorexpected = 1;
		  // don't do it since will be done later
		  // token=c_parse_next(parser);
		}
	      else
		{
		  c_create_error (parser, C_STATE_ERROR,
				  "simple expression unbalanced parenthesis");
		  return token;
		}
	    }
	  else
	    {
	      // might be a function if previous was a word...
	      c_print_json_token (parser, token);
	      token = c_parse_call_parameters (parser, NULL);
	      token =
		eat_json_token (JSON_TOKEN_CLOSE_PARENTHESIS_ID, parser,
				token);
	      if (token == NULL)
		{
		  c_show_info (parser, "INFO", "function call end");
		  operatorexpected = 1;
		  // don't do it since will be done later
		  // token=c_parse_next(parser);
		}
	      else
		{
		  c_create_error (parser, C_STATE_ERROR,
				  "simple function call expression unbalanced parenthesis");
		  return token;
		}
	    }
	}
      else if (token->token == JSON_TOKEN_OPEN_BRACKET_ID)
	{
	  if (operatorisexpected == 1)
	    {
	      token=c_parse_array_definition(parser,token);
	      if (token == NULL)
		{
		  operatorexpected = 1;
		  // don't do it since will be done later
		  // token=c_parse_next(parser);
		}
	      else
		{
		  c_create_error (parser, C_STATE_ERROR,
				  "simple expression unbalanced brackets ] missing");
		  return token;
		}
	    }
	  else
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "invalid value after bracket []");
	      return token;
	    }
	}
      else
	{
	  int arity = c_operator_arity (parser, token);
	  if (arity == 0)
	    {
	      // this is not an operator.
	      operatorexpected = 1;
	      if (operatorisexpected == 1)
		{
		  c_create_error (parser, C_STATE_ERROR,
				  "simple expression an operator was expected");
		  return token;
		}
	    }
	  else
	    {
	      // this is an operator
	      operatorexpected = 0;
	      // arity == 2 ex a + b and previous  was already an operator.
	      if ((arity > 1) && (operatorisexpected == 0))
		{
		  // printf("// invalid operator %i of arity %i > 1 used after another operator expected a value at %i\n", token->token, arity, parser->token_count);
		  c_create_error (parser, C_STATE_ERROR,
				  "simple expression operator of arity >1 unexpected");
		  return token;
		}
	    }
	  /*
	  if ( parser->last_word != TOKEN_C_NOMATCH_ID )
	    {
	      print_c_token(parser, parser->last_word);
	    }
	  else
	  */
	    {
	      c_print_json_token (parser, token);
	    }
	}
      token = c_parse_next (parser);
      operatorisexpected = operatorexpected;
    }

  return token;

}

struct al_token *
c_parse_simple_boolean_expression (struct c_parser_ctx *parser,
				   struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }

  if (token != NULL)
    {
      token = eat_json_token (JSON_TOKEN_EXCLAMATION_ID, parser, token);
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      token = c_parse_rhs (parser, token);
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      if ((token->token == JSON_TOKEN_COMPARE_EQUAL_ID)
	  || (token->token == JSON_TOKEN_COMPARE_DIFFERENT_ID)
	  || (token->token == JSON_TOKEN_SUPERIOR_ID)
	  || (token->token == JSON_TOKEN_INFERIOR_ID))
	{
	  c_print_json_token (parser, token);
	  token = c_parse_simple_boolean_expression (parser, NULL);
	}
    }
  return token;

}

// currently repalced by c_parse_simple_expression
// should be logical expression
struct al_token *
c_parse_logical_expression (struct c_parser_ctx *parser,
			    struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;

  printf ("// parse logical expression\n");
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  while (token != NULL)
    {
      if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	{
	  c_print_json_token (parser, token);
	  token = c_parse_logical_expression (parser, NULL);
	  token =
	    eat_json_token (JSON_TOKEN_CLOSE_PARENTHESIS_ID, parser, token);
	  if (token == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	  else
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "logical expression unbalanced parenthesis");
	      return token;
	    }
	}
      else
	{
	  token = c_parse_simple_boolean_expression (parser, token);
	  if (token == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	}
      if (token != NULL)
	{
	  if (token->token == JSON_TOKEN_EQUAL_ID)
	    {
	      // mixing ... ex ((buffer=1)==2)
	      token = c_parse_simple_boolean_expression (parser, NULL);
	    }
	  else
	  if (token->token == JSON_TOKEN_INCREMENT_ID)
	    {
	      // ex i++==3
	      c_print_json_token (parser, token);
	      token = c_parse_next (parser);	      
	    }
	  if ((token->token == JSON_TOKEN_LOGICAL_AND_ID)
	      || (token->token == JSON_TOKEN_LOGICAL_OR_ID))
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_next (parser);
	    }
	  else
	    {
	      if (token->token != JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		{
		  printf ("invalid logical combination token %i at %i",
			  token->token, parser->token_count);
		}
	      return token;
	    }
	}
    }
  return token;
}

struct al_token *
c_parse_enum_member (struct c_parser_ctx *parser,
		     struct c_enum_info *enum_info, int index)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  struct al_token *token = NULL;
  token = c_parse_next (parser);
  if (token == NULL)
    {
      tokenizer->last_token.token = JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }
  if (token->token == JSON_TOKEN_WORD_ID)
    {
      if (index > 0)
	{
	  printf (",\n", index);
	}
      c_print_json_token (parser, token);
      printf (" //=%i\n", index);
    }
  else
    {
      return token;
    }
  token = c_parse_next (parser);
  if (token == NULL)
    {
      printf ("// empty token in parse enum member\n");
      tokenizer->last_token.token = JSON_TOKEN_EOF_ID;
      return &tokenizer->last_token;
    }

  if (token->token == JSON_TOKEN_EQUAL_ID)
    {
      c_print_json_token (parser, token);
      token = c_parse_next (parser);
      if (token != NULL)
	{
	  if (token->token == JSON_TOKEN_NUMBER_ID)
	    {
	      c_print_json_token (parser, token);
	    }
	  else
	    {
	      printf
		("// ERROR enum value expected number value, got token %i",
		 token->token);
	      return token;
	    }
	}
      else
	{
	  // ... fixme in fact too short missing value
	  return NULL;
	}
      token = c_parse_next (parser);
      if (token == NULL)
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


struct al_token *
c_parse_define_type (struct c_parser_ctx *parser, struct al_token *token,
		     int within_typedef)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  struct alhash_datablock * type_name_value;
  struct c_full_type type;
  void * lhs_variable_data;
  
  token = c_parse_left_type (parser, &type, token, parser->last_word);

  // a full parsing of a function definition was done ...
  lhs_variable_data = parser->lhs_variable_data;

  // tentatively get name ...
  type_name_value = parser->dict_value;
  
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }

  if (token != NULL)
    {      
      // struct or enum definition.
      if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
	{
	  printf ("{ // type %i definition. (code line %i)\n", parser->last_type, __LINE__);
	  if (parser->last_type == TOKEN_C_STRUCT_ID)
	    {
	      struct c_struct_info * struct_info = c_parser_ctx_allocate_c_struct_info(parser);
	      if ( struct_info == NULL )
		{
		  fprintf(stderr,"can't allocate memory for structure .\n");
		  exit(1);
		}

	      int index = 0;
	      printf ("// struct definition \n");

	      token = c_parse_next (parser);

	      // name of struct ? lhs_variable_data ?
	      // struct_info->dict_index = lhs_variable_data;
	      struct_info->dict_index = type_name_value;
	      
	      while ((token != NULL)
		     && (token->token != JSON_TOKEN_CLOSE_BRACE_ID))
		{
		  // ??? c_parse_left_type + variable ?
		  token = c_parse_struct_member (struct_info, parser, token, index);
		  ++index;
		  // always move forward ( unless parses same token forever... )
		  // if ( token == NULL )
		  {
		    token = c_parse_next (parser);
		  }
		}

	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_CLOSE_BRACE_ID)
		    {
		      printf ("} // close struct\n");
		      if (within_typedef == 1)
			{
			  return NULL;
			}
		      // struct definition always ends with ;
		      token = eat_semi_colon (parser, NULL);
		      return token;
		    }
		  else
		    {
		      return token;
		    }
		}
	    }
	  else if (parser->last_type == TOKEN_C_ENUM_ID)
	    {
	      struct c_enum_info enum_info;
	      int index = 0;
	      token = c_parse_enum_member (parser, &enum_info, index);
	      while (token == NULL)
		{
		  ++index;
		  token = c_parse_enum_member (parser, &enum_info, index);
		}
	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_CLOSE_BRACE_ID)
		    {
		      printf ("} // close enum\n");
		      if (within_typedef == 1)
			{
			  return NULL;
			}
		      // enum definition always ends with ;
		      token = eat_semi_colon (parser, NULL);
		      return token;
		    }
		  else
		    {
		      return token;
		    }
		}
	    }
	  else
	    {
	      // another type ...
	      printf ("// ANOTHER TYPE ???\n");
	    }
	  return NULL;
	}
    }

  // this declare a variable of this type.
  if (c_parse_variable (parser, token) == NULL)
    {

      c_show_info(parser,"INFO","declaration");
      token = c_parse_next (parser);

      // potential array definition
      token = c_parse_array_definition(parser,token);

      if (token != NULL)
	{
	  if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      c_print_json_token (parser, token);
	      c_show_info(parser,"INFO","function");
	      token = c_parse_call_definition_parameters (parser);
	      token =
		eat_json_token (JSON_TOKEN_CLOSE_PARENTHESIS_ID, parser,
				token);
	      if (token != NULL)
		{
		  return token;
		}
	    }
	}

      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      
      if (token !=NULL)
	{
	  // body
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
		{
		  // block { and } will be printed by c_parse_block
		  token =
		    c_parse_block (parser, token,
				   C_STATE_FUNCTION_DEFINITION_ID);
		  if (token != NULL)
		    {
		      c_show_info (parser, "INFO", "block nested unfinished");
		      return token;
		    }
		  c_show_info (parser, "INFO", "block nested finished");
		  return NULL;
		}
	      else if (token->token == JSON_TOKEN_SEMI_COLON_ID)
		{
		  c_print_json_token (parser, token);
		  c_show_info (parser, "INFO", "no body");
		  printf("\n");
		  return NULL;
		}
	    }

	  if (token == NULL)
	    {
	      token = c_parse_next (parser);
	    }

	  if (token->token == JSON_TOKEN_EQUAL_ID)
	    {
	      c_show_info(parser,"INFO","variable initialization");
	      c_print_json_token (parser, token);
	      token = c_parse_rhs_semi_colon (parser, NULL);
	      return token;
	    }
	}
    }
  else
    {
      if (token->token == JSON_TOKEN_SEMI_COLON_ID)
	{
	  printf ("// forward type declaration\n");
	  return token;
	}
      if (token->token == JSON_TOKEN_CLOSE_BRACE_ID)
	{
	  return token;
	}
      c_show_info(parser,"INFO",
		  "unrecognized declaration expected variable");
      return token;
    }

  return token;

}

// parse statements *; case => excluded.
struct al_token *
c_parse_case (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  int ca = 0;
  parser->state = C_STATE_START_ID;
  if (token == NULL)
    {
      token = c_parse_next (parser);
    }
  while ((token = c_parse_statement (parser, NULL, C_STATE_START_ID)) == NULL)
    {
      ++ca;
      printf ("// within case line %i\n", ca);
    }
  if (token != NULL)
    {
      if (token->token == JSON_TOKEN_WORD_ID)
	{
	  int c_token = parser->last_word;
	  if (c_token == TOKEN_C_CASE_ID)
	    {
	      print_c_token (parser, c_token);
	      return token;
	    }
	  else
	    {
	      printf ("// exit case line %i NOT a case %i \n", ca, c_token);
	    }
	}
    }
  printf ("// exit case line %i\n", ca);
  return token;
}

struct al_token *c_parse_case_statement (struct c_parser_ctx *parser,
					 struct al_token *token,
					 enum c_parser_state level_state);

// parse '{' statements* '}' , returns NULL if block parsing is ok.
// all element within this block are in state state 
struct al_token *
c_parse_block (struct c_parser_ctx *parser, struct al_token *token,
	       enum c_parser_state state)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  c_show_info (parser, "INFO", "block start");
  token = eat_json_token (JSON_TOKEN_OPEN_BRACE_ID, parser, token);
  if (token == NULL)
    {
      ++parser->nested;
      printf ("\n");
      c_show_info (parser, "INFO", "block line");
      // remark token == NULL at start
      int line = 0;
      int check_token_count = parser->token_count;
      int loop_protection = 2;
      while (parser->state != C_STATE_ERROR)
	{
	  check_token_count = parser->token_count;
	  if (token == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_CLOSE_BRACE_ID)
		{
		  printf ("}\n");
		  c_show_info (parser, "INFO", "close block");
		  --parser->nested;
		  return NULL;
		}
	      else if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
		{
		  c_show_info (parser, "INFO", "block within block");
		  // remark loose state, restart ...
		  token = c_parse_block (parser, token, C_STATE_START_ID);
		}
	    }
	  if (c_parser_is_debug (parser))
	    {
	      printf ("// block state %i line %i", state, line);
	    }
	  c_show_info (parser, "INFO", "block line");
	  if (state == C_STATE_SWITCH_ID)
	    {
	      token = c_parse_case_statement (parser, token, state);
	    }
	  else
	    {
	      token = c_parse_statement (parser, token, state);
	    }
	  ++line;
	  if (parser->token_count == check_token_count)
	    {
	      printf ("// block loop parsing risk at %i \n",
		      parser->token_count);
	      --loop_protection;
	      if (loop_protection <= 0)
		{
		  printf ("// block loop protection at %i \n",
			  parser->token_count);
		  break;
		}
	    }
	}
    }
  else
    {
      if (token->token == JSON_TOKEN_SEMI_COLON_ID)
	{
	  printf (";//empty block\n");
	  return NULL;
	}
      if (state == C_STATE_SWITCH_ID)
	{
	  c_create_error (parser, C_STATE_ERROR,
			  "a case block should start with '{'");
	  return token;
	}
      else
	{
	  c_show_info (parser, "INFO", "one line block - unique statement");
	  token = c_parse_statement (parser, token, state);
	}
    }
  if (parser->state != C_STATE_ERROR)
    {
      c_create_error (parser, C_STATE_REJECT, "block end unexpected error");
    }
  return token;
}


// parse after '(' of int a,char,b){...}
struct al_token *
c_parse_function_params (struct c_parser_ctx *parser, struct al_token *token)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  c_show_info (parser, "INFO", "parse func params");
  if (token != NULL)
    {
      c_print_json_token (parser, token);
      printf ("\n");
    }
  // function call or declaration ==> not the same thing...
  if ((token = c_parse_call_parameters (parser, NULL)) == NULL)
    {
      printf ("// parse function params\n");
      token = c_tokenizer (tokenizer, parser->tokenizer_data);
    }
  if (token != NULL)
    {
      parser->state = C_STATE_START_ID;
      if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
	{
	  // function call
	  printf (")");
	  if ((token = c_parse_next (parser)) != NULL)
	    {
	      c_show_info (parser, "INFO", "function call next token");
	      if (token->token == JSON_TOKEN_SEMI_COLON_ID)
		{
		  // function declaration 
		  printf (";\n");
		  return NULL;
		}
	      else
		{
		  // function definition          
		  token =
		    c_parse_block (parser, token,
				   C_STATE_FUNCTION_DEFINITION_ID);
		}
	      return token;
	    }
	  return NULL;
	}
      else
	{
	  // declaration
	  while ((token =
		  c_parse_statement (parser, NULL,
				     C_STATE_START_ID)) == NULL);
	  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
	    {
	      printf (")");
	      return c_parse_block (parser, NULL,
				    C_STATE_FUNCTION_DECLARATION_ID);
	    }
	  else
	    {
	      printf ("[ERROR] a function parameters fails got %i token\n",
		      token->token);
	    }
	}
    }

}

// toplevel statement : statement that is not within a block.
struct al_token *
c_parse_toplevel_statement (struct c_parser_ctx *parser,
			    struct al_token *token, int c_token,
			    enum c_parser_state level_state)
{
  struct json_ctx *tokenizer = parser->tokenizer;
  if (token == NULL)
    {
      token = c_parse_next (parser);
      c_token = parser->last_word;
    }

  if (token == NULL)
    {
      printf ("// empty token in parse toplevel statement\n");
      tokenizer->last_token.token = JSON_TOKEN_EOF_ID;
      parser->state = C_STATE_ERROR;
      return &tokenizer->last_token;
    }

  switch (token->token)
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
      printf
	("// [WARNING] unexpected token %i at start of a toplevel statement\n",
	 token->token);
      parser->state = C_STATE_ERROR;
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_print_json_token (parser, token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_print_json_token (parser, token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf ("EOF id reached");
      parser->state = C_STATE_ERROR;
      return token;
      break;
    default:
      printf ("\n<%i>", token->token);
      // lazzy cut_string...
      c_cut_token_string (parser);
      printf ("</%i>", token->token);
    }

  if (token->token != JSON_TOKEN_WORD_ID)
    {
      printf
	("[ERROR] a new toplelvel statement should start with a word. got token %i \n",
	 token->token);
      return token;
    }

  if (c_token == TOKEN_C_NOMATCH_ID)
    {
      printf ("// a toplevel variable or function name at %i \n",
	      parser->token_count);
      token = c_parse_lhs (parser, token);
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      c_token = parser->last_word;
      if (token != NULL)
	{
	  if ((token->token == JSON_TOKEN_EQUAL_ID)
	      || (token->token == JSON_TOKEN_ADD_ID)
	      || (token->token == JSON_TOKEN_INCREMENT_ID)
	      || (token->token == JSON_TOKEN_DECREMENT_ID))
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_rhs_semi_colon (parser, NULL);
	    }
	  else if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      return c_parse_function_params (parser, token);
	    }
	}
      if (token != NULL)
	{
	  c_create_error (parser, C_STATE_ERROR,
			  "a new statement can not start with unrecognized word");
	}
      return token;
    }
  else if (c_token == TOKEN_C_TYPEDEF_ID)
    {
      // typedef ...
      print_c_token (parser, c_token);
      token = c_parse_define_type (parser, NULL, 1);

      if ((token != NULL) && (parser->lhs_variable_data == NULL))
	{
	  if ((token = c_parse_variable (parser, token)) == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	  else
	    {
	      printf
		("[ERROR] expected name for toplevel typedef  ... {} name expected got token %i \n",
		 token->token);
	      parser->state = C_STATE_ERROR;
	      return token;
	    }
	}

      if ((token != NULL) && (token->token != JSON_TOKEN_SEMI_COLON_ID))
	{
	  printf
	    ("[ERROR] expected ; to close typedef  ... {} name expected got token %i \n",
	     token->token);
	}
      else
	{
	  printf (";\n");
	}
    }
  else
    {
      c_show_info (parser, "INFO", "toplevel default statement");
      token = c_parse_define_type (parser, token, 0);
      if (token == NULL)
	{
	  return NULL;
	}
      else
	{
	  if (token->token != JSON_TOKEN_SEMI_COLON_ID)
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "expected ; to close type {}");
	      return token;
	    };
	}
      printf (";\n");
      return NULL;
    }

  return NULL;
}


// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read ahead was needed.
struct al_token *
c_parse_if_statement (struct c_parser_ctx *parser, struct al_token *token,
		      enum c_parser_state level_state)
{
  if (parser->last_word == TOKEN_C_IF_ID)
    {
      c_show_info (parser, "INFO", "IF START");
      print_c_token(parser, parser->last_word);
      printf (" ");
      token = eat_json_token (JSON_TOKEN_OPEN_PARENTHESIS_ID, parser, NULL);
      if (token == NULL)
	{
	  token = c_parse_simple_expression (parser, NULL);
	  token =
	    eat_json_token (JSON_TOKEN_CLOSE_PARENTHESIS_ID, parser, token);
	  if (token == NULL)
	    {
	      c_show_info (parser, "INFO", "if block start");
	      token = c_parse_block (parser, NULL, C_STATE_START_ID);
	      c_show_info (parser, "INFO", "if block finished");
	    }
	  else
	    {
	      printf ("// wrong expression for if missing ')' token %i",
		      token->token);
	    }
	}
      else
	{
	  printf ("// wrong expression for if missing '(' token %i",
		  token->token);
	}
      c_show_info (parser, "INFO", "IF END");
      parser->state = C_STATE_IF_BLOCK_ID;
      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      if (parser->last_token == JSON_TOKEN_SEMI_COLON_ID)
	{
	  c_show_info (parser, "INFO", "useless ; at end of if");
	  return NULL;
	}
      if (parser->last_word == TOKEN_C_ELSE_ID)
	{
	  if (parser->state == C_STATE_IF_BLOCK_ID)
	    {
	      print_c_token (parser, parser->last_word);
	      parser->state = C_STATE_START_ID;
	      token = c_parse_next (parser);
	      if ((token != NULL) && (parser->last_word == TOKEN_C_IF_ID))
		{
		  return c_parse_if_statement (parser, token,
					       C_STATE_IF_BLOCK_ID);
		}
	      else
		{
		  c_show_info (parser, "INFO", "last else");
		  token = c_parse_block (parser, token, C_STATE_START_ID);
		}
	    }
	  else
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "else statement not following a if block");
	      return token;
	    }
	  return token;
	}
      return token;
    }
  parser->state = C_STATE_ERROR;
  return token;
}

// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read aheadwas needed.
struct al_token *
c_parse_case_statement (struct c_parser_ctx *parser, struct al_token *token,
			enum c_parser_state level_state)
{
  struct json_ctx *tokenizer = parser->tokenizer;

  if (token == NULL)
    {
      token = c_parse_next (parser);
    }

  if (token == NULL)
    {
      printf ("// empty token in parse case statement\n");
      tokenizer->last_token.token = JSON_TOKEN_EOF_ID;
      parser->state = C_STATE_ERROR;
      return &tokenizer->last_token;
    }

  switch (token->token)
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
      parser->state = C_STATE_ERROR;
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_print_json_token (parser, token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_print_json_token (parser, token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf ("EOF id reached");
      parser->state = C_STATE_ERROR;
      return token;
      break;
    default:
      printf ("\n<%i>", token->token);
      // lazzy cut_string...
      c_cut_token_string (parser);
      printf ("</%i>", token->token);
    }

  if (token->token != JSON_TOKEN_WORD_ID)
    {
      if ((token->token == JSON_TOKEN_INCREMENT_ID)
	  || (token->token == JSON_TOKEN_DECREMENT_ID))
	{
	  c_print_json_token (parser, token);
	  token = c_parse_lhs (parser, NULL);
	  if (token == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_SEMI_COLON_ID)
		{
		  c_print_json_token (parser, token);
		  return NULL;
		}
	    }
	  parser->state = C_STATE_ERROR;
	  return token;
	}
      else
	{
	  printf
	    ("[ERROR] a new statement should start with a word. got token %i \n",
	     token->token);
	  parser->state = C_STATE_ERROR;
	  return token;
	}
    }

  if (parser->last_word == TOKEN_C_NOMATCH_ID)
    {
      c_show_info (parser, "INFO", "a variable or function name");
      token = c_parse_lhs (parser, token);

      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      if (token != NULL)
	{
	  if ((token->token == JSON_TOKEN_EQUAL_ID)
	      || (token->token == JSON_TOKEN_ADD_ID))
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_rhs_semi_colon (parser, NULL);
	    }
	  else if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      return c_parse_function_params (parser, token);
	    }
	}
      if (token != NULL)
	{
	  c_create_error (parser, C_STATE_ERROR,
			  "a new statement can not start with unrecognized word");
	}
      return token;
    }
  else
    {
      if (parser->last_word == TOKEN_C_CASE_ID)
	{
	  if (level_state == C_STATE_SWITCH_ID)
	    {
	      print_c_token (parser, parser->last_word);
	      token = c_parse_rhs (parser, NULL);
	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_COLON_ID)
		    {
		      c_print_json_token (parser, token);
		      return NULL;
		    }
		  else
		    {
		      printf ("wrong token for case %i\n", token - token);
		    }
		}
	      parser->state = C_STATE_ERROR;
	      return token;
	    }
	  else
	    {
	      printf
		("[ERROR] case statement not within a switch block (state %i %i)",
		 parser->state, level_state);
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_DEFAULT_ID)
	{
	  if (level_state == C_STATE_SWITCH_ID)
	    {
	      print_c_token (parser, parser->last_word);
	      token = c_parse_next (parser);
	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_COLON_ID)
		    {
		      c_print_json_token (parser, token);
		      return NULL;
		    }
		  else
		    {
		      printf ("wrong token for case %i\n", token - token);
		    }
		}
	      return token;
	    }
	  else
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "default statement not within a switch block");
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_RETURN_ID)
	{
	  print_c_token (parser, parser->last_word);
	  token = c_parse_rhs_semi_colon (parser, NULL);
	  parser->state = C_STATE_START_ID;
	  return token;
	}
      else if (parser->last_word == TOKEN_C_BREAK_ID)
	{
	  print_c_token (parser, parser->last_word);
	  token = eat_semi_colon (parser, NULL);
	  if (token != NULL)
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "; expected after break");
	    }
	  else
	    {
	      parser->state = C_STATE_START_ID;
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_IF_ID)
	{
	  return c_parse_if_statement (parser, token, C_STATE_IF_BLOCK_ID);
	}
      else if (parser->last_word == TOKEN_C_WHILE_ID)
	{
	  print_c_token (parser, parser->last_word);
	  printf (" ");
	  token = c_parse_next (parser);
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_simple_expression (parser, NULL);
		  if (token == NULL)
		    {
		      token = c_parse_next (parser);
		    }
		  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		    {
		      c_print_json_token (parser, token);
		      printf ("// parse while block at %i\n",
			      parser->token_count);
		      token = c_parse_block (parser, NULL, C_STATE_START_ID);
		    }
		}
	    }
	  if (token != NULL)
	    {
	      parser->state = C_STATE_ERROR;
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_SWITCH_ID)
	{
	  parser->state = C_STATE_ERROR;
	  printf ("// [ERROR] switch in switch\n");
	  return token;
	}
      else
	{
	  // non struct,enum,if,while or switch   
	  token =
	    c_parse_toplevel_statement (parser, token, parser->last_word,
					C_STATE_START_ID);
	  return token;
	}
    }
  return NULL;
}

// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read ahead was needed.
struct al_token *
c_parse_switch_statement (struct c_parser_ctx *parser, struct al_token *token,
			  enum c_parser_state level_state)
{
  if (parser->last_word == TOKEN_C_SWITCH_ID)
    {
      print_c_token (parser, parser->last_word);
      printf (" ");
      token = c_parse_next (parser);
      if (token != NULL)
	{
	  if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_rhs (parser, NULL);
	      if (token == NULL)
		{
		  token = c_parse_next (parser);
		}
	      if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_block (parser, NULL, C_STATE_SWITCH_ID);
		}
	    }
	}
      if (token != NULL)
	{
	  parser->state = C_STATE_ERROR;
	}
      return token;
    }
  else
    {
      // non struct,enum,if,while or switch       
      token =
	c_parse_toplevel_statement (parser, token, parser->last_word,
				    C_STATE_START_ID);
      return token;
    }
}


// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read ahead was needed.
struct al_token *
c_parse_for_statement (struct c_parser_ctx *parser, struct al_token *token,
			  enum c_parser_state level_state)
{
  if (parser->last_word == TOKEN_C_FOR_ID)
    {
      print_c_token(parser, parser->last_word);
      printf (" ");
      token = c_parse_next (parser);
      if (token != NULL)
	{
	  if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      c_print_json_token (parser, token);
	      c_show_info(parser,"INFO","parse init part of for");
	      token = c_parse_define_type (parser, token, 0); // init
	      c_show_info(parser,"INFO","parse termination check part of for");
	      if ( token == NULL )
		{
		  token = c_parse_rhs_semi_colon(parser, NULL); // test
		}
	      c_show_info(parser,"INFO","parse loop part of for");
	      if ( token == NULL )
		{
		  token = c_parse_define_type (parser, token, 0); // loop increment
		}	      
	      if (token == NULL)
		{
		  token = c_parse_next (parser);
		}
	      if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_block (parser, NULL, C_STATE_START_ID);
		}
	    }
	}
      if (token != NULL)
	{
	  parser->state = C_STATE_ERROR;
	}
      return token;
    }
  else
    {
      // non struct,enum,if,while or switch       
      token =
	c_parse_toplevel_statement (parser, token, parser->last_word,
				    C_STATE_START_ID);
      return token;
    }
}


// set parser->state to C_STATE_ERROR if parsing failed.
// returns token that causes non parsing or NULL if no read aheadwas needed.
struct al_token *
c_parse_statement (struct c_parser_ctx *parser, struct al_token *token,
		   enum c_parser_state level_state)
{
  struct json_ctx *tokenizer = parser->tokenizer;

  if (token == NULL)
    {
      token = c_parse_next (parser);
    }

  if (token == NULL)
    {
      printf ("// empty token in parse statement\n");
      tokenizer->last_token.token = JSON_TOKEN_EOF_ID;
      parser->state = C_STATE_ERROR;
      return &tokenizer->last_token;
    }

  switch (token->token)
    {
    case JSON_TOKEN_WORD_ID:
    case JSON_TOKEN_OPEN_BRACE_ID:
    case JSON_TOKEN_INCREMENT_ID:
    case JSON_TOKEN_DECREMENT_ID:
      break;

    case JSON_TOKEN_OPEN_PARENTHESIS_ID:
    case JSON_TOKEN_CLOSE_PARENTHESIS_ID:
    case JSON_TOKEN_OPEN_BRACKET_ID:
    case JSON_TOKEN_CLOSE_BRACKET_ID:
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
      // rejected token, can be reparsed for another purpose
      c_create_error (parser, C_STATE_REJECT,
		      "unexpected token at start of a statement");
      return token;
      break;
    case JSON_TOKEN_COMMENT_ID:
      c_print_json_token (parser, token);
      return NULL;
    case JSON_TOKEN_PRAGMA_ID:
      c_print_json_token (parser, token);
      return NULL;
    case JSON_TOKEN_EOF_ID:
      printf ("EOF id reached");
      parser->state = C_STATE_ERROR;
      return token;
      break;
    default:
      printf ("\n<%i>", token->token);
      // lazzy cut_string...
      c_cut_token_string (parser);
      printf ("</%i>", token->token);
    }

  if (token->token != JSON_TOKEN_WORD_ID)
    {
      if (token->token == JSON_TOKEN_OPEN_BRACE_ID)
	{
	  return c_parse_block (parser, token, C_STATE_START_ID);
	}
      else
	if ((token->token == JSON_TOKEN_INCREMENT_ID)
	    || (token->token == JSON_TOKEN_DECREMENT_ID))
	{
	  c_print_json_token (parser, token);
	  token = c_parse_lhs (parser, NULL);
	  if (token == NULL)
	    {
	      token = c_parse_next (parser);
	    }
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_SEMI_COLON_ID)
		{
		  c_print_json_token (parser, token);
		  return NULL;
		}
	    }
	  parser->state = C_STATE_ERROR;
	  return token;
	}
      else
	{
	  printf
	    ("[ERROR] a new statement should start with a word. got token %i \n",
	     token->token);
	  parser->state = C_STATE_ERROR;
	  return token;
	}
    }

  if (parser->last_word == TOKEN_C_NOMATCH_ID)
    {
      c_show_info (parser, "INFO", "statement a variable or function name");
      token = c_parse_lhs (parser, token);

      if (token == NULL)
	{
	  token = c_parse_next (parser);
	}
      if (token != NULL)
	{
	  if ((token->token == JSON_TOKEN_EQUAL_ID)
	      || (token->token == JSON_TOKEN_ADD_ID))
	    {
	      c_print_json_token (parser, token);
	      token = c_parse_rhs_semi_colon (parser, NULL);
	    }
	  else if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
	    {
	      return c_parse_function_params (parser, token);
	    }
	}
      if (token != NULL)
	{
	  printf
	    ("[ERROR] a new statement can not start with unrecognized word %i at %i\n",
	     token->token, parser->token_count);
	  parser->state = C_STATE_ERROR;
	}
      return token;
    }
  else
    {
      if (parser->last_word == TOKEN_C_CASE_ID)
	{
	  if (level_state = C_STATE_SWITCH_ID)
	    {
	      print_c_token (parser, parser->last_word);
	      token = c_parse_rhs (parser, NULL);
	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_COLON_ID)
		    {
		      c_print_json_token (parser, token);
		      return NULL;
		    }
		  else
		    {
		      printf ("wrong token for case %i\n", token - token);
		    }
		}
	      parser->state = C_STATE_ERROR;
	      return token;
	    }
	  else
	    {
	      printf
		("[ERROR] case statement not within a switch block (state %i %i)",
		 parser->state, level_state);
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_DEFAULT_ID)
	{
	  if (level_state = C_STATE_SWITCH_ID)
	    {
	      print_c_token (parser, parser->last_word);
	      token = c_parse_next (parser);
	      if (token != NULL)
		{
		  if (token->token == JSON_TOKEN_COLON_ID)
		    {
		      c_print_json_token (parser, token);
		      return NULL;
		    }
		  else
		    {
		      printf ("wrong token for case %i\n", token - token);
		    }
		}
	      return token;
	    }
	  else
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "default statement not within a switch block");
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_RETURN_ID)
	{
	  print_c_token (parser, parser->last_word);
	  token = c_parse_rhs_semi_colon (parser, NULL);
	  parser->state = C_STATE_START_ID;
	  return token;
	}
      else
	if ((parser->last_word == TOKEN_C_BREAK_ID)
	    || (parser->last_word == TOKEN_C_CONTINUE_ID))
	{
	  print_c_token (parser, parser->last_word);
	  token = eat_semi_colon (parser, NULL);
	  if (token != NULL)
	    {
	      c_create_error (parser, C_STATE_ERROR,
			      "expected ; after break");
	    }
	  else
	    {
	      parser->state = C_STATE_START_ID;
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_IF_ID)
	{
	  return c_parse_if_statement (parser, token, C_STATE_IF_BLOCK_ID);
	}
      else if (parser->last_word == TOKEN_C_WHILE_ID)
	{
	  print_c_token (parser, parser->last_word);
	  printf (" ");
	  token = c_parse_next (parser);
	  if (token != NULL)
	    {
	      if (token->token == JSON_TOKEN_OPEN_PARENTHESIS_ID)
		{
		  c_print_json_token (parser, token);
		  token = c_parse_simple_expression (parser, NULL);
		  if (token == NULL)
		    {
		      token = c_parse_next (parser);
		    }
		  if (token->token == JSON_TOKEN_CLOSE_PARENTHESIS_ID)
		    {
		      c_print_json_token (parser, token);
		      token = c_parse_block (parser, NULL, C_STATE_START_ID);
		    }
		}
	    }
	  if (token != NULL)
	    {
	      if (parser->state == C_STATE_ERROR)
		{
		  c_create_error (parser, C_STATE_ERROR,
				  "while unexpected end");
		}
	    }
	  return token;
	}
      else if (parser->last_word == TOKEN_C_SWITCH_ID)
	{
	  return c_parse_switch_statement (parser, token, C_STATE_START_ID);
	}
      else if (parser->last_word == TOKEN_C_FOR_ID)
	{
	  return c_parse_for_statement (parser, token, C_STATE_START_ID);
	}
      else
	{
	  // non struct,enum,if,while or switch   
	  token =
	    c_parse_toplevel_statement (parser, token, parser->last_word,
					C_STATE_START_ID);
	  return token;
	}
    }
  return NULL;
}


int
init_c_parser (struct c_parser_ctx *parser, struct json_ctx *tokenizer,
	       void *data)
{
  bzero (parser, sizeof (*parser));
  parser->state = C_STATE_START_ID;
  parser->tokenizer = tokenizer;
  parser->tokenizer_data = data;
  parser->last_type = -1;
  parser->last_word = -1;

  struct alparser_ctx * alparser = &parser->alparser;
  todo ("support a growable hashtable. here limited to 1024 words");
  todo ("support a growable word buffer. here limited to 10240 characters");
  alparser_init(alparser,1024,10240);
  
  // no debugging by default
  parser->flags = 0;
  return 1;
}

void generate_aljson_stub( struct c_parser_ctx * parser)
{
  for (int i = 0; i< parser->used_structures; i++)
    {
      int max = 1000;
      char * varname="outstructp";
      // todo
      struct alhash_datablock * vartype;
      struct alhash_datablock * datablock = (struct alhash_datablock *) parser->structure_array[i].dict_index ;
      vartype = NULL;

      // generate read side : json_c_xxxx_from_json_auto
      
      printf("int json_c_%.*s_from_json_auto(",datablock->length,datablock->data);
      printf("struct %.*s * %s, ",datablock->length,datablock->data,varname);
      printf("struct json_object * json_object)\n{\n");
      struct c_declaration_info_list * next = parser->structure_array[i].first;
      while ((next != NULL)&&(max>0))
	{
	  datablock = (struct alhash_datablock *) next->info.dict_index;
	  if ( datablock != NULL )
	    {
	      // better than nothing, still does not recognize arrays or pointers
	      switch(next->info.full_type.word_type)
		{
		case TOKEN_C_INT_ID:
		case TOKEN_C_LONG_ID:
		  printf("AL_GET_JSON_INT_WITH_NAME(%s,%.*s,json_object);\n",varname,datablock->length,datablock->data);
		  break;
			      
		case TOKEN_C_CHAR_ID:
		  printf("AL_GET_JSON_STRING_WITH_NAME(%s,%.*s,json_object);\n",varname,datablock->length,datablock->data);
		  break;

		case TOKEN_C_STRUCT_ID:
		  vartype = next->info.full_type.dict_index;
		  if (vartype != NULL)
		    {
		      if ( next->info.full_type.dereference == 0)
			{				    
			  printf("AL_GET_JSON_STRUCT(%.*s,%s,%.*s,json_object,1,WITH_NAME);\n",vartype->length,vartype->data,varname,datablock->length,datablock->data);
			}
		      else
			{
			  printf("AL_GET_JSON_STRUCT_POINTER(%.*s,%s,%.*s,json_object,1,WITH_NAME);\n",vartype->length,vartype->data,varname,datablock->length,datablock->data);
			}
		    }
		  else
		    {
		      printf("// unknwon struct name for %.*s \n", datablock->length,datablock->data);
		    }
		  break;
		default:
		  printf("// unsupported type %i.\n", next->info.full_type.word_type);
		  break;
		}

	    }
	  else
	    {
	      printf("//.\n");
	    }
	  next = next->next;
	  --max;
	}
      printf("return 1;}\n");
    }

}


int
main (int argc, char **argv)
{
  struct c_parser_ctx parser;
  struct json_import_context_data importer;
  struct json_ctx tokenizer;
  struct inputstream main_inputstream;
  struct inputstream * inputstream = NULL;
  FILE *file = NULL;

  alhash_set_debug(255);

  struct al_options * options = al_create_options(argc,argv);
    
  bzero (&tokenizer, sizeof (tokenizer));
  bzero (&importer, sizeof (importer));

  init_c_parser(&parser, &tokenizer, &importer);

  struct alhash_datablock * debugdata = al_option_get(options,"debug");
  if ( debugdata != NULL )
    {
      // set debugging for parsing
      ALC_SET_FLAG(parser.flags,ALCPARSER_DEBUG);
      // alhash_set_debug(1);
    }

  struct alhash_datablock * infiledata = al_option_get(options,"infile");
  if ( infiledata != NULL )
    {
      printf("file to parse '%.*s'\n",infiledata->length,infiledata->data);
      file = fopen((char *)infiledata->data.ptr, "r");
      if ( file == NULL )
	{
	  fprintf(stderr,"[ERROR] fail to open '%s'\n",infiledata->data);
	}
      else
	{
	  inputstream_init(&main_inputstream, fileno (file));
	  inputstream=&main_inputstream;
	}
    }
  else
    {
      fprintf(stderr,"[ERROR] missing argument infile= file to parse.");
    }

  struct alhash_datablock * outformdata = al_option_get(options,"outform");
  if (outformdata != NULL)
    {
      printf("outform '%.*s'\n",outformdata->length,outformdata->data);
      todo("support outform");
    }

  if (inputstream != NULL)
    {
      json_import_context_initialize (&tokenizer);
      importer.inputstream = inputstream;

      //json_ctx_set_debug(&tokenizer,TOKENIZER_DEBUG_ADD);
      //json_ctx_set_debug(&tokenizer,TOKENIZER_DEBUG_PUSHBACK);
      json_ctx_set_debug (&tokenizer, 0);

      struct al_token *token = NULL;
      parser.token_count = 0;
      int check_token_count = parser.token_count;
      while (parser.state != C_STATE_ERROR)
	{
	  check_token_count = parser.token_count;
	  token =
	    c_parse_toplevel_statement (&parser, NULL, TOKEN_C_NOMATCH_ID,
					C_STATE_START_ID);
	  if (token != NULL)
	    {
	      printf ("// non NULL token at toplevel parsing\n");
	      break;
	    }
	  if (parser.token_count == check_token_count)
	    {
	      printf ("// risk of parsing loop\n");
	      break;
	    }
	}

      if (token != NULL)
	{
	  if (token->token != JSON_TOKEN_EOF_ID)
	    {
	      c_show_info (&parser, "INFO", "expected eof");
	    }
	}
      else
	{
	  c_show_info (&parser, "INFO", "expected eof got NULL");
	}

      // don't care yet of what outform... 
      if (outformdata != NULL)
	{
	  // outform=aljson
	  if ( parser.used_structures != 0 )
	    {
	      for (int i = 0; i< parser.used_structures; i++)
		{
		  // todo
		  printf("// structure %i\n" ,i);
		  int max = 1000;
		  struct alhash_datablock * datablock = (struct alhash_datablock *) parser.structure_array[i].dict_index ;
		  // thanks to this format ... print non NULL terminated string
		  printf("{\"%.*s\":{",datablock->length,datablock->data);
		  struct c_declaration_info_list * next = parser.structure_array[i].first;
		  while ((next != NULL)&&(max>0))
		    {
		      datablock = (struct alhash_datablock *) next->info.dict_index;
		      if ( datablock != NULL )
			{
			  printf("\"%.*s\":0,\n",datablock->length,datablock->data);
			}
		      else
			{
			  printf(".\n");
			}
		      next = next->next;
		      --max;
		    }
		  printf("}\n");
		}
	    }

	  // outform=aljson_stub
	  if ( parser.used_structures != 0 )
	    {
	      generate_aljson_stub(&parser);
	    }

	}
      fflush (stdout);

      al_options_release(options);
      options=NULL;

    }
  else
    {
      usage ();
    }
}
