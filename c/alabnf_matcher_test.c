#include "alabnf_matcher.h"
#include "alabnf_util.h"
#include "aldebug.h"
#include <stdlib.h>
#include "aldebug_output.h"
#include "alabnf_dump.h"

int main(int argc, char * argv[])
{

  struct alabnf * alabnf = NULL;
  struct alabnf_matcher matcher;
  struct alinputstream input;
  int offset = 1;

  FILE * file = stdin;

  aldebug_start(NULL);
  
  if ( argc > offset )
    {

      int maxsteps=(int) strtol(argv[1],NULL,10);

      aldebug_printf(DBGSTREAM,"[INFO] max steps set to %i\n", maxsteps);
 
      offset=2;
      alabnf = alabnf_util_parse_abnf_filenames(argc-offset,offset,argv);

      if ( alabnf != NULL )
	{
	  // dump it
	  {
	    struct aloutputstream output;
	    aloutputstream_fd_init(&output, fileno(stdout));
	    alabnf_dump_rule(&output,&alabnf->root_rule);
	  }
	  
	  aldebug_printf(DBGSTREAM,"[INFO] parsed abnf %p\n", alabnf);
	  
	  alinputstream_init(&input,fileno(file));

	  alabnf_match_init(&matcher,alabnf,&input);

	  matcher.maxsteps=maxsteps;
	  
	  alabnf_match(&matcher);

	  // TODO release alabnf
	}
      
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[ERROR] expected abnf syntax file as first argument\n");
    }

  aldebug_end(NULL);
}
