#include <stdlib.h>
#include <string.h>
// TODO rename to aljson_to_c_stub.h
#include "json_to_c_stub.h"
#include "altodo.h"
#include "albase.h"
#include "aldebug_output.h"
#include "aljson_encoder.h"

// TODO rename to aljson_to_c_stub.c

// part of generic tools.

// return json_object pair type.
struct json_object * json_c_add_json_object_member(const char * name, struct json_object * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  aldatablock data;

  data.data.constcharptr = name;
  data.type=ALTYPE_OPAQUE;
  data.length = strlen(data.data.ptr);
  data.data.ptr = al_copy_block(allocator, &data);

  aldebug_printf(DBGSTREAM,"[DEBUG] add key:" ALPASCALSTRFMT " %i\n",
                 ALPASCALSTRARGS(data.length,(char *) data.data.ptr),
                 data.length);
  // create json pair with name of field
  struct json_object * key = aljson_new_json_object('"',  allocator, &data);
  struct json_object * pair = aljson_new_pair_key(ctx, key);
  if (( pair != NULL ) && (key != NULL ))
    {
      pair->pair.value=value;
    }

  return pair;
}

// json_object pair type.
struct json_object * json_c_add_int_member(const char * name, int value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct json_object * int_object =  aljson_encoder_int(value, ctx, allocator);
  return json_c_add_json_object_member(name, int_object, ctx, allocator);
}


// json_object pair type.
// capture char value content.
struct json_object * json_c_add_string_member(const char * name, const char * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct json_object * object = aljson_encoder_string(name, ctx, allocator);

  return json_c_add_json_object_member(name, object, ctx, allocator);
}

struct json_object * json_to_c_stub_get_ref( struct json_object * json_ref, struct json_object * json_root)
{
  char * key = json_get_cstring(json_ref);
  if ( key[0] =='&' )
    {
      return json_dict_get_value(key,json_root);
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[ERROR] key reference '%s' without &\n",key);
      return NULL;
    }
}
