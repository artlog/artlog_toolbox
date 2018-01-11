#ifndef _AL_STRING_H_
#define _AL_STRING_H_

enum altype {
  ALTYPE_OPAQUE=0, // opaque type meaning no encoding type known.
  ALTYPE_CINTLE=1, // NYI c integer in little endian, length is externaly defined in bytes 
  ALTYPE_CINTBE=2, // NYI c integer in big endian, length is externaly defined in bytes
  ALTYPE_STR0=3, // NYI c string \0 terminated
  ALTYPE_FLAG_EMBED=64, // NFI(not fully implemented) value is not a pointer in data.ptr but might be a number see aldatablock_embeded
  ALTYPE_MAX=128,
};

/** data pointing on a contiguous block of length bytes */
// legacy name since borrowed from hash -> to rename everywhere ?
/* can be direct number data copy for integers */
struct alhash_datablock {
  enum altype type;
  int length; // > 0 , EMPTY BLOCK NOT VALID
  union {
    void * ptr; // NULL value NOT VALID.
    long number;
  } data;
};

/** char buffer RING ( circular list allocated by al_token_char_buffer_alloc(int times); )
 can be filled ONLY ( no removal ).
 pointer should not move ( once allocated, can be moved ).
 can grow ( ² growth ).

token_char_buffer circular linked list is built at first time
BUT buf and its size is allocated on request only.
*/
// to rename since borrowed from json_parser project
struct token_char_buffer {
  // internal buffer to collect data
  char * buf;
  // position of token buffer where to add data
  int bufpos;
  // buffer size to be grown if needed ( see al_token_char_buffer_grow )
  int bufsize;
  // linked list
  // first always point to allocated first
  struct token_char_buffer * first;
  // next is circular and can be replaced by a next after if next is detected full.
  struct token_char_buffer * next;  
};

/* non NULL if grown, else miss
usualy double of size
search within list of possible buffers
it should not change existing allocation ( that are pointed to )
 */
struct token_char_buffer * al_token_char_buffer_grow(struct token_char_buffer * buffer, int length);

// allocate length byte within buffer, does not zero them ( keep content ).
// return pointer on first char in buffer
char * al_alloc_block(struct token_char_buffer * buffer, int length);
  
// copy data content into token_char_buffer, return pointer on first char within buffer ( to update within data.ptr )
char * al_copy_block(struct token_char_buffer * buffer,  struct alhash_datablock * data);

struct token_char_buffer * al_token_char_buffer_alloc(int times);

void al_token_char_buffer_init(struct token_char_buffer * buffer, int chars);
#endif
