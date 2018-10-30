#ifndef ALJSON_UNIFY_HEADER__
#define ALJSON_UNIFY_HEADER__

#include "aljson.h"
#include "aljson_print.h"

/**
first lazzy implementation : compare them

return 1 if equals, 0 if different.
*/
int aljson_unify_object(
	       struct json_parser_ctx * ctx, struct json_object * object,
	       struct json_parser_ctx * other_ctx, struct json_object * other_object,
	       struct print_ctx * print_ctx);


#endif
