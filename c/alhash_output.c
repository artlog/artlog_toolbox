#include "alhash_output.h"
#include "aldebug_output.h"

int alhash_output_walk_collision_callback(struct alhash_entry * entry, struct aloutputstream * output, int index)
{
  if ( output != NULL )
    {
      if ( entry != NULL )
	{
	  if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	    {	  
	      aloutputstream_printf_1k(output,
				       "(%i)'" ALPASCALSTRFMT "',",
				       entry->key.type,
				       ALPASCALSTRARGS(entry->key.length, (char *) entry->key.data.ptr));
	    }
	}
    }
  return 0;
}

int alhash_output_walk_collision_callback_cast_outputstream(struct alhash_entry * entry, void * data, int index)
{
  return alhash_output_walk_collision_callback(entry, (struct aloutputstream *) data, index);
}

int alhash_output_dump_entry(struct alhash_entry * entry, struct aloutputstream * output, int index)
{
  if ( entry != NULL )
    {
      if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	{
	  aloutputstream_printf_1k(output,
				   "[%i] %p (%i)'" ALPASCALSTRFMT "' = (%i)'" ALPASCALSTRFMT "' (hash=%lx) values/collisions=",
				   index,
				   entry, entry->key.type,
				   ALPASCALSTRARGS(entry->key.length, (char *) entry->key.data.ptr),
				   entry->value.type,
				   ALPASCALSTRARGS(entry->value.length, (char *)  entry->value.data.ptr),
				   entry->hash_key
				   );
	  //int collisions =
	  alhash_walk_collisions(entry, alhash_output_walk_collision_callback_cast_outputstream, (void *) output);
	  
	  aloutputstream_printf_1k(output,"\n");
	}
      else
	{
	  aloutputstream_printf_1k(output,"entry key %p NULL\n", entry);
	}
      return 0;
    }
  return 1;
}

int alhash_output_dump_entry_callback_cast_outputstream(struct alhash_entry * entry, void * data, int index)
{
  return alhash_output_dump_entry(entry, (struct aloutputstream *) data, index);
}

int alhash_output_walk_simple_callback_cast_outputstream (struct alhash_entry * entry, void * data, int index)
{
  struct aloutputstream * output = (struct aloutputstream *) data;
  if ( output != NULL )
    {
      if ( entry != NULL )
	{
	  if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	    {
	      aloutputstream_printf_1k(output,
				       "'" ALPASCALSTRFMT "' = '" ALPASCALSTRFMT "'\n",
				       ALPASCALSTRARGS(entry->key.length, (char *) entry->key.data.ptr),
				       ALPASCALSTRARGS(entry->value.length, (char *)  entry->value.data.ptr));
	    }
	  else
	    {
	      printf( "%p NULL\n", entry);
	    }
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] Null output\n");
	}
    }
  return 0;
}

