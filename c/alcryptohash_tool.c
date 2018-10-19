#include "alcryptohash_tool.h"

#include "altodo.h"
#include "aldebug.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void alcryptohash_tool_dump_result(struct alsha2_internal * shax, struct alhash_datablock * result)
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

void alcryptohash_tool_callback(struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldebug_printf(NULL,"[INFO] ADD block size %i \n", block->length);
	}
      alsha2x_add_block(shax,block);
    }
}

void alcryptohash_tool_finalize (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldebug_printf(NULL,"[INFO] FINAL block size %i \n", block->length);
	}
      if ( block->length > 0)
	{
	  alsha2x_add_block(shax,block);
	}

      alcryptohash_tool_dump_result(shax,
				    alsha2x_final(shax)
				    );
    }
}

struct alhash_datablock * alcryptohash_tool_from_input(  struct alsha2_internal *sha2x, struct alinputstream * input, int readblocksize)
{

  alinputstream_foreach_block(input,
			      readblocksize,
			      alcryptohash_tool_callback,
			      alcryptohash_tool_finalize,
			      sha2x);

  // should be hash
  // todo should check it was correctly set
  return &sha2x->output;
}

