#include "aljson_ld.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "aldebug_output.h"

// number of fields
const int ALJSONLD_KEYWORDS_FIELDS=3;

enum aljsonld_field
  {
   ALJSONLD_KEYWORD_FIELD=0,   
   ALJSONLD_EXPLANATION_FIELD=1,
   ALJSONLD_C_KEYWORD_FIELD=2 
  };

char * aljson_ld_keywords[] = {
  // keyword, explanation, placeholder for c keyword
  "@context","Used to define the short-hand names that are used throughout a JSON-LD document. These short-hand names are called terms and help developers to express specific identifiers in a compact manner. The @context keyword is described in detail in section 5.1 The Context.",NULL,
  "@id","Used to uniquely identify things that are being described in the document with IRIs or blank node identifiers. This keyword is described in section 5.3 Node Identifiers.",NULL,
  "@value","Used to specify the data that is associated with a particular property in the graph. This keyword is described in section 6.9 String Internationalization and section 6.4 Typed Values.",NULL,
  "@language","Used to specify the language for a particular string value or the default language of a JSON-LD document. This keyword is described in section 6.9 String Internationalization.",NULL,
  "@type","Used to set the data type of a node or typed value. This keyword is described in section 6.4 Typed Values.",NULL,
  "@container","Used to set the default container type for a term. This keyword is described in section 6.11 Sets and Lists.",NULL,
  "@list","Used to express an ordered set of data. This keyword is described in section 6.11 Sets and Lists.",NULL,
  "@set","Used to express an unordered set of data and to ensure that values are always represented as arrays. This keyword is described in section 6.11 Sets and Lists.",NULL,
  "@reverse","Used to express reverse properties. This keyword is described in section 6.12 Reverse Properties.",NULL,
  "@index","Used to specify that a container is used to index information and that processing should continue deeper into a JSON data structure. This keyword is described in section 6.16 Data Indexing.",NULL,
  "@base","Used to set the base IRI against which relative IRIs are resolved. This keyword is described in section 6.1 Base IRI.",NULL,
  "@vocab","Used to expand properties and values in @type with a common prefix IRI. This keyword is described in section 6.2 Default Vocabulary.",NULL,
  "@graph","Used to express a graph. This keyword is described in section 6.13 Named Graphs.",NULL,
  ":","The separator for JSON keys and values that use compact IRIs.",NULL,
  0
};


const int aljsons_ld_keyword_field_index(int index, int field)
{
  return (index * ALJSONLD_KEYWORDS_FIELDS) + field;
}

const char * aljson_ld_keyword(int index)
{
  return aljson_ld_keywords[aljsons_ld_keyword_field_index(index,ALJSONLD_KEYWORD_FIELD)];
}

const char * aljson_ld_c_keyword(int index)
{
  return aljson_ld_keywords[aljsons_ld_keyword_field_index(index,ALJSONLD_C_KEYWORD_FIELD)];
}

const char * aljson_ld_c_keyword_explanation(int index)
{
  return aljson_ld_keywords[aljsons_ld_keyword_field_index(index,ALJSONLD_EXPLANATION_FIELD)];
}

struct aljson_ld_context aljson_ld_global_context;
struct alhash_table * aljson_ld_keyword_table = NULL;

char * strtoupper(const char * origin, alhash_context * hash_ctx) {
  char * dest = NULL;
  if ( hash_ctx != NULL)
    {
      dest = ALALLOC(hash_ctx->allocator, strlen(origin)+1);
    }
  else
    {
      dest = (char *) malloc( strlen(origin)+1);
    }
  int skip = 0;
  int index = 0;
  char c = origin[index];
  while (c != 0) {
    if (c != '@' )
      {
	if ( c == ':' )
	  {
	    c = 'X';
	  }
	dest[index-skip] = toupper((unsigned char) c);
      }
    else
      {
	skip++;
      }
    index ++;
    c = origin[index];
  }
  dest[index-skip]=0;
  return dest;
}

int aljson_ld_is_keyword_internal(char * string, struct alhash_table * table)
{
  struct alhash_datablock key;
  struct alhash_datablock value;
  key.type=ALTYPE_OPAQUE;
  key.length=strlen(string);
  key.data.charptr=string;
  struct alhash_entry * entry = alhash_get_entry(table, &key);
  if ( entry != NULL )
    {
      // using ALTYPE_FLAG_EMBED can be 0
      return entry->value.data.number;
    }

  // NYI
  return -2;
}

void aljson_ld()
{
  printf("[INFO] json_ld is an extension of aljson for json_ld support\n");
  printf("[WARNING] NOT YET IMPLEMENTED\n");

  int index = 0;

  struct alhash_table * table;
  
  alhash_context_init(&aljson_ld_global_context.hash_context,10,32,170);
  table = &aljson_ld_global_context.hash_context.dict;


  while (  aljson_ld_keywords[index * ALJSONLD_KEYWORDS_FIELDS] != NULL )
    {
      char * keystr=aljson_ld_keywords[index * ALJSONLD_KEYWORDS_FIELDS];
      char * valuestr=aljson_ld_keywords[1+(index * ALJSONLD_KEYWORDS_FIELDS)];

      // printf("%i %s %s\n", index, keystr, valuestr);
      
      struct alhash_datablock key;
      struct alhash_datablock value;
      key.type=ALTYPE_OPAQUE;
      key.length=strlen(keystr);
      key.data.charptr=keystr;
      struct alhash_entry * entry = alhash_get_entry(table, &key);
      if ( entry == NULL )
	{
	  // use of ALTYPE_FLAG_EMBED
	  value.length=strlen(valuestr);
	  value.data.number=index;
	  value.type=ALTYPE_FLAG_EMBED;
	  entry = alhash_put(table, &key, &value);
	}
      else
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] unexpected duplicated json_ld keyword %s", keystr);
	}
      index ++;
    }

  aljson_ld_keyword_table=table;

  // fill C KEYWORD
  index = 0;
  while (  aljson_ld_keywords[index * ALJSONLD_KEYWORDS_FIELDS] != NULL )
    {
      char * up = strtoupper(aljson_ld_keywords[index * ALJSONLD_KEYWORDS_FIELDS], &aljson_ld_global_context.hash_context);
      // printf("ALJSONLD_KEYWORD_%s_IDX,\n", up);
      // fill keyword C naming.
      aljson_ld_keywords[((index+1) * ALJSONLD_KEYWORDS_FIELDS) -1]=up;
      index ++;
    }
}

void aljson_ld_list_keywords()
{
  int index = 0;
  while (  aljson_ld_keywords[index * ALJSONLD_KEYWORDS_FIELDS] != NULL )
    {
      const char * keyword = aljson_ld_keyword(index);
      const char * explanation =  aljson_ld_c_keyword_explanation(index);
      const char * c_keyword =  aljson_ld_c_keyword(index);
      printf("%i %s %s %s\n", index, keyword, c_keyword, explanation);
      index ++;
    }

}

enum aljson_ld_keyword_index aljson_ld_is_keyword(char * string)
{
  if ( aljson_ld_keyword_table != NULL )
    {
      return aljson_ld_is_keyword_internal(string, aljson_ld_keyword_table);
    }
  else
    {
      // INITIALIZATION ERROR
      return -2;
    }
}

struct aljson_ld_named_graph * aljson_ld_build_from_json(struct aljson_ld_context* context, struct json_object * root)
{
  printf("%s:%i NOT YET implemented\n",__FILE__,__LINE__);
  return NULL;
}
