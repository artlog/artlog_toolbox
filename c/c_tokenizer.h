#ifndef _C_TOKENIZER_H_
#define _C_TOKENIZER_H_

// in fact should be a json_tokenizer, will be fixed after end of full rework 
#include "aljson_parser.h"

struct al_token * c_tokenizer(struct json_ctx * ctx, void * data);

#endif
