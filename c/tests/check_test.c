#include <stdio.h>
#include <sys/resource.h>
#include <time.h>

int checktest(FILE * out, char* test, int result, time_t * start)
{
  time_t end;
  time(&end);
  if ( result == 0 )
    {
      fprintf(out,"%s OK\n", test);
    }
  else
    {
      fprintf(out,"%s KO error %i \n", test, result);
    }
  fprintf(out,"test %s took %lu second\n", test,  (end - *start));
  return result;

}

void show_memory_usage()
{

  struct rusage memuse;
  
  getrusage(RUSAGE_SELF, &memuse);

  printf( "maximum resident set size %lu\n", memuse.ru_maxrss);
  printf( "integral shared memory size %lu\n",memuse.ru_ixrss);
  printf( "integral unshared data size %lu\n", memuse.ru_idrss);
  printf( "integral unshared stack size %lu\n", memuse.ru_isrss);
  
}

