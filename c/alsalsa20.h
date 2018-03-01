#ifndef __ALSALSA20_H__
#define __ALSALSA20_H__

#include "alstrings.h"

struct alsalsa_internal {
  // internal matrix for rounds
  unsigned int a[16];
  // key , k0 16 bytes or k0,k1 32 bytes 
  unsigned char k0[16];
  unsigned char k1[16];
  // nonce, 16 bytes ( in fact 8 real nonce + 8 index )
  unsigned char n[16];
  // internal expand constants σ τ are hardcoded in implementation.
};


void alsalsa20_init(struct alsalsa_internal * salsa);

void alsalsa20_doublerounds(struct alsalsa_internal * salsa, struct alhash_datablock * block,int rounds);
  
void alsalsa20_addblock(struct alsalsa_internal * salsa, struct alhash_datablock * block);
void alsalsa20_toblock(struct alsalsa_internal * salsa, struct alhash_datablock * block);

void alsalsa20_expand32(struct alsalsa_internal * salsa, unsigned char k0[16], unsigned char k1[16], unsigned char n[16]);

void alsalsa20_cryptinit32(struct alsalsa_internal * salsa, unsigned char key[32], unsigned char nonce[8]);

void alsalsa20_edcrypt_index(struct alsalsa_internal * salsa, unsigned long long index);

#endif // #ifndef __ALSALSA20_H__
