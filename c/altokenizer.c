#include "altokenizer.h"

char altokenizer_get_next_char_default (struct altokenizer *ctx, void *data)
{
  return 0;
}

void altokenizer_set_pushback_char_default (struct altokenizer *ctx, void *data, char pushback)
{
  return;
}

// struct al_token* (*altokenizer_func) (struct altokenizer *ctx, void *data);

// DISREGARD token
int altokenizer_add_token_char_default (struct altokenizer *tokenizer, char token, char c)
{
  return altoken_char_buffer_add_char(tokenizer->token_buf, c);
}

void altokenizer_init(struct altokenizer * tokenizer)
{
  bzero(tokenizer,sizeof(*tokenizer));
  tokenizer->next_char=altokenizer_get_next_char_default;
  tokenizer->pushback_char=altokenizer_pushback_char_default;
  tokenizer->add_char=altokenizer_add_token_char_default;
}

int altokenizer_consume(struct altokenizer * ctx, void * data, char * str)
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

void altokenizer_release(struct altokenizer * tokenizer)
{
  altoken_flush_char_buffer(&tokenizer->token_buf);
}
