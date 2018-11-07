#ifndef ALABNF_HEADER__
#define ALABNF_HEADER__

#include "altokenizer.h"
#include "alhash.h"
#include "alinput.h"
#include "alstack.h"

enum alabnf_node_type {
  ALABNF_NT_ITERATOR,
  ALABNF_NT_SEQUENCE,
  ALABNF_NT_STRING,
  ALABNF_NT_ALT,
  ALABNF_NT_RANGE,
};

enum alabnf_string_type {  
  ALABNF_ST_RULENAME,
  ALABNF_ST_QUOTED,
  ALABNF_ST_HEX,
  ALABNF_ST_DEC,
  ALABNF_ST_BIN
};

struct alabnf_node;

struct alabnf_iterator {
  int min;
  int max;
  struct alabnf_node * node;
};

struct alabnf_string {
  enum alabnf_string_type type;
  aldatablock strbloc;
};

struct alabnf_sequence {
  struct alabnf_node * node; // not a list somehow like lisp car
  struct alabnf_sequence * next; // somehow like lisp cdr
};

struct alabnf_alternative {
  struct alabnf_node * node; // not a list somehow like lisp car
  struct alabnf_alternative * alt; // somehow like lisp cdr
};

struct alabnf_range {
  int start;
  int end;
};

struct alabnf_node {
  enum alabnf_node_type type;
  union {
    struct alabnf_iterator iterator;
    struct alabnf_sequence sequence;
    struct alabnf_string string;
    struct alabnf_alternative alt;
    struct alabnf_range range;
  } content;
};

struct alabnf_rule {
  struct alabnf_string rule_name;
  struct alabnf_node * value;
};

struct alabnf {
  // does contain a hash_table dict which is rulelist
  alhash_context context;
};

// parser state machine to read abnf syntax

struct alabnf_sm;


typedef void (*alabnf_one_char_method) (struct alabnf_sm * state_machine, char c);
typedef void (*alabnf_close_method) (struct alabnf_sm * state_machine, char c);

enum alabnf_parser_action {
  ALABNF_PA_CONTINUE,
  ALABNF_PA_REMATCH,
  ALABNF_PA_CLOSE,
  ALABNF_PA_FAIL
};

struct alabnf_number_sm {
  int seen; // number of char seen (usefull for hex or binary )
  int cumulated;    
};

enum alabnf_sm_state {
  ALABNF_STATE_RULENAME, // left side of =
  ALABNF_STATE_RULEDEF // right side of =
};

struct alabnf_sm {
  struct altokenizer tokenizer;
  alabnf_one_char_method one_char_method;
  alabnf_close_method close_method;
  enum alabnf_parser_action next_action;
  struct alinputstream * inputstream;
  char rematch;
  struct alabnf_number_sm number_sm;
  enum alabnf_sm_state state;
  int initial_indent;
  int current_indent;
  int rule_number;
  int lf_line;
  int linebreak;
  struct alstack * stack;
  struct alabnf * generated;
};

// setup and allocate , to dispose use alabnf_state_machine_release
void alabnf_state_machine_init(struct alabnf_sm *state_machine,   struct alinputstream * inputstream);

void alabnf_state_machine_run(struct alabnf_sm * state_machine);

void alabnf_state_machine_release(struct alabnf_sm * state_machine);

struct alabnf * alabnf_state_machine_generated(struct alabnf_sm *state_machine);

#endif
