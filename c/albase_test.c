#include "albase.h"

int main(int argc, char ** argv)
{
  struct alhash_datablock datablock;

  albase_set_debug(1);
  
  char buffer[4096];
  datablock.type=ALTYPE_OPAQUE;
  datablock.length=4096;
  datablock.data.ptr=buffer;

  for (int i = 0; i < 200; i ++)
    {
      int j = i*i*i +3;
      aljson_build_string_from_int(j,2,&datablock);
      aljson_build_string_from_int(j,10,&datablock);
      aljson_build_string_from_int(j,16,&datablock);
      aljson_build_string_from_int(-j,16,&datablock);
      aljson_build_string_from_int(-j,8,&datablock);
    }
}
