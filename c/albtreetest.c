#include "albtree.h"
#include <stdio.h>
#include <string.h>

void dummy_walk(void * data, void * datacontext, struct albtree * btree, struct albtree * parent)
{
}

void walk_enter(void * data, void * datacontext, struct albtree * btree, struct albtree * parent)
{
  if (data != NULL)
    {
      printf("%p:",btree);
      printf("'%s' ",(char*) data);
      printf("%p %p\n",btree->left,btree->right);
    }
  else
    {
      printf("NULL data\n");
    }
	
}

void basic_test()
{
  struct albtree * root = albtree_allocate();
  struct albtree * leaf = NULL;
  
  albtree_init(root, "root", NULL, NULL);

  leaf = albtree_insert_left(root,"a");
  leaf = albtree_insert_left(leaf,"b");
  leaf = albtree_insert_left(leaf,"c");
  leaf = albtree_insert_left(leaf,"d");

  leaf = albtree_insert_right(root,"A");
  leaf = albtree_insert_right(leaf,"B");
  leaf = albtree_insert_right(leaf,"C");
  leaf = albtree_insert_right(leaf,"D");
  
  printf("walk SLR\n");
  albtree_walk(root, ALBTREE_WP_SLR, walk_enter, dummy_walk,NULL,10);
  printf("walk LSR\n");
  albtree_walk(root, ALBTREE_WP_LSR, walk_enter, dummy_walk,NULL,10);
  printf("walk RSL\n");
  albtree_walk(root, ALBTREE_WP_RSL, walk_enter, dummy_walk,NULL,10);
  printf("walk LRS\n");
  albtree_walk(root, ALBTREE_WP_LRS, walk_enter, dummy_walk,NULL,10);

  if ( root != NULL )
    {
      int count = albtree_freeall(root);
      printf("%i freed\n",count);
      root=NULL;
    }

}

int string_comparator(void * left, void * right)
{
  return strcmp( (const char *) left, (const char *) right );
}

int test_insert_sorted(int argc, char ** argv)
{
  struct albtree * root = albtree_allocate();
  struct albtree * leaf = NULL;

  printf("=== sorted test ===\n");
  
  leaf=albtree_init(root, argv[0], NULL, NULL);

  for (int i=1;i<argc;i++)
    {
      leaf = albtree_insert(root,string_comparator,argv[i]);
      if ( leaf == NULL )
	{
	  printf("[FATAL] insert sorted NULL");
	  break;
	}
	 
    }
  albtree_walk(root, ALBTREE_WP_LSR, walk_enter, dummy_walk, NULL,argc);
  int count = albtree_freeall(root);
  if ( count != argc )
    {
      printf("[ERROR] %i freed != %i allocated ", count,argc);
    }
  return count;
}

int main(int argc, char ** argv)
{
  basic_test();

  {
    struct albtree * root = albtree_allocate();
    struct albtree * leaf = NULL;
  
    leaf=albtree_init(root, argv[0], NULL, NULL);

    for (int i=1;i<argc;i++)
      {
	leaf = albtree_insert_left(leaf,argv[i]);      
      }
    albtree_walk(root, ALBTREE_WP_SLR, walk_enter, dummy_walk,NULL,argc);
    int count = albtree_freeall(root);
    if ( count != argc )
      {
	printf("[ERROR] %i freed != %i allocated ", count,argc);
      }
  }

  test_insert_sorted(argc,argv);
  
}
