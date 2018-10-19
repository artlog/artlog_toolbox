#include "alcryptohash_tool.h"

#include "altodo.h"
#include "aldebug.h"
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
  alcryptohash_tool_dump_result(shax, result);
}

void alshash_callback (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldatablock_dump(block);
	}
      alcryptohash_tool_callback(block, data);
    }
}

void alshash_finalize (struct alhash_datablock * block, void * data)
{
  if (( block != NULL )&&(data != NULL))
    {
      struct alsha2_internal * shax = (struct alsha2_internal *) data;
      ALDEBUG_IF_DEBUG(shax, alsha2x, debug)
	{
	  aldatablock_dump(block);
	}
      ;
      alcryptohash_tool_finalize(block,data);
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
	      struct alsha2_internal sha2x;
	      
	      alinputstream_init(&input, fileno(f));
	      alsha2x_init(&sha2x, AL_SHA256);
	      
	      // shax broken when not using 64 bytes multiples
	      int readblocksize = 32;
	      if (argc > 2 )
		{
		  ALDEBUG_SET_DEBUG(&sha2x,alsha2x,255);
		  readblocksize = atoi(argv[2]);
		  if (readblocksize <= 0)
		    {
		      readblocksize = 64;
		    }
		}
    
	      ALDEBUG_IF_DEBUG(&sha2x,alsha2x,debug)
		{
		  aldebug_printf(NULL,"open file %s\n", filename );
		}

	      struct alhash_datablock * result = alcryptohash_tool_from_input(&sha2x, &input, readblocksize);
	      dump_result(&sha2x,result);
				      
	      fclose(f);

	      /*
	      f = fopen(filename,"r");
	      alinputstream_init(&input, fileno(f));
	      albase64_frominput(&input,NULL);	      
	      fclose(f);
	      */
	      
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
