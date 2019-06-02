#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "al_options.h"
#include "altodo.h"
#include "aldebug.h"
#include "aldebug_output.h"

// protect against too long short options.
const int MAXOPTIONS=1024;

// max characters in generated keys and parsed multivalues
#define ALOPTION_MAX_CHAR_BUFFER 1024

#define FINAL_NUL '\0'

// NO debugging, set it do DBGSTREAM to get debug
#define DBGOPTIONSTREAM NULL

ALDEBUG_DEFINE_FUNCTIONS(struct al_options, al_options, debug);

int al_options_get_duplicates(struct al_options * options, struct alhash_datablock * key)
{
  todo("al_options_get_duplicates NYI");
  // NYI
  return -1;
}

void al_options_add_duplicate(struct al_options * options,struct alhash_datablock * key,int current_index,struct alhash_datablock * value)
{
  todo("al_options_add_duplicates NYI");
  // NYI
}

// will copy block pointed by str->data and convert it to STR0 if a SUBSTR
void al_option_copy_str(struct al_options * options,
			// in and out
			struct alhash_datablock * str)
{
  alstrings_copy_str_block(&options->context.allocator.ringbuffer, str);
}

struct alhash_entry * al_option_copy_put(struct al_options * options,
					 struct alhash_datablock * key,
					 struct alhash_datablock * value)
{
  struct alhash_entry *entry =  alhash_get_entry(&options->context.dict, key);
  if (entry == NULL)
    {
      aldebug_printf( DBGOPTIONSTREAM,"new key length %i "ALPASCALSTRFMT"\n",key->length,ALPASCALSTRARGS(key->length,key->data.charptr));
      al_option_copy_str(options,key);
      al_option_copy_str(options,value);      
      entry = alhash_put(&options->context.dict, key, value);
      if (entry == NULL)
	{
	  aldebug_printf( DBGOPTIONSTREAM, "[FATAL] FAIL to insert '%s:%s' into options\n", key->data.charptr,value->data.charptr);
	}
      else
	{
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] entry '%s'='%s'\n", entry->key.data.charptr, entry->value.data.charptr);
	    }
	}
    }
  else
    {
      // todo NYI
      // could try to create "key[]"=["value1","value2",...] or ... "key[1]"="value1" "#key" = 2...
      int duplicates = al_options_get_duplicates(options,key);
      if ( duplicates >= 0 )
	{
	  // means support duplicates NYI
	  al_options_add_duplicate(options,key,duplicates,value);
	}
      else
	{	
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] DON'T add '%s'='%s' in options, key entry already exists\n",key->data.charptr,value->data.charptr);
	    }
	}
    }
  return entry;
}

void al_option_add(struct al_options * options,const char * ikey,const char * ivalue)
{
  struct alhash_datablock key;
  struct alhash_datablock value;
  
  int withnullbyte=1; // include null byte '\0'
  key.type = ALTYPE_STR0;
  key.length = strlen(ikey) + withnullbyte; 
  key.data.constcharptr = ikey;
  
  value.type = ALTYPE_STR0;
  value.data.constcharptr=ivalue;
  value.length=strlen(ivalue) + withnullbyte;

  al_option_copy_put(options,
		     &key,
		     &value);

}

void al_option_parse_multivalued(struct al_options * options,
				 struct alhash_datablock * keybloc,
				 struct alhash_datablock * valuebloc)
{
  const char * ikey = keybloc->data.constcharptr;
  int keylength = keybloc->length;
  const char * ivalue = valuebloc->data.constcharptr;
  int valuelength = valuebloc->length;

  ALDEBUG_IF_DEBUG(options, al_options, debug)  aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] parse multivalued \n");
  if (( ivalue != NULL) && (ivalue[0] == '[' ) )
    {
      ALDEBUG_IF_DEBUG(options, al_options, debug) aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] '[' match \n");
      char arraykey[ALOPTION_MAX_CHAR_BUFFER];
      // mutlivalued case ivalue[0] assumed to be '['
      int charindex = 1;
      // if set to 1 will collect word constructed in buffer.
      int collect = 0;
      // index and number of elements
      int index = 0;
      // pointer on start of value
      const char * mvalue = &ivalue[charindex];
      // length of multivalued string mvalue in chars
      int mvaluelength= 0;

      while ( charindex > 0 ) // && (charindex < valuelength ) ) NUL case to handle somehow.
	{
	  char current_char = ivalue[charindex];

	  if (( current_char == ']' ) || ( current_char == FINAL_NUL ) || ( charindex==valuelength) )
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
	      if ( mvalue  == NULL )
		{
		  mvalue = &ivalue[charindex];
		}
	      mvaluelength++;
	      charindex++;
	    }
	  if ( collect == 1 )
	    {
	      if ( mvaluelength > 0 )
		{
		  snprintf(arraykey,ALOPTION_MAX_CHAR_BUFFER,ALPASCALSTRFMT"[%i]",ALPASCALSTRARGS(keylength,ikey),index);

		  {
		    struct alhash_datablock key;
		    struct alhash_datablock value;
  
		    int withnullbyte=1; // include null byte FINAL_NUL
		    key.type = ALTYPE_STR0;
		    key.length = strlen(arraykey) + withnullbyte; 
		    key.data.constcharptr = arraykey;

		    // collected value
		    value.type = ALTYPE_SUBSTR;
		    value.data.constcharptr=mvalue;
		    value.length=mvaluelength;

		    al_option_copy_put(options,
				       &key,
				       &value);
		  }
		  mvalue=NULL;
		  mvaluelength = 0;
		}
	      index ++;
	    }
	}

      // store number of elements directly as embedded int ( system endianness )
      {
	// this is number of elements key#
	snprintf(arraykey,ALOPTION_MAX_CHAR_BUFFER,ALPASCALSTRFMT"#",ALPASCALSTRARGS(keylength,ikey));

        struct alhash_datablock key;
	int withnullbyte=1; // include null byte FINAL_NUL
	key.type = ALTYPE_STR0;
	key.length = strlen(arraykey) + withnullbyte; 
	key.data.constcharptr = arraykey;

	struct alhash_entry *entry = alhash_get_entry(&options->context.dict, &key);
	if ( entry == NULL )
	  {
	    struct alhash_datablock value;
	    key.type = ALTYPE_STR0; // why set it again ? 
	    key.data.ptr=al_copy_block(&options->context.allocator.ringbuffer, &key);
	    value.type = ALTYPE_FLAG_EMBED;
	    value.length = 0;
	    value.data.number=index;
	    entry = alhash_put (&options->context.dict, &key, &value);
	    if (entry == NULL)
	      {
		aldebug_printf( DBGOPTIONSTREAM, "[FATAL] FAIL to insert '%s:%i' into options\n", key.data.charptr,index);
	      }
	    else
	      {
		ALDEBUG_IF_DEBUG(options, al_options, debug)
		  {
		    aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] entry '%s'='i\n", entry->key.data.charptr, index);
		  }
	      }
	  }
      }

    }
  else
    {
      aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] non multivalue \n");

      al_option_copy_put(options,
			 keybloc,
			 valuebloc);

    }
}

void al_options_init(struct al_options * options)
{
  bzero(options,sizeof(*options));
  // HARDCODED 15 words, ALOPTION_MAX_CHAR_BUFFER bytes initial buffer and 78% (200/256th)
  alhash_context_init(&options->context,15,ALOPTION_MAX_CHAR_BUFFER,200);
}

// parse x=y
void al_options_parse_key_value(struct al_options * options,const char * arg)
{

  struct alhash_datablock keybloc;
  struct alhash_datablock valuebloc;

  int keylength = 0;
  int valuelength = 0;
  const char * key = NULL;
  const char * value = NULL;

  int index = 0;
  char c = arg[index];
  key=arg;
  while ( c != FINAL_NUL )
    {
      switch(c)
	{
	case '=':
	  {
	    value = &arg[index+1];
	    while ( c != FINAL_NUL )
	      {
		valuelength++;     
		index++;
		c=arg[index];
	      }	    
	  }
	  goto parsed;
	  break;
	default:
	  keylength++;      
	}
      index++;
      c=arg[index];
    }  

 parsed:
  keybloc.type=ALTYPE_SUBSTR;
  keybloc.data.constcharptr=key;
  keybloc.length=keylength;

  valuebloc.type=ALTYPE_SUBSTR;
  valuebloc.data.constcharptr=value;
  valuebloc.length=valuelength;
  
 // key=value
 // was sscanf(arg,"%m[^=]=%m[^=]",&key,&value);
 if ( ( key != NULL )  && (value != NULL))
   {
     ALDEBUG_IF_DEBUG(options, al_options, debug)
       {
	 aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] option recognized : '%s'='%s'\n",key,value);
       }

     // support key=[value0,value1,...] => "key[0]", "key[1]" ..., "key#" = 2     
     al_option_parse_multivalued(options,&keybloc,&valuebloc);
   }
 else
   {
     // key only
     // was sscanf(arg,"%m[^=]",&key);
     ALDEBUG_IF_DEBUG(options, al_options, debug)
       {
	 aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] option recognized : '--%s' ( as %s=true) \n",key,key);
       }
     al_option_add(options,key,"true");
   }

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
  // WARNING HARDCODED LIMIT ALOPTION_MAX_CHAR_BUFFER chars
  char buffer[ALOPTION_MAX_CHAR_BUFFER];
  
  al_options_init(options);
  int checkshortoptions = 1;
  int argnumber = 0;

  // starts from 1 since argv[0] is program name
  for (int i=1; i< argc;i++)
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

	  al_options_parse_key_value(options,arg);
	}
      else
	{
	  // directly create arg[0], arg[1] arg[2]...., 
	  snprintf( buffer, ALOPTION_MAX_CHAR_BUFFER, "arg[%i]", argnumber);
	  al_option_add(options,buffer,argv[i]);
	  argnumber++;
	}
    }

  options->argsnumber=argnumber;
  
  return options;
}

struct alhash_datablock * al_option_get(struct al_options * options,const char * ikey)
{
  struct alhash_datablock key;
  int withnullbyte=1; // include null byte FINAL_NUL

  key.type = ALTYPE_STR0;
  key.length = strlen(ikey) + withnullbyte;
  key.data.constcharptr = ikey;
  struct alhash_entry *entry =  alhash_get_entry (&options->context.dict, &key);
  if (entry == NULL)
    {
	  ALDEBUG_IF_DEBUG(options, al_options, debug)
	    {
	      aldebug_printf( DBGOPTIONSTREAM,"[DEBUG] option %s, NOT FOUND\n",ikey);
	    }
      return NULL;
    }
  else
    {
      return &entry->value;
    }
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
  char buffer[ALOPTION_MAX_CHAR_BUFFER];

  snprintf(buffer, ALOPTION_MAX_CHAR_BUFFER, "%s[%i]",name,arg);
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

