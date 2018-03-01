#include "alsalsa20.h"
#include "altodo.h"
#include <string.h>

#define wordsize 32
#define ROTL(x,n) ((x<<n) | (x>>(wordsize-n)))


// https://en.wikipedia.org/wiki/Salsa20
// Salsa20 specification Daniel J. Bernstein

// doubleround(x) = rowround(columnround(x)).
void alsalsa_doubleround(struct alsalsa_internal * salsa)
{

  unsigned int * x = &salsa->a[0];

  /* matrix 
   x[0] x[1] x[2] x[3]
   x[4] x[5] x[6] x[7]
   x[8] x[9] x[a] x[b]
   x[c] x[d] x[e] x[f]
  */

  // quarter round
  // ( A + B ) is in fact ( a + b ) mod 32 since using 32bits numbers.

  // columnround
  // quarterround                      quarterround
  x[ 4] ^= ROTL((x[ 0] + x[12]),7);    x[ 9] ^= ROTL((x[ 5] + x[ 1]),7);
  x[14] ^= ROTL((x[10] + x[ 6]),7);    x[ 3] ^= ROTL((x[15] + x[11]),7);
  x[ 8] ^= ROTL((x[ 4] + x[ 0]),9);    x[13] ^= ROTL((x[ 9] + x[ 5]),9);
  x[ 2] ^= ROTL((x[14] + x[10]),9);    x[ 7] ^= ROTL((x[ 3] + x[15]),9);
  x[12] ^= ROTL((x[ 8] + x[ 4]),13);   x[ 1] ^= ROTL((x[13] + x[ 9]),13);
  x[ 6] ^= ROTL((x[ 2] + x[14]),13);   x[11] ^= ROTL((x[ 7] + x[ 3]),13);
  x[ 0] ^= ROTL((x[12] + x[ 8]),18);   x[ 5] ^= ROTL((x[ 1] + x[13]),18);
  x[10] ^= ROTL((x[ 6] + x[ 2]),18);   x[15] ^= ROTL((x[11] + x[ 7]),18);

  // rowround
  // quarterround                      quarterround
  x[ 1] ^= ROTL((x[ 0] + x[ 3]),7);    x[ 6] ^= ROTL((x[ 5] + x[ 4]),7);
  x[11] ^= ROTL((x[10] + x[ 9]),7);    x[12] ^= ROTL((x[15] + x[14]),7);
  x[ 2] ^= ROTL((x[ 1] + x[ 0]),9);    x[ 7] ^= ROTL((x[ 6] + x[ 5]),9);
  x[ 8] ^= ROTL((x[11] + x[10]),9);    x[13] ^= ROTL((x[12] + x[15]),9);
  x[ 3] ^= ROTL((x[ 2] + x[ 1]),13);   x[ 4] ^= ROTL((x[ 7] + x[ 6]),13);
  x[ 9] ^= ROTL((x[ 8] + x[11]),13);   x[14] ^= ROTL((x[13] + x[12]),13);
  x[ 0] ^= ROTL((x[ 3] + x[ 2]),18);   x[ 5] ^= ROTL((x[ 4] + x[ 7]),18);
  x[10] ^= ROTL((x[ 9] + x[ 8]),18);   x[15] ^= ROTL((x[14] + x[13]),18);
  
}

void alsalsa20_init(struct alsalsa_internal * salsa)
{
  todo("salsa20_init");
}

void alsalsa20_doublerounds_internal(struct alsalsa_internal * salsa, int rounds)
{
  for (int round = 0; round < rounds; round ++ )
    {
      alsalsa_doubleround( salsa);      
    } 
}


void alsalsa20_doublerounds(struct alsalsa_internal * salsa, struct alhash_datablock * block,int rounds)
{
  // aldebug_printf(NULL,"salsa20_addblock feed block into parameter");
  if ( block->length >= sizeof(int) * 16 )
    {
      for (int n = 0; n < 16; n++)
	{
	  salsa->a[n] = block->data.uintptr[n];
	}
    }
  alsalsa20_doublerounds_internal(salsa,rounds);
}

void alsalsa20_addblock(struct alsalsa_internal * salsa, struct alhash_datablock * block)
{
  // Salsa20(x) = x + doubleround^10 (x),
  alsalsa20_doublerounds(salsa, block, 10);

  // aldebug_printf(NULL,"mix block input with salsa content");
  if ( block->length >= sizeof(int) * 16 )
    {
      for (int n = 0; n < 16; n++)
	{
	  salsa->a[n] += block->data.uintptr[n];
	}
    }

}

void alsalsa20_toblock(struct alsalsa_internal * salsa, struct alhash_datablock * block)
{
  if ( block->length >= sizeof(int) * 16 )
    {
      for (int n = 0; n < 16; n++)
	{
	  block->data.uintptr[n]=salsa->a[n];
	}
    }  
}

/*
Define σ 0 = (101, 120, 112, 97), σ 1 = (110, 100, 32, 51), σ 2 = (50, 45, 98, 121), and σ 3 = (116, 101, 32, 107). If k 0 , k 1 , n are 16-byte sequences then Salsa20 k 0 ,k 1 (n) =
Salsa20(σ 0 , k 0 , σ 1 , n, σ 2 , k 1 , σ 3 ).
Define τ 0 = (101, 120, 112, 97), τ 1 = (110, 100, 32, 49), τ 2 = (54, 45, 98, 121),
and τ 3 = (116, 101, 32, 107). If k, n are 16-byte sequences then Salsa20 k (n) =
Salsa20(τ 0 , k, τ 1 , n, τ 2 , k, τ 3 ).
*/

// "expand 32-byte k"
unsigned char o[4][4] = {
  {101, 120, 112, 97},
  {110, 100, 32, 51},
  {50, 45, 98, 121},
  {116, 101, 32, 107}
};

// “expand 16-byte k”
unsigned char t[4][4] = {
  {101, 120, 112, 97},
  {110, 100, 32, 49},
  {54, 45, 98, 121},
  {116, 101, 32, 107}
};

void alsalsa20_expand32(struct alsalsa_internal * salsa, unsigned char k0[16], unsigned char k1[16], unsigned char n[16])
{
  // o[0],k0,o[1],n,o[2],k1,o[3]
  
  unsigned int x[16];
  struct alhash_datablock block;

  block.length = sizeof(x);
  block.type= ALTYPE_OPAQUE;
  block.data.uintptr=&x[0];
  

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  x[0] = *( (unsigned int *) &o[0] );
  x[1] = *( (unsigned int *) &k0[0] );
  x[2] = *( (unsigned int *) &k0[4] );
  x[3] = *( (unsigned int *) &k0[8] );
  x[4] = *( (unsigned int *) &k0[12] );
  x[5] = *( (unsigned int *) &o[1] );
  x[6] = *( (unsigned int *) &n[0] );
  x[7] = *( (unsigned int *) &n[4] );
  x[8] = *( (unsigned int *) &n[8] );
  x[9] = *( (unsigned int *) &n[12] );
  x[10] = *( (unsigned int *) &o[2] );
  x[11] = *( (unsigned int *) &k1[0] );
  x[12] = *( (unsigned int *) &k1[4] );
  x[13] = *( (unsigned int *) &k1[8] );
  x[14] = *( (unsigned int *) &k1[12] );
  x[15] = *( (unsigned int *) &o[3] );
#else
  todo("[ERROR] salsa20 expand32 not yet coded for big endian");
#endif

  /*
  for (int i =0; i < 16; i++)
    {
      for (int j=0; j < 4; j++)
	{
	  unsigned int a = (unsigned int) ((unsigned char *) &(x[i]))[j];
	  aldebug_printf(NULL,"%u ", a);
	}
    }
  aldebug_printf(NULL,"\n");
  */

  alsalsa20_addblock(salsa,&block);
  
}

void alsalsa20_expand16(struct alsalsa_internal * salsa, unsigned char k[16], unsigned char n[16])
{
  // t[0],k,t[1],n,t[2],k,t[3]
  unsigned int * x = &salsa->a[0];
  //unsigned int x[16];
  
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  x[0] = *( (unsigned int *) &t[0] );
  x[1] = *( (unsigned int *) &k[0] );
  x[2] = *( (unsigned int *) &k[4] );
  x[3] = *( (unsigned int *) &k[8] );
  x[4] = *( (unsigned int *) &k[12] );
  x[5] = *( (unsigned int *) &t[1] );
  x[6] = *( (unsigned int *) &n[0] );
  x[7] = *( (unsigned int *) &n[4] );
  x[8] = *( (unsigned int *) &n[8] );
  x[9] = *( (unsigned int *) &n[12] );
  x[10] = *( (unsigned int *) &t[2] );
  x[11] = *( (unsigned int *) &k[0] );
  x[12] = *( (unsigned int *) &k[4] );
  x[13] = *( (unsigned int *) &k[8] );
  x[14] = *( (unsigned int *) &k[12] );
  x[15] = *( (unsigned int *) &t[3] );
#else
  todo("[ERROR] salsa20 expand16 not yet coded for big endian");
#endif

  alsalsa20_doublerounds_internal(salsa,10);
}

void alsalsa20_edcrypt_index(struct alsalsa_internal * salsa, unsigned long long index)
{

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  // index is higher part of nonce.
  *((unsigned long long *) &salsa->n[8]) = index;
  alsalsa20_expand32(salsa, salsa->k0, salsa->k1, salsa->n);
#else
  todo("[ERROR] salsa20 edcrypt index not yet coded for big endian");
#endif
  
}

void alsalsa20_cryptinit16(struct alsalsa_internal * salsa, unsigned char key[16], unsigned char nonce[8])
{
  todo("NYI set a type since it will use expand16");
  memcpy(salsa->k0,&key[0],16);
  memcpy(salsa->n,&nonce,8);
}

void alsalsa20_cryptinit32(struct alsalsa_internal * salsa, unsigned char key[32], unsigned char nonce[8])
{
  memcpy(salsa->k0,&key[0],16);
  memcpy(salsa->k1,&key[16],16);
  memcpy(salsa->n,&nonce,8);
}
