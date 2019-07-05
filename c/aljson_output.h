#ifndef ALJSON_OUTPUT_HEADER_
#define ALJSON_OUTPUT_HEADER_

// list of callback for a json dom parsing output

#include "aljson.h"

// forward
struct aljson_output_callback_context;
struct aljson_output_context;

typedef void (*aljson_output_callback_json_object) (struct json_object * object, struct aljson_output_context * output_context);

struct aljson_output_callback_context {
  
  aljson_output_callback_json_object object;

  aljson_output_callback_json_object dict_object;

  aljson_output_callback_json_object list_object;

  aljson_output_callback_json_object string;

  aljson_output_callback_json_object pair_object;

  aljson_output_callback_json_object string_number;

  aljson_output_callback_json_object variable_object;

};

struct aljson_output_context {
  struct aljson_output_callback_context callback;
  void * data; 
};

void aljson_output_init( struct aljson_output_context * output, struct aljson_output_callback_context * callback, void * data );

#endif // ALJSON_OUTPUT_HEADER_
