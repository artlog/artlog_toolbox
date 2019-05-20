#ifndef _ALINPUT_UTIL_HEADER_
#define _ALINPUT_UTIL_HEADER_

#include "alinput.h"

/**
create and allocate many inputstream correcponding to files chained altogether

int filenames : number of element strating from offset in filename** array of char*
int offset : where to start in array
char ** filename : array of strings one for each filename
**/
struct alinputstream * alinput_util_build_chain_stream_from_filenames(int filenames, int offset,char ** filename);

// dispose all after use
int  alinput_util_destroy_and_close_chain_stream(struct alinputstream * head);
  
  
#endif // _ALINPUT_UTIL_HEADER_
