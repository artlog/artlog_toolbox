#include "altoken.h"

#include <stdlib.h>

int altoken_char_buffer_add_char(struct token_char_buffer * ctx, char c)
{
  int bufsize=ALTOKEN_BUFSIZE_MIN;
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
      if ( bufsize > ALTOKEN_BUFSIZE_MAX )
	{
	  aldebug_printf(NULL,"[FATAL] huge memory consumption for a token %i > %i", bufsize, ALTOKEN_BUFSIZE_MAX);
	  exit(0);
	}
      if ( bufsize > ALTOKEN_BUFSIZE_WARNING )
	{
	  aldebug_printf(NULL,"[WARNING] huge memory consumption for a token %i > %i", bufsize, ALTOKEN_BUFSIZE_WARNING);
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
	  aldebug_printf(NULL,"FATAL memory shortage in %s %s %i\n", __FILE__, __FUNCTION__, __LINE__ );
	}
    }
  ctx->buf[ctx->bufpos++]=c;
  return 0;
}

void altoken_flush_char_buffer(struct token_char_buffer * ctx)
{
  if (ctx->buf != NULL )
    {
      free(ctx->buf);
      ctx->buf=NULL;
      ctx->bufpos=0;
      ctx->bufsize=0;
    }
}
