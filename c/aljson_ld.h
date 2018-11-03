#ifndef ALJSON_LD_HEADER_
#define ALJSON_LD_HEADER_

#include "alhash.h"

extern char * aljson_ld_keywords[];

//  Every node is an IRI, a blank node, a JSON-LD value, or a list.  
enum aljson_ld_node_type {
  ALJSONLD_NODE_TYPE_IRI,
  ALJSONLD_NODE_TYPE_BLANK,
  ALJSONLD_NODE_TYPE_VALUE,
  ALJSONLD_NODE_TYPE_LIST
};

enum aljson_ld_keyword_index {
  ALJSONLD_KEYWORD_CONTEXT_IDX,
  ALJSONLD_KEYWORD_ID_IDX,
  ALJSONLD_KEYWORD_VALUE_IDX,
  ALJSONLD_KEYWORD_LANGUAGE_IDX,
  ALJSONLD_KEYWORD_TYPE_IDX,
  ALJSONLD_KEYWORD_CONTAINER_IDX,
  ALJSONLD_KEYWORD_LIST_IDX,
  ALJSONLD_KEYWORD_SET_IDX,
  ALJSONLD_KEYWORD_REVERSE_IDX,
  ALJSONLD_KEYWORD_INDEX_IDX,
  ALJSONLD_KEYWORD_BASE_IDX,
  ALJSONLD_KEYWORD_VOCAB_IDX,
  ALJSONLD_KEYWORD_GRAPH_IDX,
  ALJSONLD_KEYWORD_COMA_IDX
};

struct aljson_ld_node {
  enum aljson_ld_node_type type;
  union {
    struct aliri iri;
    void * blank;
    void * value;
    void * list;
  };
};

//Each named graph is a pair consisting of an IRI or blank node identifier (the graph name) and a graph. Whenever practical, the graph name SHOULD be an IRI.

struct aljson_ld_named_graph {
  void * identifier;
  void * graph; // Whenever practical, the graph name SHOULD be an IRI
};

enum  aljson_ld_edge_direction {
  ALJSONLD_DIRECTION_TO,
  ALJSONLD_DIRECTION_FROM
};
struct aljson_ld_edge  {
  enum aljson_ld_edge_direction direction;
  void * label; // Whenever practical, an edge SHOULD be labeled with an IRI.
  struct aljson_ld_node * nodeA;
  struct aljson_ld_node * nodeB;
};

struct aljson_ld_typed_value {
  void * type; // IRI
  void * value;
};

struct aljson_ld_context {
  alhash_context hash_context;
  // expect a pointer on hash_context.dict
  struct alhash_table * keyword_table;
  void * todo;
};

extern struct aljson_ld_context aljson_ld_global_context;

// json_ld global initialization
void aljson_ld();

// return an index in keyword table, if not a keyword return -1
enum aljson_ld_keyword_index aljson_ld_is_keyword(char * string);

const char * aljson_ld_c_keyword(int index);

#endif // ALJSON_LD_HEADER_
