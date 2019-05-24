#ifndef ALINPUT_FILE_HEADER_
#define ALINPUT_FILE_HEADER_

#include "alcommon.h"
#include "alinput.h"
#include <stdio.h>

enum al_global_error_code  alinput_file_open_init(struct alinputstream * input, const char * filename);

void alinput_file_init(struct alinputstream * input, FILE * file);

#endif // ALINPUT_FILE_HEADER_
