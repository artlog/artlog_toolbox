#ifndef __ALOUTPUTSTREAM_H__
#define __ALOUTPUTSTREAM_H__

#include <stdio.h>

struct aloutputstream;

typedef void (*aloutput_callback_writeint32) (struct aloutputstream * stream, int word);

typedef void (*aloutput_callback_flush) (struct aloutputstream * stream, int word, int bits);

typedef void (*aloutput_callback_close) (struct aloutputstream * stream);

struct aloutputstream {
  FILE * file;
  int fd;
  int debug;
  // callback case
  aloutput_callback_writeint32 callback_writeint32;
  aloutput_callback_flush callback_flush;
  aloutput_callback_close callback_close;
  // data for callback
  void * data;  
};

void aloutputstream_init(struct aloutputstream * stream, FILE * file);

void aloutputstream_set_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_writeint32 callback_writeint32,
				 aloutput_callback_flush callback_flush,
				 aloutput_callback_close callback_close);

void aloutputstream_writeint32(struct aloutputstream * stream, int word);

/** flush last bits of word , 
this allows to handle bit level streams where all first data are sent as word
and final one is sent as bit, ( useful for bitlevel sha2 hash )
bit are set in least significant bits of words.
*/
void aloutputstream_flush(struct aloutputstream * stream, int word, int bits);

void * aloutputstream_get_data(struct aloutputstream * stream);

int aloutputstream_getfd(struct aloutputstream * stream);

FILE * aloutputstream_file(struct aloutputstream * stream);

void aloutputstream_close(struct aloutputstream * stream);

#endif // #ifndef __ALOUTPUTSTREAM_H__
