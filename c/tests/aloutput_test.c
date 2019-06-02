#include "aloutput.h"
#include "aloutput_file.h"
#include "aldebug_output.h"

#include <stddef.h>

unsigned char buffer[] = {
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
			  1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
};			  
			  
int main(int argc, char ** argv)
{
  struct aloutputstream out;

  aldebug_start(NULL);
  
  if ( argc > 1 )
    {
      char * filename = argv[1];
      enum al_global_error_code ec = aloutput_file_open_init(&out,filename);
      if ( ec == AL_EC_OK )
	{
	  aldatablock test;
	  test.type = ALTYPE_SUBSTR;
	  test.data.ucharptr=buffer;
	  test.length=sizeof(buffer);      
	  alouput_bytes_as_hex(&out, &test, 1, 4);
	  alouput_bytes_as_hex(&out, &test, 0, 4);
	  alouput_bytes_as_hex(&out, &test, 0, sizeof(buffer));
	  aloutputstream_close(&out);
	}
      else
	{
	  aldebug_printf(DBGSTREAM,"[ERROR] code %i\n",ec);
	}
    }
  
  aldebug_end();
}
