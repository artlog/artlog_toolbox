#include <stdio.h>
#include "save.h"

int main(int argc, char * argv[])
{
  if ( argc>3)
    {
      struct savecontext save_context;
      save_init_context(&save_context,argv[1],argv[2],argv[3]);
      int result=save_shift_file_name(&save_context);
      printf("result : %i\n",result);  
    }
  else
    {
      fprintf(stderr,"expecting 3 arguments directory prefix extension\n");
    }
}
