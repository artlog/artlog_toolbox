#ifndef _AL_STRING_H_
#define _AL_STRING_H_

#include "aldebug.h"

// type of one object (struct alhash_datablock ) within ( struct token_char_buffer )
enum altype {
  ALTYPE_OPAQUE=0, // opaque type meaning no encoding type known.
  ALTYPE_CINTLE=1, // NYI c integer in little endian, length is externaly defined in bytes 
  ALTYPE_CINTBE=2, // NYI c integer in big endian, length is externaly defined in bytes
  ALTYPE_STR0=3, // NYI c string \0 terminated
  ALTYPE_FLAG_EMBED=64, // NFI(not fully implemented) value is not a pointer in data.ptr but might be a number see aldatablock_embeded
  ALTYPE_MAX=128,
};

/** data pointing on a contiguous block of length bytes
 legacy name since borrowed from hash
   -> don't use this struct name, prefer aldatablock
  can be direct number data copy for integers 
**/
typedef struct alhash_datablock {
  enum altype type;
  int length; // > 0 , EMPTY BLOCK NOT VALID
  union {
    void * ptr; // NULL value NOT VALID.
    unsigned char * ucharptr;
    char * charptr;
    const char * constcharptr;
    unsigned int * uintptr;
    long number; // quick hack to store values directly (see type ).
  } data;
} aldatablock;

/** char buffer RING ( circular list allocated by al_token_char_buffer_alloc(int times); )
 can be filled ONLY ( no removal ).
 pointer should not move ( once allocated, can be moved ).
 can grow ( 2* growth ).

token_char_buffer circular linked list is built at first time
BUT buf and its size is allocated on request only.
*/
// to rename since borrowed from json_parser project
typedef struct token_char_buffer {
  // internal buffer to collect data of allocated capacity bufsize.
  char * buf;
  // position of token buffer where to add data within buf
  int bufpos;
  // buffer size to be grown if needed ( see al_token_char_buffer_grow )
  int bufsize;
  // linked list
  // first always point to allocated first
  struct token_char_buffer * first;
  // next is circular 
  struct token_char_buffer * next;
  // next potentialy free and can be replaced by a next after if next is detected full.
  struct token_char_buffer * next_free;  
} alstrings_out;

// introduced alstrings_ringbuffer_pointer , that point to right token_char_buffer within ring.
typedef alstrings_out * alstrings_ringbuffer_pointer;

// allocation context ( it can only grow or be fully released )
struct alallocation_ctx {
  ALDEBUG_DEFINE_FLAG(debug)
  alstrings_ringbuffer_pointer ringbuffer;
};

#define ALALLOC(ctx, length) al_alloc_block(&ctx.ringbuffer, length)

// just does nothing on ctx... only ALFREECTX can be used, but it releases ALL.
// created to track possible replacement with malloc/free
#define ALRELEASE(ctx, object)
		  
#define ALFREECTX(ctx) alstrings_ringbuffer_release(&ctx->ringbuffer)

/* non NULL if grown, else miss
usualy double of size
search within list of possible buffers
it should not change existing allocation ( that are pointed to )
 */
alstrings_ringbuffer_pointer al_token_char_buffer_grow(alstrings_ringbuffer_pointer buffer, int length);

// allocate length byte within buffer, does not zero them ( keep content ).
// return pointer on first char in buffer
char * al_alloc_block(alstrings_ringbuffer_pointer * ringbuffer, int length);
  
// copy data content into token_char_buffer, return pointer on first char within buffer ( to update within data.ptr )
// WARNING this can actualy change ringbufferp content.
char * al_copy_block(alstrings_ringbuffer_pointer * ringbufferp,  aldatablock * data);

/* create a circular list of struct token_char_buffer of size times.
useful for autogrowth */
alstrings_ringbuffer_pointer al_token_char_buffer_alloc(int times);

/* init char buffer to a length of chars
to use internally after al_token_char_buffer_alloc since it can't grow.
 */
// INTERNAL ONLY ( to remove from api ) use alstrings_ringbuffer_init_autogrow instead
void al_token_char_buffer_init(alstrings_ringbuffer_pointer buffer, int chars);

/* allocate a circular buffer of buckets with initial bucket char length 
will setup ringbuffer
*/
void alstrings_ringbuffer_init_autogrow(alstrings_ringbuffer_pointer * ringbuffer, int buckets, int firstbucketlength);

// release ringbuffer when sure we are not pointing to any of its data
void alstrings_ringbuffer_release(alstrings_ringbuffer_pointer * ringbufferp);

// read an unsigned int that was stored in big endian at offset in datablock
unsigned int aldatablock_get_uint32be(aldatablock * data, int offset);

// =====  use a datablock as output ========

void aldatablock_bzero(aldatablock * data,int offset, int length);

// return new offset 
int aldatablock_write_uint64be(aldatablock * data, int offset, unsigned long long value );

// return new offset 
int aldatablock_write_int32be(aldatablock * data, int offset, int word);

int aldatablock_write_byte(aldatablock * data, int offset, unsigned char value);

void aldatablock_dump(aldatablock * block );

void aldatablock_setcstring(aldatablock * block,char * cstring);

/**
 allocate needed continguous buffer of bytelength from alstrings_ringbuffer_pointer
 and fill data with that reference return a datablock that starts at offset 0.
 negative means reservation did not complete.
 **/
int alstrings_ringbuffer_reserve_datablock(alstrings_ringbuffer_pointer * ringbufferp, aldatablock * data, int bytelength);



#endif
