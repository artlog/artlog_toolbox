#ifndef _AL_STRING_H_
#define _AL_STRING_H_

#include "aldebug.h"

#define ALSTRINGBUFCANARY 0xbebedada

// type of one object (struct alhash_datablock ) within ( struct alstrings_ringbuffer )
enum altype {
  ALTYPE_OPAQUE=0, // opaque type meaning no encoding type known.
  ALTYPE_CINTLE=1, // NYI c integer in little endian, length is externaly defined in bytes 
  ALTYPE_CINTBE=2, // NYI c integer in big endian, length is externaly defined in bytes
  ALTYPE_STR0=3, // c string \0 terminated. its length includes the final '\0' byte.
  ALTYPE_SUBSTR=4, // c string NOT necessary \0 terminated, length does not include '\0' if any.
  ALTYPE_FLAG_EMBED=64, // NFI(not fully implemented) value is not a pointer in data.ptr but might be a number see aldatablock_embeded
  ALTYPE_MAX=128,
};

/** data pointing on a contiguous block of length bytes
  can be direct number data copy for integers 
**/
typedef struct {
  enum altype type;
  int length; // > 0 , EMPTY BLOCK NOT VALID
  union {
    void * ptr; // NULL value NOT VALID.
    unsigned char * ucharptr;
    char * charptr;
    const char * constcharptr;
    unsigned int * uintptr;
    long number; // quick hack to store values directly (see type ) ALTYPE_FLAG_EMBED
  } data;
} aldatablock;

struct alstrings_buffer {
  // internal buffer to collect data of allocated capacity bufsize.
  char * buf;
  // position of token buffer where to add data within buf
  int bufpos;
  // buffer size to be grown if needed ( see al_alstrings_ringbuffer_grow )
  int bufsize;
};

/** char buffer RING ( circular list allocated by al_alstrings_ringbuffer_alloc(int times); )
 can be filled ONLY ( no removal ).
 pointer should not move ( once allocated, can be moved ).
 can grow ( 2* growth ).

alstrings_ringbuffer circular linked list is built at first time
BUT buf and its size is allocated on request only.
= so initial *times* buckets drives limit =

remark initial name comes from json parser, kept to not rename everywhere
*/
typedef struct alstrings_ringbuffer {
  struct alstrings_buffer buffer;
  int canary; // bebedada
  // linked list
  // first always point to allocated first
  struct alstrings_ringbuffer * first;
  // next is circular 
  struct alstrings_ringbuffer * next;
  // next potentialy free and can be replaced by a next after if next is detected full.
  struct alstrings_ringbuffer * next_free;  
} alstrings_ringbuffer;

// introduced alstrings_ringbuffer_pointer , that point to right alstrings_ringbuffer within ring.
typedef alstrings_ringbuffer * alstrings_ringbuffer_pointer;

// allocation context ( it can only grow or be fully released )
struct alallocation_ctx {
  ALDEBUG_DEFINE_FLAG(debug)
  alstrings_ringbuffer_pointer ringbuffer;
};

#define ALALLOC(alloc_ctx, length) al_alloc_block(&alloc_ctx.ringbuffer, length)

// just does nothing on ctx... only ALFREECTX can be used, but it releases ALL.
// created to track possible replacement with malloc/free
#define ALRELEASE(alloc_ctx, object)
		  
#define ALFREECTX(alloc_ctx) alstrings_ringbuffer_release(&alloc_ctx->ringbuffer)

/* non NULL if grown, else miss
usualy double of size
search within list of possible buffers
it should not change existing allocation ( that are pointed to )
 */
alstrings_ringbuffer_pointer al_alstrings_ringbuffer_grow(alstrings_ringbuffer_pointer buffer, int length);

// allocate length byte within buffer, does not zero them ( keep content ).
// return pointer on first char in buffer
char * al_alloc_block(alstrings_ringbuffer_pointer * ringbuffer, int length);
  
// copy data content into alstrings_ringbuffer, return pointer on first char within buffer ( to update within data.ptr )
// WARNING this can actualy change ringbufferp content.
char * al_copy_block(alstrings_ringbuffer_pointer * ringbufferp,  aldatablock * data);

/* create a circular list of struct alstrings_ringbuffer of size times.
useful for autogrowth */
alstrings_ringbuffer_pointer al_alstrings_ringbuffer_alloc(int times);

/* allocate a circular buffer of buckets with initial bucket char length 
will setup ringbuffer
*/
void alstrings_ringbuffer_init_autogrow(alstrings_ringbuffer_pointer * ringbuffer, int buckets, int firstbucketlength);

// release ringbuffer when sure we are not pointing to any of its data
void alstrings_ringbuffer_release(alstrings_ringbuffer_pointer * ringbufferp);

// read an unsigned int that was stored in big endian at offset in datablock
unsigned int aldatablock_get_uint32be(aldatablock * data, int offset);

// read an unsigned int that was stored in little endian at offset in datablock
unsigned int aldatablock_get_uint32le(aldatablock * data, int offset);

// =====  use a datablock as output ========

void aldatablock_bzero(aldatablock * data,int offset, int length);

void aldatablock_fill_uchar(aldatablock * data,int offset, int length,unsigned char fill);

// return new offset 
int aldatablock_write_uint64be(aldatablock * data, int offset, unsigned long long value );

// return new offset 
int aldatablock_write_int32be(aldatablock * data, int offset, int word);

int aldatablock_write_byte(aldatablock * data, int offset, unsigned char value);

// FULLY deprecated use aloutput_bytes_as_hex(stream,block,1,8)
// void aldatablock_dump(aldatablock * block );

void aldatablock_setcstring(aldatablock * block,char * cstring);

/**
 allocate needed contiguous buffer of bytelength from alstrings_ringbuffer_pointer
 and fill data with that reference return a datablock that starts at offset 0.
 negative means reservation did not complete.
 **/
int alstrings_ringbuffer_reserve_datablock(alstrings_ringbuffer_pointer * ringbufferp, aldatablock * data, int bytelength);


/** 
will copy block pointed by str->data and convert it to STR0 if a SUBSTR
it means a final NUL is added to actualy make a printable string with %s
and return new char * of new string
WARNING alter str->data 
**/
char * alstrings_copy_str_block(alstrings_ringbuffer_pointer * ringbufferp, aldatablock * str);


/** return 0 if both a equal, else return something different than 0, not yet relevant for ordering*/
int alstrings_compare_str0_substr( aldatablock * str0,  aldatablock * substr);

/**
a is first char and b second
given a='f' and b='1' returns 0xf1 
**/
unsigned char alstrings_hex_to_byte(char a, char b);

/** 
recreate buffer if needed to newbufsize 
add char at end of buffer, increase pos
*/
int alstrings_buffer_add_char(struct alstrings_buffer * buffer, char c, int newbufsize);

/**
fully free buffer if non NULL
*/
void alstrings_buffer_flush(struct alstrings_buffer * buffer);

/**
grow it to newsize if needed
return old size, or -1 of realloc failed.
*/
int alstring_grow_buffer_if_needed(struct alstrings_buffer * buffer, int newsize);

/**
helper on ctring
if cstring txt is a prefix of keyword returns match else return miss
keywordlength should be smaller or equal to keyword cstring length
**/
int alstring_prefix(char * txt, char* keyword, int keywordlength, int match, int miss);

#endif
