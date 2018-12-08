#include "alinput.h"
#include <stdlib.h>
#include <stdio.h>

void readsome(struct alinputstream * stream, int some)
{
  for (int i =0 ; i < some;i++)
    {
      unsigned char uchar = alinputstream_shared_readuchar(stream);
      // lazzy way
      if ( uchar == 0 )
	{
	  break;
	}
      printf("'%c' %i,",uchar,uchar);
    }
  printf("\n");
}

void readall(struct alinputstream * stream)
{
  unsigned char uchar = 0;
  do
    {
      uchar = alinputstream_shared_readuchar(stream);
      // lazzy way
      if ( uchar == 0 )
	{
	  break;
	}
      printf("%c",uchar);
    }
  while( uchar != 0);
}

// TODO provide a method to dispose all after use ... 
struct alinputstream * alinput_util_build_chain_stream_from_filenames(int filenames, int offset,char ** filename)
{
  struct alinputstream * container_inputstream = NULL;
  struct alinputstream * inputstream = NULL;
  FILE * file;

  for (int i=0; i <filenames; i++)
    {
      char * fname = filename[i+offset];
      file = fopen(fname,"r");
      if ( file != NULL )
	{
	  aldebug_printf(NULL,"[DEBUG] adding '%s' as input\n", fname);
	  inputstream = malloc(sizeof(*inputstream));
	  alinputstream_init(inputstream, fileno (file));
      
	  container_inputstream=alinputstream_create_chain(container_inputstream, inputstream);
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] adding '%s' fopen failed \n", fname);
	}
    }

  return container_inputstream;

}


int main(int argc, char * argv[])
{
  struct alinputstream input;
  
  int offset = 1;

  struct alinputstream * mainstream =  alinput_util_build_chain_stream_from_filenames(argc-offset, offset,argv);

    if ( mainstream == NULL )
      {
	printf("TEST shared child, should display twice ten first char you typed\n");
	mainstream = &input;
	  
	FILE * file = stdin;
	alinputstream_init(mainstream,fileno(file));

	struct alinputstream * child;

	child = alinputstream_create_mark_shared(mainstream,100);
	printf("shared child 10 chars \n");
	readsome(child,10);

	printf("replay parent 20 characters \n");
	readsome(mainstream,20);

	printf("shared child 20 chars \n");
	readsome(child,20);

	struct alinputstream * child2;

	child2 = alinputstream_create_mark_shared(child,100);
	printf("shared child2 10 chars \n");
	readsome(child2,10);

	printf("shared child 2 chars \n");
	readsome(child,2);

	printf("align shared child on child2 \n");
	alinputstream_align_shared_with_child(child,child2);

	printf("shared child 20 chars \n");
	readsome(child,20);
  
	printf("free child\n");
	alinputstream_free_shared(child);

	printf("replay parent 20 characters \n");
	readsome(mainstream,20);

	printf("free child2\n");
	alinputstream_free_shared(child2);

	// test buffer was really release
	printf("replay parent 150 characters \n");
	readsome(mainstream,150);
      }
    else
      {
	// dump it
	readall(mainstream);
      }

}
