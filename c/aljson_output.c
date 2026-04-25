#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

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
// defined in aljson_dump ....
struct aloutputstream * aljson_get_output(struct print_ctx * print_ctx);


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


// https://www.json.org/json-en.html
void aljson_quoted_string_output( struct json_string * string, struct aloutputstream * stream ,char quote)
{
     
  // NULL terminated string ?
  // todo("implement ALTYPE_STR0 for string->internal.type");
  int length = string->internal.length;
  char * str =  string->internal.data.ptr;
  for (int i = 0 ; i < length; i ++ )
    {
      char c = str[i];
      // should do reverse of aljson_parser parse_until_escaped_level
      // TODO handle UTF8
      switch (c)
	{
	case '\n':
	  aloutputstream_write_byte(stream,'\\');
	  aloutputstream_write_byte(stream,'n');
	  break;
	case '\r':
	  aloutputstream_write_byte(stream,'\\');
	  aloutputstream_write_byte(stream,'r');
	  break;
	case '\t':
	  aloutputstream_write_byte(stream,'\\');
	  aloutputstream_write_byte(stream,'t');
	  break;
	case '\f':
	  aloutputstream_write_byte(stream,'\\');
	  aloutputstream_write_byte(stream,'f');
	  break;
	case '"':
	case '\\':
	  aloutputstream_write_byte(stream,'\\');
	default:
	  aloutputstream_write_byte(stream,c);
	}
    }
}
  
// add escape '\' for protected characters
void aljson_string_output( struct json_object * object, struct print_ctx * print_ctx)
{
  struct aloutputstream * stream=aljson_get_output(print_ctx);
  if ( object != NULL)
    {
      struct json_string * string = &object->string;
      if ( ( object->type != '$' ) && ( object->type != '0') )
	{
	  // TODO , this seems to be an error at first sight
	  // we should not be called for such object type.

	  // ARGHH in fact this is what is called, type of quoted string is char ' or "

	  if ( object->type == '"' )
	    {
	      char quote=object->type;
	      aloutputstream_write_byte(stream,quote);
	      aljson_quoted_string_output(string,stream,quote);
	      aloutputstream_write_byte(stream,quote);
	    }
	  else
	    {
	      	  aloutputstream_printf_1k(stream,"%c" ALPASCALSTRFMT "%c",
		 object->type,
		 ALPASCALSTRARGS(string->internal.length,(char *) string->internal.data).ptr,
		 object->type);
	    }
	    
	}
      else
	{
	  // NULL terminated string ?
	  // todo("implement ALTYPE_STR0 for string->internal.type");
	  aloutputstream_printf_1k(stream,ALPASCALSTRFMT,
		 ALPASCALSTRARGS(string->internal.length,(char *) string->internal.data.ptr));
	}
    }
  else
    {
      aloutputstream_printf_1k(stream,"'0");
    }
}


void aljson_json_pair_output( struct json_pair * pair, struct print_ctx * print_ctx)
{
  struct aloutputstream * output=aljson_get_output(print_ctx);
  aljson_dump_object(pair->key, print_ctx);
  aloutputstream_write_byte(output,':');
  if ( print_ctx->space_after == 1 )
    {
          aloutputstream_write_byte(output,' ');
    }
  // might not be a string, can be dict, constant, ...
  aljson_output(pair->value, print_ctx);
}

void aljson_pair_output( struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      assert(object->type == ':');
      aljson_json_pair_output(&object->pair, print_ctx);
    }
  else
    {
      struct aloutputstream * output=aljson_get_output(print_ctx);
      aloutputstream_printf_1k(output,":0");
    }
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
  print_ctx->space_after=0;
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
    case ALJSON_PRINT_SPACES:
      {
	print_ctx->indent=0;
	print_ctx->do_indent=3; // >= 1 number of space by indent.
	print_ctx->s_indent=" ";
	print_ctx->format = ALJSON_PRINT_SPACES;
      }
    default:
      {
	// this is default : 3 spaces
	print_ctx->indent=0;
	print_ctx->do_indent=3; // >= 1 number of space by indent.
	print_ctx->s_indent=" ";
	print_ctx->format = ALJSON_PRINT_SPACES;
      }
    }
}

void aljson_print_ctx_set_output(struct print_ctx * print_ctx, struct aloutputstream * output)
{
  print_ctx->outfile=output;
}
  
// print_ctx default to aljson_dump callbacks
void aljson_print_ctx_init_format(struct print_ctx * print_ctx, enum aljson_print_format format)
{  
  // 1024 levels of json MAX  HARDCODED DEFAULT
  print_ctx->depth=0;
  print_ctx->max_depth=1024;

  aljson_print_ctx_set_format(print_ctx,format);
  // NUMBER AS STRING HARDCODED DEFAULT
  print_ctx->number_encoding=ALJSON_NUMBER_ENCODING_STRING;

  print_ctx->outfile=aldebug_get_output(DBGSTREAM);
  
  print_ctx->growable_output=aljson_growable_output;
  
  print_ctx->dict_output=aljson_dump_dict_object;
  print_ctx->list_output=aljson_dump_list_object;
  print_ctx->string_output=aljson_string_output;
  print_ctx->number_output=aljson_dump_string_number;
  print_ctx->error_output=aljson_dump_error_object;
  print_ctx->pair_output=aljson_pair_output;
  print_ctx->constant_output=aljson_dump_constant_object;
  print_ctx->variable_output=aljson_dump_variable_object;

  print_ctx->printf=aljson_print_printf;
}

void aljson_print_ctx_init(struct print_ctx * print_ctx)
{
  aljson_print_ctx_init_format(print_ctx, ALJSON_PRINT_SPACES);
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


/** generic case for transposition from json to something else **/

// limited to output_context->max_depth since relying on code stack call.
void aljson_output_with_callback(struct json_object * object, struct aljson_output_context * output_context)
{
  ++output_context->depth;

  if (  output_context->depth > output_context->max_depth )
    {
      aldebug_printf(DBGSTREAM,"[ERROR] ... depth > %i ...\n", output_context->max_depth);
      -- output_context->depth;
      return;
    }
  
  if (object != NULL)
    {
      struct aljson_output_callback_context * callback=&output_context->callback;
      
      // aldebug_printf(DBGSTREAM,"[DEBUG] %p[%c]",object,object->type);
      switch(object->type)
	{
	case 'G':
	  (*callback->growable_object)( object, output_context);
	  break;
	case '{':
	  (*callback->dict_object)(object, output_context);
	  break;
	case '[':
	  (*callback->list_object)(object, output_context);
	  break;
	case '"': // DQUOTE
	case '\'': // SQUOTE
	case '$':  // VAR ?
	   (*callback->string)(object, output_context);
	  break;
	case '0':
	   (*callback->string_number)(object, output_context);
	  break;
	case ':':
	  (*callback->pair_object)(object, output_context);
	  break;
	case ',':
	  // WHY ?
	  break;	  
	case '?':
	  (*callback->variable_object)(object, output_context);
	  break;
	case 'n':
	case 't':
	case 'f':
	  (*callback->constant_object)(object, output_context);
	  break;
	case 'E':
	  (*callback->error_object)(object, output_context);
	  break;
        default:
	  aldebug_printf(DBGSTREAM,"ERROR type %c %p",object->type, output_context);
	}
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[ERROR] NULL object");
    }
  -- output_context->depth;
}

void aljson_output_init( struct aljson_output_context * output, struct aljson_output_callback_context * callback, void * data )
{
  memcpy(&output->callback,callback,sizeof(output->callback));
  output->data=data;
  // HARDCODED depth
  output->depth=0;
  output->max_depth=1024;
}
