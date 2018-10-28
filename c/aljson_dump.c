#include "aljson_dump.h"

#include <stdio.h>
#include <assert.h>

FILE * aljson_getoutfile(struct print_ctx * print_ctx)
{
  return (FILE *) print_ctx->outfile;
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

void aljson_dump_string(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  FILE * outfile=aljson_getoutfile(print_ctx);
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

void aljson_dump_string_number(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      FILE * outfile=aljson_getoutfile(print_ctx);
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

void aljson_dump_pair(struct json_parser_ctx * ctx, struct json_pair * pair, struct print_ctx * print_ctx)
{
  aljson_dump_object(ctx,pair->key, print_ctx);
  printf(":");
  aljson_dump_object(ctx,pair->value, print_ctx);
}

void aljson_dump_variable(struct json_parser_ctx * ctx, struct json_variable * variable, struct print_ctx * print_ctx)
{
  if ( variable != NULL )
    {
      FILE * outfile=aljson_getoutfile(print_ctx);
      fprintf(outfile,"?");
      if ( variable->key != NULL )
	{
	  aljson_dump_object(ctx,variable->key, print_ctx);
	}
      fprintf(outfile,"?");
      if ( variable->bound == 1 )
	{
	  fprintf(outfile,"=");
	  aljson_dump_object(ctx,variable->value, print_ctx);	  
	}
    }
}

void aljson_dump_indent(struct print_ctx * print_ctx)
{
  if ( print_ctx && print_ctx->do_indent )
    {
      FILE * outfile=aljson_getoutfile(print_ctx);
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

void aljson_dump_pair_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  if ( object != NULL)
    {
      assert(object->type == ':');
      aljson_dump_pair(ctx,&object->pair, print_ctx);
    }
  else
    {
      FILE * outfile=aljson_getoutfile(print_ctx);
      fprintf(outfile,":0");
    }
}

void aljson_dump_variable_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{

  FILE * outfile=aljson_getoutfile(print_ctx);     
  if ( object != NULL)
    {
      assert(object->type == '?');

      json_print_object_name(ctx,object,print_ctx);
      fprintf(outfile,".");

      aljson_dump_variable(ctx,&object->variable, print_ctx);
    }
  else
    {
      fprintf(outfile,":0");
    }
}

void aljson_dump_list_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  int i=0;
  FILE * outfile=aljson_getoutfile(print_ctx);
  fprintf(outfile,"%c",object->type);
  aljson_dump_enter_indent( print_ctx);
  if (object->list.nitems > 0)
    {
      aljson_dump_indent(print_ctx);
      aljson_dump_object(ctx,object->list.value[0], print_ctx);
      for(i=1;i< object->list.nitems;i++)
	{
	  fprintf(outfile,",");
	  aljson_dump_indent(print_ctx);
	  aljson_dump_object(ctx,object->list.value[i], print_ctx);
	}
    }
  aljson_dump_exit_indent( print_ctx);
  aljson_dump_indent(print_ctx);
  fprintf(outfile,"]");
}

void aljson_dump_dict_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  int i;
  FILE * outfile=aljson_getoutfile(print_ctx);
  if ( outfile == NULL )
    {
      return;
    }
  
  fprintf(outfile,"%c",object->type);
  aljson_dump_enter_indent(print_ctx);
  if (object->dict.nitems > 0)
    {
      aljson_dump_indent(print_ctx);
      aljson_dump_pair(ctx,object->dict.items[0], print_ctx);
      for(i=1;i< object->dict.nitems;i++)
	{
	  fprintf(outfile,",");
	  aljson_dump_indent(print_ctx);
	  aljson_dump_pair(ctx,object->dict.items[i], print_ctx);
	}
    }
  aljson_dump_exit_indent( print_ctx);
  aljson_dump_indent(print_ctx);
  fprintf(outfile,"}");
}

void aljson_dump_json_path(struct json_path * json_path)
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

void aljson_dump_growable(struct json_parser_ctx * ctx, struct json_growable *growable, struct print_ctx * print_ctx)
{
  FILE * outfile=aljson_getoutfile(print_ctx);
  struct json_link * link=NULL;
  link=growable->tail;
  fprintf(outfile,"|%c",growable->final_type);
  if ( link != NULL)
    {
      aljson_dump_object(ctx, growable->head.value, print_ctx);
      if ( link != &growable->head )
	{
	  link=growable->head.next;
	  while (link != NULL)
	    {
	      fprintf(outfile,",");
	      aljson_dump_object(ctx, link->value, print_ctx);
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

void aljson_dump_growable_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
{
  assert(object->type == 'G');
  struct json_growable * growable=&object->growable;
  aljson_dump_growable(ctx,growable, print_ctx);
}

void aljson_dump_constant_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
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

void aljson_dump_error_object(struct json_parser_ctx * ctx, struct json_object* object, struct print_ctx * print_ctx)
{
  FILE * outfile=aljson_getoutfile(print_ctx);
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
void aljson_dump_object(struct json_parser_ctx * ctx, struct json_object * object, struct print_ctx * print_ctx)
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
	  aljson_dump_growable_object(ctx, object, print_ctx);
	  break;
	case '{':
	  aljson_dump_dict_object(ctx,object, print_ctx);
	  break;
	case '[':
	  aljson_dump_list_object(ctx,object, print_ctx);
	  break;
	case '"':
	case '\'':
	case '$':
	  aljson_dump_string(ctx,object, print_ctx);
	  break;
	case '0':
	  aljson_dump_string_number(ctx,object, print_ctx);
	  break;
	case ':':
	  aljson_dump_pair_object(ctx,object, print_ctx);
	  break;
	case ',':
	  printf("#");
	  break;
	case '?':
	  aljson_dump_variable_object(ctx,object, print_ctx);
	  break;
	case 'n':
	case 't':
	case 'f':
	  aljson_dump_constant_object(ctx,object, print_ctx);
	  break;
	case 'E':
	  aljson_dump_error_object(ctx,object, print_ctx);
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

