#include "alhash.h"
#include "alhash_output.h"

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

int main(int argc, char ** argv)
{
  struct aloutputstream output;
  aloutputstream_fd_init(&output, fileno(stdout));

  struct aloutputstream output2;
  aloutputstream_fd_init(&output2, fileno(stderr));
  
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
	  aloutputstream_printf_1k(&output2,"internal ordering dump \n");
	  alhash_walk_table(table, alhash_output_dump_entry_callback_cast_outputstream, (void *) &output2);
	  aloutputstream_printf_1k(&output,"classical dump \n");
	  alhash_walk_table(table, alhash_output_walk_simple_callback_cast_outputstream, (void *) &output);
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
