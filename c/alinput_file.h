#ifndef ALINPUT_FILE_HEADER_
#define ALINPUT_FILE_HEADER_

#include "alcommon.h"
#include "alinput.h"

/** usage

struct alinputstream input;
if ( alinput_file_open_init(&input, filename) == AL_EC_OK)
{
(....)
alinputstream_close(&input);
}
 */
enum al_global_error_code  alinput_file_open_init(struct alinputstream * input, const char * filename);

// NOPE no FILE in api, don't want a stdio.h dependency
// #include <stdio.h>
// void alinput_file_init(struct alinputstream * input, FILE * file);

#endif // ALINPUT_FILE_HEADER_
