#include "aljson_dump.h"

#include <assert.h>
#include "aldebug_output.h"
#include "aloutput.h"
#include <stdio.h>
#include <math.h>

struct aloutputstream * aljson_get_output(struct print_ctx * print_ctx)
{
  return print_ctx->outfile;
}

void aljson_dump_enter_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent )
    {
      print_ctx->indent+=print_ctx->do_indent;
    }
}


void aljson_dump_exit_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent &&  print_ctx->indent >= print_ctx->do_indent)
    {
      print_ctx->indent-=print_ctx->do_indent;
    }
}

void aljson_dump_string( struct json_object * object, struct print_ctx * print_ctx)
{  
  struct aloutputstream * output=aljson_get_output(print_ctx);
  if ( object != NULL)
    {
      struct json_string * string = &object->string;
      if ( ( object->type != '$' ) && ( object->type != '0') )
	{
	  aloutputstream_printf_1k(output,"%c" ALPASCALSTRFMT "%c",
		 object->type,
		 ALPASCALSTRARGS(string->internal.length,(char *) string->internal.data).ptr,
		 object->type);
	}
      else
	{
	  // NULL terminated string ?
	  // todo("implement ALTYPE_STR0 for string->internal.type");
	  aloutputstream_printf_1k(output,ALPASCALSTRFMT,
		 ALPASCALSTRARGS(string->internal.length,(char *) string->internal.data.ptr));
	}      
    }
  else
    {
      aloutputstream_printf_1k(output,"'0");
    }
}

void aljson_dump_string_number( struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      struct aloutputstream * output=aljson_get_output(print_ctx);
      if ( object->type == '0' )
	{
	  float f = json_get_float(object);
	  if ( ceilf(f) == f )
	    {
	      aloutputstream_printf_1k(output,"%.0f",f);
	    }
	  else
	    {
	      aloutputstream_printf_1k(output,"%.6f",f);
	    }
	}
      else
	{
	  aloutputstream_printf_1k(output,"#ERROR not a number");
	}
    }
}

void aljson_dump_pair( struct json_pair * pair, struct print_ctx * print_ctx)
{
  struct aloutputstream * output=aljson_get_output(print_ctx);
  aljson_dump_object(pair->key, print_ctx);
  aloutputstream_printf_1k(output,":");
  aljson_dump_object(pair->value, print_ctx);
}

void aljson_dump_variable( struct json_variable * variable, struct print_ctx * print_ctx)
{
  if ( variable != NULL )
    {
      struct aloutputstream * output=aljson_get_output(print_ctx);
      aloutputstream_printf_1k(output,"?");
      if ( variable->key != NULL )
	{
	  aljson_dump_object(variable->key, print_ctx);
	}
      aloutputstream_printf_1k(output,"?");
      if ( variable->bound == 1 )
	{
	  aloutputstream_printf_1k(output,"=");
	  aljson_dump_object(variable->value, print_ctx);	  
	}
    }
}

void aljson_dump_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent )
    {
      struct aloutputstream * output=aljson_get_output(print_ctx);
      aloutputstream_printf_1k(output,"\n");
      if ( print_ctx->indent > 0 )
	{
	  // commented out because print spaces before string ( ie does not repeat string )
	  //	  printf("\n%*s",print_ctx->indent,print_ctx->s_indent);
	  int i=print_ctx->indent;
	  if ( i < 80 )
	    {
	      while ( i >0 )
		{	      
		  aloutputstream_printf_1k(output,"%s",print_ctx->s_indent);
		  --i;
		}
	    }
	}
      else
	{

	}
    }
}

void aljson_dump_pair_object( struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      assert(object->type == ':');
      aljson_dump_pair(&object->pair, print_ctx);
    }
  else
    {
      struct aloutputstream * output=aljson_get_output(print_ctx);
      aloutputstream_printf_1k(output,":0");
    }
}

void aljson_dump_variable_object( struct json_object * object, struct print_ctx * print_ctx)
{

  struct aloutputstream * output=aljson_get_output(print_ctx);     
  if ( object != NULL)
    {
      assert(object->type == '?');

      aljson_print_object_name(object,print_ctx);
      aloutputstream_printf_1k(output,".");

      aljson_dump_variable(&object->variable, print_ctx);
    }
  else
    {
      aloutputstream_printf_1k(output,":0");
    }
}

void aljson_dump_list_object( struct json_object * object, struct print_ctx * print_ctx)
{
  int i=0;
  struct aloutputstream * output=aljson_get_output(print_ctx);
  aloutputstream_printf_1k(output,"%c",object->type);
  aljson_dump_enter_indent( print_ctx);
  if (object->list.nitems > 0)
    {
      aljson_dump_indent(print_ctx);
      aljson_dump_object(object->list.value[0], print_ctx);
      for(i=1;i< object->list.nitems;i++)
	{
	  aloutputstream_printf_1k(output,",");
	  aljson_dump_indent(print_ctx);
	  aljson_dump_object(object->list.value[i], print_ctx);
	}
    }
  aljson_dump_exit_indent( print_ctx);
  aljson_dump_indent(print_ctx);
  aloutputstream_printf_1k(output,"]");
}

void aljson_dump_dict_object( struct json_object * object, struct print_ctx * print_ctx)
{
  int i;
  struct aloutputstream * output=aljson_get_output(print_ctx);
  if ( output == NULL )
    {
      return;
    }
  
  aloutputstream_printf_1k(output,"%c",object->type);
  aljson_dump_enter_indent(print_ctx);
  if (object->dict.nitems > 0)
    {
      aljson_dump_indent(print_ctx);
      aljson_dump_pair(object->dict.items[0], print_ctx);
      for(i=1;i< object->dict.nitems;i++)
	{
	  aloutputstream_printf_1k(output,",");
	  aljson_dump_indent(print_ctx);
	  aljson_dump_pair(object->dict.items[i], print_ctx);
	}
    }
  aljson_dump_exit_indent( print_ctx);
  aljson_dump_indent(print_ctx);
  aloutputstream_printf_1k(output,"}");
}

void aljson_dump_growable( struct json_growable *growable, struct print_ctx * print_ctx)
{
  struct aloutputstream * output=aljson_get_output(print_ctx);
  if ( output == NULL )
    {
      printf("[ERROR] NULL output for print ctx %p at %s:%i\n",print_ctx,__FILE__,__LINE__);
      return;
    }

  struct json_link * link=NULL;
  link=growable->tail;
  aloutputstream_printf_1k(output,"|%c",growable->final_type);
  if ( link != NULL)
    {
      aljson_dump_object( growable->head.value, print_ctx);
      if ( link != &growable->head )
	{
	  link=growable->head.next;
	  while (link != NULL)
	    {
	      aloutputstream_printf_1k(output,",");
	      aljson_dump_object( link->value, print_ctx);
	      link=link->next;
	    }
	}
    }
  else
    {
      if (growable->size != 0) aloutputstream_printf_1k(output,"#");	
    }
  aloutputstream_printf_1k(output,"%c|",growable->final_type);
}

void aljson_dump_growable_object( struct json_object * object, struct print_ctx * print_ctx)
{
  assert(object->type == 'G');
  struct json_growable * growable=&object->growable;
  aljson_dump_growable(growable, print_ctx);
}

void aljson_dump_constant_object( struct json_object * object, struct print_ctx * print_ctx)
{
  struct aloutputstream * output=aljson_get_output(print_ctx);
  if ( object->constant != NULL )
    {
      switch(object->constant->value)
	{
	case JSON_CONSTANT_TRUE:
	  aloutputstream_printf_1k(output,"true");
	  break;
	case JSON_CONSTANT_FALSE:
	  aloutputstream_printf_1k(output,"false");
	  break;
	case JSON_CONSTANT_NULL:
	  aloutputstream_printf_1k(output,"null");
	  break;
	default:
	  aldebug_printf(DBGSTREAM,"ERROR constant type %c %p",object->type, print_ctx);
	}
    }
  else
    {
      aldebug_printf(DBGSTREAM,"ERROR constant type %c %p NULL",object->type, print_ctx);
    }
    
}

void aljson_dump_error_object( struct json_object* object, struct print_ctx * print_ctx)
{
  struct aloutputstream * output=aljson_get_output(print_ctx);
  if (( object != NULL) && (object->error.string.internal.data.ptr != NULL ))
    {
      aloutputstream_printf_1k(output,"syntax error %u (line:%i,column:%i)\n" ALPASCALSTRFMT "\n",
	     object->error.erroridx,
	     object->error.where.line,object->error.where.column,
	     ALPASCALSTRARGS(object->error.string.internal.length,(char *) object->error.string.internal.data.ptr)
	     );
    }
  else
    {
      aloutputstream_printf_1k(output,"syntax error object is corrupted %p\n", object);
    }
}

// limited to print_ctx->max_depth since relying on code stack call.
void aljson_dump_object( struct json_object * object, struct print_ctx * print_ctx)
{
  static int depth = 0;
  struct aloutputstream * output=aljson_get_output(print_ctx);
  
  ++depth;

  if ( depth > print_ctx->max_depth )
    {
      aldebug_printf(DBGSTREAM,"... depth > %i ...\n", print_ctx->max_depth);
      --depth;
      return;
    }
  
  if (object != NULL)
    {
      // printf("%p[%c]",object,object->type);
      switch(object->type)
	{
	case 'G':
	  aljson_dump_growable_object( object, print_ctx);
	  break;
	case '{':
	  aljson_dump_dict_object(object, print_ctx);
	  break;
	case '[':
	  aljson_dump_list_object(object, print_ctx);
	  break;
	case '"':
	case '\'':
	case '$':
	  aljson_dump_string(object, print_ctx);
	  break;
	case '0':
	  aljson_dump_string_number(object, print_ctx);
	  break;
	case ':':
	  aljson_dump_pair_object(object, print_ctx);
	  break;
	case ',':
	  aloutputstream_printf_1k(output,"#");
	  break;
	case '?':
	  aljson_dump_variable_object(object, print_ctx);
	  break;
	case 'n':
	case 't':
	case 'f':
	  aljson_dump_constant_object(object, print_ctx);
	  break;
	case 'E':
	  aljson_dump_error_object(object, print_ctx);
	  break;
        default:
	  aldebug_printf(DBGSTREAM,"ERROR type %c(%x) %p",object->type,object->type, print_ctx);
	}
    }
  else
    {
      aldebug_printf(DBGSTREAM," NULL ");
    }

  --depth;
}

