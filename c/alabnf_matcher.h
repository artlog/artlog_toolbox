#ifndef ALABNF_MATCHER_HEADER__
#define ALABNF_MATCHER_HEADER__

#include "alabnf.h"
#include "alinput.h"

enum alabnf_match {
  ALABNF_MATCH_NONE =1,
  ALABNF_MATCH_CONTINUE,
  ALABNF_MATCH_REMATCH,
  ALABNF_MATCH_FULL,
};
typedef struct alabnf_character_ {
  unsigned char uchar;
} alabnf_character;

struct alabnf_matcher_state {
  struct alabnf_node * current_node;
  struct alabnf_rule * current_rule;
  // for sequences.
  struct alabnf_sequence * next_sequence;
  // index with datablock
  int datablock_index;
  // backtracking on alternatives
  struct alabnf_matcher_state * parent;

};

// allows to match a alabnf syntax
struct alabnf_matcher {
  struct alabnf * abnf_syntax;
  struct alinputstream * input;
  alabnf_character tempchar1;
  alabnf_character tempchar2;
  struct alabnf_matcher_state root_state;
  struct alabnf_matcher_state * current_state;
};

// init matcher with an input stream
void alabnf_match_init(struct alabnf_matcher * matcher, struct alabnf * alabnf, struct alinputstream * input;);

void alabnf_match(struct alabnf_matcher * matcher);

#endif // ALABNF_MATCHER_HEADER__
