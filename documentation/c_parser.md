build/c_parser 

USAGE:
work in progress: first goal is to generate json stub from c struct definition see json_to_c_stub.c
ex:./c_parser infile=./input_for_c_parser.h outform=aljson_stub
more advanced goal is to be a c parser ... 
ex:./c_parser debug=true infile=./c_parser.c

__________________________________________________________________________________

more input_for_c_parser.h 
// file extracted from json_to_c_stub.c which is somehow contract to honor for c_parser

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

struct flat_info {
  char * name; // name
  char * imafile; // full path to access ima file
  char * cubemappingfile; // full path to access cube mapping file corresponding to imafile
};



c$ ../build/c_parser infile=./input_for_c_parser.h outform=aljson_stub
file to parse './input_for_c_parser.h'
outform 'aljson_stub'
[TODO] support outform
struct  test_1{ // type 4 definition. (code line 1900)
[TODO] fixme hardcoded 1000 structures
// struct definition 
int   a // struct member 0
;
int   b // struct member 1
;
int   c // struct member 2
;
char  * d // struct member 3
;
} // close struct
;
struct  test_2{ // type 4 definition. (code line 1900)
[TODO] fixme hardcoded 1000 structures
// struct definition 
int   a // struct member 0
;
struct  test_1 c // struct member 1
;
struct  test_1* e // struct member 2
;
struct  test_1* f // struct member 3
;
} // close struct
;
struct  flat_info{ // type 4 definition. (code line 1900)
[TODO] fixme hardcoded 1000 structures
// struct definition 
char  * name // struct member 0
;
char  * imafile // struct member 1
;
char  * cubemappingfile // struct member 2
;
} // close struct
;
EOF id reached// non NULL token at toplevel parsing
// structure 0
{"test_1":{"a":0,
"b":0,
"c":0,
"d":0,
}
// structure 1
{"test_2":{"a":0,
"c":0,
"e":0,
"f":0,
}
// structure 2
{"flat_info":{"name":0,
"imafile":0,
"cubemappingfile":0,
}
int json_c_test_1_from_json_auto(struct test_1 * outstructp, struct json_object * json_object)
{
AL_GET_JSON_INT_WITH_NAME(outstructp,a,json_object);
AL_GET_JSON_INT_WITH_NAME(outstructp,b,json_object);
AL_GET_JSON_INT_WITH_NAME(outstructp,c,json_object);
AL_GET_JSON_STRING_WITH_NAME(outstructp,d,json_object);
return 1;}
int json_c_test_2_from_json_auto(struct test_2 * outstructp, struct json_object * json_object)
{
AL_GET_JSON_INT_WITH_NAME(outstructp,a,json_object);
AL_GET_JSON_STRUCT(test_1,outstructp,c,json_object,1,WITH_NAME);
AL_GET_JSON_STRUCT_POINTER(test_1,outstructp,e,json_object,1,WITH_NAME);
AL_GET_JSON_STRUCT_POINTER(test_1,outstructp,f,json_object,1,WITH_NAME);
return 1;}
int json_c_flat_info_from_json_auto(struct flat_info * outstructp, struct json_object * json_object)
{
AL_GET_JSON_STRING_WITH_NAME(outstructp,name,json_object);
AL_GET_JSON_STRING_WITH_NAME(outstructp,imafile,json_object);
AL_GET_JSON_STRING_WITH_NAME(outstructp,cubemappingfile,json_object);
return 1;}
struct json_object * json_c_test_1_to_json_auto(struct test_1 * instructp, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
struct json_object * growable = aljson_new_growable(ctx,'{');
ALJSON_ADD_EXPLICT_TYPE(ctx, allocator,"struct test_1",growable);
ALJSON_ADD_INT(ctx,allocator,instructp,a,growable);
ALJSON_ADD_INT(ctx,allocator,instructp,b,growable);
ALJSON_ADD_INT(ctx,allocator,instructp,c,growable);
ALJSON_ADD_STRING(ctx,allocator,instructp,d,growable);
return aljson_concrete(ctx,growable);
}
struct json_object * json_c_test_2_to_json_auto(struct test_2 * instructp, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
struct json_object * growable = aljson_new_growable(ctx,'{');
ALJSON_ADD_EXPLICT_TYPE(ctx, allocator,"struct test_2",growable);
ALJSON_ADD_INT(ctx,allocator,instructp,a,growable);
struct json_object * jobj = json_c_test_1_to_json_auto(&instructp->c, ctx,allocator);
ALJSON_ADD_JSON_OBJECT(ctx,allocator,test_1,instructp,c,jobj,growable);
// TODO ALJSON_ADD_JSON_OBJECT_POINTER(test_1,instructp,e,json_object,1,WITH_NAME);
// TODO ALJSON_ADD_JSON_OBJECT_POINTER(test_1,instructp,f,json_object,1,WITH_NAME);
return aljson_concrete(ctx,growable);
}
struct json_object * json_c_flat_info_to_json_auto(struct flat_info * instructp, struct json_parser_ctx * ctx, alstrings_ringbuffer_pointer * allocator)
{
struct json_object * growable = aljson_new_growable(ctx,'{');
ALJSON_ADD_EXPLICT_TYPE(ctx, allocator,"struct flat_info",growable);
ALJSON_ADD_STRING(ctx,allocator,instructp,name,growable);
ALJSON_ADD_STRING(ctx,allocator,instructp,imafile,growable);
ALJSON_ADD_STRING(ctx,allocator,instructp,cubemappingfile,growable);
return aljson_concrete(ctx,growable);
}
