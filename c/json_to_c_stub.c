#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "json_to_c_stub.h"
#include "todo.h"
#include "albase.h"

// part of generic tools.

// return json_object pair type.
struct json_object * json_c_add_json_object_member(char * name, struct json_object * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct alhash_datablock data;
  
  data.data.ptr = name;
  data.type=ALTYPE_OPAQUE;
  data.length = strlen(data.data.ptr);
  data.data.ptr = al_copy_block(allocator, &data);

  printf("key:%.*s %i\n",data.length,(char *) data.data.ptr,data.length);
  // create json pair with name of field
  struct json_object * key = aljson_new_json_object(ctx->tokenizer, '"',  allocator, &data);
  struct json_object * pair = aljson_new_pair_key(ctx, pair);
  pair->pair.key=key;
  if (( pair != NULL ) && (key != NULL ))
    {
      pair->pair.value=value;
    }

  return pair;
}

// json_object pair type.
struct json_object * json_c_add_int_member(char * name, int value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct alhash_datablock data;

  // convert int to json int char representation
  aljson_build_string_from_int(value, 10, allocator, &data);
  data.type=ALTYPE_OPAQUE;
  struct json_object * object = aljson_new_json_object(ctx->tokenizer, '0', allocator, &data);

  return json_c_add_json_object_member(name, object, ctx, allocator);
}

// json_object pair type.
// capture char value content.
struct json_object * json_c_add_string_member(char * name, char * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
  struct alhash_datablock data;
  
  data.type=ALTYPE_OPAQUE;
  data.data.ptr=value;
  if ( value != NULL )
    {
      data.length = strlen(data.data.ptr);
    }
  else
    {
      data.length = 1;
    }
  printf("string:%.*s %i\n",data.length,(char *) data.data.ptr,data.length);
  data.data.ptr = al_copy_block(allocator,&data);
  struct json_object * object = aljson_new_json_object(ctx->tokenizer, '"', allocator, &data);

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
      fprintf(stderr,"[ERROR] key reference '%s' without &\n",key);
      return NULL;
    }
}
