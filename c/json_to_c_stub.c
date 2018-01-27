#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "json_to_c_stub.h"
#include "todo.h"
#include "albase.h"
/*
Sample of what should be autogenerated 

add meta fields : "<type>" : contains type of structure ( within structure )
add ref : "&name" means *name, this can be etiehr real description or a string representing a type and a json path
TODO : find json path of a structure already saved.

**/

// INPUT

struct test_1 {
  int a;
  int b;
  int c;
  char * d;
};

struct test_2 {
  int a;
  struct test_1 c;
  struct test_1 * e;
  struct test_1 * f;
};


// should be part of generic tools.

// return json_object pair type.
struct json_object * json_c_add_json_object_member(char * name, struct json_object * value, struct json_parser_ctx * ctx, struct token_char_buffer * allocator)
{
  struct alhash_datablock data;
  
  data.data.ptr = name;
  data.type=ALTYPE_OPAQUE;
  data.length = strlen(data.data.ptr);
  data.data.ptr = al_copy_block(allocator, &data);

  printf("key:%.*s %i\n",data.length,(char *) data.data.ptr,data.length);
  // create json pair with name of field
  struct json_object * key = aljson_new_json_object(ctx->tokenizer, '"',  allocator, &data);
  struct json_object * pair = aljson_new_pair_key(ctx, pair);
  pair->pair.key=key;
  if (( pair != NULL ) && (key != NULL ))
    {
      pair->pair.value=value;
    }

  return pair;
}

// json_object pair type.
struct json_object * json_c_add_int_member(char * name, int value, struct json_parser_ctx * ctx, struct token_char_buffer * allocator)
{
  struct alhash_datablock data;

  // convert int to json int char representation
  aljson_build_string_from_int(value, 10, allocator, &data);
  data.type=ALTYPE_OPAQUE;
  struct json_object * object = aljson_new_json_object(ctx->tokenizer, '0', allocator, &data);

  return json_c_add_json_object_member(name, object, ctx, allocator);
}

// json_object pair type.
// capture char value content.
struct json_object * json_c_add_string_member(char * name, char * value, struct json_parser_ctx * ctx, struct token_char_buffer * allocator)
{
  struct alhash_datablock data;
  
  data.type=ALTYPE_OPAQUE;
  data.data.ptr=value;
  if ( value != NULL )
    {
      data.length = strlen(data.data.ptr);
    }
  else
    {
      data.length = 1;
    }
  printf("string:%.*s %i\n",data.length,(char *) data.data.ptr,data.length);
  data.data.ptr = al_copy_block(allocator,&data);
  struct json_object * object = aljson_new_json_object(ctx->tokenizer, '"', allocator, &data);

  return json_c_add_json_object_member(name, object, ctx, allocator);
}



// OUTPUT ( currently not autogenerated but done manualy )

int json_c_test_1_from_json_auto( struct test_1* test, struct json_object * json_object)
{
  if AL_JSON_IS_DICT(json_object)
    {
      AL_GET_JSON_INT_WITH_NAME(test,a,json_object);
      AL_GET_JSON_INT_WITH_NAME(test,b,json_object);
      AL_GET_JSON_INT_WITH_NAME(test,c,json_object);
      AL_GET_JSON_STRING_WITH_NAME(test,d,json_object);
    }
  else if AL_JSON_IS_LIST(json_object)
    {
      AL_GET_JSON_INT_BY_ORDER(test,a,json_object,0);
      AL_GET_JSON_INT_BY_ORDER(test,b,json_object,1);
      AL_GET_JSON_INT_BY_ORDER(test,c,json_object,2);
      AL_GET_JSON_STRING_BY_ORDER(test,d,json_object,3);
    }
  return 1;
}


// create a json_object direclty from struct test_1
struct json_object * json_c_test_1_to_json_auto(  struct test_1* test, struct json_parser_ctx * ctx, struct token_char_buffer * allocator)
{
    
  // 1. allocate json_object
  struct json_object * growable = aljson_new_growable(ctx,'{');
  // 2. add members

  struct json_object * pair = NULL;

  pair = json_c_add_string_member("<type>", "struct test1", ctx, allocator);  
  aljson_add_to_growable(ctx,&growable->growable,pair);

  pair = json_c_add_int_member("a", test->a, ctx, allocator);
  aljson_add_to_growable(ctx,&growable->growable,pair);

  pair = json_c_add_int_member("b", test->b, ctx, allocator);
  aljson_add_to_growable(ctx,&growable->growable,pair);

  pair = json_c_add_int_member("c", test->c, ctx, allocator);
  aljson_add_to_growable(ctx,&growable->growable,pair);

  pair = json_c_add_string_member("d", test->d, ctx, allocator);
  aljson_add_to_growable(ctx,&growable->growable,pair);

  // 3. concretize ( struct can't grow or be edited anymore ).      
  return aljson_concrete(ctx, growable);

}

// create a json_object directty from struct test_2
struct json_object * json_c_test_2_to_json_auto(struct test_2* test, struct json_parser_ctx * ctx, struct token_char_buffer * allocator)
{
    
  // 1. allocate json_object
  struct json_object * growable = aljson_new_growable(ctx,'{');
  // 2. add members

  struct json_object * pair = NULL;

  // specific meta-information
  pair = json_c_add_string_member("<type>", "struct test2", ctx, allocator);  
  aljson_add_to_growable(ctx,&growable->growable,pair);

  pair = json_c_add_int_member("a", test->a, ctx, allocator);
  aljson_add_to_growable(ctx,&growable->growable,pair);

  struct json_object * c = json_c_test_1_to_json_auto(&test->c, ctx,allocator);
  pair = json_c_add_json_object_member("c", c, ctx, allocator);
  aljson_add_to_growable(ctx,&growable->growable,pair);

  if ( test->e != NULL )
    {
      struct json_object * e = json_c_test_1_to_json_auto(test->e, ctx,allocator);
      pair = json_c_add_json_object_member("&e", e, ctx, allocator);
      aljson_add_to_growable(ctx,&growable->growable,pair);
    }

  // actualy cheating, should have a method to find json path of existing value.
  if ( test->f != NULL )
    {
      pair = json_c_add_string_member("&f", "<struct test1 *>/e", ctx, allocator);  
      aljson_add_to_growable(ctx,&growable->growable,pair);
    }

  // 3. concretize ( struct can't grow or be edited anymore ).      
  return aljson_concrete(ctx, growable);

}

int json_c_test_2_from_json_auto( struct test_2* test, struct json_object * json_object)
{
  if (( test == NULL ) || (json_object == NULL))
    {
      return -1;
    }
  if AL_JSON_IS_DICT(json_object)
    {
      AL_GET_JSON_INT_WITH_NAME(test,a,json_object);
      AL_GET_JSON_STRUCT( test_1, test,c,json_object,1,WITH_NAME);
      AL_GET_JSON_STRUCT_POINTER( test_1, test,e,json_object,2,WITH_NAME);
    }
  else if AL_JSON_IS_LIST(json_object)
    {
      AL_GET_JSON_INT_BY_ORDER(test,a,json_object,0);
      AL_GET_JSON_STRUCT( test_1, test,c,json_object,1,BY_ORDER);
      AL_GET_JSON_STRUCT_POINTER( test_1, test,e,json_object,2,BY_ORDER);
    }
  return 1;
}


int main(int argc, char ** argv)
{

  todo("parse input stream and run test_1 parsing of a given json test");
  todo("then code C struct parser ... ");

  albase_set_debug(1);
    
  struct token_char_buffer * allocator = al_token_char_buffer_alloc(10);
  if ( allocator != NULL )
    {
      struct test_1 test1;
      struct json_ctx json_tokenizer;
      struct print_ctx print_context;
      struct json_parser_ctx * ctx = calloc(1,sizeof(struct json_parser_ctx));
      ctx->tokenizer=&json_tokenizer;
      json_context_initialize(&json_tokenizer, NULL);
     
      // allocate  100 words, 1024 characters
      alparser_init(&ctx->alparser,100,1024);      

      // force small to create many buffers
      al_token_char_buffer_init(allocator,8);

      // stupid test : do we survive to NULL entries ?
      json_c_test_2_from_json_auto( NULL, NULL);

      struct json_object * json_object = NULL;

      {
	test1.d = "goutemoica";
	test1.a = 1024;
	test1.b = -1;
	test1.c = 1;
        json_object = json_c_test_1_to_json_auto( &test1, ctx, allocator);
      }
      test1.d = "OOOps :()";

        // two spaces
      print_context.do_indent = 2;
      print_context.indent = 0;
      print_context.s_indent = " ";

      ctx->max_depth=4;
      dump_object(ctx,json_object,&print_context);

      {
	struct test_2 test2;
	test2.a=756;
	test2.c.a=4;
	test2.c.b=5;
	test2.c.c=6;
	test2.c.d="youp";
	test2.e=&test1;
	test2.f=&test1;
	//test2.e=NULL;
	json_object = json_c_test_2_to_json_auto( &test2, ctx, allocator);
      }
      dump_object(ctx,json_object,&print_context);
      

    }
  return 0;
}
