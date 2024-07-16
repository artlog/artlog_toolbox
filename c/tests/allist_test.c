/**
 This is allist contract 
 drive tests on allist structure
 plays with primes, implementation limited to long long primes.

 run it with -help option to get usage.
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "check_test.h"
#include "../allist.h"
// direct acces to internal implementation for indexset_getrelindex
#include "../allist_internal.h"
// dump
#include "../aldump.h"

#define PRIMCOUNT 25

// preallcoated primes
int prims[PRIMCOUNT] = { 2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97};

struct _exec_params {
  // set debugging
  int test_debug;
  // shrink result
  int shrinkit;
  int decomp;
};

struct _prime_context {
  struct _exec_params params;
  // global prime list to be built
  struct allistof * primelp;
  // upper bound of integer set in which we want to extract primes.
  int glob_numbercount;
  struct allistcontext * context;
  // decomposition of primes, last table [ will be over-allocated in create_prime_context(...)]
  struct allistelement * elementp[PRIMCOUNT];
};

// will be created by create_prime_context
struct _prime_context * prime_context = NULL;

/**
  numbercount is the count of numbers
**/
struct _prime_context * create_prime_context( int numbercount )
{
  struct _prime_context * prime_contextp = calloc( 1, sizeof(struct _prime_context) + ( ( numbercount > PRIMCOUNT ) ? ( numbercount - PRIMCOUNT ) * sizeof(struct listelement *) : 0) );
  // calloc set everything to 0.
  if ( prime_contextp != NULL )
    {
      prime_contextp->glob_numbercount = (numbercount > PRIMCOUNT) ? numbercount : PRIMCOUNT;
    }
  else
    {
      fprintf(stderr,"[ERROR] allocation failure within %s:%i",__FILE__,__LINE__);
      exit(1);
    }    
  return prime_contextp;  
}

void set_prime_context_params( struct _prime_context * prime_contextp, struct _exec_params * params )
{
  memcpy(&prime_contextp->params, params, sizeof(prime_contextp->params));
}

/**
return 0 if all tests of indexset are ok
else return a negative number related to error
**/
int test_indexset()
{
  struct indexset set;
  int max = INDEXSET_COUNT;
  
  set.set = 0L;
  
  // set it and check that default is empty.
  for (int i=0; i < max; i++)
    {
      if ( indexset_set(&set, i) != 1 )
	{
	  return -(i+1);
	}
    }

  for (int i=0; i < max; i++)
    {
      if ( indexset_get(&set, i) != 1 )
	{
	  return -(i + 1 + max);
	}
    }

  for (int i=0; i < max; i++)
    {
      if ( indexset_reset(&set, i) != 1 )
	{
	  return -( i + 1 + 2* max);
	}
    }
  
  for (int i=0; i < max; i++)
    {
      if ( indexset_get(&set, i) != 0 )
	{
	  return -( 3* max + i + 1);
	}
    }
  
  
  for (int i=0; i < max; i+=2)
    {      
      if ( indexset_set(&set, i) != 1 )
	{
	  return -( 4 * max + i + 1);
	}
    }
  
  for (int i=0; i < max; i++)
    {
      if (( indexset_get(&set, i) == (i % 2)) )
	{
	  return -( 5 * max + i + 1);
	}
    }

  set.set=0x4100000000L;
  int rindex = indexset_getrelindex(&set,32);
  printf(" %i 0x%llx\n", rindex, set.set);
  if ( indexset_get(&set,32) != 1 )
    {
      printf("get 32 fail  0x%llx\n", set.set);
      return -1;
    }
  if ( indexset_set(&set,38) != 0 )
    {
      // was already set
      printf("set 38 fail 0x%llx\n", set.set);
      return -2;
    }
  if ( indexset_getrelindex(&set, 38) != (rindex + 1))
    {      
      return -3;
    }

  return 0;
}

/**
 list allocation test
 */
int test_allistcontext_allocation()
{
  struct allistof * listof;
  int step=1;

  // add one at end for list of primes
  prime_context->context=new_allistcontext(prime_context->glob_numbercount+1);
  if ( prime_context->context != NULL )
    {
      ++step;

      for (int i=0; i< prime_context->glob_numbercount; i++)
	{
	  listof=new_allistof(prime_context->context);
	  ++step;
	  if ( listof != NULL )
	    {
	      // membership_id is number
	      if (listof->membership_id != i)
		{
		  if (prime_context->params.test_debug) {fprintf(stderr,"unexpected membership_id %i != %i should grow linearly\n",listof->membership_id,(i+1));}
		  return -step;
		}
	      struct allistof * contextlistof = allistcontext_get_membership(prime_context->context, i);
	      ++step;
	      if (contextlistof != listof)
		{
		  if (prime_context->params.test_debug) {fprintf(stderr,"context membership %i %p does not match with created membership %p \n",i,contextlistof,listof);}
		  return -step;
		}
	    }
	  else
	    {
	      if (prime_context->params.test_debug) {fprintf(stderr,"allocation failure of listof %i\n",i);}
	      return -step;
	    }
	}
      ++step;
      // membership glob_numbercount is list of primes.
      prime_context->primelp=new_allistof(prime_context->context);
      // first is for primes
      if (prime_context->primelp == NULL)
	{
	  if (prime_context->params.test_debug) {fprintf(stderr, "allocation of listof for primes failed\n");}
	  return -step;
	}

      listof=new_allistof(prime_context->context);
      // should fail since above reservation
      if (listof != NULL)
	{
	  if (prime_context->params.test_debug) {fprintf(stderr, "allocation of listof above context reservation\n");}
	  return -step;
	}
    }
  else
    {
      return -step;
    }

  return 0;
}

struct test2_factor
{
  int value; // int and not long long ?
  struct allistelement * element;
  int error;
};

/**
callback used in foreach  to add a factor into a list.
 **/
void * test2_add_factor (struct allistof * list, struct allistelement * element, struct allistelement * next, int count, void * param)
{
  struct test2_factor * factor = (struct test2_factor *) param;
  int prime = (unsigned long long) element->data;
  if ( list != prime_context->primelp )
    {
      fprintf(stderr,"[ERROR] walking non  prime list to get factors \n");
      return NULL;
    }
  if ( factor != NULL)
    {
      // list of all numbers seen as multiple of this prime at this step.
      struct allistof * prime_multiples_list = allistcontext_get_membership(prime_context->context,prime);
      if (prime_multiples_list == prime_context->primelp)
	{
	  fprintf(stderr,"[ERROR] factor list is prime list \n");
	  return NULL;
	}
      if ( prime_multiples_list != NULL )
	{
	  // walking prime in ascending order, so if prime * 2 > element no other prime will be found to divide this factor.
	  if ( 2 * prime > factor->value )
	    {
	      return NULL;
	    }
	  // this prime is a factor of this number
	  if ( factor->value % prime == 0 )
	    {
	      if (prime_context->params.test_debug > 1) {fprintf(stderr,"factor %i prime %i count %i list %p\n", factor->value, prime, prime_multiples_list->count, prime_multiples_list);}
	      if ( allistelement_add_in(factor->element, prime_multiples_list) == NULL )
		{
		  if (prime_context->params.test_debug) {fprintf(stderr,"can't add %i in prime factor list %i %i count %i\n", factor->value, count, prime, prime_multiples_list->count);}
		  dump_list(prime_multiples_list);
		  ++factor->error;
		  return NULL;
		}
	      if ( prime_multiples_list->errors > 0 )
		{
		  if (prime_context->params.test_debug) {fprintf(stderr,"add %i in prime factor list %i %i count %i has errors %i\n", factor->value, count, prime, prime_multiples_list->count, prime_multiples_list->errors);}
		  factor->error += prime_multiples_list->errors;
		  return NULL;
		}
	    }
	}
      else
	{
	  if (prime_context->params.test_debug) {fprintf(stderr,"can't get prime list for %i count %i\n", factor->value, count);}
	}
    }
  return factor;
}
  
/**
 fill list elementp with primes
*/
int test_fill_primelist()
{
  int step=1; // use for error return, to gather where error was.
  struct test2_factor factor;

  // initialization with precomputed-list of primes.
  for (int j=0; j<PRIMCOUNT;j++)
    {
      struct allistof * listof = allistcontext_get_membership(prime_context->context,j);
      int i = prims[j];
      if ( i < prime_context->glob_numbercount )
	{
	  prime_context->elementp[i]=allistcontext_new_allistelement(prime_context->context,(void*)((long long)i));
	  step++;
	  if ( listof == NULL )
	    {
	      return -step;
	    }
	  step++;
	  if ( allistelement_add_in(prime_context->elementp[i], prime_context->primelp) == NULL )
	    {
	      if (prime_context->params.test_debug) {fprintf(stderr,"can't add %i in prime list %i\n", prims[j], j);}
	      return -step;
	    }
	}
    }

  factor.error=0;
  // walk full set of integers up to upper bound
  for (int i=0; i<prime_context->glob_numbercount; i++)
    {
      if ( prime_context->elementp[i]!=NULL)
	{
	  // already analyzed / precomputed
	  printf("already analyzed / precomputed value %i\n", i);
	  continue;
	}
      // factor_element is a node that will collect all prime factors for this number.
      struct allistelement * factor_element = allistcontext_new_allistelement(prime_context->context,(void*)((long long)i));
      prime_context->elementp[i]=factor_element;
      factor.value=i;
      factor.element=factor_element;
      step++;
      if ( factor_element == NULL )
	{
	  return -step;
	}
      // try all known primes as factor for this number, will be added in prime factors
      allist_for_each(prime_context->primelp, NULL, test2_add_factor, &factor, 1, 0);
      if ( factor.error > 0)
	{
	  return -step;
	}
      ++step;
      if (factor_element != NULL)
	{
	  if (( i > 1) && (factor_element != NULL))
	    {	  
	      // is no prime where found as of this integer i, it means it is a prime.
	      if ( allistelement_get_all_memberships(factor_element) == 0)
		{
		  // this is a new potential prime.
		  if ( allistelement_add_in(factor_element, prime_context->primelp) == NULL )
		    {
		      if (prime_context->params.test_debug) {fprintf(stderr,"can't add new prime  %i in prime list\n", i);}
		      return -step;
		    }
		  else
		    {
		      if (prime_context->params.test_debug>1) {fprintf(stderr,"[INFO] add new prime  %i in prime list\n", i);}
		    }
		}
	    }
	  if ( prime_context->params.shrinkit )
	    {
	      struct shrunkinfo shrunkinfo;
	      struct allistelement * shrunk;
	      shrunk=allistelement_shrink(factor_element,&shrunkinfo);
	      if ( ( shrunk != NULL)  && ( shrunkinfo.shrunkerrors == 0 ))
		{
		  if ( shrunk != factor_element )
		    {
		      if ( allistelement_release(factor_element) == 0 )
			{
			  factor_element = shrunk;
			}
		      else
			{
			  fprintf(stderr,"[ERROR] release failure for %p",factor_element);
			  return -step;
			}
		    }
		  else
		    {
		      if (prime_context->params.test_debug) {fprintf(stderr,"shrunk %p did nothing\n", shrunk);}
		    }
		}
	      else
		{	      
		  fprintf(stderr,"[ERROR] shrunk failure for %p\n",factor_element);
		  return -step;
		}
	      if (prime_context->params.test_debug>1) {dump_element_full(shrunk);}
	    }
	}
      else
	{
	  // how can factor_element be NULL ?
	  fprintf(stderr,"[ERROR] null list for value %i\n", i);
	}

    }
  ++step;
  // check count
  if ( prime_context->context->next_membership != (prime_context->glob_numbercount + 1) )
    {
      return -step;
    }

  // prime_context->context->next_membership == prime_context->glob_numbercount + 1
  // remark prime_context->context->list[prime_context->glob_numbercount] is list of primes ( not a decomp )
  for (int i =1; i<prime_context->glob_numbercount; i++)
    {
      step++;
      struct allistof * list = &prime_context->context->list[i];
      if ( list != NULL )
	{
	  if ( list->head != NULL)
	    {
	      unsigned long long prim = (unsigned long long)  list->head->data;
	      // quick test, at least there should not be more decomposition primes than number divide by smallest decomp prime.
	      if ( list->count < ( ( prime_context->glob_numbercount / prim  )))
		{
		  if (prime_context->params.test_debug)
		    {
		      fprintf(stderr,"membership[%i] list->count wrong prime %lld %p %p;\n( list->count %i  < ( ( prime_context->glob_numbercount / prim  ) %lld )  \n", i , prim, list, (list->head != NULL) ? list->head->data : NULL, list->count, prime_context->glob_numbercount / prim  );
		    }
		  dump_list(&prime_context->context->list[i]);
		  return -step;
		}
	    }
	}
    }

  return 0;
}

/** 0 test membership */
int test3()
{
  for (int j=0; j<PRIMCOUNT;j++)
    {
      struct allistof * listof = allistcontext_get_membership(prime_context->context,j);
      if ( allistelement_is_in(prime_context->elementp[0], listof) != 1 )
	{
	  return -j;
	}
    }
  return 0;
}

/** shrink test */
int test4()
{
  int step = 0;
  struct allistelement * shrunk;
  struct shrunkinfo info;

  info.shrunkerrors=0;
  for (int i=0; i<prime_context->glob_numbercount; i++)
    {
      step++;
      if ( prime_context->elementp[i] == NULL )
	{
	  fprintf(stderr, "NULL element[%i] in %s:%i \n", i, __func__,__LINE__);
	  return -1;
	}
      shrunk=allistelement_shrink(prime_context->elementp[i],&info);
      if ((shrunk == NULL) || (info.shrunkerrors > 0))
	{
	  if (prime_context->params.test_debug) {
	    fprintf(stderr,"shrink failure for %i\n", i);
	    fprintf(stderr,"shrunk errors  %i \n",info.shrunkerrors);
	  }
	  return -step;
	}
      /*
      else
	{
	   fprintf(stderr,"shrink ok for %i\n", i);
	}      
      */
      if ( (prime_context->elementp[i] != NULL) && (  prime_context->elementp[i] != shrunk ) )
	{
	  if ( prime_context->params.test_debug > 1) { fprintf(stderr,"cleaning element %i %p", i, prime_context->elementp[i]);}
	  if ( allistelement_release(prime_context->elementp[i]) != 0 )
	    {
	      fprintf(stderr,"[ERROR] release of %p failed %i \n", prime_context->elementp[i], i);
	      return -step;
	    }
	}
      prime_context->elementp[i]=shrunk;
      ++step;
      if ((i>1) && ( prime_context->elementp[i]->memberships == 0))
	{
	    printf("new prime %i should have been already inserted in prime list \n", i);
	    return -step;
	}
    }
  return 0;
}

/** primality check */
int primality_check_test()
{
  for (int j=0; j<PRIMCOUNT;j++)
    {
      if ( prims[j] < prime_context->glob_numbercount )
	{
	  if ( ( (prime_context->elementp[prims[j]]->flags & ALLIST_SHRUNK) != 0) && (prime_context->elementp[prims[j]]->memberships > 1) )
	    {
	      printf("WRONG PRIME %i, dividers %i", prims[j],  prime_context->elementp[prims[j]]->memberships);
	      return -j;
	    }
	}
    }
  printf("list of primes : \n");
  dump_list(prime_context->primelp);
  return 0;
}

void usage()
{
  printf(" -debug, -trace, -notestdebug, -shrinkit, 10x, -decomp \n");
}

int main(int argc, char * argv[])
{
  int count=1;
  struct _exec_params params =
    {
      .test_debug=1,
      .shrinkit=0,
      .decomp=0
    };


  printf("plays with primes, implementation limited to long long primes.");
  
  if ( argc >= 1 )
    {
      if( argc > 1)
	{
	  for (int i=1; i<argc;i++)
	    {
	      if (strcmp(argv[i],"-debug") == 0)
		{
		  printf("debug set\n");
		  allist_set_debug(1);
		}
	      else if (strcmp(argv[i],"-trace") == 0)
		{
		  printf("debug set\n");
		  allist_set_debug(2);
		}
	      else if (strcmp(argv[i],"-testdebug") == 0)
		{
		  printf("test debug set\n");
		  params.test_debug=1;
		}
	      else if (strcmp(argv[i],"-notestdebug") == 0)
		{
		  printf("test debug unset\n");
		  params.test_debug=0;
		}
	      else if (strcmp(argv[i],"-shrinkit") == 0)
		{
		  printf("shrink right after add\n");
		  params.shrinkit=1;
		}
	      else if (strcmp(argv[i],"10x") == 0)
		{
		  count=count * 10;
		  printf("count = %i\n", count);
                }
	      else if (strcmp(argv[i],"-decomp")==0)
		{
		  params.decomp=1;
		}
	      else
		{
		  fprintf(stderr,"[ERROR] unrecognized option %s", argv[i]);
		  usage();
		  exit(1);
		}
	    }
	}
      time_t start;
      
      prime_context=create_prime_context(count);
      set_prime_context_params(prime_context, &params);
      
      show_memory_usage();
      time(&start);
      checktest(stdout,"indexset test",test_indexset(),&start);

      if ( checktest(stdout,"list allocation test",test_allistcontext_allocation(),&start) != 0 )
	{
	  fprintf(stderr,"Unrecoverable error, stop test\n");
	  return 1;
	}
      
      checktest(stdout,"fill lists test",test_fill_primelist(),&start);
      show_memory_usage();
      
      checktest(stdout,"zero membership test",test3(),&start);

      // dump_context(context);
      checktest(stdout,"shrink test",test4(),&start);
      
      checktest(stdout,"primality membership test",primality_check_test(),&start);
      //dump_context(context);

      printf( "%i set \n", prime_context->glob_numbercount-1);
      if (prime_context->elementp[prime_context->glob_numbercount-1] != NULL)
	{
	  dump_indexset(&prime_context->elementp[prime_context->glob_numbercount-1]->indexset);
	}
      else
	{
	  printf("element %i null\n",prime_context->glob_numbercount-1);
	}
      
      if ( prime_context->params.decomp == 1 )
	{
	  for (int i = 1; i < prime_context->glob_numbercount; i ++)
	    {
	      if (prime_context->elementp[i] != NULL)
		{
		  printf("%i = " , i);
		  dump_indexset(&prime_context->elementp[i]->indexset);
		  printf("\n");
		}
	    }
	}
    }
  else
    {
      fprintf(stderr,"too many arguments ( currenlty supported -debug ) ");
    }

  if ( prime_context != NULL )
    {
      free(prime_context);
    }

}
