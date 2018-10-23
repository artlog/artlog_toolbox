#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <assert.h>
#include "aljson_parser.h"

#define TOKEN_BUFSIZE_MIN 256
#define TOKEN_BUFSIZE_WARNING 8192
#define TOKEN_BUFSIZE_MAX 128000

TOKEN_DEFINE_TOKENIZER(DQUOTE,'"')
TOKEN_DEFINE_TOKENIZER(SQUOTE,'\'')
TOKEN_DEFINE_TOKENIZER(VARIABLE,'?')

ALDEBUG_DEFINE_FUNCTIONS(struct json_ctx, json_ctx,debug_level)

/**
 return a json_object with a type '0' and json_string set to number if parsing is ok else return NULL
*/
struct al_token * tokenizer_NUMBER(struct json_ctx * ctx, char first, void * data)
{
  int state = parse_number_level(ctx, first, data);
  if ( state == ALJSON_NPSTATE_COMPLETE )
    {
      JSON_TOKEN(NUMBER);
    }
  return (struct al_token *) NULL;
}

struct al_token * tokenizer_CONSTANT(struct json_ctx * ctx, char first, void * data)
{
  switch(first)
    {
    case 't':  
      if ( json_ctx_consume(ctx,data,"rue") )
	{
	  ctx->add_char(ctx,first,first);
	  flush_char_buffer(&ctx->token_buf);
	  JSON_TOKEN(TRUE);
	}
      break;
    case 'f':
	if (json_ctx_consume(ctx,data,"alse") )
	  {
	    ctx->add_char(ctx,first,first);
	    flush_char_buffer(&ctx->token_buf);
	    JSON_TOKEN(FALSE);
	  }
      break;
    case 'n':
      if ( json_ctx_consume(ctx,data,"ull") )
	{
	  ctx->add_char(ctx,first,first);
	  flush_char_buffer(&ctx->token_buf);
	  JSON_TOKEN(NULL);
	}
      break;
    default:
      ctx->pushback_char(ctx,data,first);
      return NULL;
    }
  ctx->pushback_char(ctx,data,first);
  return NULL;
}

/** Initialize json_context **/
void json_context_initialize(struct json_ctx *json_context, get_next_char next_char)
{
  bzero(json_context,sizeof(*json_context));
  json_context->next_char=next_char;
  json_context->pushback_char=pushback_char;
  json_context->add_char=add_char;
}

void pushback_char(struct json_ctx *ctx, void *data, char pushback)
{
  /*
  struct json_string * str = (struct json_string *) data;
  // should not pushback something else than previous char. programming error
  assert( (ctx->pos>0) && (pushback == str->chars[ctx->pos-1]));
  */
  ctx->pos--;
  ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
    {
      aldebug_printf(NULL,"[DEBUG] (pushback %c)\n",pushback);      
    }
}

// add a char within token char buffer that will be flush at cut_string_object or flush_cahr_buffer
int token_char_buffer_add_char(struct token_char_buffer * ctx, char token, char c)
{
  int bufsize=TOKEN_BUFSIZE_MIN;
   if (ctx->buf == NULL)
    {
      ctx->buf=calloc(1,bufsize);
      //ctx->buf[bufsize-1]=0;
      ctx->bufpos=0;
      ctx->bufsize=bufsize;
    }
  if (ctx->bufpos+1>=ctx->bufsize)
    {
      bufsize=ctx->bufsize + ctx->bufsize / 2;
      if ( bufsize > TOKEN_BUFSIZE_MAX )
	{
	  aldebug_printf(NULL,"[FATAL] huge memory consumption for a token %i > %i", bufsize, TOKEN_BUFSIZE_MAX);
	  exit(0);
	}
      if ( bufsize > TOKEN_BUFSIZE_WARNING )
	{
	  aldebug_printf(NULL,"[WARNING] huge memory consumption for a token %i > %i", bufsize, TOKEN_BUFSIZE_WARNING);
	}
      char * newbuf=realloc(ctx->buf,bufsize);
      if (newbuf != NULL)
	{
	  //done by realloc
	  //memcpy(newbuf,ctx->buf,ctx->bufsize);
	  //free(ctx->buf);
	  ctx->buf[bufsize-1]=0;
	  ctx->bufsize=bufsize; 
	  ctx->buf=newbuf;
	}
      else
	{
	  memory_shortage(ctx);
	}
    }
  ctx->buf[ctx->bufpos++]=c;
  return 0;
}

// keep a growable buffer in ctx, grow it as needed
int add_char(struct json_ctx * ctx, char token, char c)
{
  if ( json_ctx_is_debug(ctx,TOKENIZER_DEBUG_ADD) )
    {
      printf("%c", c);
    }
  return token_char_buffer_add_char(&ctx->token_buf,token,c);
}

void debug_tag(struct json_ctx *ctx,char c)
{
  ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
    {
      printf("%c",c);
    }
}

void flush_char_buffer(struct token_char_buffer * ctx)
{
  if (ctx->buf != NULL )
    {
      free(ctx->buf);
      ctx->buf=NULL;
      ctx->bufpos=0;
      ctx->bufsize=0;
    }
}

int json_ctx_consume(struct json_ctx * ctx, void * data, char * str)
{
  int index = 0;
  char c = ctx->next_char(ctx,data);
  while ( (c != 0 ) && (str[index] != 0) )
    {
      if ( c != str[index] )
	{
	  return 0;
	}
      ctx->add_char(ctx,c,c);
      ++ index;
      if (str[index] == 0)
	{
	  break;
	}      
      c =ctx->next_char(ctx,data);
    }
  return (str[index] == 0);
}


/**
 return internal parsing state. ALJSON_NPSTATE_COMPLETE means parsing did find a number.
*/
enum aljson_number_parser_state parse_number_level(struct json_ctx * ctx, char first, void * data)
{
  enum aljson_number_parser_state state = ALJSON_NPSTATE_INIT;
  char c = first;
  while ( state >= ALJSON_NPSTATE_INIT )
    {
      // fixme was > 3 now i s 1
      ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
	{
	  printf("(number state %i)",state);
	}
      switch(state)
	{
	case ALJSON_NPSTATE_INIT:
	  if ( c == '-' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state = ALJSON_NPSTATE_POSITIVE;
	  break;
	case ALJSON_NPSTATE_POSITIVE:
	  if ( c == '0' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = ALJSON_NPSTATE_COMA;
	    }
	  else if ( ( c > '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = ALJSON_NPSTATE_INTEGER;	      
	    }
	  else
	    {
	      state = ALJSON_NPSTATE_ERROR;
	    }
	  break;
	case  ALJSON_NPSTATE_INTEGER:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=ALJSON_NPSTATE_COMA;
	  break;
	case ALJSON_NPSTATE_COMA:
	  if ( c == '.' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = ALJSON_NPSTATE_FLOAT;
	    }
	  else
	    {
	      state = ALJSON_NPSTATE_EXP;
	    }
	  break;
	case ALJSON_NPSTATE_FLOAT:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state = ALJSON_NPSTATE_EXP;
	  break;
	case ALJSON_NPSTATE_EXP:
	  if (( c=='e') || (c =='E'))
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = ALJSON_NPSTATE_EXP2;
	    }
	  else
	    {
	      state = ALJSON_NPSTATE_COMPLETING;
	    }
	  break;
	  // there is an exposant
	case ALJSON_NPSTATE_EXP2:
	  if (( c=='+') || (c =='-'))
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=ALJSON_NPSTATE_EXP3;
	  // no break on purpose
	case ALJSON_NPSTATE_EXP3:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=ALJSON_NPSTATE_COMPLETING;
	  // no break on purpose
	case ALJSON_NPSTATE_COMPLETING:
	  // this char is NOT part of our number
	  ctx->pushback_char(ctx,data,c);
	  ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
	    {
	      printf("(pushback end of number %c)",c);
	      char d = ctx->next_char(ctx,data);
	      if ( d != c )
		{
		  aldebug_printf(NULL,"(pushback end of number FAILS '%c'!='%c')\n",c, d);
		}
	      ctx->pushback_char(ctx,data,c);
	    }
	  state = ALJSON_NPSTATE_COMPLETE;
	  return state;
	default:
	  state = ALJSON_NPSTATE_ERROR;
	  // this is an error
	}
    }
  return state;
}


int parse_until_escaped_level(struct json_ctx * ctx, void * data, char stop, char escape)
{
  char c = ctx->next_char(ctx, data);
  while ( c != 0)
    {
      if ( c == stop )
	{
	  return 1;
	}
      else if ( c == escape )
	{
	  c=ctx->next_char(ctx, data);
	  if ( c != 0 ) ctx->add_char(ctx,stop, c);
	}
      else {
	ctx->add_char(ctx,stop, c);
      }
      c=ctx->next_char(ctx, data);
    }
  return 0;
}


// find the next token
struct al_token * json_tokenizer(struct json_ctx * ctx, void * data)
{
  char c = ctx->next_char(ctx, data);

  // main loop char by char, delegate to sub parser in many cases.
  while (c != 0)  {
    // fixme debug_level was > 2 no test with 1
    ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
      {
	switch(c)
	  {
	  case '{':
	  case '}':
	  case '[':
	  case ']':
	  case '"':
	  case '\'':
	  case ',':
	  case ':':
	  case '?':
	    printf("%c[%x]\n",c,c);
	    /*
	    if (parent != NULL)
	      {
                FIXME need a print ctx else will crash
		aljson_output(ctx,parent,NULL);
	      }
	    */
	    puts("\n-------");
	    break;
	  default:
	    printf("%c",c);
	  }
      }

    // ignored characters.
    switch(c)
      {
      case ' ':
      case 0x0a: // lf
      case 0x0c: // cr
      case 9: // tab
	// ignore
	ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
	  {
	    if ( FLAG_IS_SET(ctx->internal_flags,JSON_FLAG_IGNORE) )
	      {
		printf("%c", c);	
	      }
	    else
	      {
		// start to ignore ...
		printf("<ignore %c", c);
	      }
	  }
	ctx->internal_flags |= JSON_FLAG_IGNORE;
	break;
      default:
	ALDEBUG_IF_DEBUG(ctx,json_ctx,debug_level)
	  {
	    // we were ignoring and now will we stop ...
	    if ( FLAG_IS_SET(ctx->internal_flags,JSON_FLAG_IGNORE) )
	      {
		aldebug_printf(NULL,"ignore>");
	      }
	  }
	ctx->internal_flags &= !JSON_FLAG_IGNORE;
      };

    if ( ! FLAG_IS_SET(ctx->internal_flags,JSON_FLAG_IGNORE) )
      {
	switch(c)
	  {
	  case '{':
	    JSON_TOKEN(OPEN_PARENTHESIS);
	    break;
	  case '}':
	    JSON_TOKEN(CLOSE_PARENTHESIS);
	    break;
	  case '[':
	    JSON_TOKEN(OPEN_BRACKET);
	    break;
	  case ']':
	    JSON_TOKEN(CLOSE_BRACKET);
	    break;
	  case '"':
	    return tokenizer_DQUOTE(ctx,data);
	    // else should be handled by ':', ',' or '}' or ']' ie any close.
	    break;
	  case ',':
	    JSON_TOKEN(COMMA);
	    break;
	  case '?':
	    return tokenizer_VARIABLE(ctx,data);
	    break;
	  case ':':
	    JSON_TOKEN(COLON);
	    break;
	  case '\'':
	    return tokenizer_SQUOTE(ctx,data);
	    break;
	  case '-':
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    return tokenizer_NUMBER(ctx,c,data);
	    break;
	  case 'f': // false
	  case 't': // true
	  case 'n': // null
	    return tokenizer_CONSTANT(ctx,c,data);
	    break;
	  case 0:
	    // force failure programming error
	    assert(c!=0);
	    break;
	  default: // unexpected char ?
	    // PUSHBACK ? TO CHECK
	    aldebug_printf(NULL,"[ERROR] unexpected char %c\n,",c);
	    JSON_TOKEN(EOF);
	    // ctx->pushback_char(ctx,data,c);
	  }
      }
    
    c=ctx->next_char(ctx, data);
  }
  
  JSON_TOKEN(EOF);
}
