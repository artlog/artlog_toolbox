#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "json.h"

// TODO follow specs from  http://json.org/ http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf

int json_debug=0;

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
      fprintf(stderr,"Syntax error %u.\n", erroridx);
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
#ifdef DEBUGALL
  printf("\n<");
  dump_growable(ctx, growable);
#endif
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
#ifdef DEBUGALL
  printf("\n>");
  dump_growable(ctx, growable);
  puts("");
#endif
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

struct json_object * parse_number_level(struct json_ctx * ctx, void * data, struct json_object * parent)
{
  return NULL;
}

struct json_object * parse_constant_level(struct json_ctx * ctx, void * data, struct json_object * parent)
{
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
    if ( json_debug > 0 )
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
	  //ctx->pushback_char(ctx,data,'}');
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
      return concrete(ctx,parent);
      break;
    case '[':
      JSON_OPEN(ctx,braket,object);
      object=parse_level(ctx,data,object); // expects to parse until ']' included
      if (parent == NULL)
	{
	  // ctx->pushback_char(ctx,data,']');
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
	      if ( value == NULL )
		{
		  syntax_error(ctx,JSON_ERROR_DICT_KEY_WITHOUT_VALUE,data,object,parent);
		}
	      object->pair.value=value;
	      // where value knows its name.
	      value->owner=pair;
	      if (parent == NULL)
		{
		  return object;
		}
	    }
	  else
	    {
	      debug_tag('#');
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
      object=parse_number_level(ctx,data,object);
      break;
    case 'f':
    case 't':
    case 'n':
      object=parse_constant_level(ctx,data,object);
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
      if ( object->type != '$' )
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
      /**  can't be would need a json_object ??
      printf("==");
      json_print_object_name(ctx,variable,print_ctx);
      **/
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
}
