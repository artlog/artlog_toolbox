#include "alabnf_matcher.h"
#include "alabnf_util.h"

int main(int argc, char * argv[])
{

  struct alabnf alabnf;
  struct alabnf_matcher matcher;
  struct alinputstream input;

  alabnf_match_init(&matcher,&alabnf,&input);

  alabnf_util_parse_abnf_file(stdin,&alabnf);
}
