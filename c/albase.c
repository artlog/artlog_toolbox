#include <stdio.h>
#include <string.h>
#include "alstrings.h"
#include "aldebug.h"

#define LOCAL_BUFFER_SIZE 128

int albase_debug=0;

void albase_set_debug(int debug)
{
  albase_debug=debug;
}

// qndirty as usual
// base
// correct bases are 2 binary, 8 octal,10 decimal ,16 hexadecimal.
int aljson_build_string_from_int(int integer, int base, alstrings_ringbuffer_pointer * allocator, struct alhash_datablock * out)
{
  char buffer[LOCAL_BUFFER_SIZE];
  int v = integer;
  int p = LOCAL_BUFFER_SIZE - 1;
  int r=0;
  int negative=0;

  char * s= buffer;
  
  if (integer == 0)
    {
      s[p]=0;
      --p;
      s[p]='0';
    }
  else
    {    
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
    }
      
  int length = LOCAL_BUFFER_SIZE - p;
  if (albase_debug != 0)
    {
      aldebug_printf(NULL,"build string length %i\n",length);
    }
  out->data.ptr = &s[p];
  out->length =  length;
  if ( ( out->data.ptr = al_copy_block(allocator,out) ) != NULL )
    {
      if (albase_debug != 0 )
	{
	  // double check with libc implementation
	  aldebug_printf(NULL,"%i,%x=%s=%.*s\n",integer,integer,&s[p],length,(char *) out->data.ptr);
	}
    }
  else
    {
      aldebug_printf(NULL,"length %i allocation failure\n",length);
    }
  
  // should be 0 if everything went fine
  return v;
  
}

