#include "alhashtree.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "aldebug_output.h"
#include "aloutput_file.h"

void usage()
{
  printf("Don't use, even at your own risks\n");
  printf("");
  printf("add each argument at new rightmost element of btree with hash256 value then dump it.\n"); 
}

void alhashtree_data_process(void * data, void * contextdata, struct albtree * btree)
{
  // to improve
  struct alhashtreenode * treenode = (struct alhashtreenode *) btree;
  alhashtree_dump_treenode(NULL,treenode);
}
  
int main(int argc, char ** argv)
{
  struct alallocation_ctx context;
  struct alhashtreenode * treenode;
  struct alhashtreenode * rightmost;
  struct alhashtreenode * root;
  struct alhash_datablock block;

  if ( argc == 1 )
    {
      usage();
      exit(0);
    }

  bzero(&context,sizeof(context));
  // alstrings_ringbuffer_init_autogrow(&context.ringbuffer,20,256);
  alstrings_ringbuffer_init_autogrow(&context.ringbuffer,20,1024);

  struct aloutputstream out;
  aloutput_file_open_init(&out,"out.dbg");
  alhashtree_global_init(&out,&context);

  struct alhashtree_snapshot snapshot;
  alhashtree_snapshot_init(&snapshot,"out.dot");

  treenode = alhashtree_create(&context);

  rightmost = treenode;

  int depth = 0;
  
  for (int i = 1;(rightmost != NULL) && (i < argc); i++)
    {
      char * param = argv[i];
      aldatablock_setcstring(&block,param);
      aldebug_printf(DBGSTREAM,"rightmost %p context %p ringbuffer %p\n",rightmost, rightmost->context, rightmost->context->ringbuffer);
      rightmost = alhashtree_add_block(rightmost, &block);
      if (rightmost != NULL)
	{
	  alhashtree_set_data(rightmost,param);
	}

      // take a snapshot at each add
      {
	depth=alhashtree_depth_to_root(rightmost, &root);
	alhashtree_snapshot_to_dot(&snapshot,root);
      }
    }

  if ( rightmost == NULL )
    {
      aldebug_printf(DBGSTREAM,"[FATAL] null treenode added\n");
      exit(1);
    }
  
  depth=alhashtree_depth_to_root(rightmost, &root);

  aldebug_printf(DBGSTREAM,"depth %i\n",depth);

  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_data_process, NULL, 10);

  alhashtree_snapshot_to_dot(&snapshot,root);

  // save snapshot
  alhashtree_snapshot_close(&snapshot);

  // cleanup
  alhashtree_clean(treenode);

  alstrings_ringbuffer_release(&context.ringbuffer);
  
  return 0;  
}
