#include "alabnf_matcher.h"
#include "alabnf_util.h"
#include "aldebug.h"

int main(int argc, char * argv[])
{

  struct alabnf * alabnf = NULL;
  struct alabnf_matcher matcher;
  struct alinputstream input;

  char * abnf_filename = NULL;
  FILE * abnf_file = NULL;
  FILE * file = stdin;

  if ( argc > 1 )
    {
      abnf_filename = argv[1];
      abnf_file = fopen(abnf_filename,"r");

      if ( abnf_file != NULL )
	{
	  alabnf = alabnf_util_parse_abnf_file(abnf_file);

	  aldebug_printf(NULL,"[INFO] parsed abnf %p\n", alabnf);
	  
	  alinputstream_init(&input,fileno(file));
	  
	  alabnf_match_init(&matcher,alabnf,&input);

	  alabnf_match(&matcher);

	  // TODO release alabnf
	  
	  fclose(abnf_file);
	}
      
    }
  else
    {
      aldebug_printf(NULL,"[ERROR] expected abnf syntax file as first argument\n");
    }
}
