#ifndef ALCBOR_ENCODER_HEADER_
#define ALCBOR_ENCODER_HEADER_

#include "alcbor.h"
#include "aljson_output.h"
#include "aloutput.h"

struct alcbor_encoder {
  struct aljson_output_context output_context;
  // whatever needed like output
  struct aloutputstream * output;
  // prehaps an intermediate buffer and some state ?
};

void alcbor_encoder_json_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_growable_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_dict_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_list_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_string_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_pair_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_string_number_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_variable_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_constant_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_json_error_object(struct json_object * object, struct aljson_output_context * output_context);

void alcbor_encoder_init(struct alcbor_encoder * encoder, struct aloutputstream * output);

#endif // ALCBOR_ENCODER_HEADER_
