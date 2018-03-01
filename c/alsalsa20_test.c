#include "alsalsa20.h"
#include <stddef.h>
#include <string.h>

/*
doubleround(0x00000001, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000,
0x00000000, 0x00000000, 0x00000000, 0x00000000)
= (0x8186a22d, 0x0040a284, 0x82479210, 0x06929051,
0x08000090, 0x02402200, 0x00004000, 0x00800000,
0x00010200, 0x20400000, 0x08008104, 0x00000000,
0x20500000, 0xa0000040, 0x0008180a, 0x612a8020).
*/

unsigned int test1[] = {
  0x00000001, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x00000000, 0x00000000, 0x00000000, 0x00000000
};

unsigned int test1_result[] = {
  0x8186a22d, 0x0040a284, 0x82479210, 0x06929051,
  0x08000090, 0x02402200, 0x00004000, 0x00800000,
  0x00010200, 0x20400000, 0x08008104, 0x00000000,
  0x20500000, 0xa0000040, 0x0008180a, 0x612a8020
};

/*
doubleround(0xde501066, 0x6f9eb8f7, 0xe4fbbd9b, 0x454e3f57,
0xb75540d3, 0x43e93a4c, 0x3a6f2aa0, 0x726d6b36,
0x9243f484, 0x9145d1e8, 0x4fa9d247, 0xdc8dee11,
0x054bf545, 0x254dd653, 0xd9421b6d, 0x67b276c1)
= (0xccaaf672, 0x23d960f7, 0x9153e63a, 0xcd9a60d0,
0x50440492, 0xf07cad19, 0xae344aa0, 0xdf4cfdfc,
0xca531c29, 0x8e7943db, 0xac1680cd, 0xd503ca00,
0xa74b2ad6, 0xbc331c5c, 0x1dda24c7, 0xee928277).
*/

unsigned int test2[] = {
  0xde501066, 0x6f9eb8f7, 0xe4fbbd9b, 0x454e3f57,
  0xb75540d3, 0x43e93a4c, 0x3a6f2aa0, 0x726d6b36,
  0x9243f484, 0x9145d1e8, 0x4fa9d247, 0xdc8dee11,
  0x054bf545, 0x254dd653, 0xd9421b6d, 0x67b276c1
};

unsigned int test2_result[] = {
  0xccaaf672, 0x23d960f7, 0x9153e63a, 0xcd9a60d0,
  0x50440492, 0xf07cad19, 0xae344aa0, 0xdf4cfdfc,
  0xca531c29, 0x8e7943db, 0xac1680cd, 0xd503ca00,
  0xa74b2ad6, 0xbc331c5c, 0x1dda24c7, 0xee928277
};

unsigned int * tests[][2] = {
  {test1,test1_result},
  {test2,test2_result}
};

unsigned char hashtest1[] = {
  211,159, 13,115, 76, 55, 82,183, 3,117,222, 37,191,187,234,136,
  49,237,179, 48, 1,106,178,219,175,199,166, 48, 86, 16,179,207,
  31,240, 32, 63, 15, 83, 93,161,116,147, 48,113,238, 55,204, 36,
  79,201,235, 79, 3, 81,156, 47,203, 26,244,243, 88,118,104, 54
};

unsigned char hashtest1_result[] = {
  109, 42,178,168,156,240,248,238,168,196,190,203, 26,110,170,154,
  29, 29,150, 26,150, 30,235,249,190,163,251, 48, 69,144, 51, 57,
  118, 40,152,157,180, 57, 27, 94,107, 42,236, 35, 27,111,114,114,
  219,236,232,135,111,155,110, 18, 24,232, 95,158,179, 19, 48,202
};



/*
{101,120,112, 97, 1, 2, 3, 4,
5, 6, 7, 8,
9, 10, 11, 12,
13, 14, 15, 16,110,100, 32, 51,101,102,103,104,105,106,107,108,
109,110,111,112,113,114,115,116, 50, 45, 98,121,201,202,203,204,
 205,206,207,208,209,210,211,212,213,214,215,216,116,101, 32,107}
*/

unsigned char expand32test1_result[] = 
  { 69, 37, 68, 39, 41, 15,107,193,255,139,122, 6,170,233,217, 98,
    89,144,182,106, 21, 51,200, 65,239, 49,222, 34,215,114, 40,126,
    104,197, 7,225,197,153, 31, 2,102, 78, 76,176, 84,245,246,184,
    177,160,133,130, 6, 72,149,119,192,195,132,236,234,103,246, 74};

unsigned char expand16test1_result[] = 
  { 9,173, 46,248, 30,200, 82, 17, 48, 67,254,239, 37, 18, 13,247,
    241,200, 61,144, 10, 55, 50,185, 6, 47,246,253,143, 86,187,225,
    134, 85,110,246,161,163, 43,235,231, 94,171, 51,145,214,112, 29,
    14,232, 5, 16,151,140,183,141,171, 9,122,181,104,182,177,193};

void build_expand32_test1(  struct alhash_datablock * out)
{
  struct alsalsa_internal salsa;
  
  unsigned char k0[16];
  unsigned char k1[16];
  unsigned char n[16];

  for (int i=0; i<16;i++)
    {
      k0[i]=i+1;
      k1[i]=i+201;
      n[i]=i+101;
    }

  alsalsa20_expand32(&salsa,k0,k1,n);
  alsalsa20_toblock(&salsa,out);
}


int main(int argc, char ** argv)
{
  struct alhash_datablock block;
  unsigned int outdata[16];
  struct alhash_datablock out;
  struct alsalsa_internal salsa;

  out.type=ALTYPE_OPAQUE;
  out.length=sizeof(outdata);
  out.data.uintptr=&outdata[0];
    
  // test internal doubleround
  for (int testn = 0; testn < 2; testn++ )
    {
      block.type=ALTYPE_OPAQUE;
      block.length=sizeof(test1);
      block.data.uintptr=&tests[testn][0][0];
      alsalsa20_doublerounds(&salsa,&block,1);

      alsalsa20_toblock(&salsa,&out);

      for (int i=0; i< 16; i++ )
	{
	  if ( tests[testn][1][i] != out.data.uintptr[i] )
	    {
	      aldebug_printf(NULL,"[FATAL] mismatch in test %i value %i %08x!=%08x\n",testn, i, tests[testn][1][i],out.data.uintptr[i] );
	    }
	}

      aldatablock_dump(&out);
    }

  // test hash on little endian system
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    block.type=ALTYPE_OPAQUE;
    block.length=sizeof(hashtest1);
    block.data.uintptr=(unsigned int *) &hashtest1[0];      
    alsalsa20_addblock(&salsa,&block);
    alsalsa20_toblock(&salsa,&out);
    aldatablock_dump(&out);

    if ( memcmp( hashtest1_result, out.data.ucharptr, out.length) != 0 )
      {
      aldebug_printf(NULL,"[FATAL] mismatch between expected salsa20 hashed value and computed\n");
    }
    else
      {
      aldebug_printf(NULL,"salsa20 hashes match ( where usualy matches create ashes )\n");
    }
#else
    aldebug_printf(NULL,"[ERROR] salsa20 hashes not yet coded for big endian");
#endif

    build_expand32_test1(&out);


    if ( memcmp( expand32test1_result, out.data.ucharptr, out.length) != 0 )
      {
  aldebug_printf(NULL,"[FATAL] mismatch between expected salsa20 hashed value and computed\n");
  aldatablock_dump(&out);
  for (int i =0; i < out.length; i++)
    {
	unsigned int a = (unsigned int) out.data.ucharptr[i];
	aldebug_printf(NULL,"%u ", a);	
      }
    aldebug_printf(NULL,"\n");
 
}
    else
      {
  aldebug_printf(NULL,"salsa20 expand32 match\n");
}    

    
    return 0;
}
