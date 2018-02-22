#include "alcryptohash.h"
#include "altodo.h"
#include "aldebug.h"
#include "alinput.h"
#include "albase64.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void usage()
{
  printf("program <name of file to hash> (<debug>)\n");
  printf("if name is '' then display hash for empty value (value of size 0)\n");
}

void dump_result(struct alsha2_internal * shax, struct alhash_datablock * result)
{
  if ( result == NULL)
    {
      aldebug_printf(NULL,"[FATAL] NULL shax \n");
    }
  else
    {
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldebug_printf(NULL,"[INFO] SHAX result \n");
	  aldatablock_dump(result);
	}
      unsigned int * h = result->data.uintptr;
      // sha224
      if ( shax->algorithm == AL_SHA224 )
	{
	  printf("%08x%08x%08x%08x%08x%08x%08x\n",h[0],h[1],h[2],h[3],h[4],h[5],h[6]);
	}
      else
	{
	  printf("%08x%08x%08x%08x%08x%08x%08x%08x\n",h[0],h[1],h[2],h[3],h[4],h[5],h[6],h[7]);
	}
    }

}

void alshash_callback (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldebug_printf(NULL,"[INFO] ADD block size %i \n", block->length);
	  aldatablock_dump(block);
	}
      alsha2x_add_block(shax,block);
    }
}

void alshash_finalize (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldebug_printf(NULL,"[INFO] FINAL block size %i \n", block->length);
	  aldatablock_dump(block);
	}
      if ( block->length > 0)
	{
	  alsha2x_add_block(shax,block);
	}
      struct alhash_datablock * result;
      result = alsha2x_final(shax);

      dump_result(shax,result);
    }
}

void test_empty_hash(int argc, char ** argv)
{
  struct alsha2_internal shax;
  struct alhash_datablock * result;

  // sha256 yet
  alsha256_init(&shax);

  if (argc > 2 )
    {
      ALDEBUG_SET_DEBUG(&shax,alsha2x,255);
    }

  result = alsha2x_final(&shax);

  dump_result(&shax,result);
}


void test_base64(char * text)
{
  struct alhash_datablock block;

  block.data.charptr=text;
  block.length=strlen(text);
  block.type=ALTYPE_STR0;

  albase64(&block,NULL);
}

int main(int argc, char ** argv)
{
  if ( argc > 1 )
    {
      char * filename = argv[1];
      if ( strlen(filename) > 0)
	{
	  FILE * f = fopen(filename,"r");
	  if ( f != NULL )
	    {
	      struct alinputstream input;
	      struct alsha2_internal sha2x ;

	      alsha2x_init(&sha2x, AL_SHA256);
	      if (argc > 2 )
		{
		  ALDEBUG_SET_DEBUG(&sha2x,alsha2x,255);
		}
	      ALDEBUG_IF_DEBUG(&sha2x,alsha2x,debug)
		{
		  aldebug_printf(NULL,"open file %s\n", filename );
		}
	      alinputstream_init(&input, fileno(f));
	      alinputstream_foreach_block(&input,
					  64,
					  alshash_callback,
					  alshash_finalize,
					  &sha2x);
	  
	      fclose(f);

	      f = fopen(filename,"r");
	      alinputstream_init(&input, fileno(f));
	      albase64_frominput(&input,NULL);	      
	      fclose(f);
	      
	    }
	  else
	    {
	      aldebug_printf(NULL,"[ERROR] failed to open file '%s'\n", filename );
	      test_base64(filename);
	    }
	}
      else
	{
	  test_empty_hash(argc,argv);	  
	}
    }
  else
    {
      usage();
    }
}
