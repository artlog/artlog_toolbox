#ifndef ALABNF_HEADER__
#define ALABNF_HEADER__

#include "altokenizer.h"
#include "alhash.h"
#include "alinput.h"
#include "alstack.h"
#include "aldebug.h"

enum alabnf_node_type {
  ALABNF_NT_INVALID=1,
  ALABNF_NT_ITERATOR,
  ALABNF_NT_SEQUENCE,
  ALABNF_NT_ALT,
  ALABNF_NT_RULE_REF,
  ALABNF_NT_RANGE,
  ALABNF_NT_STRING,
};

enum alabnf_string_type {
  ALABNF_ST_UNDEFINED=1,
  ALABNF_ST_RULENAME,
  ALABNF_ST_RANGE,
  ALABNF_ST_QUOTED,
  ALABNF_ST_HEX,
  ALABNF_ST_DEC,
  ALABNF_ST_BIN
};

struct alabnf_global_ {
  ALDEBUG_DEFINE_FLAG(debug);
};

extern struct alabnf_global_  alabnf_global;

ALDEBUG_DECLARE_FUNCTIONS(struct alabnf_global_, alabnf)

struct alabnf_node;

// <min>*<max><alabnf_node>
// alabnf_node_type ALABNF_NT_ITERATOR
struct alabnf_iterator {
  int min;
  int max;
  struct alabnf_node * node;
};

// alabnf_node_type ALABNF_NT_STRING
struct alabnf_string {
  enum alabnf_string_type type;
  aldatablock strbloc;
};

// ( <node> <next> )
// alabnf_node_type ALABNF_NT_SEQUENCE
struct alabnf_sequence {
  struct alabnf_node * node; // not a list somehow like lisp car
  struct alabnf_sequence * next; // somehow like lisp cdr
};

// <node>/<alt>
// alabnf_node_type ALABNF_NT_ALT
struct alabnf_alternative {
  struct alabnf_node * node; // not a list somehow like lisp car
  struct alabnf_alternative * alt; // somehow like lisp cdr
};

// alabnf_node_type ALABNF_NT_RANGE
struct alabnf_range {
  int start;
  int end;
};

// alabnf_node_type ALABNF_NT_RULE_REF
struct alabnf_rule_ref {
  aldatablock keyblock;
  struct alabnf_node * resolved;
};

struct alabnf_node {
  // type for content, to select within union.
  enum alabnf_node_type type;
  union {
    struct alabnf_iterator iterator;
    struct alabnf_sequence sequence;
    struct alabnf_string string;
    struct alabnf_alternative alt;
    struct alabnf_range range;
    struct alabnf_rule_ref rule_ref;
  } content;
};

struct alabnf_rule {
  struct alabnf_string rule_name;
  struct alabnf_node * value;
};


struct alabnf {
  // does contain a hash_table dict which is used for rule_def resolution
  // impl info : done through altokenizer that share this context, quite dangerous
  alhash_context context;
  struct alabnf_rule root_rule;
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

enum alabnf_number_sm_state {
  ALABNF_NSM_START,
  ALABNF_NSM_MIN_SET,
  ALABNF_NSM_MAX_SET
};

struct alabnf_number_sm {
  int seen; // number of char seen (usefull for hex or binary )
  int cumulated;
  enum alabnf_number_sm_state state;
  int min;
  int max;
};

enum alabnf_sm_state {
  ALABNF_STATE_RULENAME, // left side of =
  ALABNF_STATE_RULEDEF // right side of =
};

enum alabnf_iterator_sm_state {
  ALABNF_ISM_START,
  ALABNF_ISM_MIN_START,
  ALABNF_ISM_MIN_SET,
  ALABNF_ISM_MAX_START
};


extern const int ALABNF_INFINITE_ITERATION;
  
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
  int characters;
  int state_loop;
  int linebreak;  
  // used to construct an unique id for an iterator entry
  int iterator_index;
  enum alabnf_string_type string_type;
  // use with great care
  // content is an alabnf_node
  struct alstack * stack;
  struct alabnf * generated;
  enum alabnf_iterator_sm_state it_state;
  int it_min;
  int it_max;
};

extern const int ALABNF_INFINITE_ITERATION;

// setup and allocate , to dispose use alabnf_state_machine_release
void alabnf_state_machine_init(struct alabnf_sm *state_machine,   struct alinputstream * inputstream);

void alabnf_state_machine_run(struct alabnf_sm * state_machine);

void alabnf_state_machine_release(struct alabnf_sm * state_machine);

struct alabnf * alabnf_state_machine_generated(struct alabnf_sm *state_machine);


#endif
