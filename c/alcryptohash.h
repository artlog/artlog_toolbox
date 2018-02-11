#ifndef __AL_CRYPTOHASH_H__
#define __AL_CRYPTOHASH_H__

#include "alinput.h"
#include "alstrings.h"

/*
RFC 6234

   SHA-224 [RFC3874],
   SHA-256, SHA-384, and SHA-512
*/

enum alsha2_state {
  AL_SHA2_INIT,
  AL_SHA2_DATA,
  AL_SHA2_PAD,  
  AL_SHA2_PADDED,
  AL_SHA2_COMPLETE
};

#define MAXBLOCSIZEBYTE 64

struct alsha2_internal {
  // the H !
  unsigned int H[8];
  enum alsha2_state state;
  int cumulated_length; // in byte ( bits / 8 ).  
  struct alhash_datablock input;
  struct alhash_datablock output;
  // when direct input can't be used as a 512bits block
  char inputcopy[MAXBLOCSIZEBYTE];
  char pad[MAXBLOCSIZEBYTE];
  char extrapad[MAXBLOCSIZEBYTE];
};



void alsha2x_init(struct alsha2_internal * intern);

// shaxxx applied depends on result length.
// return number of byte that are need to complete last uncompleted block. 0 if block was complete.
int alsha2x_add_block(struct alsha2_internal * intern, struct alhash_datablock * block); 

// return a pointer over a buffer containing hash.
struct alhash_datablock * alsha2x_final(struct alsha2_internal * intern);

void alsha2x_from_stream(struct alhash_datablock * result,struct alinputstream * input);

#endif // #ifndef __AL_CRYPTOHASH_H__