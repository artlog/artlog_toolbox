#ifndef ALCBOR_DECODER__
#define ALCBOR_DECODER__

#include "alinput.h"
#include "aloutput.h"
#include "aljson.h"

// field encoding type
typedef enum alcbor_field_encoding_type {
					 ALCBOR_FET_NOT_SET = -2,
					 ALCBOR_FET_NYI = -2,
					 ALCBOR_FET_ERROR = -1,
					 ALCBOR_FET_TINY = 0,
					 ALCBOR_FET_SHORT = 1,
					 ALCBOR_FER_LONG = 2
} alcbor_fet;

typedef enum alcbor_major_type {
				ALCBOR_MT0_UINT = 0,
				ALCBOR_MT1_NINT = 1,
				ALCBOR_MT2_BSTR = 2,
				ALCBOR_MT4_ARRAY = 4,
} alcbor_mt;


struct alcbor_json_output {
  struct json_parser_ctx json_ctx;
  alstrings_ringbuffer_pointer * allocator;
  struct json_object * root;
  struct json_object * parent;
  struct json_object * last;
};

  
typedef struct alcbor_parsing_context {
  struct alinputstream * input;
  alcbor_fet fet;
  aldatablock block;
  int idxob; // char index within block
  int header_byte;
  int value_length;
  struct alcbor_json_output output;
} alcbor_pc;

#endif // ALCBOR_DECODER__


void alcbor_parsing_context_init(alcbor_pc * context, struct alinputstream * input, struct aloutputstream * output);

void alcbor_parsing_context_release(alcbor_pc * context);

// once done result can be obtained with alcbor_get_json_root(context)
void alcbor_parse(alcbor_pc * context);

struct json_object * alcbor_get_json_root(alcbor_pc * context);

struct json_parser_ctx * alcbor_get_json_context(alcbor_pc * context);

