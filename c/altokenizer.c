#include "altokenizer.h"
#include "alhash.h"
#include <strings.h>
#include <stdlib.h>
#include <assert.h>

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
  return altoken_char_buffer_add_char(&tokenizer->token_buf, c);
}

void altokenizer_init(struct altokenizer * tokenizer)
{
  bzero(tokenizer,sizeof(*tokenizer));
  tokenizer->next_char=altokenizer_get_next_char_default;
  tokenizer->pushback_char=altokenizer_set_pushback_char_default;
  tokenizer->add_char=altokenizer_add_token_char_default;
  // HARDCODED 15 words, 1024 bytes initial buffer and 78% (200/256th)
  alhash_context_init(&tokenizer->context,15,1024,200);
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

void altokenizer_add_char(struct altokenizer *tokenizer, char token, char c)
{
  tokenizer->add_char(tokenizer,token,c);
}



/* return an entry pointer in global dict_index table
 */
struct alhash_entry *
altokenizer_dict_add_string (struct altokenizer *tokenizer, char * buffer, int length)
{
  alhash_context * context = &tokenizer->context;
  
  if (buffer == NULL)
    {
      assert(buffer!=NULL);
      aldebug_printf(NULL,"[FATAL] corrupted token char buffer NULL");
      return NULL;
    }

  if (length == 0)
    {
      assert(length!=0);
      aldebug_printf (NULL,"[FATAL] corrupted empty char buffer\n");
      return NULL;
    }
  if (length >= 0)
    {
      aldebug_printf(NULL,ALPASCALSTRFMT, ALPASCALSTRARGS(length, buffer));
    }
  else
    {
      aldebug_printf (NULL,
	       "[FATAL] corrupted parser token char buffer length %i <=0\n",
	       length);
      return NULL;
    }

  struct alhash_datablock key;
  struct alhash_datablock *valuep;
  
  //  create an entry in dict
  key.type = ALTYPE_OPAQUE;
  key.length = length;
  key.data.ptr = buffer;

  // search directly with buffer key, don't allocate a new buffer string yet since it can already exists

  struct alhash_entry *entry = alhash_get_entry (&context->dict, &key);
  if (entry == NULL)
    {
      // no entry found, create it
      ++tokenizer->words;
      // use al_copy_block that support autogrow
      key.data.ptr = al_copy_block(&context->allocator.ringbuffer,&key);
      if ( key.data.ptr== NULL )
	{
	  aldebug_printf (NULL,
		   "[WARNING] internal char buffer for words full %i+%i>%i",
		   context->allocator.ringbuffer->bufpos, length, context->allocator.ringbuffer->bufsize);
	  return NULL;
	}
      valuep = &key;
      entry = alhash_put (&context->dict, &key, valuep);
      if (entry == NULL)
	{
	  aldebug_printf(NULL,
		   "[FATAL] FAIL to insert '%s' into word buffer %p \n",
		   buffer, &context->dict);
	  exit (1);
	}
    }
  else
    {
      aldebug_printf(NULL,"SAME TOKEN SEEN\n");
    }

  return entry;
}

void
altokenizer_reset_buffer_pos (struct altokenizer *tokenizer)
{
  // reset when word is parsed and recognized as either a reserved word or stored in variable dict with cut_string.
  // printf("//reset token buffer\n");
  tokenizer->token_buf.bufpos = 0;
}

struct alhash_entry *
altokenizer_cut_token_string(struct altokenizer *tokenizer)
{
  struct token_char_buffer *tb = &tokenizer->token_buf;
  char *buffer = tb->buf;
  int length = tb->bufpos;

  struct alhash_entry *entry = altokenizer_dict_add_string(tokenizer, buffer, length);

  if (entry != NULL)
    {

        altokenizer_reset_buffer_pos(tokenizer);
      //  parser->last_word = TOKEN_C_DICTENTRY_ID;
	//	parser->dict_value = &entry->value;  	
      // reset_tokenizer_buffer (tokenizer);	
    }
  else
    {
      return NULL;
    }


 
  return entry;
}

struct alhash_entry * altokenizer_make_token(struct altokenizer *tokenizer, struct al_token * token, char c)
{
  // full token was build with add_char
   return altokenizer_cut_token_string(tokenizer);
}

void altokenizer_release(struct altokenizer * tokenizer)
{
  altoken_flush_char_buffer(&tokenizer->token_buf);
  alhash_context_release(&tokenizer->context);
}
