#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "al_options.h"
#include "todo.h"

void al_option_add(struct al_options * options, char * ikey, char * ivalue)
{
  struct alhash_datablock key;
  int withnullbyte=1; // include null byte '\0'
  key.length = strlen(ikey) + withnullbyte; 
  key.data = ikey;
  struct alhash_entry *entry =  alhash_get_entry(&options->table, &key);
  if (entry == NULL)
    {
      struct alhash_datablock value;
      printf("add '%s'='%s' in options\n",ikey,ivalue);
      key.data=al_copy_block(&options->buffer, &key);
      key.length -= withnullbyte; // don't keep null byte for hash...
      value.length=strlen(ivalue) + withnullbyte;
      value.data =ivalue;
      value.data=al_copy_block(&options->buffer,&value);

      entry = alhash_put (&options->table, &key, &value);
      if (entry == NULL)
	{
	  fprintf (stderr,
		   "[FATAL] FAIL to insert '%s:%s' into options\n", key,value);
	}
      else
	{
	  printf("entry '%s'='%s'\n", entry->key.data, entry->value.data);
	}
    }
  else
    {
      printf("DON'T add '%s'='%s' in options, key entry already exists\n",ikey,ivalue);
    }
}

void al_options_init(struct al_options * options)
{
  bzero(options,sizeof(*options));
  todo ("support a growable options. here limited to 1024 options");
  alhash_init (&options->table, 1024, NULL);
  todo ("support a growable word buffer. here limited to 10240 characters");
  options->buffer.bufsize = 10240;
  options->buffer.buf= malloc(options->buffer.bufsize);  
}

void al_options_release(struct al_options * options)
{
  if (options->buffer.buf != NULL )
    {
      free(options->buffer.buf);
      options->buffer.buf=NULL;
      options->buffer.bufsize=0;
    }
  alhash_release(&options->table);
}

struct al_options * al_create_options(int argc, char ** argv)
{
  struct al_options * options = malloc(sizeof(*options));
  al_options_init(options);

  for (int i=0; i< argc;i++)
    {
      // parse x=y
      char * key = NULL;
      char * value = NULL;
      sscanf(argv[i],"%m[^=]=%m[^=]",&key,&value);      
      if ( ( key != NULL )  && (value != NULL))
	{
	  printf("option recognized : '%s'='%s'\n",key,value);
	  al_option_add(options,key,value);
	  free(key);
	  free(value);
	}
    }
  return options;
}

struct alhash_datablock * al_option_get(struct al_options * options, char * ikey)
{
  struct alhash_datablock key;
  key.length = strlen(ikey);
  key.data = ikey;
  struct alhash_entry *entry =  alhash_get_entry (&options->table, &key);
  if (entry == NULL)
    {
      printf("option %s, NOT FOUND\n",ikey);
      return NULL;
    }
  else
    {
      return &entry->value;
    }
}
