#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "al_options.h"
#include "altodo.h"

ALDEBUG_DEFINE_FUNCTIONS(struct al_options, al_options, debug);

int al_options_get_duplicates(struct al_options * options,const char * ikey)
{
  // NYI
  return -1;
}

void al_option_add_duplicate(struct al_options * options,const char * ikey,int current_index,const char * ivalue)
{
  // NYI
}

void al_option_add(struct al_options * options,const char * ikey,const char * ivalue)
{
  struct alhash_datablock key;
  int withnullbyte=1; // include null byte '\0'
  key.type = ALTYPE_STR0;
  key.length = strlen(ikey) + withnullbyte; 
  key.data.constcharptr = ikey;
  struct alhash_entry *entry =  alhash_get_entry(&options->context.dict, &key);
  if (entry == NULL)
    {
      struct alhash_datablock value;

      // not true given length provided but type should the same
      key.type = ALTYPE_STR0;
      key.data.ptr=al_copy_block(&options->context.allocator.ringbuffer, &key);
      key.length -= withnullbyte; // don't keep null byte for hash...
      value.length=strlen(ivalue) + withnullbyte;
      value.data.constcharptr=ivalue;
      // using al_copy_block allows to have data block autogrowth.
      value.data.ptr=al_copy_block(&options->context.allocator.ringbuffer,&value);

      entry = alhash_put (&options->context.dict, &key, &value);
      if (entry == NULL)
	{
	  fprintf (stderr,
		   "[FATAL] FAIL to insert '%s:%s' into options\n", key.data.charptr,value.data.charptr);
	}
      else
	{
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf(NULL,"[DEBUG] entry '%s'='%s'\n", entry->key.data.charptr, entry->value.data.charptr);
	    }
	}
    }
  else
    {
      // could try to create "key[]"=["value1","value2",...] or ... "key[1]"="value1" "#key" = 2...
      int duplicates = al_options_get_duplicates(options,ikey);
      if ( duplicates >= 0 )
	{
	  // means support duplicates NYI
	  al_option_add_duplicate(options,ikey,duplicates,ivalue);
	}
      else
	{	
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf(NULL,"[DEBUG] DON'T add '%s'='%s' in options, key entry already exists\n",ikey,ivalue);
	    }
	}
    }
}

void al_options_init(struct al_options * options)
{
  bzero(options,sizeof(*options));
  alparser_init(&options->context,15,1024);
}

void al_options_release(struct al_options * options)
{
  // case of dedicated ringbuffer ... should check.
  if (options->context.allocator.ringbuffer != NULL )
    {
      alstrings_ringbuffer_release(&options->context.allocator.ringbuffer);
    }
  alhash_release(&options->context.dict);
}

struct al_options * al_options_create(int argc, char ** argv)
{
  struct al_options * options = malloc(sizeof(*options));
  al_options_init(options);

  for (int i=0; i< argc;i++)
    {
      // parse x=y
      char * key = NULL;
      char * value = NULL;
      // key=value
      sscanf(argv[i],"%m[^=]=%m[^=]",&key,&value);
      if ( ( key != NULL )  && (value != NULL))
	{
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf(NULL,"[DEBUG] option recognized : '%s'='%s'\n",key,value);
	    }
	  al_option_add(options,key,value);
	}
      else
	{
	  // key only
	  sscanf(argv[i],"%m[^=]",&key);
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf(NULL,"[DEBUG] option recognized : '--%s' ( as %s=true) \n",key,key);
	    }
	  al_option_add(options,key,"true");	  
	}
	if (key != NULL )
	  {
	  free(key);
	  key=NULL;
	}
	if (value != NULL)
	  {
	  free(value);
	  value=NULL;
	}
	
    }
  return options;
}

struct alhash_datablock * al_option_get(struct al_options * options,const char * ikey)
{
  struct alhash_datablock key;
  // not true, since strlen does not include '\0'
  key.type = ALTYPE_STR0;
  key.length = strlen(ikey);
  key.data.constcharptr = ikey;
  struct alhash_entry *entry =  alhash_get_entry (&options->context.dict, &key);
  if (entry == NULL)
    {
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf(NULL,"[DEBUG] option %s, NOT FOUND\n",ikey);
	    }
      return NULL;
    }
  else
    {
      return &entry->value;
    }
}
