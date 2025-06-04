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

void  alhashtree_to_dot_exit(void * data,
			 void * contextdata,
			 struct albtree * btree,
			 struct albtree * parent)
{
}

void  alhashtree_to_dot(void * data,
			 void * contextdata,
			 struct albtree * btree,
			 struct albtree * parent)
{
  struct alhashtree_snapshot * snapshot = (struct alhashtree_snapshot *) contextdata;
  struct aloutputstream * output = &snapshot->output;
  struct alhashtreenode * treenode = (struct alhashtreenode *) btree;

  aloutputstream_printf_1k(output,"node%p [label=<",btree);
  if ( btree->data != NULL )
    {
      aloutputstream_printf_1k(output,"<FONT POINT-SIZE=\"20\">%s</FONT><BR/>",(char *) btree->data);
    }
  
  if ( treenode->hash.length > 0 )
    {
      aloutputstream_printf_1k(output,"<FONT POINT-SIZE=\"16\">hash=");
      aloutput_bytes_as_hex(output, &treenode->hash, 0, 8);
      aloutputstream_printf_1k(output,"</FONT>");
    }

  aloutputstream_printf_1k(output,">]",btree,btree);

  if ( btree->left != NULL )
    {
      if ( btree->right != NULL )
	{
	  aloutputstream_printf_1k(output,
				   "node%p -> node%p [color=blue];\n"
				   "node%p -> node%p [color=red];\n",
				   btree,
				   btree->left,
				   btree,
				   btree->right);
	}
      else
	{
	  aloutputstream_printf_1k(output,
				   "node%p -> node%p [color=blue];\n",
				   btree,
				   btree->left);
	}
    }
  else
    {
      if ( btree->right != NULL )
	{
	  aloutputstream_printf_1k(output,
				   "node%p -> node%p [color=red];\n",
				   btree,
				   btree->right);
	}
    }
  
}

void alhashtree_snapshot_to_dot(struct alhashtree_snapshot * snapshot,struct alhashtreenode * root)
{
  struct aloutputstream * dotoutput = &snapshot->output;
  int snapid = snapshot->id;
  
  aloutputstream_printf_1k(dotoutput,"digraph root%p_%i {\n", &root->btree,snapid);
  
  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_to_dot, alhashtree_to_dot_exit, snapshot, 10);
  aloutputstream_printf_1k(dotoutput,"}\n", &root->btree);

  snapshot->id=snapid+1;
}

void  alhashtree_to_json_exit(void * data,
			 void * contextdata,
			 struct albtree * btree,
			 struct albtree * parent)
{
  struct alhashtree_snapshot * snapshot = (struct alhashtree_snapshot *) contextdata;
  struct aloutputstream * output = &snapshot->output;

  aloutputstream_printf_1k(output,"}\n");
}

void  alhashtree_to_json(void * data,
			 void * contextdata,
			 struct albtree * btree,
			 struct albtree * parent)
{
  struct alhashtree_snapshot * snapshot = (struct alhashtree_snapshot *) contextdata;
  struct aloutputstream * output = &snapshot->output;
  struct alhashtreenode * treenode = (struct alhashtreenode *) btree;
 
  if ( parent ) {
    if (parent->left == btree) {
      aloutputstream_printf_1k(output,"\n\"left\":");
    }
    else if (parent->right == btree) {
      aloutputstream_printf_1k(output,"\n,\"right\":");
    }
    else {
      // when does this happens ?
      aloutputstream_printf_1k(output,"<UNEXPECTED>");
    }
  }

  aloutputstream_printf_1k(output,"{");
  
  if ( btree->data != NULL )
    {
      aloutputstream_printf_1k(output,"\"data\":\"%s\"",(char *) btree->data);
      
    }
  
  if ( treenode->hash.length > 0 )
    {
      if ( btree->data != NULL )
	{
	  aloutputstream_printf_1k(output,",");
	}
      aloutputstream_printf_1k(output,"\"hash\":\"");
      aloutput_bytes_as_hex(output, &treenode->hash, 0, 8);
      aloutputstream_printf_1k(output,"\"");
    }
  if ( btree->left != NULL )
    {
      aloutputstream_printf_1k(output,",");
    }

}

void alhashtree_snapshot_to_json(struct alhashtree_snapshot * snapshot,struct alhashtreenode * root)
{
  struct aloutputstream * output = &snapshot->output;
  int snapid = snapshot->id;

  aloutputstream_printf_1k(output,"[\n");
  
  albtree_walk(&root->btree, ALBTREE_WP_SLR,  alhashtree_to_json,alhashtree_to_json_exit, snapshot, 10);

  aloutputstream_printf_1k(output,"]\n");
  snapshot->id=snapid+1;
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

  // save snapshot
  alhashtree_snapshot_close(&snapshot);

  // another snapshot in json
  alhashtree_snapshot_init(&snapshot,"out.json");

  // find and reset root
  depth=alhashtree_depth_to_root(rightmost, &root);
  aldebug_printf(DBGSTREAM,"depth %i\n",depth);
  alhashtree_snapshot_to_json(&snapshot,root);

  // save snapshot
  alhashtree_snapshot_close(&snapshot);

  // cleanup
  alhashtree_clean(treenode);
 
  alstrings_ringbuffer_release(&context.ringbuffer);

  aldebug_end();
  
  return 0;  
}
