#include "alhashtree.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "aldebug_output.h"
#include "aloutput_file.h"

void usage()
{
  printf("Don't use, even at your own risks\n\
\
add each argument at new rightmost element of btree with hash256 value then dump it.\n");
}

/*
to make generic for dot output

void  alhashtree_to_dot(struct aloutputstream * output, struct alhashtreenode * treenode);  
void alhashtree_snapshot_to_dot(struct alhashtree_snapshot * snapshot,struct alhashtreenode * root);
*/

void  alhashtree_to_dot(struct aloutputstream * output, struct alhashtreenode * treenode)
{
  struct albtree * btreenode = &treenode->btree;

  aloutputstream_printf_1k(output,"node%p [label=<",btreenode);
  if ( btreenode->data != NULL )
    {
      aloutputstream_printf_1k(output,"<FONT POINT-SIZE=\"20\">%s</FONT><BR/>",(char *) btreenode->data);
    }
  
  if ( treenode->hash.length > 0 )
    {
      aloutputstream_printf_1k(output,"<FONT POINT-SIZE=\"16\">hash=");
      aloutput_bytes_as_hex(output, &treenode->hash, 0, 8);
      aloutputstream_printf_1k(output,"</FONT>",btreenode,btreenode);
    }

  aloutputstream_printf_1k(output,">]",btreenode,btreenode);

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
    }
  
}

void alhashtree_snapshot_to_dot(struct alhashtree_snapshot * snapshot,struct alhashtreenode * root)
{
  struct aloutputstream * dotoutput = &snapshot->output;
  int snapid = snapshot->id;
  snapshot->output_node_func=alhashtree_to_dot;
  
  aloutputstream_printf_1k(dotoutput,"digraph root%p_%i {\n", &root->btree,snapid);
  
  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_snapshot_process, snapshot, 10);
  aloutputstream_printf_1k(dotoutput,"}\n", &root->btree);

  snapshot->id=snapid+1;
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
  aldatablock block;

  if ( argc == 1 )
    {
      usage();
      exit(0);
    }

  aldebug_start(NULL);
  
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

  // snapshot all not needed, only for debugging to generate each step addition
  int snapshot_all=0;
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

	  // take a snapshot at each add
	  if (snapshot_all)
	    {
	      depth=alhashtree_depth_to_root(rightmost, &root);
	      alhashtree_snapshot_to_dot(&snapshot,root);
	    }

	}
      
    }

  if ( rightmost == NULL )
    {      
      aldebug_printf(DBGSTREAM,"[FATAL] null treenode added\n");
      exit(1);
    }
  else
    {
      if (!snapshot_all)  {
	// find and reset root
	depth=alhashtree_depth_to_root(rightmost, &root);
	// dump tree from root
	alhashtree_snapshot_to_dot(&snapshot,root);
      }
    }

  // find and reset root
  depth=alhashtree_depth_to_root(rightmost, &root);
  aldebug_printf(DBGSTREAM,"depth %i\n",depth);
  // self left right
  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_data_process, NULL, 10);

  // save snapshot
  alhashtree_snapshot_close(&snapshot);

  // cleanup
  alhashtree_clean(treenode);

  alstrings_ringbuffer_release(&context.ringbuffer);

  aldebug_end();
  
  return 0;  
}
