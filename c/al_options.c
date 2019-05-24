#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "al_options.h"
#include "altodo.h"
#include "aldebug.h"
#include "aldebug_output.h"

// protect against too long short options.
const int MAXOPTIONS=1024;

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
	  aldebug_printf(NULL, "[FATAL] FAIL to insert '%s:%s' into options\n", key.data.charptr,value.data.charptr);
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

void al_option_parse_multivalued(struct al_options * options,const char * ikey,const char * ivalue)
{
  if (( ivalue != NULL) && (ivalue[0] == '[' ) )
    {
      char arraykey[1024];
      char buffer[1024];
      int bufindex= 0;
      // mutlivalued case ivalue[0] assumed to be '['
      int charindex = 1;
      // if set to 1 will collect word constructed in buffer.
      int collect = 0;
      // index and number of elements
      int index = 0;
      
      while (charindex > 0)
	{
	  char current_char = ivalue[charindex];
	  
	  if (( current_char == ']' ) || ( current_char == 0 ))
	    {
	      // last element collection
	      // ignore anything after ']'
	      charindex = 0;
	      collect = 1;
	    }
	  else if (current_char == ',')
	    {
	      // ignore separator but collect
	      collect = 1;
	      charindex++;
	    }
	  else
	    {
	      collect = 0;
	      buffer[bufindex]=current_char;             ;
	      bufindex++;
	      charindex++;
	    }
	  if ( collect == 1 )
	    {
	      if ( bufindex > 0 )
		{
		  buffer[bufindex]=0;
		  snprintf(arraykey,1024,"%s[%i]",ikey,index);
		  al_option_add(options,arraykey,buffer);
		  bufindex = 0;
		}
	      index ++;
	    }
	}

      // store number of elements directly as embedded int ( system endianness )
      {
	// this is number of elements key#
	snprintf(arraykey,1024,"%s#",ikey);

        struct alhash_datablock key;
	int withnullbyte=1; // include null byte '\0'
	key.type = ALTYPE_STR0;
	key.length = strlen(arraykey) + withnullbyte; 
	key.data.constcharptr = arraykey;

	struct alhash_entry *entry = alhash_get_entry(&options->context.dict, &key);
	if ( entry == NULL )
	  {
	    struct alhash_datablock value;
	    // not true given length provided but type should the same
	    key.type = ALTYPE_STR0;
	    key.data.ptr=al_copy_block(&options->context.allocator.ringbuffer, &key);
	    key.length -= withnullbyte; // don't keep null byte for hash...
	    value.type = ALTYPE_FLAG_EMBED;
	    value.length = 0;
	    value.data.number=index;
	    entry = alhash_put (&options->context.dict, &key, &value);
	    if (entry == NULL)
	      {
		aldebug_printf(NULL, "[FATAL] FAIL to insert '%s:%i' into options\n", key.data.charptr,index);
	      }
	    else
	      {
		ALDEBUG_IF_DEBUG(options, al_options, debug)
		  {
		    aldebug_printf(NULL,"[DEBUG] entry '%s'='i\n", entry->key.data.charptr, index);
		  }
	      }
	  }
      }

    }
  else
    {
      al_option_add(options,ikey,ivalue);
    }
}

void al_options_init(struct al_options * options)
{
  bzero(options,sizeof(*options));
  // HARDCODED 15 words, 1024 bytes initial buffer and 78% (200/256th)
  alhash_context_init(&options->context,15,1024,200);
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
  // WARNING HARDCODED LIMIT 1024 chars
  char buffer[1024];
  
  al_options_init(options);
  int checkshortoptions = 1;

  // parse x=y
  char * key = NULL;
  char * value = NULL;

  int argnumber = 0;

  for (int i=0; i< argc;i++)
    {

      if (checkshortoptions == 1 )
	{
	  char * arg = argv[i];
	  char first = arg[0];
	  if ( first == '-' )
	    {
	      // option case
	      if ( arg[1] == '-' )
		{
		  // long option
		  if ( arg[2] == 0 )
		    {
		      // options separator '--'
		      checkshortoptions = 0;
		    }
		  // prefix '--'
		  // TODO --longoption=value
		  // current accept only longoption without any value
		  al_option_add(options,&arg[2],"true");
		}
	      else
		{
		  // short option
		  // any following char considered as option char
		  buffer[1]=0;
		  int index = 1;
		  int maxloop = MAXOPTIONS;
		  while ((arg[index] != 0 ) && (index < maxloop))
		    {
		      // TODO should pick corresponding key longoption name if defined.
		      buffer[0]=arg[index];
		      al_option_add(options,buffer,"true");
		      index ++;
		    }
		}
	      continue;
	    }
      
	  // key=value
	  sscanf(argv[i],"%m[^=]=%m[^=]",&key,&value);
	  if ( ( key != NULL )  && (value != NULL))
	    {
	      ALDEBUG_IF_DEBUG(options, al_options, debug)
		{
		  aldebug_printf(NULL,"[DEBUG] option recognized : '%s'='%s'\n",key,value);
		}

	      // shoudl support key=[value0,value1,...] => "key[0]", "key[1]" ..., "key#" = 2
	      al_option_parse_multivalued(options,key,value);
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
      else
	{
	  // directly create arg[0], arg[1] arg[2]...., 
	  snprintf( buffer, 1024, "arg[%i]", argnumber);
	  al_option_add(options,buffer,argv[i]);
	  argnumber++;
	  key=NULL;
	}
    }

  options->argsnumber=argnumber;
  
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

void al_option_dump(struct al_options * options, struct aloutputstream output)
{
  return;
}

int al_option_get_embed_number(struct al_options * options, const char * ikey)
{
  struct alhash_datablock * entry = al_option_get(options,ikey);
  if ( entry != NULL )
    {
      if (ALC_FLAG_IS_SET(entry->type,ALTYPE_FLAG_EMBED))
	{
	  return (int) entry->data.number;
	}		
    }

  return 0;
}

int al_option_getargsnumber(struct al_options * options)
{
  if ( options->argsnumber > 0 )
    {
      return options->argsnumber;
    }
  else
    {
      return al_option_get_embed_number(options,"arg#");
    }
  return 0;
}

char * al_option_array_at(struct al_options * options, const char * name,int arg)
{
  char buffer[1024];

  snprintf(buffer, 1024, "%s[%i]",name,arg);
  struct alhash_datablock * value = al_option_get(options,buffer);
  if ( value != NULL )
    {
      return value->data.charptr;
    }
  else
    {
      return NULL;
    }
}

char * al_option_getarg(struct al_options * options, int arg)
{
  if ( arg < al_option_getargsnumber(options))
    {
      return al_option_array_at(options,"arg",arg);
    }
  else
    {
      return NULL;
    }
}

