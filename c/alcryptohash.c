#include "alcryptohash.h"
#include "alstrings.h"
#include "altodo.h"
#include <string.h>
#include <limits.h>

// somed defines to ease RFC -> C code conversion
#define OR |
#define AND &
#define XOR ^
#define NOT ~

#define wordsize 32
#define ROTR(n,x) ((x>>n) OR (x<<(wordsize-n)))

#define SHR(n,x) (x>>n)

// 5.1.  SHA-224 and SHA-256

#define CH( x, y, z) ((x AND y) XOR ( (NOT x) AND z))

#define MAJ( x, y, z) ((x AND y) XOR (x AND z) XOR (y AND z))

#define BSIG0(x) (ROTR(2,x) XOR ROTR(13,x) XOR ROTR(22,x))

#define BSIG1(x) (ROTR(6,x) XOR ROTR(11,x) XOR ROTR(25,x))

#define SSIG0(x) (ROTR(7,x) XOR ROTR(18,x) XOR SHR(3,x))

#define SSIG1(x) (ROTR(17,x) XOR ROTR(19,x) XOR SHR(10,x))

// Initialize hash values:
// (first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19):
unsigned int sha256_H0[8] = {
  0x6a09e667,
  0xbb67ae85,
  0x3c6ef372,
  0xa54ff53a,
  0x510e527f,
  0x9b05688c,
  0x1f83d9ab,
  0x5be0cd19,
};

/* SHA224
      H(0)0 = c1059ed8
      H(0)1 = 367cd507
      H(0)2 = 3070dd17
      H(0)3 = f70e5939
      H(0)4 = ffc00b31
      H(0)5 = 68581511
      H(0)6 = 64f98fa7
      H(0)7 = befa4fa4
*/
unsigned int sha224_H0[8] = {
  0xc1059ed8,
  0x367cd507,
  0x3070dd17,
  0xf70e5939,
  0xffc00b31,
  0x68581511,
  0x64f98fa7,
  0xbefa4fa4
};

// Initialize array of round constants:
// (first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311):
// k[0..63] :=
unsigned int K[] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

ALDEBUG_DEFINE_FUNCTIONS(struct alsha2_internal, alsha2x, debug);

// shaxxx applied depends on result length.
void alshax_internal_init(struct alsha2_internal * intern, unsigned int sha_H[8])
{
  bzero(intern, sizeof(struct alsha2_internal));
  intern->cumulated_length = 0;
  intern->missing_bits=0;
  memcpy(intern->H, sha_H, sizeof(intern->H));
  intern->state = AL_SHA2_INIT;

  intern->input.data.ptr=NULL;
  intern->input.length=0;
  intern->algorithm=AL_SHA2_UNKNOWN_ALGORITHM;
    
  aldatablock d;
  d.length = sizeof(intern->H);
  d.data.ptr = intern->H;
  d.type = ALTYPE_OPAQUE; 
  // aldatablock_dump(&d);

}

void alsha224_init(struct alsha2_internal * intern )
{
  alshax_internal_init(intern,sha224_H0);
  intern->algorithm=AL_SHA224;
}

void alsha256_init(struct alsha2_internal * intern )
{
  alshax_internal_init(intern,sha256_H0);
  intern->algorithm=AL_SHA256;
}

void alsha2x_init(struct alsha2_internal * intern, enum alsha2_algorithm algorithm)
{
  switch (algorithm)
    {
    case AL_SHA224:
      alsha224_init(intern);
      break;
    case AL_SHA256:
      alsha256_init(intern);
      break;
    default:      
      aldebug_printf(NULL,"[FATAL] sha2 algorithm %i not supported", algorithm);
      intern->algorithm=algorithm;
      break;
    }
}

// padding is done on end of message and with full length of message, assume smallest entity CHAR_BIT ( assumed 8 bits )
// full message lenght is stored into intern->cumulated_length
// this can be done for last 512bits block but we have to record full message length.
/**
last_block is output
**/
void alsha2_pad_to_512bits(struct alsha2_internal * intern, aldatablock * last_block)
{
  if ( intern == NULL )
    {
      aldebug_printf(NULL,"[ERROR] alsha2_pad_to_512bit NULL intern\n");
      return;
    }

  // L in bits
  long long bitlength = ( (long long) intern->cumulated_length * CHAR_BIT ) - intern->missing_bits;
  long long L = ( bitlength  ) % 512;
  aldatablock * output = last_block;

  ALDEBUG_IF_DEBUG(intern,alsha2x,debug)
    {
      aldebug_printf(NULL,"padding cumulated length %lu\n", intern->cumulated_length );
    }

  
  // 0 <= L < 512 ; and L % 8 == 0 because usualy CHAR_BIT = 8 and we are playing with bytes.
  if ( L < 448 )
    {
      // ( L + 1 + K ) = 448
      long long K = 447 - L;
      
      if ( K > 0 )
	{
	  ALDEBUG_IF_DEBUG(intern,alsha2x,debug)
	    {
	      aldebug_printf(NULL,"last block zeroed at %i length %i\n", L / CHAR_BIT , ((512 - L) / CHAR_BIT) );
	    }
	  aldatablock_bzero(output,L / CHAR_BIT, (512 - L) / CHAR_BIT);
	}
      // 1 bit on an aligned byte.
      aldatablock_write_byte(output, L / CHAR_BIT, 0x80);

      // L is stored in 64 bits big endian representation at end of 512 bits block  
      aldatablock_write_uint64be(output, 448 / CHAR_BIT, bitlength);
      
      intern->state = AL_SHA2_PADDED;
    }
  else
    {
      if ( intern->state == AL_SHA2_DATA )
	{
	  // should be padded on two blocks, first step is to pad on first block.
	  int offset = L/CHAR_BIT;
	  aldatablock_bzero(output,offset,64-offset);
	  // 1 bit on an aligned byte.
	  aldatablock_write_byte(output, L / CHAR_BIT, 0x80);

	  intern->state = AL_SHA2_PAD;
	}
      else if ( intern->state == AL_SHA2_PAD )
	{
	  // first padding ( see above ) was done, we are called again for last.
	  // second step pad on second block
	  aldatablock_bzero(output,0,64);
	  // length is stored in 64 bits big endian representation at end of this second 512 bits block  
	  aldatablock_write_uint64be(output, 448 / CHAR_BIT, bitlength);

	  intern->state = AL_SHA2_PADDED;
	}
      else {
	aldebug_printf(NULL,"sha2x padding called in wrong state %i", intern->state);
      }

    }

  ALDEBUG_IF_DEBUG(intern,alsha2x,debug)
    {
      aldebug_printf(NULL,"last block padded state %i length %i\n", intern->state, intern->cumulated_length);
      aldatablock_dump(output);
    }

}



// sha224 on input starting at offset ( same as sha256 )
void alsha224_turn(struct alsha2_internal * shainternal, int offset, aldatablock * input)
{
  unsigned int W[64];
  unsigned int H[8];
  int t;

  ALDEBUG_IF_DEBUG(shainternal,alsha2x,debug)
    {
      aldebug_printf(NULL,"alhash turn on state %i offset %i\n", shainternal->state, offset);
      aldatablock_dump(input);
    }

  memcpy(H,shainternal->H, sizeof(H));
  
  /*
        1. Prepare the message schedule W:
         For t = 0 to 15
            Wt = M(i)t
         For t = 16 to 63
            Wt = SSIG1(W(t-2)) + W(t-7) + SSIG0(w(t-15)) + W(t-16)
  */
  for ( t = 0; t<16; t++)
    {
      W[t]=aldatablock_get_uint32be(input, offset + (t*sizeof(int)));
      ALDEBUG_IF_DEBUG(shainternal,alsha2x,debug)
	{
	  aldebug_printf(NULL,"alhash W[%i]=%x\n", t, W[t]);
	}
    }
  for (t = 16; t < 64; t++)
    {
      W[t] =  SSIG1(W[t-2]) + W[t-7] + SSIG0(W[t-15]) + W[t-16];
    }

  /*
        2. Initialize the working variables:
         a = H(i-1)0
         b = H(i-1)1
         c = H(i-1)2
         d = H(i-1)3
         e = H(i-1)4
         f = H(i-1)5
         g = H(i-1)6
         h = H(i-1)7
  */
  unsigned int a,b,c,d,e,f,g,h;

  a = H[0];
  b = H[1];
  c = H[2];
  d = H[3];
  e = H[4];
  f = H[5];
  g = H[6];
  h = H[7];

  /*
        3. Perform the main hash computation:
         For t = 0 to 63 
            T1 = h + BSIG1(e) + CH(e,f,g) + Kt + Wt
            T2 = BSIG0(a) + MAJ(a,b,c)
            h = g
            g = f
            f = e
            e = d + T1
            d = c
            c = b
            b = a
            a = T1 + T2
  */
  for (t=0; t<64;t++)
    {
      unsigned int T1 = h + BSIG1(e) + CH(e,f,g) + K[t] + W[t];
      unsigned int T2 = BSIG0(a) + MAJ(a,b,c);
      h = g;
      g = f;
      f = e;
      e = d + T1;
      d = c;
      c = b;
      b = a;
      a = T1 + T2;
    }

  /*
        4. Compute the intermediate hash value H(i)
         H(i)0 = a + H(i-1)0
         H(i)1 = b + H(i-1)1
         H(i)2 = c + H(i-1)2
         H(i)3 = d + H(i-1)3
         H(i)4 = e + H(i-1)4
         H(i)5 = f + H(i-1)5
         H(i)6 = g + H(i-1)6
         H(i)7 = h + H(i-1)7
  */

  H[0] = a + H[0];
  H[1] = b + H[1];
  H[2] = c + H[2];
  H[3] = d + H[3];
  H[4] = e + H[4];
  H[5] = f + H[5];
  H[6] = g + H[6];
  H[7] = h + H[7];

  memcpy(shainternal->H, H, sizeof(H));

}
 
// shaxxx applied depends on result length.
int alsha2x_add_block( struct alsha2_internal * intern, aldatablock * input)
{

  // for all blocks of blocksizebyte
  if ( intern != NULL )
    {
      if ( (input != NULL) && (input->length > 0 ) )
	{
	  // offset within input
	  int offset = 0;
	  // 64 bytes ( 512 bits ).
	  int blocksizebyte=64;
	  // number of bytes to copy
	  int tocopy = 0;	  
	  // how many bytes do we need to fill a block
	  int missing = 0;

	  intern->cumulated_length += input->length;
      
	  // input was not flushed, remains incomplete block to complete.
	  if ( intern->input.length > 0 )
	    {
	      ALDEBUG_IF_DEBUG(intern,alsha2x,debug)
		{
		  aldebug_printf(NULL, "input was not flushed, remains incomplete block of length %i\n",  intern->input.length );
		}
	      if ( intern->input.data.charptr == NULL )
		{
		  aldebug_printf(NULL,"[FATAL] NULL intern buffer");
		}
	    
	      missing = blocksizebyte - intern->input.length;
	      if ( input->length > offset + missing )
		{
		  tocopy=missing;
		}
	      else
		{
		  tocopy=input->length - offset;
		}
	      if ( tocopy > 0 )
		{
		  memcpy(&intern->input.data.charptr[intern->input.length], &(input->data.charptr[offset]), tocopy);
		  offset+=tocopy;
		  missing-=tocopy;
		  intern->input.length += tocopy;
		}
	      // still after filling from input not enough data ... => need next.
	      if ( missing > 0 )
		{
		  return missing;
		}	  
	      if ( intern->input.length != blocksizebyte )
		{
		  aldebug_printf(NULL,"[FATAL] reconstructed block of invalid size %i \n", intern->input.length);	      
		}
	      // compute from internal reconstructed block
	      alsha224_turn(intern, 0, &intern->input);
	      intern->input.length = 0;
	      intern->input.data.ptr = NULL;
	    }

	  // compute directly on input with right aligned blocksize
	  while ( input->length >= ( offset + blocksizebyte ) )
	    {
	      alsha224_turn(intern, offset, input);
	      offset += blocksizebyte;
	    }

	  // should keep remaining until filled by another add or alsha2x_final
	  int remaining = input->length - offset;
	  if ( remaining > 0)
	    {
	      missing = blocksizebyte - remaining;
	      intern->input.data.charptr=intern->inputcopy;
	      memcpy(&intern->input.data.charptr[0], &(input->data.charptr[offset]), remaining);
	      intern->input.length = remaining;
	      return missing;
	    }
	  else
	    {
	      intern->input.data.ptr=NULL;
	      intern->input.length = 0;
	      return 0;
	    }
	}
    }
}

// shaxxx applied depends on result length.
aldatablock * alsha2x_final(struct alsha2_internal * intern)
{
  // 64 bytes ( 512 bits ).
  int blocksizebyte=64;

  if (intern->state == AL_SHA2_INIT)
    {
      // special case of empty hash
      intern->state =  AL_SHA2_DATA;
    }
  

  if ( intern->state ==  AL_SHA2_DATA )
    {
      if ( intern->input.length >=  blocksizebyte )
	{
	  // internal input length should be < blocksizebyte due to add block behavior.
	  aldebug_printf(NULL,"[FATAL] input buffer at final of length %i >= blocksize %i\n", intern->input.length, blocksizebyte);
	}
      
      // remaining incomplete buffer at end.
      if ( (intern->input.length != 0 ) && ( intern->input.length < blocksizebyte ))
	{
	  ALDEBUG_IF_DEBUG(intern,alsha2x,debug)
	    {
	      aldebug_printf(NULL,"[WARNING] non empty input buffer at final of length %i\n", intern->input.length);
	    }
	  if ( intern->input.data.ptr !=  intern->inputcopy )
	    {
	      aldebug_printf(NULL,"[FATAL] non empty input buffer at final of length %i  using an external data buffer\n", intern->input.length);
	    }	  
	  // expand it to full block, will be padded.
	  intern->input.length = blocksizebyte;
	}
      
      if ( intern->input.length == 0 )
	{
	  // use pad 
	  intern->input.data.charptr=intern->pad;
	  intern->input.length = blocksizebyte;
	}
      alsha2_pad_to_512bits(intern, &intern->input);
      alsha224_turn(intern, 0, &intern->input);
    }

  if ( intern->state ==  AL_SHA2_PAD )
    {
      if ( intern->input.length != blocksizebyte )
	{
	  aldebug_printf(NULL,"[ERROR] unexpected non empty input buffer of length %i at second padding final block\n", intern->input.length);
	}
      else
	{
	  // use extra pad
	  intern->input.data.charptr=intern->extrapad;
	  intern->input.length = blocksizebyte;
	}
      alsha2_pad_to_512bits(intern, &intern->input);
      alsha224_turn(intern, 0, &intern->input);
    }

  if ( intern->state ==  AL_SHA2_PADDED )
    {
      intern->state = AL_SHA2_COMPLETE;
    }
  else
    {
      aldebug_printf(NULL,"intern state wrong after padding %i\n", intern->state);
    }
   

  // return pointer over internal H
  intern->output.type=ALTYPE_OPAQUE;
  intern->output.data.ptr=&intern->H;
  intern->output.length=sizeof(intern->H);
  return &intern->output;
}

