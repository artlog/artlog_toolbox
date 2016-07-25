#include <stdlib.h>
#include <stdio.h>

#include "allist.h"
// direct acces to internal implementation
#include "allist_internal.h"


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

void * dump_element_full(struct allistelement * element)
{
  if ( element != NULL )
    {
      printf("dump full element %p\n", element);
      printf("data %ul\n" , element->data);
      printf("memberships %i\n" , element->memberships);
      printf("flags %u\n" , element->flags);
      for (int i =0; i < element->memberships; i++)
	{
	  if ( element->link[i].memberof != NULL)
	    {
	      printf("rel %i membership %i \n",i, element->link[i].memberof->membership_id);
	      dump_list(element->link[i].memberof);
	    }
	}
      if ( (element->flags & ALLIST_EXT) != 0 )
	{
	  struct allistextlink * ext = element->extlink;
	  while (ext != NULL)
	    {
	      for (int i =0; i < INDEXSET_COUNT; i++)
		{
		  if ( ext->link[i].memberof != NULL)
		    {
		      printf("ext rel %i membership %i\n",i, ext->link[i].memberof->membership_id);
		      dump_list(ext->link[i].memberof);
		    }
		}	      
	      ext = ext->nextextlink;
	    }
	}
      printf("indexset %lx \n", element->indexset);
      dump_indexset(&element->indexset);
    }
  return  element->data;
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
