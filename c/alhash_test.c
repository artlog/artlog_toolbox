#include "alhash.h"

#include <stdio.h>
#include <stdlib.h>

/** use very few alxxx function on purpose 
to not stress alhash library beside this test
**/

void usage()
{
  printf("please provide filepath of file to insert into alhash. format is a file \n");
  printf("content of file is a list of lines where key is seperated from value with a space or a tabulation\n");
}

int alhash_walk_callback_simple_dump (struct alhash_entry * entry, void * data, int index)
{
    if ( entry != NULL )
    {
      if ( ( entry->key.data.ptr != NULL ) && ( entry->value.data.ptr != NULL ) )
	{
	  printf(
			 "'" ALPASCALSTRFMT "' = '" ALPASCALSTRFMT "'\n",
			 ALPASCALSTRARGS(entry->key.length, (char *) entry->key.data.ptr),
			 ALPASCALSTRARGS(entry->value.length, (char *)  entry->value.data.ptr));
	}
      else
	{
	   printf( "%p NULL\n", entry);
	}
    }
  return 0;
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
	  alhash_context context;
	  struct alhash_table * table;	  

	  // 10 buckets (different expected words) starting with 32 bytes with 170/256th autogrowth
	  alhash_context_init(&context,10,32,170);
	  table = &context.dict;
	  
	  while ((read = getline(&line, &len, f)) != -1) {
	    int i = 0;
	    while ( (i<=read) && ( line[i] != 0 )  && ( line[i] != ' ' ) && ( line[i] != '\t') )
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
		  struct alhash_entry * entry = alhash_get_entry(table, &key);
		  if ( entry == NULL )
		    {
		      // remove heading space and last char ( return )
		      value.length=read-i-2;
		      value.data.ptr=&line[i+1];
		      value.type=ALTYPE_OPAQUE;
		      entry = alhash_put(table, &key, &value);
		      if ( entry != NULL )
			{
			  alhash_dump_entry_as_string(entry);
			}
		      else
			{
			  fprintf(stderr,"[ERROR] alhash_put '%s' failed\n", key.data.charptr);
			}
		    }
		  else
		    {
		      // printf("%s ALREADY ADDED", line);
		      free(line);
		    }
		}
	    }
	    line = NULL; // force realloc for every line, see manpage getline(3)
	  }	     
	  fclose(f);
	  printf("\ninternal ordering dump \n");
	  alhash_walk_table(table, alhash_walk_callback_dump, NULL);
	  printf("\nclassical dump \n");
	  alhash_walk_table(table, alhash_walk_callback_simple_dump, NULL);
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
