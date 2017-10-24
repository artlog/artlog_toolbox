#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "alstack.h"
#include "alcommon.h"

/*
setup a alstack, content of alstack is considered irrelevant (ie can be zeroed)
init_flags defines wether this is a static or dynamic stack ( initial flags)
*/
struct alstack * stack_init(struct alstack * stack, int init_flags)
{
  bzero(stack, sizeof(*stack));
  stack->flags=init_flags | ALSTACK_VALID_FLAG;
  stack->size=ALSTACK_CHUNK;
  return stack;
}

/*
allocate dynamically first chunk for alstack
will call init on alstack
*/
struct alstack * alstack_allocate()
{
  struct alstack * stack = malloc(sizeof(struct alstack));
  if (stack != NULL)
    {
      stack=stack_init(stack,ALSTACK_DYNAMIC_FLAG);
    }
  return stack;
}

struct alstack * alstack_find_last(struct alstack * stack)
{
  while (FLAG_IS_SET(stack->flags,ALSTACK_NEXT_FLAG))    
    {
      stack=stack->next;
    }
  return stack;
}

void alstack_set_next(struct alstack * stack, struct alstack * next )
{
  stack->next=next;
  stack->flags |= ALSTACK_NEXT_FLAG;
  if ( next != NULL )
    {      
      next->previous=stack;
      next->flags |= ALSTACK_PREVIOUS_FLAG;
    }
}

struct alstackelement * alstackchunk_push_ref(struct alstack * stack, void * reference )
{
  struct alstackelement * element = NULL;
  if ( stack->index < stack->size )
    {
      element = &stack->element[stack->index];
      element->reference=reference;
      stack->index ++;
    }
  if ( stack->index == stack->size )
    {
      stack->flags |= ALSTACK_FULL_FLAG;
    }
  return element;
}

/* push a pointer to an element on the stack */
struct alstackelement * alstack_push_ref(struct alstack * stack, void * reference )
{
  stack = alstack_find_last(stack);
  if (stack != NULL)
    {
      // last chunk is full, allocate a new one.
      if (FLAG_IS_SET(stack->flags,ALSTACK_FULL_FLAG))
	{
	  struct alstack * next = NULL;
	  next=alstack_allocate();
	  if ( next != NULL )
	    {
	      alstack_set_next(stack,next);
	    }
	  stack = next;
	}
    }
  if ( stack != NULL)
    {
      return alstackchunk_push_ref(stack,reference);
    }
  return NULL;
}

void alstack_debug(char * text)
{
  fprintf(stderr,"[DEBUG] %s\n",text);
}

void alstack_error(char * text)
{
  fprintf(stderr,"[ERROR] %s\n",text);
}

struct alstackelement * alstackchunk_pop(struct alstack * stack)
{
  if ( stack != NULL)
    {
      if ( stack->index > 0 )
	{
	  int index = stack->index -1 ;
	  stack->index = index;
	  if ( FLAG_IS_SET(stack->flags,ALSTACK_FULL_FLAG) )
	    {
	      stack->flags ^= ALSTACK_FULL_FLAG;
	    }
	  return &stack->element[index];
	}
      else
	{
	  // should look in previous stack.
	  if ( FLAG_IS_SET(stack->flags,ALSTACK_PREVIOUS_FLAG ))
	    {
	      struct alstack * previous = stack->previous;
	      struct alstackelement * result = alstackchunk_pop(previous);

	      // release stack 
	      // not that efficient if many pop/push on the same boundary...
	      if (  FLAG_IS_SET(previous->flags,ALSTACK_NEXT_FLAG))
		{
		  previous->flags ^=  ALSTACK_NEXT_FLAG;
		  stack->flags ^= ALSTACK_PREVIOUS_FLAG;
		}
	      else
		{
		  alstack_error("next flag not set on previous");
		}
	      if ( FLAG_IS_SET(stack->flags,ALSTACK_DYNAMIC_FLAG) )
		{
		  free(stack);
		}
	      return result;
	    }
	  else
	    {
	      alstack_debug("no previous stack");
	      return NULL;
	    }
	}	  
    }
  else
    {
      alstack_debug("last empty stack");
      return NULL;
    }
}

/* pop an element from the stack */
struct alstackelement * alstack_pop(struct alstack * stack)
{
  stack = alstack_find_last(stack);
  return alstackchunk_pop(stack);
}

/* pop all elements and call a function on them */
int alstack_popall(struct alstack * stack, int (*callback)(struct alstackelement * stackelement))
{
  int count = 0;
  if ( stack != NULL )
    {
      struct alstackelement * element = alstack_pop(stack);
      while (element != NULL)
	{
	  ++count;
	  if ( callback != NULL )
	    {
	      (*callback)(element);
	    }
	  element = alstack_pop(stack);
	}	      
      if (element == NULL )
	{
	  // this is an error case ? only if last->index is not 0.
	  if ( stack->index != 0 )
	    {
	      alstack_error("non empty pop fails");
	    }
	  printf("last element\n");
	}
    }
  return count;
}

/*
int alstack_popall(struct alstack * stack, int (*callback)(struct alstackelement * stackelement))
{
  int count = 0;
  if ( stack != NULL )
    {
      struct alstack * todestroy = alstack_find_last(stack);
      struct alstack * last = todestroy;
      while (todestroy != NULL)
	{
	  while ( ! todestroy->index == 0 )
	    {
	      struct alstackelement * element = alstack_pop(todestroy);
	      if (element == NULL )
		{
		  // this is an error case ? only if last->index is not 0.
		  if ( todestroy->index != 0 )
		    {
printf("wrong index after destroy :%i\n",todestroy->index);
		      alstack_error("non empty pop fails");
		    }
printf("last element\n");
		  break;
		}
	      else
		{
		  ++count;
		  if ( callback != NULL )
		    {
		      (*callback)(element);
		    }
		}
	    }
	  // BUGBUGBUG there , last might be empty.
	  last=alstack_find_last(stack);
	  // if was already destroyed 
	  if (last == todestroy )
	    {
	      // stop
printf("stop last=%p stack=%p \n", last, stack);
	      todestroy = NULL;
	    }
	  else
	    {
	      // continue
printf("continue %p \n", last);
	      todestroy = last;
	    }
	}
      // at the end last == stack...
      if ( last != stack )
	{
	  alstack_error("non empty stack after destroy");
	}
    }
  return count;
}
*/

/* destroy stack ( and all next stack up to tail ) */
int alstack_destroy(struct alstack * stack, int (*unlink_element)(struct alstackelement * stackelement))
{
  return  alstack_popall(stack, unlink_element);
}
