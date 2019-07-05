#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#include "aljson_output.h"
#include "aljson_dump.h"
#include "aldebug_output.h"

#ifdef JSON_TODO
#include "altodo.h"
#else
// ignore :-(
#define todo(text) printf("todo(%s)\n",text);
#endif

/*
 ----------------
 yet aljson_xxx_output relies on aljson_dump_xxx_object
 should be rewritten for dump to either be independent or to use aljson_xxx_ouput ( ie reverse ).
--------------------
*/

void aljson_print_printf(struct print_ctx * print_ctx, const char *format, ...)
{
  va_list args;
  va_start(args, format);

  aloutputstream_vprintf_1k(print_ctx->outfile,format, args);
  
  va_end(args);
}

void aljson_json_growable_output(struct json_growable * growable, struct print_ctx * print_ctx)
{
  struct json_link * link=NULL;
  link=growable->tail;  
  print_ctx->printf(print_ctx,"|%c",growable->final_type);
  if ( link != NULL)
    {
      aljson_output( growable->head.value, print_ctx);
      if ( link != &growable->head )
	{
	  link=growable->head.next;
	  while (link != NULL)
	    {
	      print_ctx->printf(print_ctx,",");
	      aljson_output( link->value, print_ctx);
	      link=link->next;
	    }
	}
    }
  else
    {
      if (growable->size != 0) aljson_print_printf(print_ctx,"#");
    }
  print_ctx->printf(print_ctx,"%c|",growable->final_type);
}


void aljson_growable_output(struct json_object * object, struct print_ctx * print_ctx)
{
  assert(object->type == 'G');
  struct json_growable * growable=&object->growable;
  aljson_json_growable_output(growable, print_ctx);
}

// limited to print_ctx->max_depth since relying on code stack call.
void aljson_output(struct json_object * object, struct print_ctx * print_ctx)
{
  ++print_ctx->depth;

  if (  print_ctx->depth > print_ctx->max_depth )
    {
      aldebug_printf(DBGSTREAM,"[ERROR] ... depth > %i ...\n", print_ctx->max_depth);
      -- print_ctx->depth;
      return;
    }
  
  if (object != NULL)
    {
      // aldebug_printf(DBGSTREAM,"[DEBUG] %p[%c]",object,object->type);
      switch(object->type)
	{
	case 'G':
	  (*print_ctx->growable_output)( object, print_ctx);
	  break;
	case '{':
	  (*print_ctx->dict_output)(object, print_ctx);
	  break;
	case '[':
	  (*print_ctx->list_output)(object, print_ctx);
	  break;
	case '"':
	case '\'':
	case '$':
	  (*print_ctx->string_output)(object, print_ctx);
	  break;
	case '0':
	  (*print_ctx->number_output)(object, print_ctx);
	  break;
	case ':':
	  (*print_ctx->pair_output)(object, print_ctx);
	  break;
	case ',':
	  // WHY ?
	  aljson_print_printf(print_ctx,"#");
	  break;	  
	case '?':
	  (*print_ctx->variable_output)(object, print_ctx);
	  break;
	case 'n':
	case 't':
	case 'f':
	  (*print_ctx->constant_output)(object, print_ctx);
	  break;
	case 'E':
	  (*print_ctx->error_output)(object, print_ctx);
	  break;
        default:
	  aldebug_printf(DBGSTREAM,"ERROR type %c %p",object->type, print_ctx);
	}
    }
  else
    {
      aljson_print_printf(print_ctx," NULL ");
    }
  -- print_ctx->depth;
}


void aljson_print_ctx_set_format(struct print_ctx * print_ctx, enum aljson_print_format format)
{
  print_ctx->format = format;
  switch(format)
    {
    case ALJSON_PRINT_TABS:
      {      
	// tabs
	print_ctx->do_indent = 1;
	print_ctx->indent = 0;
	print_ctx->s_indent = "\t";
      }
      break;
    case ALJSON_PRINT_FLAT:
      {
	//flat canonical
	print_ctx->do_indent = 0;
	print_ctx->indent = 0;
	print_ctx->s_indent = NULL;
      }
      break;
    case ALJSON_PRINT_2SPACE:
      // this is default
    default:
      {
	print_ctx->indent=0;
	print_ctx->do_indent=2; // 0 no indent, >= 1 number of space by indent.
	print_ctx->s_indent=" ";
	print_ctx->format = ALJSON_PRINT_2SPACE;
      }
    }
}

// print_ctx default to aljson_dump callbacks
void aljson_print_ctx_init_format(struct print_ctx * print_ctx, enum aljson_print_format format)
{
  // 1024 levels of json MAX
  print_ctx->depth=0;
  print_ctx->max_depth=1024;

  aljson_print_ctx_set_format(print_ctx,format);
  

  print_ctx->outfile=aldebug_get_output(DBGSTREAM);
  
  print_ctx->growable_output=aljson_growable_output;
  
  print_ctx->dict_output=aljson_dump_dict_object;
  print_ctx->list_output=aljson_dump_list_object;
  print_ctx->string_output=aljson_dump_string;
  print_ctx->number_output=aljson_dump_string_number;
  print_ctx->error_output=aljson_dump_error_object;
  print_ctx->pair_output=aljson_dump_pair_object;
  print_ctx->constant_output=aljson_dump_constant_object;
  print_ctx->variable_output=aljson_dump_variable_object;

  print_ctx->printf=aljson_print_printf;
}

void aljson_print_ctx_init(struct print_ctx * print_ctx)
{
  aljson_print_ctx_init_format(print_ctx, ALJSON_PRINT_2SPACE);
}

void aljson_print_object_name(struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL )
    {
      if ( object->type != 0 )
	{
	  if (object->owner != NULL )
	    {
	      aljson_print_object_name(object->owner,print_ctx);
	      if ( object->type == ':' )
		{
		  aljson_print_printf(print_ctx,
				      "." ALPASCALSTRFMT,
				      ALPASCALSTRARGS(object->pair.key->string.internal.length,(char *)object->pair.key->string.internal.data.ptr));
		}
	      else if ( object->owner->type  == '[' )
		{
		  aljson_print_printf(print_ctx,".%u", object->index);
		}
	    }
	}
      else
	{
	  aljson_print_printf(print_ctx,"!");
	}
    } 
}

