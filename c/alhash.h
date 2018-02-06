#ifndef __ALHASH_HEADER__
#define __ALHASH_HEADER__

#include "alstrings.h"
#include "aldebug.h"

/**
a fixed hash table
you can add and retrieve keyed values in it ( but not removal ).
**/

// minimal/default bucket_size
//#define ALHASH_BUCKET_SIZE 127
#define ALHASH_BUCKET_SIZE 32
  
struct alhash_entry {
  // hash key of key within key datablock
  long hash_key;
  struct alhash_datablock key;
  struct alhash_datablock value;
  // another entry at the very same place in the bucket of the hash
  // this is a ring, ie a loop.
  struct alhash_entry * collision_ring; 
};

enum alhash_match_result {
  ALH_MR_NOT_EQUAL=0, // it does not match ( or empty )
  ALH_MR_EQUAL=1, // it matches ( or non empty )
  ALH_MR_INVALID=2, // it does not match because one value is invalid ( internal error ).  
};


// at least ALHASH_BUCKET_SIZE , in fact it will dynamically allocated at table init and can contain really more entries.
struct alhash_bucket {
  struct alhash_entry entries[ALHASH_BUCKET_SIZE];
};

struct alparser_ctx;

struct alhash_table {
  struct alparser_ctx * context;
  int bucket_size; // number of possible entries in this table
  int used; // number of entries in  use
  int autogrow; // 0 don't grow automatically, else grow if alhash_get_free < autogrow.
  struct alhash_bucket * inner;
  long (*alhash_func) (void * value, int length);
};

// allocation of words, dict
struct alparser_ctx {
  ALDEBUG_DEFINE_FLAG(debug)
  struct alallocation_ctx allocator;
  struct alhash_table dict;
  int words;
};

ALDEBUG_DECLARE_FUNCTIONS(struct alparser_ctx, alparser_ctx);

long alhash_hash_string(void * string, int length);

// callback to use as a filter, when it returns 0 it means OK any other value is filtered out
typedef int (*alhash_callback) (struct alhash_entry * entry, void * data, int index);
  
/**
Warning : does not check for duplicates
if object is already there, it will be read twice, use alhash_get_entry first.
 */
struct alhash_entry * alhash_put(struct alhash_table * table, struct alhash_datablock * key, struct alhash_datablock * value);

/**
return entry owning same key content ( content of key->data over length bytes)
 */
struct alhash_entry * alhash_get_entry(struct alhash_table * table, struct alhash_datablock * key);

// length in number of entries [ at least ALHASH_BUCKET_SIZE will be used ]
// if length is 0 : AUTO : autogrowth is set and length = ALHASH_BUCKET_SIZE)
// if alhash_func is set to NULL then default string hash is used (alhash_hash_string)
// DON'T use externally, use alparser_init ( that comes with a dedicated context ) and use alparser_ctx->dict as hashtable.
void alhash_init(struct alhash_table * table, int length, long (*alhash_func) (void * value, int length));

// walk entry and all collisions.
// if callback returns a value != 0 it stops.
// return number of elements accepted by callback ( for which callback return value was 0 ).
int alhash_walk_collisions(struct alhash_entry * entry, alhash_callback callback, void * data);

// walk table in order of buckets entries
// if callback returns a value != 0 it stops.
// return number of elements accepted by callback ( for which callback return value was 0 ).
int alhash_walk_table( struct alhash_table * table, alhash_callback callback, void * data);

// release whole table glue ( ie does not free data content )
void alhash_release(struct alhash_table * table);

// init word buffer todo rename it.
// number of words is used for length of alhash_init, so can be 0 then automatic.
int alparser_init(  struct alparser_ctx * alparser, int words, int chars);

// allows to grow ( or shrink ) a table
// return number of used element in new table, -1 means error
int alhash_reinit(struct alhash_table * table, int length);

// return number of element in use in this table
int alhash_get_used(struct alhash_table * table);

// return size of table
int alhash_get_size(struct alhash_table * table);

// return usage ( in 256th means 128 is half used 255 full)
int alhash_get_usage(struct alhash_table * table);

// dump & debug

int alhash_walk_callback_collision(struct alhash_entry * entry, void * data, int index);

void alhash_dump_entry_as_string(struct alhash_entry * entry);

int alhash_walk_callback_dump (struct alhash_entry * entry, void * data, int index);

#endif
