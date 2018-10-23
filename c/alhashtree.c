#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "alhashtree.h"
#include "altodo.h"
#include "alcryptohash.h"

aldatablock emptyhash;

// compute empty hash.
void alhashtree_global_init_sha256(struct alallocation_ctx * context)
{
  // use emptyhash first as an empty block in input.
  bzero(&emptyhash,sizeof(emptyhash));  
  struct alsha2_internal intern;
  alsha256_init(&intern);
  alsha2x_add_block(&intern,&emptyhash);
  aldatablock * result=alsha2x_final(&intern);
  memcpy(&emptyhash,result,sizeof(emptyhash));
  // data pointer is in alsha2_internal intern on stack, use this of context.
  emptyhash.data.charptr=al_copy_block(&context->ringbuffer,result);
}
  
struct alhashtreenode * alhashtree_allocate()
{
  return (struct alhashtreenode *) calloc(1,sizeof(struct alhashtreenode));
}

void alhashtree_clean(struct alhashtreenode * treenode)
{
  free(treenode);
}

void alsha256hashfunc(
		      struct alhashtreenode * treenode,
		      aldatablock * blockA ,
		      aldatablock * blockB)
{
  if ( treenode != NULL )
    {
      struct alsha2_internal intern;
      alsha256_init(&intern);
      alsha2x_add_block(&intern,blockA);
      if ( blockB != NULL )
	{
	  // aldebug_printf(NULL,"length of left %i and right %i\n",blockA->length, blockB->length);
	  if ( blockA->length != treenode->func.emptyhash.length )
	    {
	      aldebug_printf(NULL,"[FATAL] incoherent length of block %i and emptyhash length %i\n",blockA->length, treenode->func.emptyhash.length);
	    }
	  if (blockB->length != blockA->length )
	    {
	      aldebug_printf(NULL,"[FATAL] incoherent length of left %i and right %i\n",blockA->length, blockB->length);
	    }
	  alsha2x_add_block(&intern,blockB);
	}
      aldatablock * result=alsha2x_final(&intern);
      memcpy(&treenode->hash,result,sizeof(treenode->hash));
      // data pointer is in alsha2_internal intern on stack, use this of context.
      treenode->hash.data.charptr=al_copy_block(&treenode->context->ringbuffer,result);
    }
  else
    {
      aldebug_printf(NULL, "[FATAL] alsha256hashfunc for a NULL treenode\n");
    }
  
}

void alhashtree_specific_init(
			     struct alhashtreenode * treenode,
			     struct alallocation_ctx * context,
			     struct alhashtreenode * left,
			     struct alhashtreenode * right,
			     struct alhashtreenode * parent)
{
  if (( left == NULL ) && (right == NULL))
    {
      treenode->nodetype=AL_TREELEAF;
    }
  else
    {
      treenode->nodetype=AL_TREENODE;
    }
  treenode->context=context;
  if (parent ==NULL)
    {
      treenode->func.hashmethod=alsha256hashfunc;
      memcpy(&treenode->func.emptyhash,&emptyhash,sizeof(treenode->func.emptyhash));
    }
  else
    {
      memcpy(&treenode->func, &parent->func, sizeof(treenode->func));
    }
  treenode->parent=parent;
  if ( left != NULL )
    {
      left->parent = treenode;
    }
  if ( right != NULL)
    {
      right->parent = treenode;
    }
}

// called once for initial root, after it is done through albtree allocation and requere a call to alhashtree_specific_init
// first call it is a leaf with an empty block.
void alhashtree_init(struct alhashtreenode * treenode, struct alallocation_ctx * context, struct albtree * left, struct albtree * right)
{
  struct albtree * btree = &treenode->btree;
  albtree_init(btree,NULL,left,right);
  btree->allocate = (albtreeallocator) alhashtree_allocate;
  btree->clean = (albtrecleaner) alhashtree_clean;
  alhashtree_specific_init(
			   treenode,
			   context,
			   (struct alhashtreenode *) left,
			   (struct alhashtreenode *) right,
			   NULL);
}

struct alhashtreenode * alhashtree_create(struct alallocation_ctx * context)
{
  struct alhashtreenode * treenode = alhashtree_allocate();
  alhashtree_init(treenode,context,NULL,NULL);
}

// return depth
int alhashtree_depth_to_root(struct alhashtreenode *intree, struct alhashtreenode ** newroot)
{
  struct alhashtreenode * root = intree;
  struct alhashtreenode * parent = intree->parent;
  int depth = 0;
  while ( parent != NULL )
    {
      root = parent;
      ++ depth;
      parent=parent->parent;
    }
  (*newroot) = root;
  return depth;
}

void alhashtree_recompute_direct_children(struct alhashtreenode *intree)
{
  struct alhashtreenode * left = NULL;
  struct alhashtreenode * right = NULL;

  left=(struct alhashtreenode *) albtree_get_left(&intree->btree);
  right=(struct alhashtreenode *) albtree_get_right(&intree->btree);
  
  aldatablock * blockA = NULL;
  aldatablock * blockB = NULL;
  if ( left == NULL )
    {
      blockA = &intree->func.emptyhash;
    }
  else
    {
      blockA = &left->hash;
    }
  
  if ( right == NULL )
    {
      blockB = &intree->func.emptyhash;
    }
  else
    {
      blockB = &right->hash;
    }

  intree->nodetype=AL_TREENODE;
  (*intree->func.hashmethod)(intree,blockA,blockB);
}

// recompute all hash up to root
int alhashtree_recompute_upto_root(struct alhashtreenode *intree)
{
  struct alhashtreenode * current = intree;
  struct alhashtreenode * parent = intree->parent;
  int depth = 0;
  while ( parent != NULL )
    {
      current = parent;
      alhashtree_recompute_direct_children(current);
      ++ depth;           
      parent=parent->parent;
    }
  aldebug_printf(NULL,"recomputed %i parent nodes\n", depth);
  return depth;
}

// assuming intree is already rightmost deeper leaf.
struct alhashtreenode * alhashtree_create_sibling(struct alhashtreenode *intree, struct alhashtreenode ** newroot)
{
  int depth = alhashtree_depth_to_root(intree, newroot);
  struct alhashtreenode * previous_root = (*newroot);
  struct alhashtreenode * root = previous_root;
  struct alhashtreenode * right = NULL;
  struct alhashtreenode * added = NULL;
  struct alhashtreenode * parent = NULL;

  parent = intree->parent;
  if ( parent != NULL )
    {      
      right=(struct alhashtreenode *) albtree_get_right(&parent->btree);
    }
  else
    {
      right=NULL;
    }

  if ( right == intree )
    {
      aldebug_printf(NULL,"create a new right -child or parent- for %p\n", intree);
      right = alhashtree_allocate();
      alhashtree_init(right,intree->context, NULL, NULL);
      added=right;
    }
  else if (right == NULL)
    {
      aldebug_printf(NULL,"create a new right child for %p\n", intree);
      right = alhashtree_allocate();
      alhashtree_init(right,intree->context, NULL, NULL);
      if ( parent != NULL )
	{
	  albtree_set_right(&parent->btree,&right->btree);
	  right->parent=parent;
	  return right;
	}
      added = right;
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] unexpected case, disrespecting calling assertion that element is rightmost leaf");
      return right;
    }

  aldebug_printf(NULL,"create a new root with previous root at left for depth %i\n", depth);
  root = alhashtree_allocate();
  alhashtree_init(root,intree->context, &previous_root->btree,&right->btree);

  parent=right;
  
  for (int i;i<depth;i++)
    {
      // add_left children.
      added = (struct alhashtreenode *) albtree_insert_left(&parent->btree, NULL);
      alhashtree_specific_init(added, added->context, NULL, NULL, parent);
      parent=added;
    }

  (*newroot) = root;
  
  return added;
}

// assuming intree is already rightmost deeper leaf.
struct alhashtreenode * alhashtree_add_block(struct alhashtreenode *intree, aldatablock * block)
{
  struct alhashtreenode * added = NULL;
  struct alhashtreenode * search = NULL;
  int depth=0;
  
  todo("find rightmost leaf");

  if (intree != NULL )
    {
      if ( intree->hash.length == 0 )
	{
	  added=intree;
	}
      else
	{
	  struct alhashtreenode * root;
	  added = alhashtree_create_sibling(intree, &root);
	}
      
      if ( added == NULL )
	{
	  aldebug_printf(NULL,"[FATAL] added treenode is NULL\n");
	}
      else
	{
	  (*intree->func.hashmethod)(added,block,NULL);
	  alhashtree_recompute_upto_root(added);
	}
    }
  
  return added;;

}

// store hash in hashout
void alhashtree_get_hash(struct alhashtreenode * treenode, aldatablock * hashout)
{
  memcpy(hashout, &treenode->hash, sizeof(*hashout));
}

void alhashtree_set_data(struct alhashtreenode * treenode, void * data)
{
  treenode->btree.data=data;
}

void * alhashtree_get_data(struct alhashtreenode * treenode)
{
  return treenode->btree.data;
}

