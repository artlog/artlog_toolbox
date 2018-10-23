#ifndef __ALOUTPUTSTREAM_H__
#define __ALOUTPUTSTREAM_H__

#include "alstrings.h"

#include <stdio.h>

struct aloutputstream;

typedef void (*aloutput_callback_write_byte) (struct aloutputstream * stream, unsigned char byte);

typedef void (*aloutput_callback_writeint32) (struct aloutputstream * stream, int word);

typedef void (*aloutput_callback_flush) (struct aloutputstream * stream, int word, int bits);

typedef void (*aloutput_callback_close) (struct aloutputstream * stream);

enum aloutput_target {
  ALOUTPUT_TARGET_FILE = 1,
  ALOUTPUT_TARGET_FD = 2,
  ALOUTPUT_TARGET_BUFFER = 3
};

struct aloutputstream {
  FILE * file;
  int fd;
  int debug;
  enum aloutput_target target;
  aldatablock buffer;
  // offset within buffer
  unsigned int offset;
  // callback case
  aloutput_callback_write_byte callback_write_byte;
  aloutput_callback_writeint32 callback_writeint32;
  aloutput_callback_flush callback_flush;
  aloutput_callback_close callback_close;
  // data for callback
  void * data;  
};

void aloutputstream_init(struct aloutputstream * stream, FILE * file);

/** create an outputstream over a contiguous prereserved buffer */
void aloutputstream_init_shared_buffer(struct aloutputstream * stream, aldatablock * buffer, int offset);

void aloutputstream_set_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_write_byte callback_write_byte,
				 aloutput_callback_writeint32 callback_writeint32,
				 aloutput_callback_flush callback_flush,
				 aloutput_callback_close callback_close);


void aloutputstream_write_byte(struct aloutputstream * stream, unsigned char byte);

void aloutputstream_writeint32(struct aloutputstream * stream, int word);

/** flush last bits of word , 
this allows to handle bit level streams where all first data are sent as word
and final one is sent as bit, ( useful for bitlevel sha2 hash )
bit are set in least significant bits of words.
*/
void aloutputstream_flush(struct aloutputstream * stream, int word, int bits);

/** in buffer target it is possible to obtain a pointer over a contigous buffer */
void * aloutputstream_get_data(struct aloutputstream * stream);

int aloutputstream_getfd(struct aloutputstream * stream);

FILE * aloutputstream_file(struct aloutputstream * stream);

void aloutputstream_close(struct aloutputstream * stream);

#endif // #ifndef __ALOUTPUTSTREAM_H__
