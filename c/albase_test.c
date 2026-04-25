#include "albase.h"
#include <stdlib.h>
#include <stdio.h>
#include "aldebug.h"
#include "aldebug_output.h"

int albase_test_string_from_int(int integer, int base, alstrings_ringbuffer_pointer * allocator, aldatablock * out)
{
  int result=albase_build_string_from_int(integer,base,allocator,out);
  printf("%s\n",out->data.charptr);
  return result;
}

int main(int argc, char ** argv)
{
  aldatablock datablock;

  aldebug_start(NULL);

  alstrings_ringbuffer_pointer allocator;
  if ( allocator != NULL )
    {
      char buffer[128];
      datablock.type=ALTYPE_OPAQUE;
      datablock.length=4096;
      datablock.data.ptr=buffer;

      for (int i = 0; i < 256; i ++)
	{
	  int j = (i*i*i*i) +3;

	  alstrings_ringbuffer_init_autogrow(&allocator,2,256);

	  printf("== %i ==\n",i);
	  albase_test_string_from_int(j,2,&allocator,&datablock);
	  albase_test_string_from_int(-j,2,&allocator,&datablock);
	  albase_test_string_from_int(j,8,&allocator,&datablock);
	  albase_test_string_from_int(-j,8,&allocator,&datablock);
	  albase_test_string_from_int(j,10,&allocator,&datablock);
	  albase_test_string_from_int(-j,10,&allocator,&datablock);
	  albase_test_string_from_int(j,16,&allocator,&datablock);
	  albase_test_string_from_int(-j,16,&allocator,&datablock);

	  alstrings_ringbuffer_release(&allocator);
	}
    }

  aldebug_end();
  exit(0);
}
