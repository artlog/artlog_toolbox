#ifndef ALJSON_PRINT_HEADER__
#define ALJSON_PRINT_HEADER__

struct print_ctx;
struct json_parser_ctx;
struct json_object;

typedef void (*aljson_print_callback)(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

typedef void (*aljson_print_printf_callback)(struct print_ctx * print_ctx, const char * format, ...);

/* parameters for pretty printing 

THIS is required to call aljson_print_ctx_init(struct print_ctx * print_ctx) on it any use.
*/
struct print_ctx
{
  int max_depth;
  int indent;
  int do_indent; // 0 no indent, >= 1 number of space by indent.
  char * s_indent;

  // todo use outputstream, currently void * will be a FILE *
  void * outfile;

  aljson_print_callback growable_output;
  aljson_print_callback dict_output;
  aljson_print_callback list_output;
  aljson_print_callback string_output;
  aljson_print_callback number_output;
  aljson_print_callback error_output;
  aljson_print_callback pair_output;
  aljson_print_callback constant_output;

  aljson_print_printf_callback printf;
};

// REQUIRED before any use of a print_ctx
void aljson_print_ctx_init(struct print_ctx * print_ctx);

/** dump object == aljson_output
struct json_parser_ctx * ctx can be NULL ( and in fact we should rely only on print_ctx)
TOOD FIX it to not use struct json_parser_ctx at ALL
*/
void aljson_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

/** TODO change to aljson_print... */
void json_print_object_name(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

#endif
