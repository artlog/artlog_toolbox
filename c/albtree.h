#ifndef __ALBTREE_H__
#define __ALBTREE_H__

/** binary tree

    (self)
    /   \
  LEFT  RIGHT
   / \  / \
...
*/

struct albtree {
  void * data;
  struct albtree * left;
  struct albtree * right;  
};

/** way to walk a btree uniformely */
// first : all permutations of left self right, WP == walkprocess
enum albtreewalkprocess
{
  ALBTREE_WP_LSR, // left self right
  ALBTREE_WP_RSL, // right self left

  ALBTREE_WP_SLR, // self left right
  ALBTREE_WP_SRL, // self right left

  ALBTREE_WP_LRS, // left right self
  ALBTREE_WP_RLS, // right left self
};

/** allocate a binray tree */
struct albtree * albtree_allocate();

int albtree_freeall(struct albtree * btree);

/** init btree  */
struct albtree * albtree_init(struct albtree * btree, void * data, struct albtree * left, struct albtree * right);

/** get left binary tree */
struct albtree * albtree_get_left(struct albtree * btree);

/** get right binary tree */
struct albtree * albtree_get_right(struct albtree * btree);

struct albtreepath {
  // depth.
  int depth;
  // bit to 0 means left else right.
  int path;
  // index when using a walkprocess;
  int index;
  enum albtreewalkprocess walkprocess;
  // to find or found
  struct albtree * found;
  // data to find
  void * data;
};
  
// this requires to walk full tree
/** will find child or data within child by walking whole tree */
struct albtreepath * albtree_get_path(struct albtree * btree, struct albtree * child, void * data);

/** return tree constructed with data and set to left of btree */
struct albtree * albtree_insert_left(struct albtree * btree, void * data);

/** return tree constructed with data and set to right of btree */
struct albtree * albtree_insert_right(struct albtree * btree, void * data);

/** walk btree using wlaklingprocess ordering given and callback for each data data_process
    for a maximum depth limit of depth ( MANDATORY ) */
void albtree_walk(struct albtree * btree, enum albtreewalkprocess walkprocess, void (* data_process) (void * data, void * contextdata, struct albtree * btree), void * contextdata, int depth);


/** comparator return < 0 if value(left)<value(right) ; 0 if value(left)== value(right) and > 0 else */
typedef int (*alcomparator_t) (void * left, void * right);

/** insert data into btree using comparator */
struct albtree * albtree_insert( struct albtree * btree, alcomparator_t comparator, void * data);

#endif // #ifndef __ALBTREE_H__
