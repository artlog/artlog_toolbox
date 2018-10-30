#include "aljson_walk.h"
#include "aljson_dump.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct json_object * json_dict_path_get_value(struct json_path * path, struct json_object * object)
{
  int i;
  struct json_object * value = NULL;
  if (object->dict.nitems > 0)
    {
      for(i=0;(value == NULL) && (i< object->dict.nitems) ;i++)
	{
	  struct json_pair * pair = object->dict.items[i];
	  if ( pair != NULL )
	    {
	      if ( path->string.internal.length == pair->key->string.internal.length )
		{
		  if ( strncmp( pair->key->string.internal.data.ptr, path->string.internal.data.ptr, path->string.internal.length) == 0 )
		    {
		      value = pair->value;
		    }
		}
	    }
	}
    }
  return value;
}

/**
 WARNING json_path itself is used for strings 
if given_pathes is NULL then it will be dynamically allocated else it should have been zeroed before use.
**/
struct json_path * create_json_path(int maxlength, char * json_path, struct json_parser_ctx * ctx, int max_depth, struct json_path * given_pathes)
{
  struct json_path * pathes = given_pathes;
  if ( pathes == NULL )
    {      
      pathes=calloc(max_depth, sizeof(struct json_path));
    }
  if ( pathes != NULL )
    {      
      char * current = json_path;
      int pos = 0; // position of current within json_path
      int digits = 1;
      int json_path_index = -1;
      int next_path = 0;
      char next_type = '*';
      while ( ( pos < maxlength) && ( *current != 0 ) && (json_path_index < max_depth) )
	{
	  char c = *current;
	  if ( c == '.' )
	    {
	      // next path
	      next_path=1;
	      next_type='*';
	    }
	  else if ( (  c == '['  ) || ( ( c == '{') ))
	    {
	      next_path=1;
	      next_type=c;
	    }
	  else if ( ( next_type == '[' ) && ((c) == ']') )
	    {
	      next_path=1;
	      next_type='*';
	    }
	  else if ( ( next_type == '{' ) && ((c) == '}') )
	    {
	      next_path=1;
	      next_type='*';
	    }
	  else
	    {
	      if ( next_path == 1 )
		{
		  ++ json_path_index;
		  next_path = 0;
		}
	      ALDEBUG_IF_DEBUG(&ctx->alparser,alparser_ctx,1)
		{
		  printf("json_path_index:%i pos:%i %s\n", json_path_index, pos, current);
		}

	      if ( json_path_index < 0 )
		{
		  json_path_index = 0;
		}
	      pathes[json_path_index].child = NULL;
	      if (pathes[json_path_index].string.internal.data.ptr == NULL )
		{
		  pathes[json_path_index].type = next_type;
		  pathes[json_path_index].string.internal.data.ptr = current;
		  pathes[json_path_index].string.internal.length = 1;
		  pathes[json_path_index].index = 0;
		  digits=1;
		}
	      else
		{
		  pathes[json_path_index].string.internal.length ++;
		}
	      if ( json_path_index > 0 )
		{
		  pathes[json_path_index-1].child=&pathes[json_path_index];
		}
	      if ( digits > 0 )
		{
		  if ( ( (c) <= '9' ) && ( (c) >= '0' ) )
		    {
		      pathes[json_path_index].index+=((c) - '0') * digits;
		      digits *= 10;
		    }
		  else
		    {
		      pathes[json_path_index].index=-1;
		      digits=0;
		    }
		}
	    }
	  ++ pos;
	  current=&(json_path[pos]);	  
	}
      return &pathes[0];
    }
  else
    {
      return NULL;
    }
}

struct json_object * aljson_walk_path(char * json_path, struct json_parser_ctx * ctx, struct json_object * object)
{
  struct json_path * json_path_object = create_json_path(JSON_PATH_MAX_CHARS,json_path,ctx,JSON_PATH_DEPTH,NULL);
  struct json_object * current_object = object;
  
  if ( json_path_object != NULL )
    {
      struct json_path * current_path = json_path_object;
      aljson_dump_json_path(json_path_object);

      int watchguard = JSON_PATH_DEPTH ;
      while ( ( current_path != NULL ) && ( watchguard > 0 ) && (current_object != NULL ) )
	{
	  ++ watchguard;
	  if (( current_path->type == '*' ) || ( current_path->type ==  current_object->type ))
	    {
	      if (current_object->type == '{' )
		{
		  // should find the right key
		  struct json_object * value = json_dict_path_get_value(current_path, current_object);
		  if ( value == NULL )
		    {
		      printf("[ERROR] path keyname " ALPASCALSTRFMT " not found\n",
			     ALPASCALSTRARGS(current_path->string.internal.length, (char *) current_path->string.internal.data.ptr));
		      current_object = NULL;
		    }
		  else
		    {
		      current_object = value;
		    }
		}
	      else if (current_object->type == '[' )
		{
		  if ( current_path->index < 0 )
		    {
		      printf("[ERROR] path type index %i invalid\n", current_path->index);
		      current_object = NULL;
		    }
		  else
		    {
		      // should find the right index
		      struct json_object * value = json_list_get(current_object,current_path->index);
		      if ( value == NULL )
			{
			  printf("[ERROR] path index %i not found.\n", current_path->index);
			  current_object = NULL;
			}
		      else
			{
			  current_object = value;
			}
		    }
		}
	      else
		{
		  printf("[FATAL] json object type not supported for path search %c", current_object->type);
		  current_object = NULL;
		}
	    }
	  else
	    {
	      printf("[ERROR] path type non matching object %c != %c\n",  current_object->type, current_path->type);
	      current_object = NULL;
	    }
	  current_path = current_path->child;
	}
      free(json_path_object);
    }
  return current_object;
}
