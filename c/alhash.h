#ifndef __ALHASH_HEADER__
#define __ALHASH_HEADER__

#include "alstrings.h"

/**
a fixed hash table
you can add and retrieve keyed values in it ( but not removal ).
**/

// minimal hash
#define ALHASH_BUCKET_SIZE 127
  
struct alhash_entry {
  long hash_key;
  struct alhash_datablock key;
  struct alhash_datablock value;
  // another entry at the very same place in the bucket of the hash
  // this is a ring, ie a loop.
  struct alhash_entry * collision_ring; 
};

// at least ALHASH_BUCKET_SIZE , in fact it will dynamically allocated at table init and can contain really more entries.
struct alhash_bucket {
  struct alhash_entry entries[ALHASH_BUCKET_SIZE];
};

struct alhash_table {
  int bucket_size; // number of possible entries in this table
  int used; // number of entries in  use
  struct alhash_bucket * inner;
  long (*alhash_func) (void * value, int length);
};

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
// if allhsh_func is set to NULL then default string hash is used (alhash_hash_string)
void alhash_init(struct alhash_table * table, int length, long (*alhash_func) (void * value, int length));

// walk entry and all collisions.
// if callback returns a value != 0 it stops.
// return number of elements accepted by callback ( for which callback return value was 0 ).
int alhash_walk_collisions(struct alhash_entry * entry, alhash_callback callback, void * data);

// walk table in order of buckets entries
// if callback returns a value != 0 it stops.
// return number of elements accepted by callback ( for which callback return value was 0 ).
int alhash_walk_table( struct alhash_table * table, alhash_callback callback, void * data);

void alhash_set_debug(int debug);

void alhash_release(struct alhash_table * table);

#endif
