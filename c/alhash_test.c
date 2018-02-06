#include "alhash.h"

#include <stdio.h>
#include <stdlib.h>


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
	  struct alparser_ctx context;
	  struct alhash_table * table;	  

	  alparser_init(&context,10,32);
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
	  alhash_walk_table(table, alhash_walk_callback_dump, NULL);
	}
      else
	{
	  aldebug_printf(NULL,"%s not found", filename);
	}
    }
  else
    {
      usage();
    }
}
