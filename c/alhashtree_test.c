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

struct aloutputstream * dotoutput;

void  alhashtree_to_dot(struct aloutputstream * output, struct alhashtreenode * treenode)
{
  struct albtree * btreenode = &treenode->btree;
  if ( btreenode->left != NULL )
    {
      if ( btreenode->right != NULL )
	{
	  aloutputstream_printf_1k(output,
				   "node%p -> node%p [color=blue];\n"
				   "node%p -> node%p [color=red];\n",
				   btreenode,
				   btreenode->left,
				   btreenode,
				   btreenode->right);
	}
      else
	{
	  aloutputstream_printf_1k(output,
				   "node%p -> node%p [color=blue];\n",
				   btreenode,
				   btreenode->left);
	}
    }
  else
    {
      if ( btreenode->right != NULL )
	{
	  aloutputstream_printf_1k(output,
				   "node%p -> node%p [color=red];\n",
				   btreenode,
				   btreenode->right);
	}
      else
	{
	  if ( btreenode->data != NULL )
	    {
	      aloutputstream_printf_1k(output,"node%p [label=\"%s\",color=blue];\n",btreenode,(char *) btreenode->data);
	    }
	  else
	    {
	      aloutputstream_printf_1k(output,"node%p [color=red];\n",btreenode);
	    }

	}

    }

  
}


void alhashtree_data_process(void * data, void * contextdata, struct albtree * btree)
{
  // to improve
  struct alhashtreenode * treenode = (struct alhashtreenode *) btree;
  alhashtree_dump_treenode(NULL,treenode);
  if ( dotoutput != NULL )
    {
      alhashtree_to_dot(dotoutput,treenode);
    }
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

  struct aloutputstream dotout;
  aloutput_file_open_init(&dotout,"out.dot");
  dotoutput = &dotout;

  treenode = alhashtree_create(&context);

  rightmost = treenode;

  for (int i = 1;(rightmost != NULL) && (i < argc); i++)
    {
      char * param = argv[i];
      aldatablock_setcstring(&block,param);
      aldebug_printf(NULL,"rightmost %p context %p ringbuffer %p\n",rightmost, rightmost->context, rightmost->context->ringbuffer);
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

  if ( dotoutput != NULL )
    {
      aloutputstream_printf_1k(dotoutput,"digraph root%p {\n", &root->btree);
    }

  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_data_process, NULL, 10);
  alhashtree_clean(treenode);

  alstrings_ringbuffer_release(&context.ringbuffer);

  
  if ( dotoutput != NULL )
    {
      aloutputstream_printf_1k(dotoutput,"}\n", &root->btree);
    }

  return 0;  
}
