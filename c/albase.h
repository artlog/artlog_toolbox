#ifndef __ALBASE_H__
#define __ALBASE_H__

#include "alstrings.h"

// for debug display
void albase_set_debug(int debug);

// correct bases are 2 binary, 8 octal,10 decimal ,16 hexadecimal.
// allocate buffer on allocator, result reference is in out.
int aljson_build_string_from_int(int integer, int base, struct token_char_buffer * allocator, struct alhash_datablock * out );

#endif // __ALBASE_H__
