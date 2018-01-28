#include "albtree.h"
#include <stdio.h>
#include <string.h>

void process_element(void * data, void * datacontext, struct albtree * btree)
{
  if (data != NULL)
    {
      printf("%s\n",(char*) data);
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

  
  albtree_walk(root, ALBTREE_WP_SLR, process_element, NULL,10);
  albtree_walk(root, ALBTREE_WP_LSR, process_element, NULL,10);
  albtree_walk(root, ALBTREE_WP_RSL, process_element, NULL,10);
  albtree_walk(root, ALBTREE_WP_LRS, process_element, NULL,10);

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
  albtree_walk(root, ALBTREE_WP_LSR, process_element, NULL,argc);
  int count = albtree_freeall(root);
  if ( count != argc )
    {
      printf("[ERROR] %i freed != %i allocated ", count,argc);
    }
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
    albtree_walk(root, ALBTREE_WP_SLR, process_element, NULL,argc);
    int count = albtree_freeall(root);
    if ( count != argc )
      {
	printf("[ERROR] %i freed != %i allocated ", count,argc);
      }
  }

  test_insert_sorted(argc,argv);
  
}
