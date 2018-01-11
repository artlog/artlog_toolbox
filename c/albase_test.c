#include "albase.h"
#include <stdlib.h>

int main(int argc, char ** argv)
{
  struct alhash_datablock datablock;

  albase_set_debug(1);

  struct token_char_buffer * allocator = al_token_char_buffer_alloc(10);
  if ( allocator != NULL )
    {
      al_token_char_buffer_init(allocator,256);
      
      char buffer[128];
      datablock.type=ALTYPE_OPAQUE;
      datablock.length=4096;
      datablock.data.ptr=buffer;

      for (int i = 0; i < 200; i += 17)
	{
	  int j = i*i*i +3;
	  aljson_build_string_from_int(j,2,allocator,&datablock);
	  aljson_build_string_from_int(j,10,allocator,&datablock);
	  aljson_build_string_from_int(j,16,allocator,&datablock);
	  aljson_build_string_from_int(-j,16,allocator,&datablock);
	  aljson_build_string_from_int(-j,8,allocator,&datablock);
	}
    }
}
