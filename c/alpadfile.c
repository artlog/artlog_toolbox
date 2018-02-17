#include "albitfieldreader.h"
#include "albitfieldwriter.h"
#include "al_options.h"
#include "aldebug.h"

void usage()
{
  printf("add some character at end of file\n");
  printf("outfile: file to edit\n");
}

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);
  al_options_set_debug(options,0);
  usage();

  struct alinputstream input;
  struct aloutputstream output;
  struct alhash_datablock * opt1 = al_option_get(options,"outfile");
  if ( opt1 != NULL )
    {
      FILE * fout = fopen(opt1->data.charptr, "a");
      fwrite("104",3,1,fout);
      fflush(fout);
      fclose(fout);
    } 
  al_options_release(options);
}
  
