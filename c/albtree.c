#include "albtree.h"
#include <stdlib.h>

/** allocate a binary tree */
struct albtree * albtree_allocate()
{
  return calloc(1,sizeof(struct albtree));
}

struct albtreeprocessinfo {
  int count;
};

void freealbtreeprocess(void * data, void * datacontext, struct albtree * btree)
{
  if ( btree != NULL )
    {
      if ( datacontext != NULL )
	{
	  struct albtreeprocessinfo * info = (struct albtreeprocessinfo *) datacontext;
	  ++ info->count;
	}
      btree->right = NULL;
      btree->left = NULL;
      free(btree);
    }
}

int albtree_freeall(struct albtree * btree)
{
  struct albtreeprocessinfo info;
  info.count=0;
  // well walk left right and S at the end.
  // TODO collect orphans due to depth... ( memory leak ).
  albtree_walk(btree, ALBTREE_WP_LRS, freealbtreeprocess, &info,500);
  return info.count;
}

/** init btree  */
struct albtree * albtree_init(struct albtree * btree, void * data, struct albtree * left, struct albtree * right)
{
  btree->data = data;
  btree->left=left;
  btree->right=right;
  return btree;
}

/** get left binary tree */
struct albtree * albtree_get_left(struct albtree * btree)
{
  return btree->left;
}

/** get right binary tree */
struct albtree * albtree_get_right(struct albtree * btree)
{
  return btree->right;
}

struct albtree * albtree_insert_left(struct albtree * btree, void * data)
{
  struct albtree * lefttree = albtree_allocate();
  // if there is already a left part then this is inserted before it.
  albtree_init(lefttree,data,btree->left,NULL);
  btree->left=lefttree;
  return lefttree;
}

// very same as albtree_insert_left, but with right ...
struct albtree * albtree_insert_right(struct albtree * btree, void * data)
{
  struct albtree * righttree = albtree_allocate();
  // if there is already a right  part then this is inserted before it.
  albtree_init(righttree,data,btree->right,NULL);
  btree->right=righttree;
  return righttree;
}

// recursive implementation
 void albtree_walk_recursive(struct albtree * btree, enum albtreewalkprocess walkprocess, void (* data_process) (void * data, void * contextdata, struct albtree * btree), void * contextdata, int depth)
{
  if ( depth > 0 )
    {
      if (btree != NULL)
	{
	  // first
	  switch(walkprocess)
	    {      
	    case ALBTREE_WP_SLR:
	    case ALBTREE_WP_SRL:
	      data_process(btree->data,contextdata,btree);
	      break;

	    case ALBTREE_WP_LSR:
	    case ALBTREE_WP_LRS:
	      albtree_walk_recursive(btree->left, walkprocess,data_process,contextdata,depth-1);
	      break;

	    case ALBTREE_WP_RSL:
	    case ALBTREE_WP_RLS:      
	      albtree_walk_recursive(btree->right, walkprocess,data_process,contextdata,depth-1);
	      break;
	    }
	  // second
	  switch(walkprocess)
	    {
	    case ALBTREE_WP_SLR:
	    case ALBTREE_WP_RLS:
	      albtree_walk_recursive(btree->left, walkprocess,data_process,contextdata,depth-1);
	      break;
      
	    case ALBTREE_WP_LSR:
	    case ALBTREE_WP_RSL:
	      data_process(btree->data,contextdata,btree);
	      break;
	
	    case ALBTREE_WP_LRS:
	    case ALBTREE_WP_SRL:
	      albtree_walk_recursive(btree->right, walkprocess,data_process,contextdata, depth-1);
	      break;
	    }
      
	  // third
	  switch(walkprocess)
	    {
	    case ALBTREE_WP_SLR:
	    case ALBTREE_WP_LSR:
	      albtree_walk_recursive(btree->right, walkprocess,data_process,contextdata,depth-1);
	      break;

	    case ALBTREE_WP_RSL:
	    case ALBTREE_WP_SRL:
	      albtree_walk_recursive(btree->left, walkprocess,data_process,contextdata,depth-1);
	      break;

	    case ALBTREE_WP_RLS:
	    case ALBTREE_WP_LRS:
	      data_process(btree->data,contextdata,btree);
	      break;
	    }
	}
    }
}


 void albtree_walk(struct albtree * btree, enum albtreewalkprocess walkprocess, void (* data_process) (void * data, void * contextdata, struct albtree * btree), void * contextdata, int depth)
{
  albtree_walk_recursive(btree, walkprocess,data_process,contextdata, depth);
}
