#ifndef ALJSON_PRINT_HEADER__
#define ALJSON_PRINT_HEADER__

#include "aloutput.h"

struct print_ctx;
struct json_parser_ctx;
struct json_object;

typedef void (*aljson_print_callback)(struct json_object * object, struct print_ctx * print_ctx);

typedef void (*aljson_print_printf_callback)(struct print_ctx * print_ctx, const char * format, ...);

enum aljson_print_format {
			  ALJSON_PRINT_TABS,
			  ALJSON_PRINT_SPACES,
			  ALJSON_PRINT_FLAT
};

enum aljson_number_encoding {
  ALJSON_NUMBER_ENCODING_STRING,
  ALJSON_NUMBER_ENCODING_FLOAT
};

/* parameters for pretty printing 

THIS is required to call aljson_print_ctx_init(struct print_ctx * print_ctx) on it any use.
*/
struct print_ctx
{
  int depth;
  int max_depth;
  int indent;
  int do_indent; // 0 no indent, >= 1 number of char 's_indent' by indent.
  int space_after; // 0 no space after : pair, 1 one space.
  char * s_indent; // activate line by line indentation
  enum aljson_print_format format;
  enum aljson_number_encoding number_encoding;
  
  struct aloutputstream * outfile;

  aljson_print_callback growable_output;
  aljson_print_callback dict_output;
  aljson_print_callback list_output;
  aljson_print_callback string_output;
  aljson_print_callback number_output;
  aljson_print_callback error_output;
  aljson_print_callback pair_output;
  aljson_print_callback constant_output;
  aljson_print_callback variable_output;

  aljson_print_printf_callback printf;
};

// REQUIRED before any use of a print_ctx
void aljson_print_ctx_init(struct print_ctx * print_ctx);

void aljson_print_ctx_set_format(struct print_ctx * print_ctx, enum aljson_print_format format);

void aljson_print_ctx_set_output(struct print_ctx * print_ctx, struct aloutputstream * output);

/** dump object == aljson_output

TODO to be renamed to aljson_print
*/
void aljson_output(struct json_object * object, struct print_ctx * print_ctx);

void aljson_print_object_name(struct json_object * object, struct print_ctx * print_ctx);


#endif // ALJSON_PRINT_HEADER__
