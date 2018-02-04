#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "alcommon.h"
#include "aljson_import_internal.h"

char json_read_char(struct json_import_context_data * json_import_data)
{
  if (json_import_data->inputstream != NULL )
    {
       json_import_data->last = alinputstream_readuchar(json_import_data->inputstream);
    }
  else
    {
      return 0;
    }
  return json_import_data->last;
}

char json_import_next_char(struct json_ctx* ctx, void * data)
{
  struct json_import_context_data * json_import_data = (struct json_import_context_data *) data;
  if ( FLAG_IS_SET( json_import_data->flags,ALSFLAG_PUSHBACK ) )
    {
      if ( FLAG_IS_SET( ctx->debug_level, TOKENIZER_DEBUG_PUSHBACK) )
	{
	  printf("<pushback>%c</pushback>\n",json_import_data->last);
	}
      json_import_data->flags ^= ALSFLAG_PUSHBACK;
      return json_import_data->last;
    }
    
  if ( json_read_char(json_import_data) != 0 )
    {
      if ( json_import_data->last ==  0x0a )
	{	  
	  ++ctx->pos_info.line;
	  ctx->pos_info.column=0;
	}
      else if ( json_import_data->last != 0x0c )
	{
	  ++ctx->pos_info.column;
	}
      ++ctx->pos;
      return json_import_data->last;
    }
  else
    {
      return 0;
    }
}

void json_import_pushback_char(struct json_ctx *ctx, void *data, char pushback)
{
  struct json_import_context_data * json_import_data = (struct json_import_context_data *) data;

  // should not pushback something else than previous char. programming error
  assert((ctx->pos>0) && (pushback == json_import_data->last ));
  if ( FLAG_IS_SET( json_import_data->flags, ALSFLAG_PUSHBACK ) )
    {
      // two pushback ... NOT SUPPORTED
      fprintf(stderr,"[ERROR] 2 pushbacks line %i column %i. NOT SUPPORTED.\n", ctx->pos_info.line, ctx->pos_info.column);
    }    
  json_import_data->flags |= ALSFLAG_PUSHBACK;
  ctx->pos--;
}

void json_import_context_initialize(struct json_ctx *ctx)
{
  json_context_initialize( ctx, json_import_next_char);
  ctx->pushback_char=json_import_pushback_char;
  ctx->pos_info.column=0;
  ctx->pos_info.line=0;
}

void json_import_print_context_initialize(struct print_ctx * print_context)
{
    // two spaces
  print_context->do_indent = 2;
  print_context->indent = 0;
  print_context->s_indent = " ";
}
