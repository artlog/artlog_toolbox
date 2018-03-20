/**
'a hash within your tools allow you to do smoke tests'

implementation of a hashtable in a miserable way.
**/

#include "alhash.h"
#include "alcommon.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

struct alparser_ctx alparser_ctx_default = {
  .debug = 1,
};

ALDEBUG_DEFINE_FUNCTIONS(struct alparser_ctx, alparser_ctx, debug);

// make sure index is within bucket size
static unsigned int al_get_index(long hash, int length)
{
  unsigned int index = ((unsigned long) hash) % length;
  return index;
}

int aldatablock_embeded(struct alhash_datablock * key)
{
  // return ALC_FLAG_IS_SET(key->type,ALTYPE_FLAG_EMBED);
  return 0;
}

// return  ALH_MR_EQUAL if empty , ALH_MR_INVALID if invalid, ALH_MR_NOT_EQUAL
enum alhash_match_result aldatablock_is_empty(struct alhash_datablock * key)
{
  // valid if positive length  and ( either embeded value or ptr is non null )
  // this means that embeded 0 value is OK.
  if ( key->length  < 0 )
    {
      aldebug_printf(NULL,"[FATAL] key->length %i < 0\n",key->length);
      return ALH_MR_INVALID;
    }
  // don't accept unknow types
  if ( key->type > ALTYPE_MAX )
    {
      aldebug_printf(NULL,"[FATAL] key->type %i > ALTYPE_MAX(%i) in datablock %p length %i \n", key->type, ALTYPE_MAX, key, key->length);
      return ALH_MR_INVALID;
    }
  // return (key->length >0) && ( aldatablock_embeded(key) || (key->data.ptr != NULL));
  return ((key->length >0) && (key->data.ptr != NULL)) ? ALH_MR_NOT_EQUAL : ALH_MR_EQUAL;
}

/**
return if key matches entry ( see enum alhash_match_result comments )
**/
enum alhash_match_result alhash_match(struct alhash_datablock * key, struct alhash_entry * entry, long hash, int alhash_debug)
{
  if ( (key != NULL) && (entry != NULL) )
    {
      if ( ! (
	      ( aldatablock_is_empty(key) ==  ALH_MR_NOT_EQUAL)
	      && ( aldatablock_is_empty( &entry->key ) == ALH_MR_NOT_EQUAL )
	      )
	)
	{
	  // NULL or EMTPY BLOCKS NOT VALID values are wrong.
	  if ( alhash_debug ) {aldebug_printf(NULL,"[DEBUG] NULL values are wrong.\n");}
	  return ALH_MR_INVALID;
	}
      if ( key->type != entry->key.type )
	{
	  if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] DIFFERENT KEY TYPE %i!=%i \n", key->type,entry->key.type);}
	  return  ALH_MR_NOT_EQUAL;
	}
      if (key->length == entry->key.length)
	{
	  // if one is embed the other too since type have been checked to be the same
	  if ( aldatablock_embeded(key))
	    {
	      // Don't even care of hash, use number.
	      if ( key->data.number == entry->key.data.number )
		{
		  // obvious case, point on very same value of same size.
		  if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] IDENTICAL VALUES\n");}
		  return ALH_MR_EQUAL;
		}
	      else
		{
		  if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] DIFFERENT VALUES\n");}
		  return  ALH_MR_NOT_EQUAL;
		}	      
	    }
	  else
	    {
	      if ( key->data.ptr == entry->key.data.ptr )
		{
		  // obvious case, point on very same value of same size.
		  if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] IDENTICAL KEY\n");}
		  return ALH_MR_EQUAL;
		}
	      else if ( hash == entry->hash_key )
		{
		  // should be the very same key.
		  if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] SAME KEY\n");}
		  return (memcmp(entry->key.data.ptr, key->data.ptr, key->length) == 0) ? ALH_MR_EQUAL : ALH_MR_NOT_EQUAL;
		}
	      else
		{
		  if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] DIFFERENT KEY %ld!=%ld \n", hash,entry->hash_key);}
		  return  ALH_MR_NOT_EQUAL;
		}
	    }
	}
      if (alhash_debug) {aldebug_printf(NULL,"[DEBUG] DIFFERENT SIZE\n");}
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
  // todo use context allocator.
  table->inner = (struct alhash_bucket *) (calloc(table->bucket_size,sizeof(struct alhash_entry)));
  table->used = 0;
  if ( length == 0 )
    {
      // 78 % fill
      table->autogrow = 200;
    }
  else
    {
      table->autogrow = 0;
    }
  table->context = &alparser_ctx_default;
}

void alparser_ctx_alhash_init(struct alparser_ctx * ctx, struct alhash_table * table, int length, long (*alhash_func) (void * value, int length))
{
  alhash_init(table,length,alhash_func);
  // if set override current else use this of init (expecting default)
  if ( ctx != NULL )
    {
      table->context = ctx;
    }
  
  ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug)
    {aldebug_printf(NULL,"[DEBUG] alhash init %p autogrow %i \n",table,table->autogrow );}

}

void alhash_release(struct alhash_table * table)
{
  {aldebug_printf(NULL,"[DEBUG] alhash release %p autogrow %i \n",table,table->autogrow );}

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

  if ( ( table != NULL ) && ( key != NULL) && ( value != NULL ) )
    {
      ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug)
	{
	  aldebug_printf(NULL,
			 "[DEBUG] alhash put entry '" ALPASCALSTRFMT "'\n",
			 ALPASCALSTRARGS(key->length,(char *) key->data.ptr));
	}
	
      // automatically grow table if over a % fill
      if ( table->autogrow > 0 )
	{
	  if ( alhash_get_usage(table) > table->autogrow  )
	    {
	      ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug)
		{
		  aldebug_printf(NULL,
				 "[DEBUG] alhash %p grow from %i to %i .\n",
				 table, table->bucket_size, table->bucket_size * 2);
		}
	      if ( alhash_reinit(table, table->bucket_size * 2 ) <= 0 )
		{
		  return NULL;
		}
	    }
	}
      
      long hash = table->alhash_func(key->data.ptr,key->length);
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
	      if ( ( entry->key.length == 0 ) && ( entry->key.data.ptr == NULL ) && ( entry->collision_ring == NULL) )
		{
		  // found an empty place
		  entry->hash_key=hash;
		  memcpy(&entry->key,key, sizeof(entry->key));
		  memcpy(&entry->value, value, sizeof(entry->value));
		  // append this entry in ring of initial entry ( ie where it should have been ).		  
		  if ( entry != initial_entry )
		    {
		      entry->collision_ring = initial_entry->collision_ring;
		    }
		  // if entry == initial_entry this is a loop over self.
		  initial_entry->collision_ring = entry;
		  table->used = table->used + 1;
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
	      aldebug_printf(NULL,"[FATAL] alhash guard reached hashtable full %p size %i autogrow %i\n",table, table->bucket_size, table->autogrow);
	    }

	}
      return NULL;
    }
  return NULL;
}

struct alhash_entry * alhash_get_entry(struct alhash_table * table, struct alhash_datablock * key)
{
  if (( table != NULL ) && ( key != NULL ))
    {
      ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug) {
	aldebug_printf(NULL,
		       "[DEBUG] alhash get entry '" ALPASCALSTRFMT "'\n",
		       ALPASCALSTRARGS(key->length,(char *) key->data.ptr));
      }
      // first should compute hash key
      if ( table->alhash_func != NULL )
	{
	  long hash = table->alhash_func(key->data.ptr,key->length);
	  unsigned int index = al_get_index(hash,table->bucket_size);
	  struct alhash_bucket * bucket = table->inner;
	  if ( bucket != NULL )
	    {
	      int guard = table->bucket_size;
	      struct alhash_entry * initial_entry = &bucket->entries[index];
	      struct alhash_entry * entry = initial_entry;
	      int valid = aldatablock_is_empty(&entry->key);
	      if ( valid == ALH_MR_NOT_EQUAL )
		{
		  do {		
		    if ( alhash_match(key, entry, hash, alparser_ctx_is_debug(table->context,1)) == ALH_MR_EQUAL )
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
	      else
		{
		  // if empty valid = ALH_MR_NOT_EQUAL
		  if ( valid == ALH_MR_INVALID )
		    {		      
		      aldebug_printf(NULL,
				     "alhash_get entry '" ALPASCALSTRFMT "' failed bucket index %i\n",
				     ALPASCALSTRARGS(key->length,(char *) key->data.ptr),
				     index);
		      alhash_dump_entry_as_string(entry);
		      abort();
		    }
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
	  if ( aldatablock_is_empty(&entry->key) == ALH_MR_NOT_EQUAL )
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
	      if ( aldatablock_is_empty(&entry->key) ==  ALH_MR_NOT_EQUAL )
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

int alparser_init(  struct alparser_ctx * alparser, int words, int chars)
{
  // length in number of entries [ at least ALHASH_BUCKET_SIZE will be used ]
  bzero(alparser,sizeof(*alparser));
  alparser_ctx_alhash_init(alparser, &alparser->dict, words, NULL);
  // autogrow
  alparser->dict.autogrow = 170;
  // WARNING will set alparser->ringbuffer content
  alstrings_ringbuffer_init_autogrow(&alparser->allocator.ringbuffer, 15, chars);

  return 1;
}

int alhash_copyentry(struct alhash_entry * entry, void * data, int index)
{
  if (( entry != NULL ) && (data != NULL))
    {
      struct alhash_table * destination = (struct alhash_table *) data;
      alhash_put(destination, &entry->key, &entry->value);
      // continue
      return 0;
    }
  // continue 
  return 0;
}

int alhash_reinit(struct alhash_table * table, int length)
{
  int size = -1;
  // new table should have enough place
  if (table != NULL)
    {
      ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug)
	{
	  aldebug_printf(NULL,"[DEBUG] REINIT length of hash table to %i current usage %i used %i\n", length, alhash_get_usage(table), alhash_get_used(table));
	}

      if ( table->used <= length )
	{
          struct alhash_table temporary;
	  bzero(&temporary, sizeof(temporary));
	  // autogrow is NOT set ( else would be recursive )
	  alparser_ctx_alhash_init(table->context,&temporary,length,table->alhash_func);
	  alhash_walk_table(table,alhash_copyentry,&temporary);
	  // drink this soup
	  if ( table->inner != NULL)
	    {
               free(table->inner);
	       // somehow a canary
	       table->inner = (void *) 0xdeadbed1;
            }
	  // set autogrow since copied back to table.
	  temporary.autogrow=table->autogrow;
	  memcpy(table,&temporary,sizeof(*table));
	  size =  alhash_get_size(table);
	  ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug)
	    {
	      aldebug_printf(NULL,"[DEBUG] REINITIALIZED length of hash table to %i new usage %i used %i \n", length, alhash_get_usage(table), alhash_get_used(table));
	    }	  
      }
      else
	{
	  ALDEBUG_IF_DEBUG(table->context,alparser_ctx,debug)
	    {
	    aldebug_printf(NULL,"[DEBUG] alhash length %i requested for reinit too small < used %i \n",length, table->used);
	  }
      }
    }
  return size;
}

int alhash_get_used(struct alhash_table * table)
{
  int use = -1;
  if (table != NULL)
    {
      use = table->used;
    }
  return use;
}


int alhash_get_size(struct alhash_table * table)
{
  int size=0;
  if (table != 0)
    {
      size = table->bucket_size;
    }
  return size;
}

int alhash_get_usage(struct alhash_table * table)
{
  int size=0;
  if ( table != 0) 
    {
      if (table->bucket_size > 0 )
	{
	  size = table->used;
	  if ( size > ( INT_MAX / 256 ) )
	    {
	      size = ( size / table->bucket_size ) * 256;
	    }
	  else
	    {
	      size = ( size * 256 ) / table->bucket_size;
	    }
	}
      else
	{
	  size = 255;
	}
	
    }
  return size;
}


int alhash_walk_callback_collision(struct alhash_entry * entry, void * data, int index)
{
  if ( entry != NULL )
    {
      if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	{
	  aldebug_printf(NULL,
			 "(%i)'" ALPASCALSTRFMT "',",
			 entry->key.type,
			 ALPASCALSTRARGS(entry->key.length, (char *) entry->key.data.ptr));
	}
    }
  return 0;
}

void alhash_dump_entry_as_string(struct alhash_entry * entry)
{
  if ( entry != NULL )
    {
      if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	{
	  aldebug_printf(NULL,"dump entry\n");
	  aldebug_printf(NULL,
			 "%p (%i)'" ALPASCALSTRFMT "' = (%i)'" ALPASCALSTRFMT "' (hash=%lx) (collisions=",
			 entry, entry->key.type,
			 ALPASCALSTRARGS(entry->key.length, (char *) entry->key.data.ptr),
			 entry->value.type,
			 ALPASCALSTRARGS(entry->value.length, (char *)  entry->value.data.ptr),
			 entry->hash_key);
	  int collisions = alhash_walk_collisions(entry, alhash_walk_callback_collision, NULL);
	  aldebug_printf( NULL, "#%i)\n",collisions);
	}
      else
	{
	   printf( "%p NULL ", entry);
	}
    }

}

int alhash_walk_callback_dump (struct alhash_entry * entry, void * data, int index)
{
  printf("%i ) ",index);
  alhash_dump_entry_as_string(entry);
  printf("\n");
  return 0;
}
