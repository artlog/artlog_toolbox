#include "alabnf_matcher.h"
#include "alabnf_util.h"
#include "aldebug.h"
#include <stdlib.h>

int main(int argc, char * argv[])
{

  struct alabnf * alabnf = NULL;
  struct alabnf_matcher matcher;
  struct alinputstream input;
  int offset = 1;

  FILE * file = stdin;

  if ( argc > offset )
    {

      int maxsteps=(int) strtol(argv[1],NULL,10);

      aldebug_printf(NULL,"[INFO] max steps set to %i\n", maxsteps);
 
      offset=2;
      alabnf = alabnf_util_parse_abnf_filenames(argc-offset,offset,argv);

      if ( alabnf != NULL )
	{
	  aldebug_printf(NULL,"[INFO] parsed abnf %p\n", alabnf);
	  
	  alinputstream_init(&input,fileno(file));

	  alabnf_match_init(&matcher,alabnf,&input);

	  matcher.maxsteps=maxsteps;
	  
	  alabnf_match(&matcher);

	  // TODO release alabnf
	}
      
    }
  else
    {
      aldebug_printf(NULL,"[ERROR] expected abnf syntax file as first argument\n");
    }
}
