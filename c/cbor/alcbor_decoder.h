#ifndef ALCBOR_DECODER__
#define ALCBOR_DECODER__

#include "alcbor.h"

#include "alinput.h"
#include "aloutput.h"
#include "aljson.h"

struct alcbor_json_output {
  struct json_parser_ctx json_ctx;
  alstrings_ringbuffer_pointer * allocator;
  struct json_object * root;
  struct json_object * parent;
  struct json_object * last;
};
  
typedef struct alcbor_parsing_context {
  int depth;
  int maxdepth;
  struct alinputstream * input;
  alcbor_fet fet;
  aldatablock block;
  int idxob; // char index within block
  int header_byte;
  int value_length;
  struct alcbor_json_output output;
} alcbor_pc;

void alcbor_parsing_context_init(alcbor_pc * context, struct alinputstream * input, struct aloutputstream * output);

void alcbor_parsing_context_release(alcbor_pc * context);

// once done result can be obtained with alcbor_get_json_root(context)
void alcbor_parse(alcbor_pc * context);

struct json_object * alcbor_get_json_root(alcbor_pc * context);

struct json_parser_ctx * alcbor_get_json_context(alcbor_pc * context);

#endif // ALCBOR_DECODER__
