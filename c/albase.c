#include <stdio.h>
#include <string.h>
#include "alstrings.h"

#define LOCAL_BUFFER_SIZE 128

int albase_debug=0;

void albase_set_debug(int debug)
{
  albase_debug=debug;
}

// qndirty as usual
// base
// correct bases are 2 binary, 8 octal,10 decimal ,16 hexadecimal.
int aljson_build_string_from_int(int integer, int base, struct alhash_datablock * out)
{
  char buffer[LOCAL_BUFFER_SIZE];
  int v = integer;
  int p = LOCAL_BUFFER_SIZE - 1;
  int r=0;
  int negative=0;

  char * s= buffer;
    
  if ( v < 0 )
    {
      v = -v;
      negative=1;
    }

  s[p]=0;
  
  while ( ( v > 0 ) && ( p > 0 ) )
    {  
      r=v%base;
      --p;
      if ( r < 10 )
	{
	  s[p] = '0' + r;
	}
      else
	{
	  s[p] = 'a' + (r - 10);
	}
      v=(v-r);
      if ( v >= base )
	{
	  v/=base;
	}
    }
  if ( negative && (p>0) )    
    {
      --p;
      s[p]='-';
    }
      
  int length = LOCAL_BUFFER_SIZE - p;
  if ( ( out->data.ptr != NULL ) && ( out->length > length ) )
    {
      memcpy(out->data.ptr,&s[p],length);
      if (albase_debug != 0 )
	{
	  // double check with libc implementation
	  printf("%i,%x=%s=%.*s\n",integer,integer,&s[p],length,out->data.ptr);
	}

    }
  
  // should be 0 if everything went fine
  return v;
  
}

