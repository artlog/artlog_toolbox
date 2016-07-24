/**
this is allist contract 
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

#include "allist.h"
// direct acces to internal implementation
#include "allist_internal.h"

#define PRIMCOUNT 25
// with 200000 get disposed by kernel while processing due to mermoyr conu
#define NUMBERCOUNT 20000

int test_debug=1;

int prims[PRIMCOUNT] = { 2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97};

//int glob_numbercount=NUMBERCOUNT;

struct allistcontext * context = NULL;
struct allistelement * elementp[NUMBERCOUNT];

struct allistof * primelp = NULL;

void * dump_element(struct allistof * list, struct allistelement * element, struct allistelement * next, int count, void * param)
{
  if ( next != NULL )
    {
      printf(" %i," , element->data);
    }
  else
    {
      printf(" %i.\n" , element->data);
    }
  return param;
}

void dump_list(struct allistof * list)
{
  void * param = list;
  if ( list->errors > 0 )
    {
      printf("list %p has errors %i\n", list, list->errors);
    }
     
  param = allist_for_each(list, list->head, dump_element, param, 1, 0);
  if (param != list )
    {
      printf( "dump error expected %p got %p\n", list, param);
    }
}

void dump_indexset(struct indexset * setp)
{
  int max=INDEXSET_COUNT;
  for (int i=0; i < max; i++)
    {
      if ( indexset_get(setp, i) != 0 )
	{
	  printf("%i,",i);
	}
    }
}

void dump_context( struct allistcontext * context)
{
  printf("context %p {\n", context);
  if (context != NULL)
    {
      printf("membership_reservation %i;\n", context->membership_reservation);
      printf("next_membership %i;\n", context->next_membership);
      for (int i =0; i<context->next_membership; i++)
	{
	  printf("membership[%i] list.count %i list.errors %i;\n", i, context->list[i].count,  context->list[i].errors );
	}
    }
  else
    {
      printf("!NULL!\n");
    }
  printf("}\n");
}

int test0()
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
  printf(" %i 0x%lx\n", rindex, set.set);
  if ( indexset_get(&set,32) != 1 )
    {
      printf("get 32 fail  0x%lx\n", set.set);
      return -1;
    }
  if ( indexset_set(&set,38) != 0 )
    {
      // was already set
      printf("set 38 fail 0x%lx\n", set.set);
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
int test1()
{
  struct allistof * listof;
  int step=1;

  // add one at end for list of primes
  context=new_allistcontext(NUMBERCOUNT+1);
  if ( context != NULL )
    {
      ++step;
      primelp=new_allistof(context);
      // first is for primes
      if (primelp == NULL)
	{
	  if (test_debug) {fprintf(stderr, "allocation of listof for primes failed\n");}
	  return -step;
	}

      for (int i=0; i< NUMBERCOUNT; i++)
	{
	  listof=new_allistof(context);
	  ++step;
	  if ( listof != NULL )
	    {
	      if (listof->membership_id != (i+1))
		{
		  if (test_debug) {fprintf(stderr,"unexpected membership_id %i != %i should grow linearly\n",listof->membership_id,(i+1));}
		  return -step;
		}
	      struct allistof * contextlistof = allistcontext_get_membership(context, (i+1));
	      ++step;
	      if (contextlistof != listof)
		{
		  if (test_debug) {fprintf(stderr,"context membership %i %p does not match with created membership %p \n",i,contextlistof,listof);}
		  return -step;
		}
	    }
	  else
	    {
	      if (test_debug) {fprintf(stderr,"allocation failure of listof %i\n",i);}
	      return -step;
	    }
	}
      ++step;
      listof=new_allistof(context);
      // should fail since above reservation
      if (listof != NULL)
	{
	  if (test_debug) {fprintf(stderr, "allocation of listof above context reservation\n");}
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
  int value;
  struct allistelement * element;
  int error;
};

void * test2_add_factor (struct allistof * list, struct allistelement * element, struct allistelement * next, int count, void * param)
{
  struct test2_factor * factor = (struct test2_factor *) param;
  int prime = (unsigned long long) element->data;
  if ( factor != NULL)
    {
      struct allistof * primelist = allistcontext_get_membership(context,prime+1); // get (prime+1)th element in context.
      if ( primelist != NULL )
	{
	  if ( 2 * prime > factor->value )
	    {
	      return NULL;
	    }
	  if ( factor->value % prime == 0 )
	    {
	      if (test_debug > 1) {fprintf(stderr,"factor %i prime %i count %i list %p\n", factor->value, prime, primelist->count, primelist);}
	      if ( allistelement_add_in(factor->element, primelist) == NULL )
		{
		  if (test_debug) {fprintf(stderr,"can't add %i in prime factor list %i %i count %i\n", factor->value, count, prime, primelist->count);}
		  dump_list(primelist);
		  ++factor->error;
		  return NULL;
		}
	    }
	}
    }
  return factor;
}
  
/**
 fill list test
*/
int test2()
{
  int step=1;
  struct test2_factor factor;
  
  for (int j=0; j<PRIMCOUNT;j++)
    {
      struct allistof * listof = allistcontext_get_membership(context,j);
      int i = prims[j];
      elementp[i]=allistcontext_new_allistelement(context,(void*)((long long)i));
      step++;
      if ( listof == NULL )
	{
	  return -step;
	}
      step++;
      if ( allistelement_add_in(elementp[i], primelp) == NULL )
	{
	  if (test_debug) {fprintf(stderr,"can't add %i in prime list %i\n", prims[j], j);}
	  return -step;
	}	      
    }

  factor.error=0;
  for (int i=0; i<NUMBERCOUNT; i++)
    {
      if ( elementp[i]!=NULL)
	{
	  // already analyzed
	  continue;
	}
      elementp[i]=allistcontext_new_allistelement(context,(void*)((long long)i));
      factor.value=i;
      factor.element=elementp[i];
      step++;
      if ( elementp[i] == NULL )
	{
	  return -step;
	}
      allist_for_each(primelp, NULL, test2_add_factor, &factor, 1, 0);
      if ( factor.error > 0)
	{
	  return -step;
	}
      ++step;
      if ( i > 1)
	{
	  if ( allistelement_get_memberships(elementp[i]) == 0)
	    {
	      // this is a new potential prime.
	      if ( allistelement_add_in(elementp[i], primelp) == NULL )
		{
		  if (test_debug) {fprintf(stderr,"can't add new prime  %i in prime list\n", i);}
		  return -step;
		}
	      else
		{
		  if (test_debug) {fprintf(stderr,"[INFO] add new prime  %i in prime list\n", i);}
		}

	      // this stress memory => process stopped early...
	      if ( 0 )
	      {
		struct shrunkinfo shrunkinfo;
		struct allistelement * shrunk;
		shrunk=allistelement_shrink(elementp[i],&shrunkinfo);
		if ( ( shrunk != NULL)  && ( shrunkinfo.shrunkerrors == 0 ))
		  {
		    if ( shrunk != elementp[i] )
		      {
			if ( allistelement_release(elementp[i]) == 0 )
			  {
			    elementp[i] = shrunk;
			  }
			else
			  {
			    fprintf(stderr,"[ERROR] release failure for %p",elementp[i]);
			    return -step;
			  }
		      }
		    else
		      {
			if (test_debug) {fprintf(stderr,"shrunk %p did nothing\n", shrunk);}
		      }
		  }
		else
		  {
		    fprintf(stderr,"[ERROR] shrunk failure for %p\n",elementp[i]);
		    return -step;
		  }
	      }	      
	    }
	}
    }
  ++step;
  // check count
  if ( context->next_membership != (NUMBERCOUNT + 1) )
    {
      return -step;
    }
  for (int i =1; i<context->next_membership; i++)
    {
      step++;
      struct allistof * list = &context->list[i];
      if ( list != NULL )
	{
	  if ( list->head != NULL)
	    {
	      unsigned long long prim = (unsigned long long)  list->head->data;
	      if ( list->count < ( ( NUMBERCOUNT / prim  )))
		{
		  if (test_debug) {fprintf(stderr,"membership[%i] list.count wrong %i %p %p;\n", i , list->count, list, (list->head != NULL) ? list->head->data : NULL );}
		  dump_list(&context->list[i]);
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
      struct allistof * listof = allistcontext_get_membership(context,j);
      if ( allistelement_is_in(elementp[0], listof) != 1 )
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
  
  for (int i=0; i<NUMBERCOUNT; i++)
    {
      step++;
      if ( elementp[i] == NULL )
	{
	  fprintf(stderr, "NULL element in %s:%i \n", __func__,__LINE__);
	  return -1;
	}
      shrunk=allistelement_shrink(elementp[i],&info);
      if ((shrunk == NULL) || (info.shrunkerrors > 0))
	{
	  if (test_debug) {
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
      if ( (elementp[i] != NULL) && (  elementp[i] != shrunk ) )
	{
	  if ( test_debug > 1) { fprintf(stderr,"cleaning element %i %p", i, elementp[i]);}
	  if ( allistelement_release(elementp[i]) != 0 )
	    {
	      fprintf(stderr,"[ERROR] release of %p failed %i \n", elementp[i], i);
	      return -step;
	    }
	}
      elementp[i]=shrunk;
      ++step;
      if ((i>1) && ( elementp[i]->memberships == 0))
	{
	    printf("new prime %i should have been already insterted in prime list \n", i);
	    return -step;
	}
    }
  return 0;
}

/** primality check */
int test5()
{
  for (int j=0; j<PRIMCOUNT;j++)
    {
      if ( ( (elementp[prims[j]]->flags & ALLIST_SHRUNK) != 0) && (elementp[prims[j]]->memberships > 1) )
	{
	  printf("WRONG PRIME %i, dividers %i", prims[j],  elementp[prims[j]]->memberships);
	  return -j;
	}
    }
  dump_list(primelp);
  return 0;
}

int checktest(char* test, int result, time_t * start)
{
  time_t end;
  time(&end);
  if ( result == 0 )
    {
      printf("%s OK\n", test);
    }
  else
    {
      printf("%s KO error %i \n", test, result);
    }
  printf("test %s took %lu second\n", test,  (end - *start));
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

int main(int argc, char * argv[])
{
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

	      else
		if (strcmp(argv[i],"-notestdebug") == 0)
		  {
		    printf("test debug unset\n");
		    test_debug=0;
		  }
		else
		  {
		    fprintf(stderr,"[ERROR] unrecognized option %s", argv[i]);
		    exit(1);
		  }
	    }
	}
      int step = 0;
      time_t start;
      show_memory_usage();
      time(&start);
      checktest("indexset test",test0(),&start);

      if ( checktest("list allocation test",test1(),&start) != 0 )
	{
	  fprintf(stderr,"Unrecoverable error, stop test\n");
	  return 1;
	}
      
      checktest("fill lists test",test2(),&start);
      checktest("zero membership test",test3(),&start);

      // dump_context(context);
      checktest("shrink test",test4(),&start);
      
      checktest("primality membership test",test5(),&start);
      // dump_context(context);

      printf( "%i set=", 77);
      dump_indexset(&elementp[77]->indexset);
    }
  else
    {
      fprintf(stderr,"too many arguments ( currenlty supported -debug ) ");
    }
}
