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

int test1()
{
  struct allistof * listof;
  int step=1;

  context=new_allistcontext(PRIMCOUNT);
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

int test2()
{
  int step=1;

  for (int i=0; i<NUMBERCOUNT; i++)
    {
      elementp[i]=allistcontext_new_allistelement(context,(void*)((long long)i));
      step++;
      if ( elementp[i] == NULL )
	{
	  return -step;
	}
      for (int j=0; j<PRIMCOUNT;j++)
	{
	  step++;
	  if ( (i % prims[j]) == 0 )
	    {
	      struct allistof * listof = allistcontext_get_membership(context,j);
	      if ( listof == NULL )
		{
		  return -step;
		}
	      if ( allistelement_add_in(elementp[i], listof) == NULL )
		{
		  fprintf(stderr,"can't add %i in primary list %i %i\n", i, j, prims[j]);
		  return -step;
		}
	    }
	}
      
    }
  // check count
  for (int i =0; i<context->next_membership; i++)
    {
      step++;
      if ( context->list[i].count < ( ( NUMBERCOUNT / prims[i] ) - 1  ))
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
      /*
      if ( elementp[i]->memberships == 0)
	{
	    printf("potential new prime %i\n", i);
	}
      */
    }
  return 0;
}

/** primality check */
int test5()
{
  for (int j=0; j<PRIMCOUNT;j++)
    {
      if ( (elementp[prims[j]]->shrunk == 1) && (elementp[prims[j]]->memberships > 1) )
	{
	  printf("WRONG PRIME %i, dividers %i", prims[j],  elementp[prims[j]]->memberships);
	  return -j;
	}
    }
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
      checktest("test1",test1());
      checktest("test2",test2());
      checktest("zero membership test",test3());
      show_memory_usage();
      checktest("shrink test",test4());
      show_memory_usage();
      checktest("primality membership test",test5());
      dump_context(context);
    }
  else
    {
      fprintf(stderr,"no argument expected");
    }
}
