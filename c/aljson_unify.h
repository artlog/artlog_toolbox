#ifndef ALJSON_UNIFY_HEADER__
#define ALJSON_UNIFY_HEADER__

/**
first lazzy implementation : compare them

return 1 if equals, 0 if different.
*/
int json_unify_object(
	       struct json_parser_ctx * ctx, struct json_object * object,
	       struct json_parser_ctx * other_ctx, struct json_object * other_object,
	       struct print_ctx * print_ctx);

/**
dump object in a flat view, ie using object name=value notation for every value.
 */
void json_flatten(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);


#endif
