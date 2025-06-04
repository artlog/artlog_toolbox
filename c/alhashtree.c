#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "alhashtree.h"
#include "altodo.h"
#include "alcryptohash.h"
#include "aldebug_output.h"
#include "aloutput_file.h"

aldatablock emptyhash;

struct aloutputstream * globalout;

// compute and initialize emptyhash
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
  aldebug_printf(DBGSTREAM,"[DEBUG] emptyhash charptr %p\n",emptyhash.data.charptr);
}

// compute and initialize emptyhash
void alhashtree_global_init(struct aloutputstream * output, struct alallocation_ctx * context)
{
  globalout = output;
  alhashtree_global_init_sha256(context);
}

struct alhashtreenode * alhashtree_allocate()
{
  struct alhashtreenode * newnode = (struct alhashtreenode *) calloc(1,sizeof(struct alhashtreenode));
  newnode->canary=ALHASHTREECANARY;
  aldebug_printf(DBGSTREAM,"[DEBUG] alhashtree_allocate %p\n",newnode);
  return newnode;
}

void alhashtree_dump_btreenode(struct aloutputstream * output, struct albtree * btreenode)  
{
  aloutputstream_printf_1k(output,
		   "btreenode %p\n"
		   "allocate %p (should be %p and not %p)\n"
		   "left %p\n"
		   "right %p\n",
		   btreenode,
		   btreenode->allocate,
		   // the only thing specific to this hashtree...
		   alhashtree_allocate,
		   albtree_allocate,
		   btreenode->left,
		   btreenode->right);
}

void alhashtree_dump_treenode(struct aloutputstream * output, struct alhashtreenode * treenode)
{

  if ( output == NULL )
    {
      output = globalout;
    }

  aloutputstream_printf_1k(output,
		 "treenode %p\n"
		 "parent %p\n"
		 "nodetype %x\n"
		 "canary %x\n"
		 "context %p\n"
		 "hash length %i\n",
		 treenode,
		 treenode->nodetype,
		 treenode->parent,
		 treenode->canary,
		 treenode->context,
		 treenode->hash.length
		 );

  if ( treenode->hash.length > 0 )
    {
      aloutput_bytes_as_hex(output, &treenode->hash, 0, 4);
      // quite coslty for a newline ...
      aloutputstream_printf_1k(output,"\n");
    }

  if ( treenode->btree.data != NULL )
    {
      aloutputstream_printf_1k(output," data : '%s'\n",(char *) treenode->btree.data);
    }

  struct albtree * btreenode = &treenode->btree;

  alhashtree_dump_btreenode(output, btreenode);

  /* with parent ...
  if ( treenode->parent !=NULL)
    {
      btreenode = &treenode->parent->btree;      
      alhashtree_dump_btreenode(output, btreenode);
    }
  */
  
}


void alhashtree_clean(struct alhashtreenode * treenode)
{
  aldebug_printf(DBGSTREAM,"[DEBUG] alhashtree_clean %p\n",treenode);
  free(treenode);
}

void alhashtree_snapshot_init(struct alhashtree_snapshot * snapshot,const char * filename)
{
  aloutput_file_open_init(&snapshot->output,filename);
  snapshot->id=0;
}

void alhashtree_snapshot_close(struct alhashtree_snapshot * snapshot)
{
  aloutputstream_close(&snapshot->output);
  
}

void alsha256hashfunc(
		      struct alhashtreenode * treenode,
		      aldatablock * blockA ,
		      aldatablock * blockB)
{
  if ( treenode != NULL )
    {
      if ( treenode->canary != ALHASHTREECANARY )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] wrong canary %i for treenode %p\n",treenode->canary, treenode);
	  alhashtree_dump_treenode(NULL,treenode);
	  
	}
      if ( treenode->hash.data.ptr != NULL)
	{
	  aldebug_printf(DBGSTREAM,"[WARNING] treenode hash data already set %p %i\n", treenode->hash.data.ptr, treenode->hash.length);
	}	 
      
      struct alsha2_internal intern;
      alsha256_init(&intern);      
      alsha2x_add_block(&intern,blockA);
      if ( blockB != NULL )
	{
	  // aldebug_printf(DBGSTREAM,"length of left %i and right %i\n",blockA->length, blockB->length);
	  if ( blockA->length != treenode->func.emptyhash.length )
	    {
	      aldebug_printf(DBGSTREAM,"[FATAL] incoherent length of block %i and emptyhash length %i\n",blockA->length, treenode->func.emptyhash.length);
	    }
	  if (blockB->length != blockA->length )
	    {
	      aldebug_printf(DBGSTREAM,"[FATAL] incoherent length of left %i and right %i\n",blockA->length, blockB->length);
	    }
	  alsha2x_add_block(&intern,blockB);
	}
      aldatablock * result=alsha2x_final(&intern);

      if ( treenode->context == NULL )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] NULL context %p\n",treenode);
	  alhashtree_dump_treenode(NULL,treenode);
	}
      // data pointer is in alsha2_internal intern on stack, use this of context.
      char * newdata=al_copy_block(&treenode->context->ringbuffer,result);
      aldebug_printf(DBGSTREAM,"[DEBUG] %p -> newdata %p length  %i ringbuffer %p\n", result->data.ptr, newdata, result->length, &treenode->context->ringbuffer);

      //memcpy(&treenode->hash,result,sizeof(treenode->hash));
      treenode->hash.type=ALTYPE_OPAQUE;
      treenode->hash.length=result->length;
      treenode->hash.data.charptr=newdata;
    }
  else
    {
      aldebug_printf(DBGSTREAM, "[FATAL] alsha256hashfunc for a NULL treenode\n");
    }
  
}

void alhashtree_fatal()
{
  // should stop here
  aldebug_printf(DBGSTREAM,"[FATAL] exiting program on fatal error\n");
}

void alhashtree_set_left( struct alhashtreenode * treenode,
			   struct alhashtreenode * left)
{
  if ( left != NULL )
    {
      albtree_set_left(&treenode->btree,&left->btree);
      left->parent = treenode;
    }
  else
    {
      albtree_set_left(&treenode->btree,NULL);
    }
}

void alhashtree_set_right( struct alhashtreenode * treenode,
			   struct alhashtreenode * right)
{
  if ( right != NULL )
    {
      albtree_set_right(&treenode->btree,&right->btree);
      right->parent = treenode;
    }
  else
    {
      albtree_set_right(&treenode->btree,NULL);
    }
}


void alhashtree_specific_init(
			     struct alhashtreenode * treenode,
			     struct alallocation_ctx * context,
			     struct alhashtreenode * left,
			     struct alhashtreenode * right,
			     struct alhashtreenode * parent)
{

  struct albtree * btree = &treenode->btree;  
  if ( btree->allocate != (albtreeallocator) alhashtree_allocate )
    {
      aldebug_printf(DBGSTREAM,"[FATAL] specific init a node %p with wrong allocation method %p / default %p\n",btree, btree->allocate,alhashtree_allocate);
      alhashtree_fatal();
    }

  if (( left == NULL ) && (right == NULL))
    {
      treenode->nodetype=AL_TREELEAF;
    }
  else
    {
      treenode->nodetype=AL_TREENODE;
    }
  treenode->context=context;

  if ( treenode->func.hashmethod != NULL )
    {
      aldebug_printf(DBGSTREAM,"[WARNING] hashmethod already set %p\n", treenode->func.hashmethod);
    }

  if (parent == NULL)
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

// called once for initial root, after it is done through albtree allocation and reqiere a call to alhashtree_specific_init
// first call it is a leaf with an empty block.
void alhashtree_init(struct alhashtreenode * treenode, struct alallocation_ctx * context, struct albtree * left, struct albtree * right)
{
  struct albtree * btree = &treenode->btree;
  if ( btree->allocate != NULL )
    {
      aldebug_printf(DBGSTREAM,"[WARNING] reinit a btree %p that has an allocate method %p / default %p\n",btree, btree->allocate,alhashtree_allocate);
    }
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

// alter newroot
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
      aldebug_printf(DBGSTREAM,"recompute current node %p at depth %i\n", current, depth);
      alhashtree_recompute_direct_children(current);
      ++ depth;           
      parent=parent->parent;
    }
  aldebug_printf(DBGSTREAM,"recomputed %i parent nodes\n", depth);
  return depth;
}

// WARNING set *newroot with computed root from parent links and with new root if created
// assuming intree is already rightmost deeper leaf.
// will return a new rightmost element
struct alhashtreenode * alhashtree_create_sibling(struct alhashtreenode *intree, struct alhashtreenode ** newroot)
{
  struct albtree * previous_root = NULL;
  struct alhashtreenode * root = NULL;
  struct alhashtreenode * added = NULL;
  struct alhashtreenode * parent = NULL;

  parent = intree->parent;
  
      aldebug_printf(DBGSTREAM,"create a new right -child or parent- for %p\n", intree);

      {
	struct alhashtreenode * freeparent = parent;	
      	struct alhashtreenode * left = NULL;
	// one level deeper than left ( when not the same ).
	struct albtree * deeperleft = NULL;

	// create deepest
	added = alhashtree_allocate();
	alhashtree_init(added,intree->context, NULL, NULL);
	left=added;
	deeperleft = &left->btree;
	    
	// create left children from ground on top of previous deeperleft
	// should find a free entry at right in parent hierarchy
	while ( freeparent != NULL )
	  {
	    // freeparent found
	    if ( albtree_get_right(&freeparent->btree) == NULL )
	      {
		// attach left only tree on right of free parent.
		alhashtree_set_right(freeparent,left);

		(*newroot) = (struct alhashtreenode *) previous_root;
		// no root involved.
		return added;
	      }

	    left = alhashtree_allocate();
	    alhashtree_init(left,intree->context, deeperleft, NULL);
	    deeperleft = &left->btree;

	    previous_root=&freeparent->btree;
	    freeparent=freeparent->parent;
	  }

	// now we are at root level and left only tree is created behind left/deeperlef
	root = alhashtree_allocate();  
	alhashtree_init(root,intree->context, previous_root,deeperleft);
	
	(*newroot) = root;
      
	return added;

      }
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
	  // root = NULL; // out only, don't care its value
	  added = alhashtree_create_sibling(intree, &root);
	}
      
      if ( added == NULL )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] added treenode is NULL\n");
	}
      else
	{
	  aldebug_printf(DBGSTREAM,"to hash alhashtree context ringbuffer %p\n",&added->context->ringbuffer);
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
