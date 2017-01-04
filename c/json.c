#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "json.h"

struct json_constant json_constant_object[JSON_CONSTANT_LAST]=
  {
    {.value=JSON_CONSTANT_TRUE},
    {.value=JSON_CONSTANT_FALSE},
    {.value=JSON_CONSTANT_NULL},    
  };

// TODO follow specs from  http://json.org/ http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf

int json_debug=0;

void flush_char_buffer(struct json_ctx * ctx)
{
  if (ctx->buf != NULL )
    {
      free(ctx->buf);
      ctx->buf=NULL;
      ctx->bufpos=0;
      ctx->bufsize=0;
    }
}

int json_set_debug(int debug)
{
  int previous=json_debug;
  json_debug = debug;
  return previous;
}

int json_ctx_set_debug(struct json_ctx * ctx, int debug)
{
  if ( ctx != NULL )
    {
      int previous=ctx->debug_level;
      ctx->debug_level = debug;
      return previous;
    }
  else
    {
      return json_debug;
    }
}

/**
a complicated json stream ( one char ahead ) parser 
**/

JSON_DEFINE_TOGGLE(squote,'\'')
JSON_DEFINE_TOGGLE(dquote,'"')
JSON_DEFINE_NEW(parenthesis,'{')
JSON_DEFINE_NEW(braket,'[')
JSON_DEFINE_TOGGLE(variable,'?')

void debug_tag(char c)
{
  if (json_debug>0)
    {
      printf("%c",c);
    }
}

void memory_shortage(struct json_ctx * ctx)
{
  fprintf(stderr,"Memory heap shortage. Exiting \n");
  exit(2);
}

void syntax_error(struct json_ctx * ctx, enum json_syntax_error erroridx, void * data,struct json_object * object,struct json_object * parent)
{
  char c=0;  
  if ( ctx->debug_level > 0 )
    {
      // should do better...
      fprintf(stderr,"\nSyntax error %u.\n", erroridx);
      c = ctx->next_char(ctx, data);
      printf("here :^%c",c);
      while ( c!= 0)
	{
	  printf("%c",c);
	  c = ctx->next_char(ctx, data);
	}
      printf("while parsing object :\n");
      dump_object(ctx,object,NULL);
      printf("\n parent :\n");
      dump_object(ctx,parent,NULL);
    }
  else
    {
      printf("syntax error %u\n",erroridx);
    }
}

struct json_object * cut_string_object(struct json_ctx * ctx, char objtype)
{
  struct json_object * object=calloc(1,sizeof(struct json_object));
  if ( object != NULL)
    {
      object->type=objtype;
      // warning, should keep a place for final 0
      if ( (ctx->bufpos + 1) < ctx->bufsize )
	{
	  // reduce it...
	  if ( json_debug > 0 )
	    {
	      printf("(%s,%s,%i) reduce string '%s' from %i to %i\n",__FILE__,__FUNCTION__,__LINE__,ctx->buf,ctx->bufsize,ctx->bufpos+1);
	    }
	  ctx->buf=realloc(ctx->buf,ctx->bufpos+1);
	  ctx->bufsize=ctx->bufpos;
	}
      object->string.chars=ctx->buf;
      object->string.length=ctx->bufsize;
      ctx->buf=NULL;
      ctx->bufpos=0;
      ctx->bufsize=0;
    }
  else
    {
      memory_shortage(ctx);
    }
  return object;
}

struct json_object * new_growable(struct json_ctx * ctx, char final_type)
{
 struct json_object * object=malloc(sizeof(struct json_object));
 if (object != NULL)
   {
     object->type='G';
     memcpy(&object->pos_info,&ctx->pos_info,sizeof(object->pos_info));
     object->growable.final_type=final_type;
     object->growable.tail=NULL;
     object->growable.head.value=NULL;
     object->growable.head.next=NULL;
     object->growable.size=0;
     object->owner=NULL;
   }
 else
   {
     memory_shortage(ctx);
   }
 return object;
}

struct json_link * new_link(struct json_ctx * ctx)
{
 struct json_link * link=malloc(sizeof(struct json_link));
 if (link != NULL)
   {
     link->value=NULL;
     link->next=NULL;
   }
 else
   {
     memory_shortage(ctx);
   }
 return link;
}

void growable_release_object(struct json_object * object)
{
  if (object->growable.size > 1)
    {
      struct json_link * link = object->growable.head.next;
      while ((link != NULL)&&(object->growable.size>0))
	{
	  link=link->next;
	  free(link);
	  object->growable.size--;
	}
    }
  free(object);
}

void release_object(struct json_object * object)
{
  // FIXME since object might link to other objects...
  assert(object != NULL);
  free(object);
}

void dump_growable(struct json_ctx * ctx, struct json_growable * growable, struct print_ctx * print_ctx);

void add_to_growable(struct json_ctx * ctx,struct json_growable * growable,struct json_object * object)
{
  if (json_debug > 0 )
    {
      printf("%c+=%c\n",growable->final_type,object->type);
      if (json_debug > 2)
	{
	  printf("before<");
	  dump_growable(ctx, growable,NULL);
	}
    }
  if ( growable->tail == NULL)
    {
      // growable was empty. add first element
      growable->tail = &growable->head;
      growable->head.value=object;
      growable->head.next=NULL;
      growable->size++;
    }
  else
    {
      // growable had already a first element
      struct json_link * link=new_link(ctx);
      growable->size++;
      growable->tail->next=link;
      link->value = object;
      // growable->tail was &growable->head for first item, then should have been overwritten
      assert(growable->head.next != NULL);
      growable->tail=link;
    }
  if ( json_debug > 0 )
    {
      printf("\nafter<");
      if ( json_debug > 0 )
	{
	  dump_growable(ctx, growable,NULL);
	  printf("\n>>");
	}
    }
}


void add_object(struct json_ctx * ctx,struct json_object * parent,struct json_object * object)
{
  struct json_object * oldtail= ctx->tail;
  //dump_ctx(ctx);
  // oldtail is current context.
  if (oldtail != NULL)
    {
      if (oldtail->type == 'G')
	{
	  debug_tag('+');
	  // add object to a growable.
	  add_to_growable(ctx,&oldtail->growable,object);
	  return;
	}
      else
	{
	  debug_tag('*');
	}
    }
  else
    {
      if ( json_debug  > 0 )
	{
	  printf("(oldtail == NULL %c", object->type);
	}
    }
  struct json_object * newtail=new_growable(ctx,object->type);
  if (newtail != NULL)
    {
      if ( oldtail != NULL) 
	{
	  debug_tag('*');
	  add_to_growable(ctx,&oldtail->growable,newtail);
	}
      else
	{
	  if (ctx->root == NULL)
	    {
	      debug_tag('x');
	      ctx->root=newtail;
	    }
	  else
	    {
	      debug_tag('.');
	    }
	}
      ctx->tail=newtail;
    }
  else
    {
      memory_shortage(ctx);
    }
}

struct json_object * new_pair_key(struct json_ctx * ctx, struct json_object * key)
{
  struct json_object * object=malloc(sizeof(struct json_object));
  debug_tag(':');
  if (object != NULL)
    {
      object->type=':';
      memcpy(&object->pos_info,&ctx->pos_info,sizeof(object->pos_info));
      object->pair.key=key;
      object->pair.value=NULL;
    }
  else
    {
      memory_shortage(ctx);
    }
  return object;
}

struct json_object * new_variable(struct json_ctx * ctx, struct json_object * key)
{
  struct json_object * object=malloc(sizeof(struct json_object));
  debug_tag('?');
  if (object != NULL)
    {
      object->type='?';
      object->owner=NULL;
      object->index=0;
      memcpy(&object->pos_info,&ctx->pos_info,sizeof(object->pos_info));
      object->variable.key=key;
      object->variable.value=NULL;
      object->variable.bound=0;
    }
  else
    {
      memory_shortage(ctx);
    }
  return object;
}

// should set owner of values to new object
struct json_object * create_json_list(struct json_ctx * ctx, struct json_object * obj)
{
  struct json_growable * growable = &obj->growable;
  struct json_link * link=NULL;
  int size=growable->size;
  int i=0;  
  struct json_object * object=malloc(sizeof(struct json_object) + size*sizeof(struct json_object *)); // a little bigger than needed
  if (object != NULL)
    {
      struct json_list * list=&object->list;
      object->type=growable->final_type;
      memcpy(&object->pos_info,&obj->pos_info,sizeof(object->pos_info));
      link=growable->tail;
      if ( link != NULL)
	{
	  growable->head.value->owner=object;
	  growable->head.value->index=i;
	  list->value[i++]=growable->head.value;
	  if ( link != &growable->head )
	    {
	      link=growable->head.next;
	      while ((link != NULL)&&(i<size))
		{
		  link->value->owner=object;
		  link->value->index=i;

		  list->value[i++]=link->value;
		  link=link->next;
		}
	    }
	};
      list->nitems=i;
    }
  else
    {
      memory_shortage(ctx);
    }
  return object;
}

// collect only pairs
// should set owner of values to new object
struct json_object * create_json_dict(struct json_ctx * ctx, struct json_object * obj)
{
  struct json_growable * growable= &obj->growable;
  struct json_link * link=NULL;
  int size=growable->size;
  int i=0;
  struct json_object * object=malloc(sizeof(struct json_object) + size*sizeof(struct json_pair *)); // a little bigger than needed
  if (object != NULL)
    {
      struct json_dict * dict=&object->dict;
      memcpy(&object->pos_info,&obj->pos_info,sizeof(object->pos_info));
      object->type=growable->final_type;
      link=growable->tail;
      if ( link != NULL)
	{
	  if (growable->head.value != NULL)
	    { 
	      if (growable->head.value->type == ':') {
		dict->items[i]=&growable->head.value->pair;
		growable->head.value->owner=object;
		++i;
	      }
	      else  { release_object(growable->head.value);}
	    }
	  if ( link != &growable->head )
	    {
	      link=growable->head.next;
	      while ((link != NULL)&&(i<size))
		{
		  if (link->value!=NULL) 
		    {
		      if (link->value->type == ':')
			{
			  dict->items[i++]=&link->value->pair;
			  link->value->owner=object;
			}
		      else
			{
			  release_object(link->value);
			}
		    }
		  link=link->next;
		}
	    }
	};
      dict->nitems=i;
    }
  else
    {
      memory_shortage(ctx);
    }
  return object;
}

/** Where growables becomes real json objects **/
struct json_object * concrete(struct json_ctx * ctx, struct json_object * object)
{
  if (object !=NULL)
    {
      struct json_object * newobject = NULL;
      if (object->type == 'G')
	{
	  struct json_growable * growable=&object->growable;
	  switch(growable->final_type)
	    {
	    case '{':
	      newobject=create_json_dict(ctx,object);
	      growable_release_object(object);
	      object=newobject;
	      break;
	    case '[':
	      newobject=create_json_list(ctx,object);
	      growable_release_object(object);
	      object=newobject;
	      break;
	    }
	}
    }
  return object;
}

/**
 return a json_object with a type '0' and json_string set to number if parsing is ok else return NULL
*/
struct json_object * parse_number_level(struct json_ctx * ctx, char first, void * data, struct json_object * parent)
{
  int state = 0;
  char c = first;
  while ( state >= 0 )
    {
      if ( json_debug > 3 )
	{
	  printf("(number state %i)",state);
	}
      switch(state)
	{
	case 0:
	  if ( c == '-' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state = 1;
	  break;
	case 1:
	  if ( c == '0' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 3;
	    }
	  else if ( ( c > '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 2;	      
	    }
	  else
	    {
	      state = -1;
	    }
	  break;
	case 2:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=3;
	  break;
	case 3:
	  if ( c == '.' )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 4;
	    }
	  else
	    {
	      state = 5;
	    }
	  break;
	case 4:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state = 5;
	  break;
	case 5:
	  if (( c=='e') || (c =='E'))
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	      state = 6;
	    }
	  else
	    {
	      state = 8;
	    }
	  break;
	case 6:
	  if (( c=='+') || (c =='-'))
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=7;
	  // no break on purpose
	case 7:
	  while ( ( c >= '0' ) && ( c <='9' ) )
	    {
	      ctx->add_char(ctx,'0',c);
	      c = ctx->next_char(ctx,data);
	    }
	  state=8;
	  // no break on purpose
	case 8:
	  // this char is NOT part of our number
	  ctx->pushback_char(ctx,data,c);
	  if ( json_debug > 0 )
	    {
	      printf("(pushback end of number %c)",c);
	      char d = ctx->next_char(ctx,data);
	      if ( d != c )
		{
		  fprintf(stderr,"(pushback end of number FAILS '%c'!='%c')\n",c, d);
		}
	      ctx->pushback_char(ctx,data,c);
	    }
	  state = 9;
	  return cut_string_object(ctx,'0');
	default:
	  state = -1;
	  // this is an error
	}      
    }
  return (struct json_object *) NULL;
}

int json_ctx_consume(struct json_ctx * ctx, void * data, char * str)
{
  int index = 0;
  char c = ctx->next_char(ctx,data);
  while ( (c != 0 ) && (str[index] != 0) )
    {
      if ( c != str[index] )
	{
	  return 0;
	}
      ctx->add_char(ctx,c,c);
      ++ index;
      if (str[index] == 0)
	{
	  break;
	}      
      c =ctx->next_char(ctx,data);
    }
  return (str[index] == 0);
}

struct json_object * new_json_constant_object(struct json_ctx * ctx, char t, enum json_internal_constant constant)
{
  struct json_object * object=malloc(sizeof(struct json_object));
  debug_tag(t);
  if (object != NULL)
    {
      object->type=t;
      object->owner=NULL;
      object->index=0;
      memcpy(&object->pos_info,&ctx->pos_info,sizeof(object->pos_info));
      object->constant = &json_constant_object[constant];
      if ( json_debug > 0 )
	{
	  printf("(constant %c)",t);
	}
    }
  else
    {
      memory_shortage(ctx);
    }
  return object;  
}

struct json_object * parse_constant_level(struct json_ctx * ctx, char first, void * data, struct json_object * parent)
{
  ctx->add_char(ctx,first,first);
  switch(first)
    {
    case 't':  
      if ( json_ctx_consume(ctx,data,"rue") )
	{
	  flush_char_buffer(ctx);
	  return new_json_constant_object(ctx, first,JSON_CONSTANT_TRUE);	  
	}
      break;
    case 'f':
	if (json_ctx_consume(ctx,data,"alse") )
	  {
	    flush_char_buffer(ctx);
	    return new_json_constant_object(ctx, first,JSON_CONSTANT_FALSE);
	  }
      break;
    case 'n':
      if ( json_ctx_consume(ctx,data,"ull") )
	{
	  flush_char_buffer(ctx);
	  return new_json_constant_object(ctx, first,JSON_CONSTANT_NULL);
	}
      break;
    }
  return NULL;
}

/**
 Actual parsing step

json_ctx current context ( will be updated )
void * data represent stream currently parsed, actual type depends on next_char function.
json_object parent of json object to be currently parsed.
 **/
struct json_object * parse_level(struct json_ctx * ctx, void * data, struct json_object * parent)
{
  char c = ctx->next_char(ctx, data);
  struct json_object * object=NULL;

  // main loop char by char, delegate to sub parser in many cases.
  while (c != 0)  {
    if ( json_debug > 2 )
      {
	switch(c)
	  {
	  case '{':
	  case '}':
	  case '[':
	  case ']':
	  case '"':
	  case '\'':
	  case ',':
	  case ':':
	  case '?':
	    printf("%c[%x]\n",c,c);
	    if (parent != NULL)
	      {
		dump_object(ctx,parent,NULL);
	      }
	    puts("\n-------");
	    break;
	  default:
	    printf("%c",c);
	  }
      }
    switch(c)
    {
    case '{': 
      JSON_OPEN(ctx,parenthesis,object);
      object=parse_level(ctx,data,object); // expects to parse until '}' included
      if (parent == NULL)
	{
	  return object;
	}
      break;
    case '}':
      JSON_CLOSE(ctx,parenthesis);
      if (object !=NULL)
	{
	  if ((parent != NULL) && ( parent->type=='G'))
	    {
	      if ( parent->growable.final_type == '{')
		{
		  add_to_growable(ctx,&parent->growable,object);
		}
	      else
		{
		  syntax_error(ctx,JSON_ERROR_DICT_CLOSE_NON_DICT,data,object,parent);
		}
	    }
	  else
	    {
	      syntax_error(ctx,JSON_ERROR_DICT_CLOSE_INVALID_PARENT,data,object,parent);
	    }
	  object=NULL;
	}
      else
	{
	  if ( json_debug > 0 )
	    {
	      printf( "} hit NULL object");
	    }
	}
      return concrete(ctx,parent);
      break;
    case '[':
      JSON_OPEN(ctx,braket,object);
      object=parse_level(ctx,data,object); // expects to parse until ']' included
      if (parent == NULL)
	{
	  return object;
	}
      break;
    case ']':
      JSON_CLOSE(ctx,braket);
      if (object !=NULL)
	{
	  if ( parent !=NULL ) 
	    {
	      if (parent->type=='G')
		{
		  if ( parent->growable.final_type == '[')
		    {
		      add_to_growable(ctx,&parent->growable,object);
		    }
		  else
		    {
		      syntax_error(ctx,JSON_ERROR_LIST_CLOSE_NON_LIST,data,object,parent);
		    }
		}
	      else
		{
		  syntax_error(ctx,JSON_ERROR_LIST_CLOSE_INVALID_PARENT,data,object,parent);
		}
	    }
	  else
	    {
	      syntax_error(ctx,JSON_ERROR_LIST_CLOSE_NO_PARENT, data,object,parent);
	    }
	  object=NULL;
	}
      return concrete(ctx,parent);
      break;
    case '"':
      JSON_TOGGLE(ctx,dquote);
      if ((parent != NULL) && (parent->type == '"'))
	{
	  // can't parse a string within a string
	  syntax_error(ctx,JSON_ERROR_STRING_IN_STRING, data,object,parent);
	}
      else
	{
	  object=parse_dquote_level(ctx,data,parent);
	  if (parent == NULL)
	    {
	      return object;
	    }
	  // else should be handled by ':', ',' or '}' or ']' ie any close.
	}
      break;
    case ',':
      if (object !=NULL)
	{
	  if ( json_debug > 0 )
	    {
	      printf("(%c parent:%p object:%p",c,parent,object);
	    }
	  if (parent !=NULL) 
	    {
	      if ( parent->type=='G')
		{
		  add_to_growable(ctx,&parent->growable,object);
		}
	      else
		{
		  syntax_error(ctx,JSON_ERROR_LIST_ELEMENT_INVALID_PARENT,data,object,parent);
		}
	    }
	  else
	    {
	      // special case if parent is NULL then it is up to caller to retrieve parent to add this result into it.
	      ctx->pushback_char(ctx,data,c);
	      return object;
	    }
	  object=NULL;
	}
      else
	{
	  if (json_debug > 0 )
	    {
	      printf("(ignore , NULL object)");
	    }
	}
      break;
    case '?':
      if ( object != NULL )
	{
	  syntax_error(ctx,JSON_ERROR_VARIABLE_BOUNDARY,data,object,parent);
	}
      else
	{
	  debug_tag('?');
	  JSON_TOGGLE(ctx,variable);
	  struct json_object * variable_name = parse_variable_level(ctx,data,parent);	  
	  if (variable_name == NULL )
	    {
	      syntax_error(ctx,JSON_ERROR_VARIABLE_NAME_NULL,data,object,parent);
	    }
	  else
	    {
	      // need to set type to '$' due to parse_variable_level that is defined for '?'
	      variable_name->type='$';
	      if ( json_debug > 0 )
		{
		  printf("new variable key %p\n", variable_name);
		}
	      object = new_variable(ctx,variable_name);
	      if (parent == NULL)
		{
		  return object;
		}
	      // else should be handled by ':', ',' or '}' or ']' ie any close.	      
	    }
	}
      break;
    case ':':
      // object pair : replace current object with a pair...
      // previous should be a string
      if (object !=NULL)
	{
	  if (object->type == '\"') 
	    {
	      struct json_object * pair=new_pair_key(ctx,object);
	      object=pair;
	      struct json_object * value=parse_level(ctx,data,NULL);
	      object->pair.value=value;
	      if ( value == NULL )
		{
		  syntax_error(ctx,JSON_ERROR_DICT_KEY_WITHOUT_VALUE,data,object,parent);
		}
	      else
		{
		  // where value knows its name.
		  value->owner=pair;
		}
	      if (parent == NULL)
		{
		  return object;
		}
	    }
	  else
	    {
	      debug_tag(object->type);
	      syntax_error(ctx,JSON_ERROR_DICT_KEY_NON_QUOTED,data,object,parent);
	      object=NULL;
	    }
	}
      else
	{
	  debug_tag('#');
	  syntax_error(ctx,JSON_ERROR_DICT_VALUE_WITHOUT_KEY,data,object,parent);
	}
      break;
    case '\'':
      JSON_TOGGLE(ctx,squote);
      object=parse_squote_level(ctx,data,object);
      break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      if ( object == NULL )
	{
	  object=parse_number_level(ctx,c,data,object);
	  if (parent == NULL)
	    {
	      return object;
	    }
	}
      else
	{
	  if (json_debug > 0 )
	    {
	      printf("non NULL object for number parsing\n");
	    }
	}
      break;
    case 'f':
    case 't':
    case 'n':
      object=parse_constant_level(ctx,c,data,object);
      if (parent == NULL)
	{
	  return object;
	}
      break;
    case 0:
      // force failure programming error
      assert(c!=0);
      break;
    default:
      if ( ctx->debug_level > 0 )
	{
	  printf("ignore '%c'\n", c);
	}
    }
    c=ctx->next_char(ctx, data);
  }
  
  return concrete(ctx,object);
}

char next_char(struct json_ctx* ctx, void * data)
{
  struct json_string * str = (struct json_string *) data;
  if ( json_debug > 0 )
  {
    printf("P%u,",ctx->pos);
  }
  if ( ctx->pos < str->length )
    {
      return str->chars[ctx->pos++];
    }
  else
    {
      return 0;
    }
}

void pushback_char(struct json_ctx *ctx, void *data, char pushback)
{
  struct json_string * str = (struct json_string *) data;
  // should not pushback something else than previous char. programming error
  assert( (ctx->pos>0) && (pushback == str->chars[ctx->pos-1]));
  ctx->pos--;
  if ( json_debug > 0 )
    {
      printf("(pushback %c)",pushback);      
    }
}

int add_char(struct json_ctx * ctx, char token, char c)
{
  int bufsize=256;
#ifdef DEBUGALL
  printf("%c", c);
#endif
  if (ctx->buf == NULL)
    {
      ctx->buf=calloc(1,bufsize);
      //ctx->buf[bufsize-1]=0;
      ctx->bufpos=0;
      ctx->bufsize=bufsize;
    }
  if (ctx->bufpos+1>=ctx->bufsize)
    {
      bufsize=ctx->bufsize + ctx->bufsize / 2;
      char * newbuf=realloc(ctx->buf,bufsize);
      if (newbuf != NULL)
	{
	  //done by realloc
	  //memcpy(newbuf,ctx->buf,ctx->bufsize);
	  //free(ctx->buf);
	  ctx->buf[bufsize-1]=0;
	  ctx->bufsize=bufsize; 
	  ctx->buf=newbuf;
	}
      else
	{
	  memory_shortage(ctx);
	}
    }
  ctx->buf[ctx->bufpos++]=c;
  return 0;
}


void dump_string(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      struct json_string * string = &object->string;
      if ( ( object->type != '$' ) && ( object->type != '0') )
	{
	  printf("%c%s%c",object->type,string->chars,object->type);
	}
      else
	{
	  printf("%s",string->chars);
	}      
    }
  else
    {
      printf("'0");
    }
}

void dump_pair(struct json_ctx * ctx, struct json_pair * pair, struct print_ctx * print_ctx)
{
  dump_object(ctx,pair->key, print_ctx);
  printf(":");
  dump_object(ctx,pair->value, print_ctx);
}

void dump_variable(struct json_ctx * ctx, struct json_variable * variable, struct print_ctx * print_ctx)
{
  if ( variable != NULL )
    {
      printf("?");
      if ( variable->key != NULL )
	{
	  dump_object(ctx,variable->key, print_ctx);
	}
      printf("?");
      if ( variable->bound == 1 )
	{
	  printf("=");
	  dump_object(ctx,variable->value, print_ctx);	  
	}
    }
}

void enter_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent )
    {
      print_ctx->indent+=print_ctx->do_indent;
    }
}

void dump_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent )
    {
      printf("\n");
      if ( print_ctx->indent > 0 )
	{
	  // commented out because print spaces before string ( ie does not repeat string )
	  //	  printf("\n%*s",print_ctx->indent,print_ctx->s_indent);
	  int i=print_ctx->indent;
	  while ( i >0 )
	    {	      
	      printf("%s",print_ctx->s_indent);
	      --i;
	    }
	}
      else
	{

	}
    }
}

void exit_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent &&  print_ctx->indent >= print_ctx->do_indent)
    {
      print_ctx->indent-=print_ctx->do_indent;
    }
}

void dump_pair_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      assert(object->type == ':');
      dump_pair(ctx,&object->pair, print_ctx);
    }
  else
    {
      printf(":0");
    }
}

void dump_variable_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      assert(object->type == '?');

      json_print_object_name(ctx,object,print_ctx);
      printf(".");

      dump_variable(ctx,&object->variable, print_ctx);
    }
  else
    {
      printf(":0");
    }
}

void dump_list_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  int i=0;
  printf("%c",object->type);
  enter_indent( print_ctx);
  if (object->list.nitems > 0)
    {
      dump_indent(print_ctx);
      dump_object(ctx,object->list.value[0], print_ctx);
      for(i=1;i< object->list.nitems;i++)
	{
	  printf(",");
	  dump_indent(print_ctx);
	  dump_object(ctx,object->list.value[i], print_ctx);
	}
    }
  exit_indent( print_ctx);
  dump_indent(print_ctx);
  printf("]");
}

struct json_object * json_list_get( struct json_object * object, int index)
{
  if ( index < object->list.nitems )
    {
      return object->list.value[index];
    }
  return NULL;
}



void dump_dict_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  int i;
  printf("%c",object->type);
  enter_indent(print_ctx);
  if (object->dict.nitems > 0)
    {
      dump_indent(print_ctx);
      dump_pair(ctx,object->dict.items[0], print_ctx);
      for(i=1;i< object->dict.nitems;i++)
	{
	  printf(",");
	  dump_indent(print_ctx);
	  dump_pair(ctx,object->dict.items[i], print_ctx);
	}
    }
  exit_indent( print_ctx);
  dump_indent(print_ctx);
  printf("}");
}

struct json_object * json_dict_get_value(char * keyname, struct json_object * object)
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
	      if ( strcmp( pair->key->string.chars, keyname) == 0 )
		{
		  value = pair->value;
		}
	    }
	}
    }
  return value;
}

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
	      if ( path->string.length == pair->key->string.length )
		{
		  if ( strncmp( pair->key->string.chars, path->string.chars, path->string.length) == 0 )
		    {
		      value = pair->value;
		    }
		}
	    }
	}
    }
  return value;
}

void dump_growable(struct json_ctx * ctx, struct json_growable * growable, struct print_ctx * print_ctx)
{
  struct json_link * link=NULL;
  link=growable->tail;
  printf("|%c",growable->final_type);
  if ( link != NULL)
    {
      dump_object(ctx, growable->head.value, print_ctx);
      if ( link != &growable->head )
	{
	  link=growable->head.next;
	  while (link != NULL)
	    {
	      printf(",");
	      dump_object(ctx, link->value, print_ctx);
	      link=link->next;
	    }
	}
    }
  else
    {
      if (growable->size != 0) printf("#");	
    }
  printf("%c|",growable->final_type);
}

void dump_growable_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  assert(object->type == 'G');
  struct json_growable * growable=&object->growable;
  dump_growable(ctx,growable, print_ctx);
}

void dump_constant_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object->constant != NULL )
    {
      switch(object->constant->value)
	{
	case JSON_CONSTANT_TRUE:
	  printf("true");
	  break;
	case JSON_CONSTANT_FALSE:
	  printf("false");
	  break;
	case JSON_CONSTANT_NULL:
	  printf("null");
	  break;
	default:
	  printf("ERROR constant type %c %p",object->type, print_ctx);
	}
    }
  else
    {
      printf("ERROR constant type %c %p NULL",object->type, print_ctx);
    }
    
}

void dump_object(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if (object != NULL)
    {
      // printf("%p[%c]",object,object->type);
      switch(object->type)
	{
	case 'G':
	  dump_growable_object(ctx, object, print_ctx);
	  break;
	case '{':
	  dump_dict_object(ctx,object, print_ctx);
	  break;
	case '[':
	  dump_list_object(ctx,object, print_ctx);
	  break;
	case '"':
	case '\'':
	case '$':
	case '0': // number is special string
	  dump_string(ctx,object, print_ctx);
	  break;
	case ':':
	  dump_pair_object(ctx,object, print_ctx);
	  break;
	case ',':
	  printf("#");
	  break;
	case '?':
	  dump_variable_object(ctx,object, print_ctx);
	  break;
	case 'n':
	case 't':
	case 'f':
	  dump_constant_object(ctx,object, print_ctx);
	  break;
        default:
	  printf("ERROR type %c %p",object->type, print_ctx);
	}
    }
  else
    {
      printf(" NULL ");
    }
}


int json_unify_string(struct json_ctx * ctx, struct json_object * object,
		      struct json_ctx * other_ctx, struct json_object * other_object,
		      struct print_ctx * print_ctx)
{
  if ( ( object != NULL) && (other_object != NULL ) )
    {
      struct json_string * string = &object->string;
      if ( strcmp(string->chars, other_object->string.chars) == 0 )
	{
	  printf("%c%s%c",object->type,string->chars,object->type);
	  return 1;
	}
      else
	{
	  return 0;
	}      
    }
  else
    {
      printf("'0");
      return 0;
    }  
}

int json_unify_list(struct json_ctx * ctx, struct json_object * object,
		     struct json_ctx * other_ctx, struct json_object * other_object,
		     struct print_ctx * print_ctx)
{
  int i=0;
  int ok = 0;
  if ( object->list.nitems != other_object->list.nitems )
    {
      return 0;
    }
  printf("%c",object->type);
  enter_indent( print_ctx);
  if (object->list.nitems > 0)
    {
      dump_indent(print_ctx);
      ok = json_unify_object(ctx,object->list.value[0],
			     other_ctx,other_object->list.value[0],
			     print_ctx);
      for(i=1;(ok == 1) && (i< object->list.nitems);i++)
	{
	  printf(",");
	  dump_indent(print_ctx);
	  ok= json_unify_object(ctx,object->list.value[i],
				other_ctx,other_object->list.value[i],
				print_ctx);
	}
    }
  else
    {
      ok = 1;
    }
  exit_indent( print_ctx);
  dump_indent(print_ctx);
  printf("]");
  return ok;
}

int json_unify_pair(struct json_ctx * ctx, struct json_pair * pair,
		     struct json_ctx * other_ctx, struct json_pair * other_pair,
		     struct print_ctx * print_ctx)
{
  int ok =  json_unify_object(ctx,pair->key,
			      other_ctx, other_pair->key,
			      print_ctx);
  printf(":");
  if ( ok == 1 )
    {
      ok = json_unify_object(ctx,pair->value,
		       other_ctx,other_pair->value,
		       print_ctx);
    }
  return ok;
}

int json_unify_pair_object(struct json_ctx * ctx, struct json_object * object,
		            struct json_ctx * other_ctx, struct json_object * other_object,
		            struct print_ctx * print_ctx)
{
  if ( ( object != NULL) && (other_object != NULL ) )
    {
      assert(object->type == ':');
      return json_unify_pair(ctx,&object->pair,
			     other_ctx,&other_object->pair,
			     print_ctx);
    }
  else
    {
      printf(":0");
      return 0;
    }

}

int json_unify_dict(
	       struct json_ctx * ctx, struct json_object * object,
	       struct json_ctx * other_ctx, struct json_object * other_object,
	       struct print_ctx * print_ctx)
{
  int i;
  int ok = 0;
  
  if ( object->dict.nitems != other_object->dict.nitems )
    {
      return 0;
    }
  printf("%c",object->type);
  enter_indent(print_ctx);
  if (object->dict.nitems > 0)
    {
      dump_indent(print_ctx);
      ok = json_unify_pair(ctx,object->dict.items[0],
			   other_ctx, other_object->dict.items[0],
			   print_ctx);
      for(i=1;(ok == 1) && ( i< object->dict.nitems);i++)
	{
	  printf(",");
	  dump_indent(print_ctx);
	  ok = json_unify_pair(ctx,object->dict.items[i],
			       other_ctx, other_object->dict.items[i],
			       print_ctx);
	}
    }
  else
    {
      ok = 1;
    }    
  exit_indent( print_ctx);
  dump_indent(print_ctx);
  printf("}");

  return ok;
}

/** assume ctx & object are non-variables and variable_objet is 
 **/
int json_unify_variable(
	       struct json_ctx * ctx, struct json_object * object,
	       struct json_ctx * variable_ctx, struct json_object * variable_object,
	       struct print_ctx * print_ctx)
{
  if ( variable_object->variable.bound != 0 )
    {
      return variable_object->variable.value == object;
    }
  
  variable_object->variable.bound=1;
  variable_object->variable.value=object;

  dump_variable_object(variable_ctx,variable_object,print_ctx);
  
  return 1;
}

int json_unify_object(
	       struct json_ctx * ctx, struct json_object * object,
	       struct json_ctx * other_ctx, struct json_object * other_object,
	       struct print_ctx * print_ctx)
{
  int unify = 0; // 1 == this; 2 == other
  
  // test identity
  if ( object == other_object )
    {
      return 1;
    }

  if ( (object != NULL) && (other_object != NULL ) )
    {
      if ( other_object->type != object->type )
	{
	  if ( object->type == '?' ) 
	    {
	      unify=1;
	    }
	  else if (other_object->type == '?' )
	    {
	      unify=2;
	    }
	  else
	    {
	      return 0;
	    }
	}

      if (unify != 0 )
	{
	  if (unify == 1)
	    {
	      return json_unify_variable(other_ctx, other_object,
					 ctx,object,
					 print_ctx);
	    }
	  else
	    {
	      return json_unify_variable(ctx,object,
				     other_ctx, other_object,
				     print_ctx);
	    }
	}
      else
	{
	 
	  // printf("%p[%c]",object,object->type);
	  switch(object->type)
	    {
	    case 'G':
	      dump_growable_object(ctx, object, print_ctx);
	      return 0;
	      break;
	    case '{':
	      return json_unify_dict(ctx,object,
				     other_ctx, other_object, print_ctx);
	      break;
	    case '[':
	      return json_unify_list(ctx,object,
				     other_ctx, other_object,
				     print_ctx);
	      break;
	    case '"':
	    case '\'':
	    case '$':
	      return json_unify_string(ctx,object,
				       other_ctx, other_object,
				       print_ctx);
	      break;
	    case ':':
	      return json_unify_pair_object(ctx,object,
					    other_ctx, other_object,
					    print_ctx);
	      break;
	    case ',':
	      printf("#");
	      break;
	    default:
	      printf("ERROR unify type %c %p",object->type, print_ctx);
	    }
	}
    }
  else
    {
      printf(" NULL ");
      return 0;
    }
  
  return 0;
}


void dump_ctx(struct json_ctx * ctx)
{
  static int count=0;
  if (ctx != NULL)
    {
      if (ctx->root != NULL)
	{
	  printf("\n%u)--------------------------------\n",count++);
	  dump_object(ctx,ctx->root, NULL);
	  printf("\n--------------------------------\n");
	}
      else
	{
	  printf(" ROOTNULL ");
	}
    }
  else
    {
      printf(" CTXNULL ");
    }
}

/** Initialize json_context **/
void json_context_initialize(struct json_ctx *json_context)
{
  json_context->unstack=parse_level;
  json_context->next_char=next_char;
  json_context->pushback_char=pushback_char;
  json_context->add_char=add_char;
  json_context->pos=0;
  json_context->buf=NULL;
  json_context->bufsize=0;
  json_context->bufpos=0;
  json_context->root=NULL;
  json_context->tail=NULL;
  json_context->debug_level=0;
}

void json_print_object_name(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL )
    {
      if ( object->type != 0 )
	{
	  if (object->owner != NULL )
	    {
	      json_print_object_name(ctx,object->owner,print_ctx);
	      if ( object->type == ':' )
		{
		  printf(".%s",object->pair.key->string.chars);
		}
	      else if ( object->owner->type  == '[' )
		{
		  printf(".%u", object->index);
		}
	    }
	}
      else
	{
	  printf("!");
	}
    } 
}

void dump_json_path(struct json_path * json_path)
{
  int watchguard = JSON_PATH_DEPTH ;
  while ( ( json_path != NULL ) && ( watchguard > 0 ) )    
    {
      ++ watchguard;
      printf(".%.*s", json_path->string.length, json_path->string.chars);
      json_path = json_path->child;
    }
}

/**
 WARNING json_path itself is used for strings 
if given_pathes is NULL then it will be dynamically allocated else it should have been zeroed before use.
**/
struct json_path * create_json_path(int maxlength, char * json_path, struct json_ctx * ctx, int max_depth, struct json_path * given_pathes)
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
	      if ( json_debug > 0 )
		{
		  printf("json_path_index:%i pos:%i %s\n", json_path_index, pos, current);
		}

	      if ( json_path_index < 0 )
		{
		  json_path_index = 0;
		}
	      pathes[json_path_index].child = NULL;
	      if (pathes[json_path_index].string.chars == NULL )
		{
		  pathes[json_path_index].type = next_type;
		  pathes[json_path_index].string.chars = current;
		  pathes[json_path_index].string.length = 1;
		  pathes[json_path_index].index = 0;
		  digits=1;
		}
	      else
		{
		  pathes[json_path_index].string.length ++;
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

struct json_object * json_walk_path(char * json_path, struct json_ctx * ctx, struct json_object * object)
{
  struct json_path * json_path_object = create_json_path(JSON_PATH_MAX_CHARS,json_path,ctx,JSON_PATH_DEPTH,NULL);
  struct json_object * current_object = object;
  
  if ( json_path_object != NULL )
    {
      struct json_path * current_path = json_path_object;
      dump_json_path(json_path_object);

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
		      printf("[ERROR] path keyname %.*s not found\n", current_path->string.length, current_path->string.chars);
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

void json_flatten(struct json_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  // don't use todo here since in an external library
  // todo("json_flatten could be done with a special print_ctx and rely on dump");
}

