#ifndef __ALHASHTREE_H__
#define __ALHASHTREE_H__

// todo hash tree known as Merkle tree
// https://en.wikipedia.org/wiki/File:Hash_Tree.svg

/** since now we are in blockchain era having a merkle tree
    in your tools hide the fact that implementation is crap indeed */

#include "albtree.h"
#include "alstrings.h"

enum alhashnodetype {
  AL_TREELEAF,
  AL_BTREENODE,
  AL_TREENODE
};

struct alhashtreenode;

// blockA for intial leaf computation then blockA is left and blockB is right
typedef void (*alhashtreehashfunc) (struct alhashtreenode * treenode, struct alhash_datablock * blockA, struct alhash_datablock * blockB);

struct alhashtreefunc {
  //
  alhashtreehashfunc hashmethod;
  // value for hashing NULL. used when right child is not yet known.
  struct alhash_datablock emptyhash; 
};

struct alhashtreenode {
  // HACK btree should be frist element since used for allocation/clean of alhashtreenode.
  // for which data can be used at will to attach from api user.
  // left point on a alhashtreenode
  // right point on a alhashtreenode
  struct albtree btree;
  // add parent to be able to walf from a child.
  struct alhashtreenode * parent;
  enum alhashnodetype nodetype;
  struct alhash_datablock  hash;
  struct alhashtreefunc func;
  // allocation context
  struct alallocation_ctx * context;
};


// compute empty sha256 hash globally.
void alhashtree_global_init_sha256(struct alallocation_ctx * context);

struct alhashtreenode * alhashtree_create(struct alallocation_ctx * context);

void alhashtree_clean(struct alhashtreenode * treenode);
  
// inoutroot :
// at input to walk tree to find a free place in binary tree for insert
// at output return newly added rightmost leaf.
// trigger a recomputation of hash
struct alhashtreenode * alhashtree_add_block(struct alhashtreenode *intree, struct alhash_datablock * block);

// store hash in hashout
void alhashtree_get_hash(struct alhashtreenode * treenode, struct alhash_datablock * hashout);

void alhashtree_set_data(struct alhashtreenode * treenode, void * data);

void * alhashtree_get_data(struct alhashtreenode * treenode);

int alhashtree_depth_to_root(struct alhashtreenode *intree, struct alhashtreenode ** newroot);

#endif // #ifndef __ALHASHTREE_H__
