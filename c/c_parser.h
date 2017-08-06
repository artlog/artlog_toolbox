#ifndef _C_PARSER_H_
#define _C_PARSER_H_

#include "c_tokenizer.h"
#include "alhash.h"

// reserved c words
enum c_word_token {
  TOKEN_C_NOMATCH_ID,
  TOKEN_C_STRUCT_ID,
  TOKEN_C_UNION_ID,
  TOKEN_C_ENUM_ID,
  TOKEN_C_SWITCH_ID,
  TOKEN_C_CASE_ID,
  TOKEN_C_BREAK_ID,
  TOKEN_C_DEFAULT_ID,
  TOKEN_C_WHILE_ID,
  TOKEN_C_IF_ID,
  TOKEN_C_ELSE_ID,
  TOKEN_C_RETURN_ID,
  TOKEN_C_VOID_ID,
  TOKEN_C_INT_ID,
  TOKEN_C_FLOAT_ID,
  TOKEN_C_LONG_ID,
  TOKEN_C_CHAR_ID,
  TOKEN_C_SIGNED_ID,
  TOKEN_C_UNSIGNED_ID,
};

enum c_parser_state {
  C_STATE_START_ID,
  C_STATE_STRUCT_ID,
  C_STATE_STRUCT_NAME_ID,
  C_STATE_STRUCT_DECLARATION_ID,
  C_STATE_STRUCT_DEFINITION_ID,
  C_STATE_DECLARE_TYPE_ID,
  C_STATE_ENUM_ID,
  C_STATE_ENUM_NAME_ID,
  C_STATE_ENUM_DECLARATION_ID,
  C_STATE_ENUM_DEFINITION_ID,
  C_STATE_TYPE_ID,
};

struct c_parser_ctx {
  enum c_parser_state state;
  struct json_ctx* tokenizer;
  void * tokenizer_data;
  enum c_word_token last_type;
  enum c_word_token last_word;

  struct alhash_table dict;
  int words;
  int word_buffer_length;
  int word_buffer_pos;
  char * word_buffer;
};

struct c_type_list {
  void * data; // todo
  struct c_type_list * next;
};

enum c_dict_flags {
  C_DICT_FLAG_NULTERMINATED,
  C_DICT_FLAG_RESERVED,
};

struct c_dict_entry {
  int index;
  char * name;
  int length;
  int flags;
};

struct c_declaration_info {
  struct c_type_list * first;
  void * dict_index;
};

struct c_declaration_info_list {
  struct c_declaration_info info;
  struct c_declaration_info_list * next;
};

struct c_struct_info {
  void * dict_index;
  struct c_declaration_info_list * first;
};

struct c_enum_value {
  void * dict_index;
};
  
struct c_enum_value_list {
  struct c_enum_value  value;
  struct c_enum_value_list * next;
};

struct c_enum_info {
  void * dict_index;
  struct c_declaration_info_list * first;
};

// return NULL if parsed, else return unrecognized left token
struct al_token * c_parse_statement(struct c_parser_ctx * parser);

#endif
