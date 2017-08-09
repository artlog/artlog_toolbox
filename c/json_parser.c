#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include <assert.h>
#include "alcommon.h"
#include "json_parser.h"

#define TOKEN_BUFSIZE_MIN 256
#define TOKEN_BUFSIZE_WARNING 8192
#define TOKEN_BUFSIZE_MAX 128000

TOKEN_DEFINE_TOKENIZER(DQUOTE,'"')
TOKEN_DEFINE_TOKENIZER(SQUOTE,'\'')
TOKEN_DEFINE_TOKENIZER(VARIABLE,'?')

/**
 return a json_object with a type '0' and json_string set to number if parsing is ok else return NULL
*/
struct al_token * tokenizer_NUMBER(struct json_ctx * ctx, char first, void * data)
{
  int state = parse_number_level(ctx, first, data);
  if ( state == 9 )
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
  if ( json_context_get_debug(ctx) )
    {
      printf("(pushback %c)",pushback);      
    }
}

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
	  fprintf(stderr,"[FATAL] huge memory consumption for a token %i > %i", bufsize, TOKEN_BUFSIZE_MAX);
	  exit(0);
	}
      if ( bufsize > TOKEN_BUFSIZE_WARNING )
	{
	  fprintf(stderr,"[WARNING] huge memory consumption for a token %i > %i", bufsize, TOKEN_BUFSIZE_WARNING);
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
  if ( FLAG_IS_SET(json_context_get_debug(ctx),TOKENIZER_DEBUG_ADD) )
    {
      printf("%c", c);
    }
  return token_char_buffer_add_char(&ctx->token_buf,token,c);
}

void debug_tag(struct json_ctx *ctx,char c)
{
  if (json_context_get_debug(ctx))
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

int json_context_get_debug(struct json_ctx * ctx)
{
  if ( ctx != NULL )
    {
      return ctx->debug_level;
    }
  else
    {
      return 0;
    }
}
			   
int json_ctx_set_debug(struct json_ctx * ctx, int debug)
{
  if ( ctx != NULL )
    {
      int previous=ctx->debug_level;
      ctx->debug_level = debug;
      return previous;
    }
  else
    {
      return 0;
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
 return internal parsing state. 9 means parsing did find a number.
*/
int parse_number_level(struct json_ctx * ctx, char first, void * data)
{
  int state = 0;
  char c = first;
  while ( state >= 0 )
    {
      if ( json_context_get_debug(ctx) > 3 )
	{
	  printf("(number state %i)",state);
	}
      switch(state)
	{
	case 0:
	  if ( c == '-' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state = 1;
	  break;
	case 1:
	  if ( c == '0' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 3;
	    }
	  else if ( ( c > '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 2;	      
	    }
	  else
	    {
	      state = -1;
	    }
	  break;
	case 2:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=3;
	  break;
	case 3:
	  if ( c == '.' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 4;
	    }
	  else
	    {
	      state = 5;
	    }
	  break;
	case 4:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state = 5;
	  break;
	case 5:
	  if (( c=='e') || (c =='E'))
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 6;
	    }
	  else
	    {
	      state = 8;
	    }
	  break;
	case 6:
	  if (( c=='+') || (c =='-'))
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=7;
	  // no break on purpose
	case 7:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=8;
	  // no break on purpose
	case 8:
	  // this char is NOT part of our number
	  ctx->pushback_char(ctx,data,c);
	  if ( json_context_get_debug(ctx) > 0 )
	    {
	      printf("(pushback end of number %c)",c);
	      char d = ctx->next_char(ctx,data);
	      if ( d != c )
		{
		  fprintf(stderr,"(pushback end of number FAILS '%c'!='%c')\n",c, d);
		}
	      ctx->pushback_char(ctx,data,c);
	    }
	  state = 9;
	  return state;
	default:
	  state = -1;
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
    if ( ctx->debug_level > 2 )
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
		dump_object(ctx,parent,NULL);
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
	if ( ctx->debug_level > 0 )
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
	if ( ctx->debug_level > 0 )
	  {
	    // we were ignoring and now will we stop ...
	    if ( FLAG_IS_SET(ctx->internal_flags,JSON_FLAG_IGNORE) )
	      {
		printf("ignore>");
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
	    fprintf(stderr,"[ERROR] unexpected char %c\n,",c);
	    JSON_TOKEN(EOF);
	    // ctx->pushback_char(ctx,data,c);
	  }
      }
    
    c=ctx->next_char(ctx, data);
  }
  
  JSON_TOKEN(EOF);
}
