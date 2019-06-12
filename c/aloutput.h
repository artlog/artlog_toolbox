#ifndef ALOUTPUT_HEADER_
#define ALOUTPUT_HEADER_

#include "alstrings.h"

struct aloutputstream;

typedef void (*aloutput_callback_write_byte) (struct aloutputstream * stream, unsigned char byte);

typedef void (*aloutput_callback_writeint32) (struct aloutputstream * stream, int word);

typedef void (*aloutput_callback_flush) (struct aloutputstream * stream, int word, int bits);

typedef void (*aloutput_callback_close) (struct aloutputstream * stream);

enum aloutput_target {
		      ALOUTPUT_TARGET_FILE = 1, // deprecated
		      ALOUTPUT_TARGET_FD = 2,
		      ALOUTPUT_TARGET_BUFFER = 3
};

struct aloutputstream {
  int fd;
  int debug;
  enum aloutput_target target;
  // prereserved buffer
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

// please see aloutput_file_open_init(struct aloutputstream * output, const char * filename) in aloutput_file.h
// void aloutputstream_init(struct aloutputstream * stream, FILE * file);

void aloutputstream_fd_init(struct aloutputstream * stream, int fd);

/** create an outputstream over a contiguous prereserved buffer */
void aloutputstream_init_shared_buffer(struct aloutputstream * stream, aldatablock * buffer, int offset);

void aloutputstream_set_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_write_byte callback_write_byte,
				 aloutput_callback_writeint32 callback_writeint32,
				 aloutput_callback_flush callback_flush,
				 aloutput_callback_close callback_close);

void aloutputstream_set_close_callback(
				 struct aloutputstream * stream,
				 aloutput_callback_close callback_close,
				 void * data);

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

// DON'T provide access to inner backend anymore
// int aloutputstream_getfd(struct aloutputstream * stream);
// FILE * aloutputstream_file(struct aloutputstream * stream);

void aloutputstream_close(struct aloutputstream * stream);

/** printf to stream limited to 1kiB **/
int aloutputstream_printf_1k(struct aloutputstream * stream, const char *format, ...);

/** printf to stream limited to 1kiB **/
#include <stdarg.h>
int aloutputstream_vprintf_1k(struct aloutputstream * stream, const char *format, va_list args);

// order 1 or 0
// 1 : big endian, 0 little endian
// group = 2 short, 4 word, 8 long word ...
void aloutput_bytes_as_hex(struct aloutputstream * stream,  aldatablock * datablock, int order, int group);

// copy written bytes from localbuffer to stream
int aloutputstream_memcpy(int written, struct aloutputstream * stream, char * localbuffer);

#endif // #ifndef ALOUTPUT_HEADER_
