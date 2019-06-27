#ifndef ALJSON_ENCODER_HEADER_
#define ALJSON_ENCODER_HEADER_

#include "aljson.h"

// return json_object int type
struct json_object * aljson_encoder_int( int value,
				     struct json_parser_ctx * ctx,
				     alstrings_ringbuffer_pointer * allocator);

// capture char value content , deprecated, uses strlen.
struct json_object * aljson_encoder_string(const char * value,
					   struct json_parser_ctx * ctx,
					   alstrings_ringbuffer_pointer * allocator);

struct json_object * aljson_encoder_pascal_string(int length, const char * value, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator);


#endif // ALJSON_ENCODER_HEADER_
