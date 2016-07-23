/**
this is allist contract 
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>


#include "allist.h"

#define PRIMCOUNT 25
#define NUMBERCOUNT 10000

int prims[PRIMCOUNT] = { 2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97};

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
  param = allist_for_each(list, list->head, dump_element, param, 1, PRIMCOUNT);
  if (param != list )
    {
      printf( "dump error expected %p got %p\n", list, param);
    }
}

void dump_indexset(struct indexset * setp)
{
  int max=64;
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
	  printf("membership[%i] list.count %i ;\n", i, context->list[i].count );
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
  int max = 64;

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
  context=new_allistcontext(PRIMCOUNT+1);
  if ( context != NULL )
    {
      for (int i=0; i< PRIMCOUNT; i++)
	{
	  listof=new_allistof(context);
	  ++step;
	  if ( listof != NULL )
	    {
	      if (listof->membership_id != i)
		{
		  fprintf(stderr,"unexpected membership_id %i != %i should grow linearly\n",listof->membership_id,i);
		  return -step;
		}
	      struct allistof * contextlistof = allistcontext_get_membership(context, i);
	      ++step;
	      if (contextlistof != listof)
		{
		  fprintf(stderr,"context membership %i %p does not match with created membership %p \n",i,contextlistof,listof);
		  return -step;
		}
	    }
	  else
	    {
	      fprintf(stderr,"allocation failure of listof %i\n",i);
	      return -step;
	    }
	}
      ++step;
      primelp=new_allistof(context);
      // last is for primes
      if (primelp == NULL)
	{
	  fprintf(stderr, "allocation of listof for primes failed\n");
	  return -step;
	}
      ++step;
      listof=new_allistof(context);
      // should fail since above reservation
      if (listof != NULL)
	{
	  fprintf(stderr, "allocation of listof above context reservation\n");
	  return -step;
	}
    }
  else
    {
      return -step;
    }
  return 0;
}

/**
 fill list test
*/
int test2()
{
  int step=1;

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
	  fprintf(stderr,"can't add %i in prime list %i\n", prims[j], j);
	  return -step;
	}	      
    }

  for (int i=0; i<NUMBERCOUNT; i++)
    {
      if ( elementp[i]!=NULL)
	{
	  // already analyzed
	  continue;
	}
      elementp[i]=allistcontext_new_allistelement(context,(void*)((long long)i));
      step++;
      if ( elementp[i] == NULL )
	{
	  return -step;
	}
      for (int j=0; (j<PRIMCOUNT) && (prims[j] < i) ;j++)
	{
	  struct allistof * listof = allistcontext_get_membership(context,j);	  
	  step++;
	  if ( listof == NULL )
	    {
	      return -step;
	    }
	  step++;
	  if ( (i % prims[j]) == 0 )
	    {
	      if ( allistelement_add_in(elementp[i], listof) == NULL )
		{
		  fprintf(stderr,"can't add %i in multiple list %i %i\n", i, j, prims[j]);
		  return -step;
		}
	    }
	}
      /* TODO growable membership ...
      struct allistelement * current = primelp->head;
      for (int j=0; j<primlp->count;j++)
	{
	struct allistof * listof = allistcontext_get_membership(context,j);	  
	step++;
	if ( listof == NULL )
	{
	return -step;
	}
		step++;
		if ( (i % ( (unsigned int) current->data) ) == 0 )
		{
		if ( allistelement_add_in(elementp[i], listof) == NULL )
		{
		fprintf(stderr,"can't add %i in multiple list %i %i\n", i, j, prims[j]);
		      return -step;
		    }
		}
	}
      */
      ++step;
      if ( allistelement_get_memberships(elementp[i]) == 0)
	{
	  // this is a new potential prime.
	  if ( allistelement_add_in(elementp[i], primelp) == NULL )
	    {
	      fprintf(stderr,"can't add new prime  %i in prime list\n", i);
	      return -step;
	    }
	}
    }
  ++step;
  // check count
  if ( context->next_membership != (PRIMCOUNT + 1) )
    {     
      return -step;
    }
  for (int i =0; i<(context->next_membership -1); i++)
    {
      step++;
      if ( context->list[i].count < ( ( NUMBERCOUNT / prims[i] ) - 2  ))
	{
	  fprintf(stderr,"membership[%i] list.count wrong %i ;\n", i , context->list[i].count);
	  return -step;
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
      shrunk=allistelement_shrink(elementp[i],&info);
      if (shrunk == NULL)
	{
	  fprintf(stderr,"shrink failure for %i\n", i);
	  fprintf(stderr,"shrunk errors  %i \n",info.shrunkerrors);
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
	  // cleanup, at least zero in head ( fixed size part ) of element.
	  bzero(elementp[i],sizeof(struct allistelement));
	  // fprintf(stderr,"cleaning element %i %p", i, elementp[i]);
	  free(elementp[i]);
	}
      elementp[i]=shrunk;
      ++step;
      if ( elementp[i]->memberships == 0)
	{
	    printf("new prime %i shold have been already insterted in prime list \n", i);
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

int checktest(char* test, int result)
{
  if ( result == 0 )
    {
      printf("%s OK\n", test);
    }
  else
    {
      printf("%s KO error %i \n", test, result);
    }
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
  if ( argc == 1 )
    {
      int step = 0;
      show_memory_usage();
      int fulltest = checktest("indexset test",test0())
	|| checktest("list allocation test",test1())
	|| checktest("fill lists test",test2())
	|| checktest("zero membership test",test3())
	|| checktest("shrink test",test4())
	|| checktest("primality membership test",test5());
      dump_context(context);
      printf( "%i set=", 77);
      dump_indexset(&elementp[77]->indexset);
    }
  else
    {
      fprintf(stderr,"no argument expected");
    }
}
