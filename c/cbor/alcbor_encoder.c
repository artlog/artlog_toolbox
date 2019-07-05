#include "alcbor_encoder.h"
#include <stddef.h>

void alcbor_encoder_json_object(struct json_object * object, struct aljson_output_context * output_context)
{
  //
}

void alcbor_encoder_json_growable_object(struct json_object * object, struct aljson_output_context * output_context){
} //

void alcbor_encoder_json_dict_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_list_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_string_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_pair_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_string_number_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_variable_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_constant_object(struct json_object * object, struct aljson_output_context * output_context){}

void alcbor_encoder_json_error_object(struct json_object * object, struct aljson_output_context * output_context){}



void alcbor_encoder_init(struct aljson_output_context * output_context)
{
  struct aljson_output_callback_context * callback = & output_context->callback;

  callback->object=alcbor_encoder_json_object;
  callback->growable_object=alcbor_encoder_json_growable_object; 
  callback->dict_object=alcbor_encoder_json_dict_object;
  callback->list_object=alcbor_encoder_json_list_object;
  callback->string=alcbor_encoder_json_string_object;
  callback->pair_object=alcbor_encoder_json_pair_object;
  callback->string_number=alcbor_encoder_json_string_number_object;
  callback->variable_object=alcbor_encoder_json_variable_object;
  callback->constant_object=alcbor_encoder_json_error_object;
  callback->error_object=alcbor_encoder_json_error_object;

  // TODO this is where cbor construction is linked.
  output_context->data=NULL;

  
}
