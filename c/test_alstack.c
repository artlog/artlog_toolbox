#include "alstack.h"
#include <stdlib.h>
#include <stdio.h>

int printelement( struct alstackelement* elt)
{
  if ( elt != NULL)
    {
      printf("%s\n",(char *) elt->reference);
    }
  return 1;
}

int main(int argc, char **argv)
{
  struct alstack * stack = alstack_allocate();

  for (int i=0; i <argc; i++)
    {
      alstack_push_ref(stack,(void *) argv[i]);
    }

  struct alstackelement* elt = alstack_pop(stack);
  printelement(elt);

  
  alstack_destroy(stack, printelement);
}
