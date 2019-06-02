#ifndef __ALHASHTREE_H__
#define __ALHASHTREE_H__

// todo hash tree known as Merkle tree
// https://en.wikipedia.org/wiki/File:Hash_Tree.svg

/** since now we are in blockchain era having a merkle tree
    in your tools hide the fact that implementation is crap indeed */

#define ALHASHTREECANARY 0xdabe000f

#include "albtree.h"
#include "alstrings.h"
#include "aloutput.h"

// not used yet ( somehow lazzily set )
enum alhashnodetype {		     
		     AL_TREELEAF, // is a leaf, ie left and right are NULL
		     AL_TREENODE // is not a leaf
};

struct alhashtreenode;

// blockA for initial leaf computation then blockA is left and blockB is right
typedef void (*alhashtreehashfunc) (struct alhashtreenode * treenode, aldatablock * blockA, aldatablock * blockB);

struct alhashtreefunc {
  //
  alhashtreehashfunc hashmethod;
  // value for hashing NULL. used when right child is not yet known.
  aldatablock emptyhash; 
};

struct alhashtreenode {
  // HACK btree should be first element since used for allocation/clean of alhashtreenode.
  // for which data can be used at will to attach from api user.
  // left point on a alhashtreenode
  // right point on a alhashtreenode
  struct albtree btree;
  // add parent to be able to walk from a child.
  struct alhashtreenode * parent;
  enum alhashnodetype nodetype;
  aldatablock  hash;
  int canary;
  struct alhashtreefunc func;
  // allocation context
  struct alallocation_ctx * context;
};

struct alhashtree_snapshot {
  int id;
  struct aloutputstream output;
};

// compute empty sha256 hash globally.
void alhashtree_global_init_sha256(struct alallocation_ctx * context);

// MANDATORY to setup up this lib
void alhashtree_global_init(struct aloutputstream * output, struct alallocation_ctx * context);

struct alhashtreenode * alhashtree_create(struct alallocation_ctx * context);

void alhashtree_clean(struct alhashtreenode * treenode);
  
// in out root 
// at input to walk tree to find a free place in binary tree for insert
// at output return newly added rightmost leaf.
// trigger a recomputation of hash
struct alhashtreenode * alhashtree_add_block(struct alhashtreenode *intree, aldatablock * block);

// store hash in hashout
void alhashtree_get_hash(struct alhashtreenode * treenode, aldatablock * hashout);

void alhashtree_set_data(struct alhashtreenode * treenode, void * data);

void * alhashtree_get_data(struct alhashtreenode * treenode);

int alhashtree_depth_to_root(struct alhashtreenode *intree, struct alhashtreenode ** newroot);

// if output is NULL will use output from alhashtree_glogal_init
void alhashtree_dump_treenode(struct aloutputstream * output, struct alhashtreenode * treenode);

void  alhashtree_to_dot(struct aloutputstream * output, struct alhashtreenode * treenode
			);

void alhashtree_snapshot_init(struct alhashtree_snapshot * snapshot,const char * filename);

void alhashtree_snapshot_to_dot(struct alhashtree_snapshot * snapshot,struct alhashtreenode * root);

void alhashtree_snapshot_close(struct alhashtree_snapshot * snapshot);

  
#endif // #ifndef __ALHASHTREE_H__
