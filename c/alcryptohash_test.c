#include "alcryptohash.h"
#include "altodo.h"
#include "aldebug.h"
#include "alinput.h"
#include <stdlib.h>
#include <stdio.h>

void dump_result(  struct alhash_datablock * result)
{
  if ( result == NULL)
    {
      aldebug_printf(NULL,"[FATAL] NULL shax \n");
    }
  else
    {
      aldebug_printf(NULL,"[INFO] SHAX result \n");
      aldatablock_dump(result);
      unsigned int * h = result->data.uintptr;
      // sha224
      printf("%08x%08x%08x%08x%08x%08x%08x\n",h[0],h[1],h[2],h[3],h[4],h[5],h[6]);
    }

}

void alshash_callback (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      aldebug_printf(NULL,"[INFO] ADD block size %i \n", block->length);
      aldatablock_dump(block);      
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      alsha2x_add_block(shax,block);
    }
}

void alshash_finalize (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      aldebug_printf(NULL,"[INFO] FINAL block size %i \n", block->length);
      aldatablock_dump(block);
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      if ( block->length > 0)
	{
	  alsha2x_add_block(shax,block);
	}
      struct alhash_datablock * result;
      result = alsha2x_final(shax);

      dump_result(result);
    }
}

void test_empty_hash()
{
  struct alsha2_internal shax;
  struct alhash_datablock * result;

  // sha224 yet
  alsha2x_init(&shax);

  result = alsha2x_final(&shax);

  dump_result(result);
}
		     

int main(int argc, char ** argv)
{
  todo("call some crypto hash here ...");

  if ( argc > 1 )
    {
      char * filename = argv[1];
      FILE * f = fopen(filename,"r");
      if ( f != NULL )
	{
	  struct alinputstream input;
	  struct alsha2_internal sha2x ;

	  alsha2x_init(&sha2x);
	    
	  aldebug_printf(NULL,"open file %s\n", filename );
	  alinputstream_init(&input, fileno(f));
	  alinputstream_foreach_block(&input,
				      64,
				      alshash_callback,
				      alshash_finalize,
				      &sha2x);
	  
	  fclose(f);
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] failed to open file '%s'\n", filename );	  
	}
    }
  else
    {
      test_empty_hash();
    }
}
