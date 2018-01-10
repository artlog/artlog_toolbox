#include "alhash.h"

#include <stdio.h>
#include <stdlib.h>

int alhash_walk_callback_collision(struct alhash_entry * entry, void * data, int index)
{
  if ( entry != NULL )
    {
      if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	{
	  printf("(%i)'%.*s',",entry->key.type, entry->key.length, (char *) entry->key.data.ptr);
	}
    }
  return 0;
}

void dump_entry_as_string(struct alhash_entry * entry)
{
  if ( entry != NULL )
    {
      if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	{
	  printf( "%p (%i)'%.*s' = (%i)'%.*s' (hash=%lx) (collisions=", entry, entry->key.type, entry->key.length, (char *) entry->key.data.ptr,  entry->value.type, entry->value.length, (char *)  entry->value.data.ptr, entry->hash_key);
	  int collisions = alhash_walk_collisions(entry, alhash_walk_callback_collision, NULL);
	  printf( "#%i)\n",collisions);
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
  dump_entry_as_string(entry);
  printf("\n");
  return 0;
}

void usage()
{
  printf("please provide filepath of file to insert into alhash. format is a file \n");
}

int main(int argc, char ** argv)
{
  if ( argc > 1 )
    {
      char * filename = argv[1];
      FILE * f = fopen( filename, "r");
      if ( f != NULL )
	{
	      char * line = NULL;
	      size_t len = 0;
	      ssize_t read;
	      struct alhash_table table;
	      alhash_init(&table,0,NULL);
	      while ((read = getline(&line, &len, f)) != -1) {
		int i = 0;
		while ( (i<=read) && ( line[i] != 0 )  && ( line[i] != ' ' ))
		  {
		    i++;
		  }
		{
		  if (( i>0) && (i<read-2))
		    {
		      struct alhash_datablock key;
		      struct alhash_datablock value;
		      key.type=ALTYPE_OPAQUE;
		      key.length=i;
		      key.data.ptr=line;
		      struct alhash_entry * entry = alhash_get_entry(&table, &key);
		      if ( entry == NULL )
			{
			  // remove heading space and last char ( return )
			  value.length=read-i-2;
			  value.data.ptr=&line[i+1];
			  entry = alhash_put(&table, &key, &value);
			  if ( entry != NULL )
			    {
			      dump_entry_as_string(entry);
			    }
			}
		      else
			{
			  // printf("%s ALREADY ADDED", line);
			  free(line);
			}
		    }
		}
		line = NULL; // force realloc for every line.
	      }	     
	     fclose(f);
	     printf("\ninternal ordering dump \n");
	     alhash_walk_table(&table, alhash_walk_callback_dump, NULL);
	}
      else
	{
	  fprintf(stderr,"%s not found", filename);
	}
    }
  else
    {
      usage();
    }
}
