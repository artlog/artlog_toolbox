#include "alhastree.h"
#include <stddef.h>
#include <stdlib.h>

void alhashtree_data_process(void * data, void * contextdata, struct albtree * btree)
{
  // to improve
  struct alhashtreenode * treenode = (struct alhashtreenode *) btree;
  aldebug_printf(NULL,"dumping treenode %p\n hash :", treenode);
  aldatablock_dump(&treenode->hash);
  if ( treenode->btree.data != NULL )
    {
      /*
      struct alhash_datablock * block = (struct alhash_datablock *) treenode->btree.data;
      aldebug_printf(NULL,"\n data :");
      aldatablock_dump(block);
      */
      aldebug_printf(NULL," data : %s\n",(char *) data);
    }
}
  
int main(int argc, char ** argv)
{
  struct alallocation_ctx context;
  struct alhashtreenode * treenode;
  struct alhashtreenode * rightmost;
  struct alhashtreenode * root;
  struct alhash_datablock block;
  
  alstrings_ringbuffer_init_autogrow(&context.ringbuffer,10,256);
				     
  alhashtree_global_init_sha256(&context);

  treenode = alhashtree_create(&context);

  rightmost = treenode;

  for (int i = 1;(rightmost != NULL) && (i < argc); i++)
    {
      char * param = argv[i];
      aldatablock_setcstring(&block,param);
      rightmost = alhashtree_add_block(rightmost, &block);
      if (rightmost != NULL)
	{
	  alhashtree_set_data(rightmost,param);
	}
    }

  if ( rightmost == NULL )
    {
      aldebug_printf(NULL,"[FATAL] null treenode added\n");
      exit(1);
    }
  int depth=alhashtree_depth_to_root(rightmost, &root);

  aldebug_printf(NULL,"depth %i\n",depth);

  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_data_process, NULL, 10);
  alhashtree_clean(treenode);

  alstrings_ringbuffer_release(&context.ringbuffer);

  return 0;  
}
