#ifndef __ALINPUTSTREAM_HEADER__
#define __ALINPUTSTREAM_HEADER__

#include "alstrings.h"
#include "aldebug.h"

struct alinputstream;

enum alinputstream_type {
  ALINPUTSTREAM_TYPE_FD,
  ALINPUTSTREAM_TYPE_SHARED,
  ALINPUTSTREAM_TYPE_SHARED_CHILD,
};

struct alinputstream_share_child {
  // when forked
  struct alinputstream * parent;
  // offset from mark in parent;
  int offset;
  // next child sharing same parent should be a ALINPUTSTREAM_TYPE_SHARED_CHILD
  // that should have an offset >= current
  struct alinputstream * next;
};

struct alinputstream_fd {
  // TODO
  int todo;
};

struct alinputstream {
  ALDEBUG_DEFINE_FLAG(debug)
  enum alinputstream_type type;
  int fd;
  int eof;
  int bits; // last bits read during last operation ( with eof )
  // memory inputstream.
  aldatablock input;
  // offset within input
  int offset;
  union {
    // when a child type
    struct alinputstream_share_child self;
    struct alinputstream * ptr;
  } child;
  int mark;
  int self_offset;
  struct alinputstream * next_chain;
};


ALDEBUG_DECLARE_FUNCTIONS(struct alinputstream,alinputstream)
			 
void alinputstream_init(struct alinputstream * stream, int fd);

/**
if 4 bytes can't be read, result eof will be set
and bits will be set to number of bits read ( a multiple of 8 obvioulsy )
in which case returned value is relevant for highter most bits
*/
unsigned int alinputstream_readuint32(struct alinputstream * stream);

int alinputstream_get_readbits(struct alinputstream * stream);

// WARNING 0 char considered as EOF.
// FIXME UGLY
unsigned char alinputstream_readuchar(struct alinputstream * stream);

void alinputstream_foreach_block(
				 struct alinputstream * stream,
				 int blocksize,
				 void (*callback) (aldatablock * block, void * data),
				 void (*finalize) (aldatablock * block, void * data),
				 void * data);

/** will read in memory from datablock starting at offset byte */
void alinputstream_setdatablock(struct alinputstream * stream, aldatablock * block, int offset);

/**
create a stream that starts at the very same place but detached from parent
currently usable only with 
alinputstream_shared_readuchar(struct alinputstream * stream)
**/
struct alinputstream * alinputstream_create_mark_shared(struct alinputstream * parent, int blocksize);

// when child usage is complete, release it to cleanup parent buffer
void alinputstream_free_shared(struct alinputstream * child);

unsigned char alinputstream_shared_readuchar(struct alinputstream * childstream);

/** create a chained input, read current, then read next 
 **/
struct alinputstream * alinputstream_create_chain(struct alinputstream * current, struct alinputstream * next);

/** return 1 is stream is eof */
int alinputstream_iseof(struct alinputstream * stream);

#endif
