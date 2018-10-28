#ifndef ALJSON_DUMP_HEADER__
#define ALJSON_DUMP_HEADER__

#include "aljson.h"
#include "aljson_walk.h"
#include "aljson_print.h"


void aljson_dump_enter_indent(struct print_ctx * print_ctx);

void aljson_dump_exit_indent(struct print_ctx * print_ctx);

// limited to print_ctx->max_depth since relying on code stack call.
void aljson_dump_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);


void aljson_dump_dict_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_list_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_string(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_pair_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_string_number(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_variable_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_json_path(struct json_path * json_path);

void aljson_dump_indent(struct print_ctx * print_ctx);

void aljson_dump_error_object(struct json_parser_ctx * ctx, struct json_object* object, struct print_ctx * print_ctx);

void aljson_dump_constant_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

void aljson_dump_growable_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);

#endif
