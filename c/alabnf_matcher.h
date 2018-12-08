#ifndef ALABNF_MATCHER_HEADER__
#define ALABNF_MATCHER_HEADER__

#include "alabnf.h"
#include "alinput.h"

enum alabnf_match {
  ALABNF_MATCH_ERROR =1, // internal error
  ALABNF_MATCH_NONE,
  ALABNF_MATCH_CONTINUE,
  ALABNF_MATCH_REMATCH,
  ALABNF_MATCH_FULL,
  ALABNF_MATCH_FAIL_UNSTACK, // failure
  ALABNF_MATCH_SUCCESS_UNSTACK // success
};

enum alabnf_matcher_state_type {
  ALABNF_MATCHER_ST_UNSET = 16,
  ALABNF_MATCHER_ST_OR, // alternative
  ALABNF_MATCHER_ST_AND, // sequence
  ALABNF_MATCHER_ST_IT, // iterator
  ALABNF_MATCHER_ST_NODE
};

typedef struct alabnf_character_ {
  unsigned char uchar;
} alabnf_character;

struct alabnf_matcher_state {
  // can be ROOT AND, OR or NODE
  enum alabnf_matcher_state_type type;
  // node at state creation
  struct alabnf_node * initial_node;
  struct alabnf_node * current_node;
  struct alabnf_rule * current_rule;
  // index with datablock
  int datablock_index;
  // backtracking on alternatives or unflaten sequences
  struct alabnf_matcher_state * parent;
  // for sequences.
  struct alabnf_sequence * next_sequence;
  // for alternatives
  struct alabnf_alternative * alt;
  // for iterator
  int iteration;
  struct alinputstream * input;
  alabnf_character tempchar2;
};

// allows to match a alabnf syntax
struct alabnf_matcher {
  struct alabnf * abnf_syntax;
  struct alinputstream * input;
  struct alinputstream * current_input;
  alabnf_character tempchar1;
  struct alabnf_matcher_state root_state;
  struct alabnf_matcher_state * current_state;
  enum alabnf_match last_match;
  // protect against rematches infinite loop
  int rematches;
  int maxrematches;
  
};

// init matcher with an input stream
void alabnf_match_init(struct alabnf_matcher * matcher, struct alabnf * alabnf, struct alinputstream * input;);

void alabnf_match(struct alabnf_matcher * matcher);

#endif // ALABNF_MATCHER_HEADER__
