#include <stdio.h>
#include "alsave.h"

int main(int argc, char * argv[])
{
  if ( argc>3)
    {
      struct alsavecontext save_context;
      alsave_set_debug(1);
      alsave_init_context(&save_context,argv[1],argv[2],argv[3]);
      int result=alsave_shift_file_name(&save_context);
      printf("result : %i\n",result);  
    }
  else
    {
      fprintf(stderr,"expecting 3 arguments directory prefix extension\n");
    }
}
