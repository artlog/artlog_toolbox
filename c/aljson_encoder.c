#include "aljson_encoder.h"
#include "aldebug_output.h"
#include "albase.h"

#include <stddef.h>
#include <string.h>


struct json_object * aljson_encoder_int(int value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct alhash_datablock data;

  // convert int to json int char representation
  albase_build_string_from_int(value, 10, allocator, &data);
  data.type=ALTYPE_OPAQUE;
  // '0' means int type
  return aljson_new_json_object('0', allocator, &data);
}


struct json_object * aljson_encoder_pascal_string(int length, const char * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct alhash_datablock data;
  
  data.type=ALTYPE_OPAQUE;
  data.data.constcharptr=value;
  if ( value != NULL )
    {
      data.length = length;
    }
  else
    {
      // not that good ERRORPRONE
      data.length = 1;
    }
  aldebug_printf(DBGSTREAM,"[DEBUG] add string:" ALPASCALSTRFMT " %i\n",
	 ALPASCALSTRARGS(data.length,data.data.charptr),
	 data.length);
  data.data.ptr = al_copy_block(allocator,&data);
  struct json_object * object = aljson_new_json_object('"', allocator, &data);

  return object;
}

struct json_object * aljson_encoder_string(const char * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  int length = 0;
  if ( value != NULL )
    {
      length = strlen(value);
    }
  else
    {
      // not that good ERRORPRONE and done twice and useless.
      length = 1;
    }

  return aljson_encoder_pascal_string(length,value, ctx, allocator);
}
