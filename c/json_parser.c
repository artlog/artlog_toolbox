#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "json_parser.h"


/** Initialize json_context **/
void json_context_initialize(struct json_ctx *json_context)
{
  json_context->unstack=parse_level;
  //json_context->unstack=parse_level_legacy;
  json_context->next_char=next_char;
  json_context->pushback_char=pushback_char;
  json_context->add_char=add_char;
  json_context->pos=0;
  json_context->buf=NULL;
  json_context->bufsize=0;
  json_context->bufpos=0;
  json_context->debug_level=0;
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

int add_char(struct json_ctx * ctx, char token, char c)
{
  int bufsize=256;
#ifdef DEBUGALL
  if ( json_context_get_debug(ctx) )
    {
      printf("%c", c);
    }
#endif
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

void memory_shortage(struct json_ctx * ctx)
{
  fprintf(stderr,"Memory heap shortage. Exiting \n");
  exit(2);
}

void debug_tag(struct json_ctx *ctx,char c)
{
  if (json_context_get_debug(ctx))
    {
      printf("%c",c);
    }
}

void flush_char_buffer(struct json_ctx * ctx)
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
