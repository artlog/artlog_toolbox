/**
a hash within your tools allow you to do smoke tests
**/

#include "alhash.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum alhash_match_result {
  ALH_MR_NOT_EQUAL=0, // it does not match
  ALH_MR_EQUAL=1, // it matches
  ALH_MR_INVALID=2, // it does not match because one value is invalid ( internal error ).
};

static int alhash_debug = 0;

// make sure index is within bucket size
static unsigned int al_get_index(long hash, int length)
{
  unsigned int index = ((unsigned long) hash) % length;
  return index;
}

void alhash_set_debug(int debug)
{
  alhash_debug=debug;
  if ( alhash_debug ) {printf("alhash debug activated.\n");}
}
/**
return if key matches entry ( see enum alhash_match_result comments )
**/
enum alhash_match_result alhash_match(struct alhash_datablock * key, struct alhash_entry * entry, long hash)
{
  if ( (key != NULL) && (entry != NULL) )
    {
      if ( ( key->data == NULL )  || ( entry->key.data == NULL ) )
	{
	  // NULL values are wrong.
	  if ( alhash_debug ) {printf("NULL values are wrong.\n");}
	  return ALH_MR_INVALID;
	}
      if ( (key->length <= 0) || (entry->key.length <= 0) )
	{
	  // EMTPY BLOCKS NOT VALID
	  if (alhash_debug) {printf("EMTPY BLOCKS NOT VALID.\n");}
	  return ALH_MR_INVALID;
	}
      if (key->length == entry->key.length)
	{	  
	  if ( key->data == entry->key.data )
	    {
	      // obvious case, point on very same value of same size.
	      if (alhash_debug) {printf("IDENTICAL KEY\n");}
	      return ALH_MR_EQUAL;
	    }
	  else if ( hash == entry->hash_key )
	    {
	      // should be the very same key.
	      if (alhash_debug) {printf("SAME KEY\n");}
	      return (memcmp(entry->key.data, key->data, key->length) == 0) ? ALH_MR_EQUAL : ALH_MR_NOT_EQUAL;
	    }
	  else
	    {
	      if (alhash_debug) {printf("DIFFERENT KEY %ld!=%ld \n", hash,entry->hash_key);}
	      return  ALH_MR_NOT_EQUAL;
	    }
	}
      if (alhash_debug) {printf("DIFFERENT SIZE\n");}
      return ALH_MR_NOT_EQUAL;
    }
  return ALH_MR_INVALID;
}

// this is the core element that should spread values to long space
// current implementation is quick n' dirty, don't search for smart here.
long alhash_hash_string(void * value, int length)
{
  char * string = (char *) value;
  long hash = 0xdeadbeef00112233;
  if ( length >= 8 )
    {
      hash = hash ^ *((long*) (value));
    }
  else if ( length >= 4 )
    {
      hash = hash ^ (  *((int*) (value)) << 12 );
    }
  else if ( length >= 2  )
    {
      hash = hash ^ ( ((long) string[1]) << 27 ) ^ ( ((long) string[2]) << 43 );
    }
    
    
  return (length > 0) ?
    (
     hash 
     ^ ((long) string[0]) << 55
     ^ (((long) string[length/2]) << 13 )
     ^ (((long) string[length-1]) << 21 )
     ^ (((long) string[ 371 % length ]) << 2 ))
    : 0;
}

void alhash_init(struct alhash_table * table, int length, long (*alhash_func) (void * value, int length))
{
  if ( alhash_func == NULL )
    {
      table->alhash_func=alhash_hash_string;
    }
  else
    {
      table->alhash_func=alhash_func;
    }
  table->bucket_size=( length > ALHASH_BUCKET_SIZE ) ? length : ALHASH_BUCKET_SIZE;
  table->inner = (struct alhash_bucket *) (calloc(table->bucket_size,sizeof(struct alhash_entry)));
  table->used = 0;
}

void alhash_release(struct alhash_table * table)
{
  if ( table->inner != NULL)
    {
      free(table->inner);
      table->inner=NULL;
    }
  table->alhash_func=NULL;
  table->bucket_size=0;
  table->used = 0xdeadbeef;
}

struct alhash_entry * alhash_put(struct alhash_table * table, struct alhash_datablock * key, struct alhash_datablock * value)  
{
  if ( alhash_debug ) {printf("alhash put entry .\n");}
  if ( ( table != NULL ) && ( key != NULL) && ( value != NULL ) )
    {
      long hash = table->alhash_func(key->data,key->length);
      int index = al_get_index(hash,table->bucket_size);
      struct alhash_bucket * bucket = table->inner;
      if ( bucket != NULL )
	{
	  int guard = table->bucket_size;
	  // initial entry is where we would like to push our key value.
	  struct alhash_entry * initial_entry = &bucket->entries[index];
	  struct alhash_entry * entry = initial_entry;
	  do 
	    {
	      if ( ( entry->key.length == 0 ) && ( entry->key.data == NULL ) && ( entry->collision_ring == NULL) )
		{
		  // found an empty place
		  entry->hash_key=hash;
		  entry->key.length = key->length;
		  entry->key.data = key->data;
		  entry->value.length = value->length;
		  entry->value.data = value->data;
		  // append this entry in ring of initial entry ( ie where it should have been ).		  
		  if ( entry != initial_entry )
		    {
		      entry->collision_ring = initial_entry->collision_ring;
		    }
		  // if entry == initial_entry this is a loop over self.
		  initial_entry->collision_ring = entry;
		  return entry;
		}
	      else
		{
		  // move to another place , hoping for an empty place.
		  index = (index + 1) % table->bucket_size;
		  entry =  &bucket->entries[index];
		  --guard;
		}
	    }
	  while (
		 ( entry != NULL ) // internal error ... how could it be NULL this a place within the bucket.
		 && ( entry != initial_entry) // loop containing initial entry : normal ring structure.
		 && ( guard > 0) // if loop or other bad content
		 );
	  if ( guard <= 0 )
	    {
	      fprintf(stderr,"[FATAL] alhash guard reached hashtable full %p size %i\n",table, table->bucket_size);
	    }

	}
      return NULL;
    }
  return NULL;
}

struct alhash_entry * alhash_get_entry(struct alhash_table * table, struct alhash_datablock * key)
{
  if ( alhash_debug ) {printf("alhash get entry .\n");}
  if (( table != NULL ) && ( key != NULL ))
    {
      // first should compute hash key
      if ( table->alhash_func != NULL )
	{
	  long hash = table->alhash_func(key->data,key->length);
	  unsigned int index = al_get_index(hash,table->bucket_size);
	  struct alhash_bucket * bucket = table->inner;
	  if ( bucket != NULL )
	    {
	      int guard = table->bucket_size;
	      struct alhash_entry * initial_entry = &bucket->entries[index];
	      struct alhash_entry * entry = initial_entry;
	      if ( entry->key.data != NULL )
		{
		  do {		
		    if ( alhash_match(key, entry, hash) == ALH_MR_EQUAL )
		      {
			return entry;
		      }
		    else
		      {
			entry =  entry->collision_ring;
			--guard;
		      }
		  }
		  while (
			 ( entry != NULL )
			 && ( entry != initial_entry) // loop containing initial entry : normal ring structure.
			 && ( guard > 0) // if loop or other bad content
			 );
		}
	    }
	}
    }
  return NULL;
}

int alhash_walk_callback_null (struct alhash_entry * entry, void * data, int index)
{
  return 0;
}
   
int alhash_walk_collisions(struct alhash_entry * entry, alhash_callback callback, void * data)
{
  int step = 0;
  if ( callback == NULL )
    {
      callback = alhash_walk_callback_null;
    }
  if ( entry != NULL )
    {
      int guard = 0x7fffffff; // 4G collision ( due to fact that we don't have table size, at least protect against loops ). 
      struct alhash_entry * initial_entry = entry;
      do
	{
	  if ( ( entry->key.length != 0 ) && ( entry->key.data != NULL ) )
	    {
	      if ( callback(entry,data,step) != 0 ) 
		{
		  return step;
		}
	      ++ step;
	    }
	  entry = entry->collision_ring;
	  -- guard;
	} while ( ( guard > 0) && ( entry != NULL ) && (entry != initial_entry ) );
    }
  return step;
}

int alhash_walk_table( struct alhash_table * table, alhash_callback callback, void * data)
{
  int step = 0;
  if ( callback == NULL )
    {
      callback = alhash_walk_callback_null;
    }
  if ( table != NULL )
    {
      struct alhash_bucket * bucket = table->inner;
      if ( bucket != NULL )
	{
	  for (int index =0 ; index < table->bucket_size; index ++)
	    {
	      struct alhash_entry * entry = &bucket->entries[index];
	      if ( ( entry->key.length != 0 ) && ( entry->key.data != NULL ) )
		{
		  if ( callback(entry,data,step) != 0 )
		    {
		      return step;
		    }
		  ++ step;
		}
	    }
	}
    }
  return step;
}
