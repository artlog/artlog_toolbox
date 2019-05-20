#ifndef ALABNF_UTIL_HEADER__
#define ALABNF_UTIL_HEADER__

#include <stdio.h>

#include "alabnf.h"
#include "alinput_util.h"

struct alabnf * alabnf_util_parse_abnf_file(FILE * file);

// open all filename in order and create a stream to parse all in one.
struct alabnf * alabnf_util_parse_abnf_filenames(int filenames, int offset,char ** filename);

#endif // ALABNF_UTIL_HEADER__
