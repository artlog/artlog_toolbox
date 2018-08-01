#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "alcommon.h"
#include "aljson.h"
#include "alstack.h"

#ifdef JSON_TODO
#include "todo.h"
#else
// ignore :-(
#define todo(text) printf("todo(%s)\n",text);
#endif

struct json_constant json_constant_object[JSON_CONSTANT_LAST]=
  {
    {.value=JSON_CONSTANT_TRUE},
    {.value=JSON_CONSTANT_FALSE},
    {.value=JSON_CONSTANT_NULL},    
  };


// forward definition used only localy
void dump_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx);
  
// TODO follow specs from  http://json.org/ http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf

int json_debug=0;

struct print_ctx aljson_print_ctx_debug;

int json_set_debug(int debug)
{
  int previous=json_debug;
  json_debug = debug;

  if ( debug != 0 )
    {
      // should setup aljson_print_ctx_debug.
      aljson_print_ctx_init(&aljson_print_ctx_debug);
      aljson_print_ctx_debug.outfile=stderr;
    }
  return previous;
}

/**
a complicated json stream ( one char ahead ) parser 
dual implementiation call stack and data stack 
for in-depth parsing, if max-depth is hit then switch to non recursive implementation
**/

JSON_DEFINE_TOGGLE(squote,'\'')
JSON_DEFINE_TOGGLE(dquote,'"')
JSON_DEFINE_NEW(parenthesis,'{')
JSON_DEFINE_NEW(braket,'[')
JSON_DEFINE_TOGGLE(variable,'?')

struct json_object * aljson_new_json_string(struct json_parser_ctx * ctx, char objtype, struct alhash_datablock * data)
{  
  struct json_object * object=(struct json_object *) ALALLOC(ctx->alparser.allocator,sizeof(struct json_object));

  if (object != NULL)
    {
      object->type=objtype;
      memcpy(&object->string.internal,data,sizeof(object->string.internal));
    }
  else
    {
      memory_shortage(NULL);
    }
  return object;
}

struct json_object * new_json_error(struct json_parser_ctx * parser, enum json_syntax_error erroridx)
{
  struct json_ctx * ctx = parser->tokenizer;
  struct json_object * object = (struct json_object *) ALALLOC(parser->alparser.allocator,sizeof(struct json_object));
  if (object != NULL)
    {
      object->type='E';
      memcpy(&object->pos_info,&ctx->pos_info,sizeof(object->pos_info));
      memcpy(&object->error.where,&ctx->pos_info,sizeof(object->error.where));      
      object->error.erroridx=erroridx;
    }
  else
    {
      memory_shortage(NULL);
    }
  return object;
}

struct json_object * syntax_error(struct json_parser_ctx * parser, enum json_syntax_error erroridx, void * data,struct json_object * object,struct json_object * parent)
{
  char c=0;
  int max_buff = 1024;
  int buf_idx=0;
  char * err_buf = malloc(max_buff);
  struct json_ctx * ctx = parser->tokenizer;
  if ( err_buf == NULL )
    {
      memory_shortage(NULL);
    }
  struct json_object * err_object = new_json_error(parser,erroridx);
  c = ctx->next_char(ctx, data);
  // capture some context ( 5 lines, cumulated < max_buff chars )
  // eat all to end => only one error will be captured.
  while ( c!= 0)
    {
      if ( (ctx->pos_info.line < err_object->pos_info.line + 5 ) && ( buf_idx < max_buff ) )
	{	  
	  err_buf[buf_idx]=c;
	  buf_idx ++;
	}      
      c = ctx->next_char(ctx, data);
    }
  err_object->error.string.internal.type=ALTYPE_OPAQUE;
  err_object->error.string.internal.data.ptr=err_buf;
  // always null terminated
  if ( buf_idx >= max_buff )
    {
      buf_idx=max_buff-1;
    }
  err_buf[buf_idx]=0;  
  err_object->error.string.internal.length=buf_idx;  
  if ( ctx->debug_level > 0 )
    {
      dump_object(parser,err_object,&aljson_print_ctx_debug);
      printf("while parsing object :\n");
      dump_object(parser,object,&aljson_print_ctx_debug);
      printf("\n parent :\n");
      dump_object(parser,parent,&aljson_print_ctx_debug);
    }
  return err_object;
}

struct json_object * aljson_new_json_object(struct json_ctx * ctx, char objtype, alstrings_ringbuffer_pointer * allocator, struct alhash_datablock * data)
{
  struct json_object * object=(struct json_object *) al_alloc_block(allocator,sizeof(struct json_object));
  if (object != NULL)
    {
      object->type=objtype;
      if ( data != NULL )
	{
	  memcpy(&object->string.internal,data,sizeof(object->string.internal));
	}
      else
	{
	  aldebug_printf(NULL,"new json object with empty data %p\n",object);
	}
    }
  return object;
}

// allocate a new json_object
// borrow buf buffer of json_ctx and set it into a json_string
// reset json_ctx buffer.
struct json_object * cut_string_object(struct json_parser_ctx * ctx, char objtype)
{
  struct json_object * object=NULL;
  struct token_char_buffer * tb = &ctx->tokenizer->token_buf;
  // warning, should keep a place for final 0
  if ( (tb->bufpos + 1) > tb->bufsize )
    {
      // realloc for one character ... too bad.
      if ( FLAG_IS_SET(json_debug,1)  )
	{
	  printf("(%s,%s,%i) grow string '%s' from %i to %i\n",__FILE__,__FUNCTION__,__LINE__,tb->buf,tb->bufsize,tb->bufpos+1);
	}
      tb->buf=realloc(tb->buf,tb->bufpos+1);
      tb->bufsize=tb->bufpos;
    }
  
  struct alhash_datablock  data;
  data.data.ptr = tb->buf;
  data.length = tb->bufpos;
  data.type = ALTYPE_OPAQUE;
  object = aljson_new_json_string(ctx,objtype,&data);

  // buffer will be re-allocated -> where is it done ?
  bzero(&ctx->tokenizer->token_buf, sizeof(ctx->tokenizer->token_buf));
      
  return object;
}

struct json_object * aljson_new_growable(struct json_parser_ctx * ctx, char final_type)
{
 struct json_object * object=malloc(sizeof(struct json_object));
 if (object != NULL)
   {
     object->type='G';
     memcpy(&object->pos_info,&ctx->tokenizer->pos_info,sizeof(object->pos_info));
     object->growable.final_type=final_type;
     object->growable.tail=NULL;
     object->growable.head.value=NULL;
     object->growable.head.next=NULL;
     object->growable.size=0;
     object->owner=NULL;
   }
 else
   {
     memory_shortage(NULL);
   }
 return object;
}

struct json_link * new_link(struct json_parser_ctx * ctx)
{
 struct json_link * link=malloc(sizeof(struct json_link));
 if (link != NULL)
   {
     link->value=NULL;
     link->next=NULL;
   }
 else
   {
     memory_shortage(NULL);
   }
 return link;
}

void json_growable_release_object(struct json_object * object)
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
  // fixme
  // free(object);
}

void json_release_object(struct json_object * object)
{
  // FIXME since object might link to other objects...
  assert(object != NULL);
  // fixme
  // free(object);
}

void dump_growable(struct json_parser_ctx * ctx, struct json_growable * growable, struct print_ctx * print_ctx);

void aljson_add_to_growable(struct json_parser_ctx * ctx,struct json_growable * growable,struct json_object * object)
{
  if (json_debug > 0 )
    {
      printf("%c+=%c\n",growable->final_type,object->type);
      if (json_debug > 2)
	{
	  printf("before<");
	  dump_growable(ctx, growable,&aljson_print_ctx_debug);
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
	  dump_growable(ctx, growable,&aljson_print_ctx_debug);
	  printf("\n>>");
	}
    }
}

struct json_object * aljson_new_pair_key(struct json_parser_ctx * parser, struct json_object * key)
{
  struct json_ctx * ctx = parser->tokenizer;
  struct json_object * object=malloc(sizeof(struct json_object));
  debug_tag(ctx,':');
  if (object != NULL)
    {
      object->type=':';
      memcpy(&object->pos_info,&ctx->pos_info,sizeof(object->pos_info));
      object->pair.key=key;
      object->pair.value=NULL;
    }
  else
    {
      memory_shortage(NULL);
    }
  return object;
}

struct json_object * new_variable(struct json_parser_ctx * parser, struct json_object * key)
{
  struct json_ctx * ctx = parser->tokenizer;
  struct json_object * object=malloc(sizeof(struct json_object));
  debug_tag(ctx,'?');
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
      memory_shortage(NULL);
    }
  return object;
}

// used by aljson_concrete for json_list case
// should set owner of values to new object
struct json_object * create_json_list(struct json_parser_ctx * parser, struct json_object * obj)
{
  struct json_growable * growable = &obj->growable;
  struct json_link * link=NULL;
  int size=growable->size;
  int i=0;  
  struct json_object * object= (struct json_object *) ALALLOC(parser->alparser.allocator,sizeof(struct json_object) + size*sizeof(struct json_object *)); // a little bigger than needed
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
      memory_shortage(NULL);
    }
  return object;
}

// fixeme, currently focus on string since used only for dict key (which is json_string )
void aljson_fill_datablock(struct json_object * object, struct alhash_datablock * datablock)
{
  // todo depending on type
  switch(object->type)
    {
    case '"':
    case '\'':
    case '$':
      // is a string
      memcpy(datablock, &object->string.internal, sizeof(*datablock));
      break;
    case 'G':
      // growable
    case '{':
      // get datablock description for a dict
    case '[':
      // get datablock description for a list
    case ':':
    case ',':
	     printf("#");
    default:
	     todo("data type no yet supported for datablock");
	     aldebug_printf(NULL,"ERROR not supported datatype %c ",object->type);
    }  
  
}

void * json_dict_hashadd_callback (struct json_object * key, struct json_object * value, void * data)
{
  if ( data != NULL )
    {
      struct alhash_table * dict = (struct alhash_table *) data;
      struct alhash_datablock key_datablock;
      struct alhash_datablock value_datablock;
      // key is really datablock content
      aljson_fill_datablock(key, &key_datablock);
      // value point to json_object
      value_datablock.type=ALTYPE_OPAQUE;
      value_datablock.data.ptr=value;
      value_datablock.length = sizeof(struct json_object);
      struct alhash_entry * entry = alhash_put(dict, &key_datablock, &value_datablock);
      alhash_dump_entry_as_string(entry);
      return NULL;
    }
  // something non NULL ...
  return json_dict_hashadd_callback;
}

// collect only pairs, used by aljson_concrete to obtain a dict from a growable '{'
// should set owner of values to new object
struct json_object * create_json_dict(struct json_parser_ctx * parser, struct json_object * obj)
{
  struct json_growable * growable= &obj->growable;
  struct json_link * link=NULL;
  int size=growable->size;
  // index of element keypair in dict.
  int i=0;
  struct json_object * object= (struct json_object *) ALALLOC(parser->alparser.allocator,sizeof(struct json_object) + size*sizeof(struct json_pair *)); // a little bigger than needed
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
	      else  { json_release_object(growable->head.value);}
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
			  json_release_object(link->value);
			}
		    }
		  link=link->next;
		}
	    }
	};
      dict->nitems=i;

      // this is where dict htable backend is populated
      if ( dict->localcontext.dict.context == NULL )
	{
	  // twice the space to limit collisions, should not overflow
	  aldebug_printf(NULL,"init internal json hashtable size %i * 2\n" ,  dict->nitems );
	  alparser_init(&dict->localcontext, 1, dict->nitems * 2);
	  alparser_ctx_set_debug(&dict->localcontext,255);
	  aljson_dict_foreach(object, json_dict_hashadd_callback,&dict->localcontext.dict );
	}
    }
  else
    {
      memory_shortage(NULL);
    }
  return object;
}

/** Where growables becomes real json objects **/
struct json_object * aljson_concrete(struct json_parser_ctx * parser, struct json_object * object)
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
	      newobject=create_json_dict(parser,object);
	      json_growable_release_object(object);
	      object=newobject;
	      break;
	    case '[':
	      newobject=create_json_list(parser,object);
	      json_growable_release_object(object);
	      object=newobject;
	      break;
	    }
	}
    }
  return object;
}


struct json_object * new_json_constant_object(struct json_parser_ctx * parser, char t, enum json_internal_constant constant)
{
  struct json_ctx * ctx = parser->tokenizer;
  struct json_object * object= (struct json_object *) ALALLOC(parser->alparser.allocator,sizeof(struct json_object));
  debug_tag(ctx,t);
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
      memory_shortage(NULL);
    }
  return object;  
}

// forward defintion to fallback if depth is too big.
struct json_object * parse_level_non_recursive(struct json_parser_ctx * ctx, void * data, struct json_object * parent);

// rely on tokenizer.
struct json_object * parse_level_recursive(struct json_parser_ctx * ctx, void * data, struct json_object * parent)
{
  struct al_token * last_token;
  struct json_object * object=NULL;

  ++ctx->parsing_depth;
  
  last_token = json_tokenizer(ctx->tokenizer,data);

  while (last_token != NULL)  {
 
	switch(last_token->token)
	  {
	  case JSON_TOKEN_OPEN_PARENTHESIS_ID: 
	    JSON_OPEN(ctx,parenthesis,object);
	    object=parse_level(ctx,data,object); // expects to parse until '}' included
	    if (parent == NULL)
	      {
		--ctx->parsing_depth;
		return object;
	      }
	    break;
	  case JSON_TOKEN_CLOSE_PARENTHESIS_ID:
	    JSON_CLOSE(ctx,parenthesis);
	    if (object !=NULL)
	      {
		if ((parent != NULL) && ( parent->type=='G'))
		  {
		    if ( parent->growable.final_type == '{')
		      {
			aljson_add_to_growable(ctx,&parent->growable,object);
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
	    --ctx->parsing_depth;
	    return aljson_concrete(ctx,parent);
	    break;
	  case JSON_TOKEN_OPEN_BRACKET_ID:
	    JSON_OPEN(ctx,braket,object);
	    object=parse_level(ctx,data,object); // expects to parse until ']' included
	    if (parent == NULL)
	      {
		--ctx->parsing_depth;
		return object;
	      }
	    break;
	  case JSON_TOKEN_CLOSE_BRACKET_ID:
	    JSON_CLOSE(ctx,braket);
	    if (object !=NULL)
	      {
		if ( parent !=NULL ) 
		  {
		    if (parent->type=='G')
		      {
			if ( parent->growable.final_type == '[')
			  {
			    aljson_add_to_growable(ctx,&parent->growable,object);
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
	    --ctx->parsing_depth;
	    return aljson_concrete(ctx,parent);
	    break;
	  case JSON_TOKEN_DQUOTE_ID:
	    if ((parent != NULL) && (parent->type == '"'))
	      {
		// can't parse a string within a string
		syntax_error(ctx,JSON_ERROR_STRING_IN_STRING, data,object,parent);
	      }
	    else
	      {
		object=cut_string_object(ctx,'"');
		if (parent == NULL)
		  {
		    --ctx->parsing_depth;
		    return object;
		  }
	      }
	    break;
	  case JSON_TOKEN_COMMA_ID:
	    if (object !=NULL)
	      {
		if ( json_debug > 0 )
		  {
		    printf("(%i parent:%p object:%p",last_token->token,parent,object);
		  }
		if (parent !=NULL) 
		  {
		    if ( parent->type=='G')
		      {
			aljson_add_to_growable(ctx,&parent->growable,object);
		      }
		    else
		      {
			syntax_error(ctx,JSON_ERROR_LIST_ELEMENT_INVALID_PARENT,data,object,parent);
		      }
		  }
		else
		  {
		    // special case if parent is NULL then it is up to caller to retrieve parent to add this result into it.
		    todo("special case if parent is NULL then it is up to caller to retrieve parent to add this result into it.");
		    // somehow a ctx->pushback_token
		    --ctx->parsing_depth;
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
	  case JSON_TOKEN_VARIABLE_ID:
	    if ( object != NULL )
	      {
		syntax_error(ctx,JSON_ERROR_VARIABLE_BOUNDARY,data,object,parent);
	      }
	    else
	      {
		debug_tag(ctx->tokenizer,'?');
		JSON_TOGGLE(ctx,variable);
		struct json_object * variable_name = cut_string_object(ctx,'?');	  
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
			--ctx->parsing_depth;
			return object;
		      }
		    // else should be handled by ':', ',' or '}' or ']' ie any close.	      
		  }
	      }
	    break;
	  case JSON_TOKEN_COLON_ID:
	    // object pair : replace current object with a pair...
	    // previous should be a string
	    if (object !=NULL)
	      {
		if (object->type == '\"') 
		  {
		    struct json_object * pair=aljson_new_pair_key(ctx,object);
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
			--ctx->parsing_depth;
			return object;
		      }
		  }
		else
		  {
		    debug_tag(ctx->tokenizer,object->type);
		    syntax_error(ctx,JSON_ERROR_DICT_KEY_NON_QUOTED,data,object,parent);
		    object=NULL;
		  }
	      }
	    else
	      {
		debug_tag(ctx->tokenizer,'#');
		syntax_error(ctx,JSON_ERROR_DICT_VALUE_WITHOUT_KEY,data,object,parent);
	      }
	    break;
	  case JSON_TOKEN_SQUOTE_ID:
	    object=cut_string_object(ctx,'\'');
	    break;
	  case JSON_TOKEN_NUMBER_ID:
	    if ( object == NULL )
	      {
		object=cut_string_object(ctx,'0');
		if (parent == NULL)
		  {
		    --ctx->parsing_depth;
		    return object;
		  }
	      }
	    else
	      {
		--ctx->parsing_depth;
		return syntax_error(ctx,JSON_ERROR_NUMBER_MISPLACED,data, object,parent);
	      }
	    break;
	  case JSON_TOKEN_TRUE_ID:
	  case JSON_TOKEN_FALSE_ID:
	  case JSON_TOKEN_NULL_ID:
	    if ( object == NULL )
	      {
		switch( last_token->token)
		  {
		  case JSON_TOKEN_TRUE_ID:
		    object=new_json_constant_object(ctx, 't',JSON_CONSTANT_TRUE);
		    break;
		  case JSON_TOKEN_FALSE_ID:
		    object=new_json_constant_object(ctx, 'f',JSON_CONSTANT_FALSE);
		    break;
		  case JSON_TOKEN_NULL_ID:
		    object=new_json_constant_object(ctx, 'n',JSON_CONSTANT_NULL);
		    break;
		  } 
		if (parent == NULL)
		  {
		    --ctx->parsing_depth;
		    return object;
		  }
	      }
	    else
	      {
		--ctx->parsing_depth;
		return syntax_error(ctx,JSON_ERROR_CONSTANT_MISPLACED,data, object,parent);
	      }
	    break;
	  case JSON_TOKEN_EOF_ID:
	    aldebug_printf(NULL,"EOF\n");
	  default:
	    if ( object != NULL )
	      {
		--ctx->parsing_depth;
		return syntax_error(ctx,JSON_ERROR_VALUE_CHAR_UNEXPECTED, data, object,parent);
	      }
	    else
	      {
		--ctx->parsing_depth;
		return syntax_error(ctx,JSON_ERROR_TOKEN_CHAR_UNEXPECTED, data, object,parent);
	      }
	  }
	  
	last_token = json_tokenizer(ctx->tokenizer,data);
  }

  --ctx->parsing_depth;
  return aljson_concrete(ctx,object);
}

struct json_object * check_parent_is_pair(struct json_parser_ctx * ctx, void * data, struct alstack * stack, struct json_object * parent, struct json_object ** object)
{
  struct alstackelement * element;
  if ( object == NULL )
    {
      // FATAL coding error
      return NULL;
    }
  
  if ( ( parent != NULL ) && (parent->type == ':' ) )
    {
      if ( json_debug > 0 )
      {
	printf( "process pair \n");
      }

      parent->pair.value=*object;
      if ( *object == NULL )
	{
	  syntax_error(ctx,JSON_ERROR_DICT_KEY_WITHOUT_VALUE,data,*object,parent);
	}
      else
	{
	  // where value knows its name.
	  (*object)->owner=parent;
	  // replace object
	  *object=parent;
	}
      element = alstack_pop(stack);
      if ( element != NULL )
	{
	  parent = element->reference;
	}
      else
	{
	  parent = NULL;
	}
    }
  return parent;
}

// rely on tokenizer and uses an internal stack to avoid recursive call.
struct json_object * parse_level_non_recursive(struct json_parser_ctx * ctx, void * data, struct json_object * parent)
{
  struct al_token * last_token;
  struct alstack * stack;
  struct alstackelement * element;
  struct json_object * object=NULL; // previous object on same parsing level
  int stopwithparent = 0;
  
  stack = alstack_allocate();
  element = NULL;

  if ( stack == NULL )
    {
      return NULL;
    }

  if ( parent != NULL )
    {
      alstack_push_ref(stack,(void *) parent);
      parent = NULL;
      stopwithparent = 1;
    }

  last_token = json_tokenizer(ctx->tokenizer,data);

  // what is object : previous object on same parsing level
  // what is parent : direct parent of object
  
  while (last_token != NULL)  {

    if ( json_debug > 0 )
      {
	printf( "process token %i\n",last_token->token);
      }

    switch(last_token->token)
      {
      case JSON_TOKEN_OPEN_PARENTHESIS_ID:
	// create object 
	JSON_OPEN(ctx,parenthesis,object);
	alstack_push_ref(stack,(void *) object);
	object=NULL;
	// expects to parse until '}' included
	break;
      case JSON_TOKEN_CLOSE_PARENTHESIS_ID:
	JSON_CLOSE(ctx,parenthesis);
	element = alstack_pop(stack);
	if ( element != NULL )
	  {
	    parent = element->reference;
	    // object is there to be added in parent later with '}',']' or ','
	    parent = check_parent_is_pair(ctx,data,stack,parent,&object);
	  }

	// last object for this } group
	if (object !=NULL)
	  {
	    if ((parent != NULL) && ( parent->type=='G'))
	      {
		if ( parent->growable.final_type == '{')
		  {
		    if ( json_debug > 0 )
		      {
			printf("LOOK HERE 0\n");
			dump_object(ctx,object,&aljson_print_ctx_debug);
		      }

		    aljson_add_to_growable(ctx,&parent->growable,object);
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
	if ( json_debug > 0 )
	  {
	    // before concrete
	    dump_object(ctx,parent,&aljson_print_ctx_debug);
	  }
	object = aljson_concrete(ctx,parent);
	if ( json_debug > 0 )
	  {
	    // after concrete
	    dump_object(ctx,object,&aljson_print_ctx_debug);
	  }
	break;
      case JSON_TOKEN_OPEN_BRACKET_ID:
	JSON_OPEN(ctx,braket,object);
	alstack_push_ref( stack, (void *) object);
	object = NULL;
	// expects to parse until ']' included
	break;
      case JSON_TOKEN_CLOSE_BRACKET_ID:
	JSON_CLOSE(ctx,braket);
	element = alstack_pop(stack);
	if ( element != NULL )
	  {
	    parent = element->reference;
	    if ( ( parent != NULL ) && (parent->type == ':' ) )
	      {
		// ERROR no reason to end a pair in an array.
		syntax_error(ctx,JSON_ERROR_LIST_ELEMENT_INVALID_PARENT,data,object,parent);
	      }
	  }

	if (object !=NULL)
	  {
	    if ( parent !=NULL ) 
	      {
		if (parent->type=='G')
		  {
		    if ( parent->growable.final_type == '[')
		      {
			aljson_add_to_growable(ctx,&parent->growable,object);
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
	object=aljson_concrete(ctx,parent);
	break;
      case JSON_TOKEN_DQUOTE_ID:
	if ( object != NULL )
	  {
	    // should be a syntax error
	    aldebug_printf(NULL,"%s\n","WARNING non attached object before string");
	    if ( json_debug > 0 )
	      {
		dump_object(ctx,object,&aljson_print_ctx_debug);
	      }			    
	  }
	object=cut_string_object(ctx,'"');
	break;
      case JSON_TOKEN_COMMA_ID:
	if (object !=NULL)
	  {
	    // TODO could just fetch it (but poped then should be pushed again)
	    element = alstack_pop(stack);
	    if ( element != NULL )
	      {
		parent = element->reference;		    
		parent = check_parent_is_pair(ctx,data,stack,parent,&object);		    
	      }

	    if ( json_debug > 0 )
	      {
		printf("(%i parent:%p object:%p",last_token->token,parent,object);
	      }

	    if (parent !=NULL) 
	      {
		if ( parent->type=='G')
		  {
		    aljson_add_to_growable(ctx,&parent->growable,object);
		  }
		else
		  {
		    syntax_error(ctx,JSON_ERROR_LIST_ELEMENT_INVALID_PARENT,data,object,parent);
		  }
		// readd parent [ since poped ]
		alstack_push_ref(stack, (void *) parent);
	      }
	    object=NULL;
	  }
	else
	  {
	    if (json_debug > 0 )
	      {
		// should be a syntax error ( coma wihtout anything ).
		printf("(ignore , NULL object)");
	      }
	  }
	break;
      case JSON_TOKEN_VARIABLE_ID:
	if ( object != NULL )
	  {
	    syntax_error(ctx,JSON_ERROR_VARIABLE_BOUNDARY,data,object,parent);
	  }
	else
	  {
	    debug_tag(ctx->tokenizer,'?');
	    JSON_TOGGLE(ctx,variable);
	    struct json_object * variable_name = cut_string_object(ctx,'?');	  
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
		// else should be handled by ':', ',' or '}' or ']' ie any close.	      
	      }
	  }
	break;
      case JSON_TOKEN_COLON_ID:
	// object pair : replace current object with a pair...
	// previous should be a string
	if (object !=NULL)
	  {
	    if (object->type == '\"') 
	      {
		struct json_object * pair=aljson_new_pair_key(ctx,object);

		// rely on unstacking to detect pair.
		alstack_push_ref(stack,(void *) pair);

		// next object will be value, detected at '}' or ','
		object = NULL;
    
	      }
	    else
	      {
		debug_tag(ctx->tokenizer,object->type);
		syntax_error(ctx,JSON_ERROR_DICT_KEY_NON_QUOTED,data,object,parent);
		object=NULL;
	      }
	  }
	else
	  {
	    debug_tag(ctx->tokenizer,'#');
	    syntax_error(ctx,JSON_ERROR_DICT_VALUE_WITHOUT_KEY,data,object,parent);
	  }
	break;
      case JSON_TOKEN_SQUOTE_ID:
	if ( object != NULL )
	  {
	    object=cut_string_object(ctx,'\'');
	  }
	else
	  {
	    // syntax error
	    aldebug_printf(NULL,"%s","syntax error string following an object without separator");
	  }
	break;
      case JSON_TOKEN_NUMBER_ID:
	if ( object == NULL )
	  {
	    object=cut_string_object(ctx,'0');
	  }
	else
	  {
	    alstack_destroy(stack, NULL);
	    return syntax_error(ctx,JSON_ERROR_NUMBER_MISPLACED,data, object,parent);
	  }
	break;
      case JSON_TOKEN_TRUE_ID:
      case JSON_TOKEN_FALSE_ID:
      case JSON_TOKEN_NULL_ID:
	if ( object == NULL )
	  {
	    switch( last_token->token)
	      {
	      case JSON_TOKEN_TRUE_ID:
		object=new_json_constant_object(ctx, 't',JSON_CONSTANT_TRUE);
		break;
	      case JSON_TOKEN_FALSE_ID:
		object=new_json_constant_object(ctx, 'f',JSON_CONSTANT_FALSE);
		break;
	      case JSON_TOKEN_NULL_ID:
		object=new_json_constant_object(ctx, 'n',JSON_CONSTANT_NULL);
		break;
	      } 
	  }
	else
	  {
	    alstack_destroy(stack, NULL);
	    return syntax_error(ctx,JSON_ERROR_CONSTANT_MISPLACED,data, object,parent);
	  }
	break;
      case JSON_TOKEN_EOF_ID:
	element = alstack_pop(stack);
	if ( element != NULL )
	  {
	    aldebug_printf(NULL,"EOF with parent \n");
	  }
	object=aljson_concrete(ctx,object);
	alstack_destroy(stack, NULL);
	return object;
	
      default:
	alstack_destroy(stack, NULL);
	if ( object != NULL )
	  {	    
	    return syntax_error(ctx,JSON_ERROR_VALUE_CHAR_UNEXPECTED, data, object,parent);
	  }
	else
	  {
	    return syntax_error(ctx,JSON_ERROR_TOKEN_CHAR_UNEXPECTED, data, object,parent);
	  }
      }
    
    // case where a parent was given
    if ( stopwithparent && (alstack_used(stack) == 0) )
      {
	break;
      }

    last_token = json_tokenizer(ctx->tokenizer,data);
  }

  object=aljson_concrete(ctx,object);
  alstack_destroy(stack, NULL);
  return object;

}

// try recursive only if depth is not > ctx->max_depth
struct json_object * parse_level(struct json_parser_ctx * ctx, void * data, struct json_object * parent)
{
  if ( ctx->parsing_depth > ctx->max_depth)
    {
      return parse_level_non_recursive(ctx,data,parent);
    }
  else
    {
      return parse_level_recursive(ctx,data,parent);
    }
}

void dump_string(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  FILE * outfile=print_ctx->outfile;
  if ( object != NULL)
    {
      struct json_string * string = &object->string;
      if ( ( object->type != '$' ) && ( object->type != '0') )
	{
	  fprintf(outfile,"%c" ALPASCALSTRFMT "%c",
		 object->type,
		 ALPASCALSTRARGS(string->internal.length,(char *) string->internal.data).ptr,
		 object->type);
	}
      else
	{
	  // NULL terminated string ?
	  // todo("implement ALTYPE_STR0 for string->internal.type");
	  fprintf(outfile,ALPASCALSTRFMT,
		 ALPASCALSTRARGS(string->internal.length,(char *) string->internal.data.ptr));
	}      
    }
  else
    {
      fprintf(outfile,"'0");
    }
}

void dump_string_number(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      FILE * outfile=print_ctx->outfile;
      if ( object->type == '0' )
	{
	  float value = json_get_float(object);
	  fprintf(outfile,"%f",value);
	}
      else
	{
	  fprintf(outfile,"#ERROR not a number");
	}
    }
}

void dump_pair(struct json_parser_ctx * ctx, struct json_pair * pair, struct print_ctx * print_ctx)
{
  dump_object(ctx,pair->key, print_ctx);
  printf(":");
  dump_object(ctx,pair->value, print_ctx);
}

void dump_variable(struct json_parser_ctx * ctx, struct json_variable * variable, struct print_ctx * print_ctx)
{
  if ( variable != NULL )
    {
      FILE * outfile=print_ctx->outfile;
      fprintf(outfile,"?");
      if ( variable->key != NULL )
	{
	  dump_object(ctx,variable->key, print_ctx);
	}
      fprintf(outfile,"?");
      if ( variable->bound == 1 )
	{
	  fprintf(outfile,"=");
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
      FILE * outfile=print_ctx->outfile;
      fprintf(outfile,"\n");
      if ( print_ctx->indent > 0 )
	{
	  // commented out because print spaces before string ( ie does not repeat string )
	  //	  printf("\n%*s",print_ctx->indent,print_ctx->s_indent);
	  int i=print_ctx->indent;
	  if ( i < 80 )
	    {
	      while ( i >0 )
		{	      
		  fprintf(outfile,"%s",print_ctx->s_indent);
		  --i;
		}
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

void dump_pair_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      assert(object->type == ':');
      dump_pair(ctx,&object->pair, print_ctx);
    }
  else
    {
      fprintf(print_ctx->outfile,":0");
    }
}

void dump_variable_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  FILE * outfile=print_ctx->outfile;
  if ( object != NULL)
    {
      assert(object->type == '?');

      json_print_object_name(ctx,object,print_ctx);
      fprintf(outfile,".");

      dump_variable(ctx,&object->variable, print_ctx);
    }
  else
    {
      fprintf(outfile,":0");
    }
}

void dump_list_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  int i=0;
  FILE * outfile=print_ctx->outfile;
  fprintf(outfile,"%c",object->type);
  enter_indent( print_ctx);
  if (object->list.nitems > 0)
    {
      dump_indent(print_ctx);
      dump_object(ctx,object->list.value[0], print_ctx);
      for(i=1;i< object->list.nitems;i++)
	{
	  fprintf(outfile,",");
	  dump_indent(print_ctx);
	  dump_object(ctx,object->list.value[i], print_ctx);
	}
    }
  exit_indent( print_ctx);
  dump_indent(print_ctx);
  fprintf(outfile,"]");
}

struct json_object * json_list_get( struct json_object * object, int index)
{
  if ( index < object->list.nitems )
    {
      return object->list.value[index];
    }
  return NULL;
}



void dump_dict_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  int i;
  FILE * outfile=print_ctx->outfile;
  fprintf(outfile,"%c",object->type);
  enter_indent(print_ctx);
  if (object->dict.nitems > 0)
    {
      dump_indent(print_ctx);
      dump_pair(ctx,object->dict.items[0], print_ctx);
      for(i=1;i< object->dict.nitems;i++)
	{
	  fprintf(outfile,",");
	  dump_indent(print_ctx);
	  dump_pair(ctx,object->dict.items[i], print_ctx);
	}
    }
  exit_indent( print_ctx);
  dump_indent(print_ctx);
  fprintf(outfile,"}");
}

void * aljson_dict_foreach(
			 struct json_object * object,
			 void * (* callback) (struct json_object * key, struct json_object * value, void * data),
			 void * data)				     
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
	      void * result = callback( pair->key, pair->value, data);
	      if ( result != NULL )
		{
		  return result;
		}
	    }
	}
    }
  return NULL;
}

void * aljson_dict_match_value_callback(struct json_object * key, struct json_object * value, void * data)
{
  struct alhash_datablock * searchkey = (struct alhash_datablock *) data;

  // handle non zero terminated pair key internal string
  if ( searchkey->length ==  key->string.internal.length )
    {
      if ( strncmp( key->string.internal.data.ptr, searchkey->data.ptr, key->string.internal.length) == 0 )
	{
	  return value;
	}
    }  
  return NULL;
}

// search in hashtable if created, fallback to list walk
struct json_object * json_dict_get_value(const char * keyname, struct json_object * object)
{
  if ( keyname == NULL )
    {
      return NULL;
    }

  int foundstatus = 0;
  struct json_object * value = NULL;

  struct alhash_datablock searchkey;
  searchkey.length = strlen(keyname);
  searchkey.type = ALTYPE_OPAQUE;
  searchkey.data.constcharptr = keyname;  
  
  // htable search
  if ( object->type == '{' )
    {
      // there is a hashtable bound.
      if ( object->dict.localcontext.dict.context ==  &object->dict.localcontext )
	{
	  struct alhash_entry * alhash_entry = alhash_get_entry( &object->dict.localcontext.dict, &searchkey);
	  if (alhash_entry != NULL )
	    {
	      if (alhash_entry->value.length == sizeof(struct json_object))
		{
		  // WE DID IT !
		  aldebug_printf(NULL, "json_value for dict key '%s' FOUND  \n", keyname);
		  value = (struct json_object *) alhash_entry->value.data.ptr;
		  foundstatus = 255;
		}
	      else
		{
		  aldebug_printf(NULL,"value in dict is not a json_object length %i pointer %p\n", alhash_entry->value.length, alhash_entry->value.data.ptr);
		  foundstatus = 1;
		}
	    }
	  else
	    {
	      foundstatus = 2;
	    }
	}
    }

  // it is not possible to find a NULL value since it points to a json structure
  if ( value != NULL)
    {
      return value;
    }

  // fallback to foreach walk
  value = (struct json_object *) aljson_dict_foreach(object,aljson_dict_match_value_callback,(void *) &searchkey);
		      

  // collect error case were not found  in hashtable but was in json_dict items
  if ( ( foundstatus != 0 ) && ( value != NULL ) )
    {
      // this indicates an internal coding error or a memory corruption.
      aldebug_printf(NULL,"[FATAL] key '%s' value is in json_dict items but not backed in hastable %p\n", keyname, object);
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

void dump_growable(struct json_parser_ctx * ctx, struct json_growable * growable, struct print_ctx * print_ctx)
{
  FILE * outfile = print_ctx->outfile;
  struct json_link * link=NULL;
  link=growable->tail;
  fprintf(outfile,"|%c",growable->final_type);
  if ( link != NULL)
    {
      dump_object(ctx, growable->head.value, print_ctx);
      if ( link != &growable->head )
	{
	  link=growable->head.next;
	  while (link != NULL)
	    {
	      fprintf(outfile,",");
	      dump_object(ctx, link->value, print_ctx);
	      link=link->next;
	    }
	}
    }
  else
    {
      if (growable->size != 0) printf("#");	
    }
  fprintf(outfile,"%c|",growable->final_type);
}

void dump_growable_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  assert(object->type == 'G');
  struct json_growable * growable=&object->growable;
  dump_growable(ctx,growable, print_ctx);
}

void dump_constant_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
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

void dump_error_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  FILE * outfile = print_ctx->outfile;
  if (( object != NULL) && (object->error.string.internal.data.ptr != NULL ))
    {
      fprintf(outfile,"syntax error %u (line:%i,column:%i)\n" ALPASCALSTRFMT "\n",
	     object->error.erroridx,
	     object->error.where.line,object->error.where.column,
	     ALPASCALSTRARGS(object->error.string.internal.length,(char *) object->error.string.internal.data.ptr)
	     );
    }
  else
    {
      fprintf(outfile,"syntax error object is corrupted %p\n", object);
    }
}

// limited to print_ctx->max_depth since relying on code stack call.
void dump_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  static int depth = 0;

  ++depth;

  if ( depth > print_ctx->max_depth )
    {
      printf("... depth > %i ...\n", print_ctx->max_depth);
      --depth;
      return;
    }
  
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
	case '0':
	  dump_string_number(ctx,object, print_ctx);
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
	case 'E':
	  dump_error_object(ctx,object, print_ctx);
	  break;
        default:
	  printf("ERROR type %c %p",object->type, print_ctx);
	}
    }
  else
    {
      printf(" NULL ");
    }

  --depth;
}


/*
 ----------------
 yet aljson_xxx_output relies on dump_xxx_object
 should be rewritten for dump to either be independent or to use aljson_xxx_ouput ( ie reverse ).
--------------------
*/
void aljson_print_printf(struct print_ctx * print_ctx, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  vfprintf(print_ctx->outfile,format, args);
  
  va_end(args);
}

void aljson_json_growable_output(struct json_parser_ctx * ctx, struct json_growable * growable, struct print_ctx * print_ctx)
{
  struct json_link * link=NULL;
  link=growable->tail;  
  print_ctx->printf(print_ctx,"|%c",growable->final_type);
  if ( link != NULL)
    {
      aljson_output(ctx, growable->head.value, print_ctx);
      if ( link != &growable->head )
	{
	  link=growable->head.next;
	  while (link != NULL)
	    {
	      print_ctx->printf(print_ctx,",");
	      aljson_output(ctx, link->value, print_ctx);
	      link=link->next;
	    }
	}
    }
  else
    {
      if (growable->size != 0) printf("#");	
    }
  print_ctx->printf(print_ctx,"%c|",growable->final_type);
}


void aljson_growable_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  assert(object->type == 'G');
  struct json_growable * growable=&object->growable;
  aljson_json_growable_output(ctx,growable, print_ctx);
}

void aljson_dict_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  dump_dict_object(ctx,object, print_ctx);
}

void aljson_list_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  dump_list_object(ctx,object, print_ctx);
}

void aljson_string_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)  
{
  dump_string(ctx,object, print_ctx);
}

void aljson_number_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)  
{
  dump_string_number(ctx,object, print_ctx);
}

void aljson_error_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  dump_error_object(ctx,object, print_ctx);
}

void aljson_pair_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  dump_pair_object(ctx,object, print_ctx);
}

void aljson_constant_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  dump_constant_object(ctx,object, print_ctx);
}

// limited to print_ctx->max_depth since relying on code stack call.
void aljson_output(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  // FIXME to move within print_ctx.
  static int depth = 0;

  ++depth;

  if (  depth > print_ctx->max_depth )
    {
      printf("... depth > %i ...\n", print_ctx->max_depth);
      --depth;
      return;
    }
  
  if (object != NULL)
    {
      // printf("%p[%c]",object,object->type);
      switch(object->type)
	{
	case 'G':
	  aljson_growable_output(ctx, object, print_ctx);
	  break;
	case '{':
	  aljson_dict_output(ctx,object, print_ctx);
	  break;
	case '[':
	  aljson_list_output(ctx,object, print_ctx);
	  break;
	case '"':
	case '\'':
	case '$':
	  aljson_string_output(ctx,object, print_ctx);
	  break;
	case '0':
	  aljson_number_output(ctx,object, print_ctx);
	  break;
	case ':':
	  aljson_pair_output(ctx,object, print_ctx);
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
	  aljson_constant_output(ctx,object, print_ctx);
	  break;
	case 'E':
	  aljson_error_output(ctx,object, print_ctx);
	  break;
        default:
	  printf("ERROR type %c %p",object->type, print_ctx);
	}
    }
  else
    {
      printf(" NULL ");
    }

  --depth;
}


void aljson_print_ctx_init(struct print_ctx * print_ctx)
{
  // 1024 levels of json MAX
  print_ctx->max_depth=1024;
  print_ctx->indent=0;
  print_ctx->do_indent=2; // 0 no indent, >= 1 number of space by indent.
  print_ctx->s_indent=" ";

    /* tabs
  print_ctx.do_indent = 1;
  print_ctx.indent = 0;
  print_ctx.s_indent = "\t";
  */

  /* flat canonical
  print_ctx.do_indent = 0;
  print_ctx.indent = 0;
  print_ctx.s_indent = NULL;
  */

  print_ctx->outfile=stdout;
  
  print_ctx->growable_output=aljson_growable_output;
  print_ctx->dict_output=aljson_dict_output;
  print_ctx->list_output=aljson_list_output;
  print_ctx->string_output=aljson_string_output;
  print_ctx->number_output=aljson_number_output;
  print_ctx->error_output=aljson_error_output;
  print_ctx->pair_output=aljson_pair_output;
  print_ctx->constant_output=aljson_constant_output;

  print_ctx->printf=aljson_print_printf;
}

int json_unify_string(struct json_parser_ctx * ctx, struct json_object * object,
		      struct json_parser_ctx * other_ctx, struct json_object * other_object,
		      struct print_ctx * print_ctx)
{
  if ( ( object != NULL) && (other_object != NULL ) )
    {
      struct json_string * string = &object->string;
      if ( strcmp(string->internal.data.ptr, other_object->string.internal.data.ptr) == 0 )
	{
	  printf("%c" ALPASCALSTRFMT "%c",
		 object->type,
		 ALPASCALSTRARGS(string->internal.length,(char *)string->internal.data.ptr),
		 object->type);
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

int json_unify_list(struct json_parser_ctx * ctx, struct json_object * object,
		     struct json_parser_ctx * other_ctx, struct json_object * other_object,
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

int json_unify_pair(struct json_parser_ctx * ctx, struct json_pair * pair,
		     struct json_parser_ctx * other_ctx, struct json_pair * other_pair,
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

int json_unify_pair_object(struct json_parser_ctx * ctx, struct json_object * object,
		            struct json_parser_ctx * other_ctx, struct json_object * other_object,
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
	       struct json_parser_ctx * ctx, struct json_object * object,
	       struct json_parser_ctx * other_ctx, struct json_object * other_object,
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
	       struct json_parser_ctx * ctx, struct json_object * object,
	       struct json_parser_ctx * variable_ctx, struct json_object * variable_object,
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
	       struct json_parser_ctx * ctx, struct json_object * object,
	       struct json_parser_ctx * other_ctx, struct json_object * other_object,
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


void dump_ctx(struct json_parser_ctx * ctx)
{
  todo("really dump context only");
}


void json_print_object_name(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
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
		  printf("." ALPASCALSTRFMT,
			 ALPASCALSTRARGS(object->pair.key->string.internal.length,(char *)object->pair.key->string.internal.data.ptr));
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
      printf("." ALPASCALSTRFMT,
	     ALPASCALSTRARGS(json_path->string.internal.length, (char *) json_path->string.internal.data.ptr));
      json_path = json_path->child;
    }
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
	      if ( json_debug > 0 )
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

struct json_object * json_walk_path(char * json_path, struct json_parser_ctx * ctx, struct json_object * object)
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

void json_flatten(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  todo("json_flatten could be done with a special print_ctx and rely on dump");
}

// return number of char read == relative position of first unrecognized char
int json_get_int_internal(struct json_string * number, int pos , int * resultp)
{
  int cumul = 0;
  int result = 0;
  int negative = 0;
  int i = 0;
  int limit = number->internal.length - pos;
  if ( limit > 0 )
    {
      char * chars = &number->internal.data.charptr[pos];
      if ( chars != NULL )
	{

	  if (( limit > 0 ) && ( chars[0] == '-' ))
	    {
	      negative=1;
	      i=1;	      
	    }
	  for (; i < limit ; i++)
	    {
	      char c = chars[i];
	      if ( c == '\0' )
		{
		  if ( i ==  limit - 1 )
		    {
		      // c string NULL terminated bypass
		      continue;
		    }
		  else
		    {
		      aldebug_printf(NULL,"[FATAL] unexpected NUL terminated string in '%s' at %i length %i for int\n", (char *) number->internal.data.ptr, i, number->internal.length);
		    }
		}
	      if ( ( c >= '0' ) && ( c <= '9' ) )
		{
		  cumul = result*10 + (c - '0');
		  if ( cumul < result )
		    {
		      // overflow.
		      todo("[ERROR] should handle overflow for json_get_int(..)");
		      break;
		    }
		  else
		    {
		      result=cumul;
		    }		      
		}
	      else
		{
		  if ( ( c != '.' ) && ( c != 'e' ) && ( c != 'E') )
		    {
		      aldebug_printf(NULL,"[FATAL] unexpected '%c' in '" ALPASCALSTRFMT "' at %i length %i during number parsing\n",
				     c,
				     ALPASCALSTRARGS(number->internal.length,
						     (char *) number->internal.data.charptr),
				     i + pos, number->internal.length);
		    }
		  break;
		}
	    }
	  if (negative)
	    {
	      result=-result;
	    }
	}

      *(resultp) = result;
    }
  return i;
}

// very naive implementation
float json_get_float(struct json_object * object )
{
  float fresult = 0.0;
  if ( ( object != NULL ) && ( object->type == '0' ) )
    {
      struct json_string * number = &object->string;
      int integer = 0;
      int remainder = 0;
      int exponent = 0;
      int limit = number->internal.length;
      int pos = json_get_int_internal(number, 0, &integer);
      fresult = (float) integer;
      if (pos < limit)
	{
	  // expecting pos to point at first unrecognized char.
	  char separator = number->internal.data.charptr[pos];
	  int rpos = 0;
	  if ( separator == '.' )
	    {
	      rpos = json_get_int_internal(number, pos+1, &remainder);
	      pos += rpos + 1;
	      float r = remainder;
	      while ( rpos > 0 )
		{
		  r = r / 10;
		  rpos --;
		}
	      fresult += r;
	    }
	  if ( pos < limit )
	    {
	      separator = number->internal.data.charptr[pos];
	      if ( ( separator == 'E' ) || ( separator = 'e' ) )
		{
		  int negative = 0;
		  if ( pos +1 < limit )
		    {
		      char c = number->internal.data.charptr[pos+1];
		      if ( c == '-' )
			{
			  negative=1;
			  ++pos;
			}
		      else if ( c == '+' )
			{
			  ++pos;
			}
		      //else expects a number
		    }
		  rpos = json_get_int_internal(number, pos+1, &exponent);
		  if ( negative )
		    {
		      while (exponent > 0 )
			{
			  exponent --;
			  fresult = fresult /10;
			}
		    }
		  else
		    {
		      while (exponent > 0 )
			{
			  exponent --;
			  fresult = fresult *10;
			}
		    }
		}
	      else
		{
		  aldebug_printf(NULL,"[FATAL] unexpected '%c' in '" ALPASCALSTRFMT "' at %i length %i during float parsing\n",
				 separator,
				 ALPASCALSTRARGS(number->internal.length,
						 (char *) number->internal.data.charptr),
				 pos, number->internal.length);
		}
	    }
	}
    }
  else
    {
      todo("[ERROR] should handle wrong object type or NULL for json_get_float(..)");
    }
  return fresult;

}

// convert a string value into an int value...
// here assume encoded as string
int json_get_int(struct json_object * object )
{
  int result = 0;
  if ( ( object != NULL ) && ( object->type == '0' ) )
    {
      struct json_string * number = &object->string;
      int limit = number->internal.length;
      int pos = json_get_int_internal(number, 0, &result);
      // expecting pos to point at end of string value
      if (pos < limit )
	{
	  aldebug_printf(NULL,"[FATAL] unexpected in '%s' at %i length %i for int\n", (char *) number->internal.data.ptr, pos, number->internal.length);
	}
    }
  else
    {
      todo("[ERROR] should handle wrong object type or NULL for json_get_int(..)");
    }
  return result;
}

// use with caution, final \0 might be missing
char * json_get_string(struct json_object * object)
{
  if ( ( object != NULL ) && ( ( object->type == '"' ) || ( object->type == '\'' ) ) )
    {
      return  (char *) object->string.internal.data.ptr;
    }
  return NULL;
}

// Q&D fix, should use an allocator...
char * json_get_cstring(struct json_object * object)
{
  char * cstring =  json_get_string(object);
  // fixme use internal.type ALTYPE_STR0
  if ( cstring != NULL )
    {
      // ugly,  memory leak
      if ( object->string.internal.length > 0 )
	{
	  if ( cstring[object->string.internal.length-1] != '\0' )
	    {
	      char * newstring = malloc(object->string.internal.length+1);
	      memcpy(newstring,cstring,object->string.internal.length);
	      newstring[object->string.internal.length]='\0';
	      cstring = newstring;
	      aldebug_printf(NULL,"[WARNING] converted to cstring '%s'\n",newstring);
	    }
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] cstring converted from string with empty length\n");
	  cstring = NULL;
	}
    }
  return cstring;
}

int aljson_attach_private_data(void * kindnyi, struct json_object * json, void * data)
{
  if ( json->private_data == NULL )
    {
      json->private_data = data;
      return 1;
    }
  return 0;
}
