#include "albtree.h"
#include <stdlib.h>
#include <string.h>

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
      (*btree->clean)( (void *) btree);
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
  btree->data=data;
  btree->left=left;
  btree->right=right;
  btree->allocate=(albtreeallocator) albtree_allocate;
  btree->clean=(albtrecleaner) free;
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

/** set left binary tree */
void albtree_set_left(struct albtree * btree,struct albtree * left)
{
  btree->left=left;
}

/** set right binary tree */
void albtree_set_right(struct albtree * btree, struct albtree * right)
{
  btree->right=right;
}

struct albtree * albtree_insert_left(struct albtree * btree, void * data)
{
  struct albtree * lefttree = (*btree->allocate)();
  // if there is already a left part then this is inserted before it.
  albtree_init(lefttree,data,btree->left,NULL);
  btree->left=lefttree;
  return lefttree;
}

// very same as albtree_insert_left, but with right ...
struct albtree * albtree_insert_right(struct albtree * btree, void * data)
{
  struct albtree * righttree = (*btree->allocate)();
  // if there is already a right  part then this is inserted before it.
  albtree_init(righttree,data,NULL,btree->right);
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

// to complete, need for more inputs.
void searchprocess(void * data, void * datacontext, struct albtree * btree)
{
  if ( btree != NULL )
    {
      if ( datacontext != NULL )
	{
	  struct  albtreepath * path = (struct albtreepath *) datacontext;
	  if ( btree == path->found )
	    {
	    }
	  else if ( data == path->data )
	    {
	      if ( path->found != NULL )
		{
		  path->found = btree;
		}
	    }
	}
    }
}

struct albtreepath * albtree_get_path(struct albtree * btree, struct albtree * child, void * data)
{
  struct albtreepath path;

  path.found=child;
  path.data=data;
  path.depth=0;
  path.index=0;
  path.walkprocess = ALBTREE_WP_SLR;
  
  albtree_walk(btree,path.walkprocess,searchprocess,&path,64);
  struct albtreepath * result = malloc(sizeof(*result));
  memcpy(&path,result,sizeof(path));
  return result;
}

// warning stack overflow risk ( no depth limit given )
struct albtree * albtree_insert_recursive( struct albtree * btree, alcomparator_t comparator, void * data, albtreeallocator allocate )
{
  struct albtree * result;
  result = btree;

  if ( btree == NULL )
    {
      result = (*allocate)();
      result = albtree_init(result,data,NULL,NULL);
    }
  else
    {
      int diff = comparator(data,btree->data);
      if ( diff == 0 )
	{
	  /// oups was already there ?
	  return result;
	}
      if ( diff < 0 )
	{
	  result->left=albtree_insert_recursive(btree->left,comparator,data,btree->allocate);
	}
      else
	{
	  // insert right
	  result->right=albtree_insert_recursive(btree->right,comparator,data,btree->allocate);
	}
    }
  return result;
}

struct albtree * albtree_insert( struct albtree * btree, alcomparator_t comparator, void * data)
{
  return albtree_insert_recursive(btree,comparator,data,btree->allocate);
}
