#include "alinput.h"
#include <stdlib.h>
#include <stdio.h>

void readsome(struct alinputstream * stream, int some)
{
  for (int i =0 ; i < some;i++)
    {
      unsigned char uchar = alinputstream_shared_readuchar(stream);
      printf("'%c' %i,",uchar,uchar);
    }
  printf("\n");
}

int main(int argc, char * argv[])
{
  struct alinputstream input;

  printf("TEST shared child, should display twice ten first char you typed\n");
  FILE * file = stdin;
  alinputstream_init(&input,fileno(file));

  struct alinputstream * child;

  child = alinputstream_create_mark_shared(&input,100);
  printf("shared child 10 chars \n");
  readsome(child,10);

  printf("replay parent 20 characters \n");
  readsome(&input,20);

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
  readsome(&input,20);

  printf("free child2\n");
  alinputstream_free_shared(child2);

  // test buffer was really release
  printf("replay parent 150 characters \n");
  readsome(&input,150);

}
