#ifndef __JSON_IMPORT_INTERNAL_HEADER__
#define __JSON_IMPORT_INTERNAL_HEADER__

#include "alinput.h"
#include "aljson.h"
#include "aljson_print.h"

/** fully rely on stream buffering */
struct json_import_context_data {
  char last;
  struct alinputstream * inputstream;
  int flags;
  int debug;
};

enum als_flag {
  ALSFLAG_PUSHBACK = 1
};

/** WELL ... really extensive documentation here 
 **/
void aljson_init(
	       struct json_parser_ctx * json_context,
	       struct json_ctx * json_tokenizer,
	       struct print_ctx * print_context);

char json_import_next_char(struct json_ctx* ctx, void * data);

void json_import_pushback_char(struct json_ctx *ctx, void *data, char pushback);

void json_import_context_initialize(struct json_ctx *ctx);

void json_import_print_context_initialize(struct print_ctx * print_context);
#endif
