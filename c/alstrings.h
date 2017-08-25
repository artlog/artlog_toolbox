#ifndef _AL_STRING_H_
#define _AL_STRING_H_

/** data pointing on a contiguous block of length bytes */
// legacy name since borrowed from hash -> to rename everywhere ?
struct alhash_datablock {
  int length; // > 0 , EMPTY BLOCK NOT VALID
  void * data; // NULL value NOT VALID.
};

// to rename since borrwoed from json_parser project
struct token_char_buffer {
  // internal buffer to collect data
  char * buf;
  // position of token buffer where to add data
  int bufpos;
  // buffer size to be grown if needed ( see al_token_char_buffer_grow )
  int bufsize;
};

/* non NULL if grown, else miss
usualy double of size.
 */
struct token_char_buffer * al_token_char_buffer_grow(struct token_char_buffer * buffer);

char * al_copy_block(struct token_char_buffer * buffer,  struct alhash_datablock * data);

#endif
