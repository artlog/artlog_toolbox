#include "allist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int debug=0;

int allist_set_debug( int d)
{
  debug=d;
}

/*
http://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
*/

int popcount(unsigned int i)
{
     i = i - ((i >> 1) & 0x55555555);
     i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
     return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

int indexset_get(struct  indexset * indexset, int pabs)
{
  return ( (indexset->set & (1L << pabs)) != 0 );
}

int indexset_reset(struct indexset * indexset, int pabs)
{
  if ( (indexset->set & (1L << pabs)) == 0 )
    {
      // already unset
      return 0;
    }
  else
    {
      indexset->set = indexset->set ^ (1L << pabs);
      -- indexset->count;
      return 1;
    }
}

int indexset_set(struct indexset * indexset, int pabs)
{
  if ( (indexset->set & (1L << pabs)) == 0 )
    {      
      indexset->set = indexset->set | (1L << pabs);
      ++ indexset->count;
      return 1;
    }
  else
    {
      // already set
      return 0;
    }
}

/** from an absolute index get relative index 
 * this is used if shrunk
 * == number of bits set to 1 bellow position ( minus one : index starts at 0 )
 */
int indexset_getrelindex(struct indexset * indexset, int pabs)
{
  unsigned long long set = indexset->set;
  if ( set == 0 )
    {
      return -1;
    }
  else
    // full
    if ( set == 0xffffffffffffffff )
      {
	return pabs;
      }
  int p1=0;
  int p2=0;
  if ( pabs >= 32 )
  {
    p1 = popcount( ((unsigned int) (set >>32)) && ( ((unsigned int) 0xffffffff) >> (63-pabs) ));
    p2 = popcount((unsigned int) set) ;
  } else {
    p1 = popcount(set & (((unsigned int)(0xffffffff)) >> (31-pabs))) ;
  }
  if ( debug) {     fprintf(stderr,"indexset %lx %i %i %i \n", indexset->set, pabs, p1,p2); }
  return p1 + p2 -1;
}
  
/**
 * from a relative index in indexset get absolute index
 * ( position of nth bit set )
 * this is used if shrunk.
 */
int indexset_getabsindex(struct indexset * indexset, int prel)
{
  unsigned long long set = indexset->set;
  int index = 0;
  if ( set == 0 )
    {
      return -1;
    }
  else
    // full
    if ( set == 0xffffffffffffffff )
      {
	return prel;
      }
  // walk number of bit set, and gather index;
  while ((index < ((sizeof set) * 8)) && (prel > 0 ) && ( set != 0 ))
    {
      unsigned int last =  (( set >> index ) & 0xffffffff);
      if ( (last & 0x1) == 1 )
	{
	  --prel;
	  if ( prel == 0 )
	    {
	      break;
	    }
	}
      else
      if ( last == 0 )
	{
	  index = index + 32;
	}
      else
      if ( (last & 0xffffff) == 0 )
	{
	  index = index + 24;
	}
      else
      if ( (last & 0xffff) == 0 )
	{
	  index = index + 16;
	}
      else
      if ( (last & 0xfff) == 0 )
	{
	  index = index + 12;
	}
      else
      if ( (last & 0xff) == 0 )
	{
	  index = index + 8;
	}
      else
	{
	  ++index;
	}
    }
  if ( prel == 0 )
    {
      return index;
    }
  else
    {
      return -1;
    }
}


int indexset_dump(struct indexset * indexset, FILE* where)
{
  fprintf(where,"indexset count %i set %lx\n",indexset->count,  indexset->set);
}

/**
when number of membership is known
*/
struct allistelement * new_allistelement(int memberships, void * data, int delta)
{
  int newlength = sizeof(struct allistelement) + sizeof(struct allistlink) * (memberships-1);
  struct allistelement * allocated = calloc(1,newlength);
  if ( allocated != NULL )
    {
      allocated->memberships=memberships;
      allocated->data=data;
      allocated->flags=ALLIST_MALLOC;
    }
  return allocated;
}

int allistelement_is_shrunk(struct allistelement * element)
{
  return (element->flags & ALLIST_SHRUNK) != 0;
}
  
int allistelement_getrelindex( struct allistelement * current, int absindex)
{
  int relindex = 0;
  if ( allistelement_is_shrunk(current) )
    {
      relindex=indexset_getrelindex(&current->indexset,absindex);
    }
  else
    {
      relindex = absindex;
    }
  return relindex;
}

int allistelement_get_memberships(struct allistelement * this)
{
  int memberships=0;
  for (int i=0; i< this->memberships; i++)
    {
      if ( this->link[i].memberof != NULL )
	{
	  ++ memberships;
	}
    }
  return memberships;
}

/*
 shrink allistelement to take a minimal size
 WARNING this rewrite full linkage ( next previous head tail ).
 */
struct allistelement * allistelement_shrink(struct allistelement * this, struct shrunkinfo * shrunkhealth)
{
  int shrunkerror = 0;
  int nowayback = 0;
  // already skrunk.
  if (allistelement_is_shrunk(this))
    {
      return this;
    }
  int memberships = allistelement_get_memberships(this);

  // no gain... no pain !
  if ( memberships == this->memberships )
    {
      if ( debug) { fprintf(stderr,"no gain no pain %p %i", this, memberships);}
      return this;
    }

  // current limit of indexset
  if ( memberships > 64 )
    {
      return this;
    }

  int newlength = sizeof(*this) + sizeof(this->link) * memberships;
  struct allistelement * shrunk = malloc(newlength);
  if ( shrunk != NULL )
    {
      int shrunkpos = 0;
      memcpy(shrunk,this,newlength);
      // reset pointers to shrunk
      // i is absolute ( in this ) that is >= relative
      // shrunkpos is relative, in shrunk
      for (int i=0; (i < this->memberships) && (shrunkerror==0); i++)
	{
	  if ( this->link[i].memberof != NULL )
	    {
	      if ( indexset_set(&shrunk->indexset, i) == 0)
		{
		  // was already set
		  if (debug) {fprintf(stderr,"indexset already set for %i",i);}
		}
	    }
	  else
	    {
	      if ( indexset_reset(&shrunk->indexset, i) != 0 )
		{
		  // was set
		  if (debug) {fprintf(stderr,"indexset set for %i ( should be 0) ",i);}
		}
	      // we are not interested.
	      continue;
	    }
	  
	  if ( shrunkpos < i )
	    {
	      if ( shrunkpos != indexset_getrelindex(&shrunk->indexset,i) )
		{
		  // might even be an assert , this means indexset implementation is wrong.
		  fprintf(stderr,"internal error, indexset implementation inconsistent. contact developper (abs %i, rel %i != %i)\n",
			  i,
			  shrunkpos,
			  indexset_getrelindex(&shrunk->indexset,i)
			  );
		  indexset_dump(&shrunk->indexset,stderr);
		  ++shrunkerror;
		  break;
		}
	      memcpy(&shrunk->link[shrunkpos], &this->link[i], sizeof(shrunk->link[1]));
	      if ( debug) { fprintf(stderr,"link %i copied to %i\n", i, shrunkpos);}
	    }

	  struct allistelement * next = shrunk->link[shrunkpos].next;
	  struct allistelement * previous = shrunk->link[shrunkpos].previous;

	  if ( this->link[i].next != next )
	    {
	      fprintf(stderr,"shrunk %p and this %p next differs", this->link[i].next, next );
	      ++ shrunkerror;
	    }

	  if ( this->link[i].previous != previous )
	    {
	      fprintf(stderr,"shrunk %p and this %p previous differs", this->link[i].previous , previous );
	      ++ shrunkerror;
	    }

	  if (debug)
	    {
	      fprintf(stderr,"%i, %i, %p %p\n",i, shrunkpos, shrunk->data, shrunk->link[shrunkpos].memberof);
	      fprintf(stderr,"Next %p %p\n", next, (next !=NULL) ? next->data : NULL);
	      fprintf(stderr,"Previous %p %p\n", previous, (previous !=NULL) ? previous->data : NULL);
	    }

	  if (  shrunk->link[shrunkpos].memberof == NULL )
	    {
	      if (debug) { fprintf(stderr,"unexpected null membership");}
	      ++ shrunkerror;
	      break;
	    }
	  
	  if ( previous == NULL )
	    {	  
	      if ( shrunk->link[shrunkpos].memberof->head == this )
		{
		  shrunk->link[shrunkpos].memberof->head=shrunk;		  
		  ++nowayback;
		}
	      else
		{
		  if (debug) { fprintf(stderr, "head of owning list mismatch\n" );}
		  ++shrunkerror;
		}	     
	    }
	  else
	    {
	      int relindex = 0;
	      if (allistelement_is_shrunk(previous))
		{
		  relindex = indexset_getrelindex(&previous->indexset,i);
		  if ( relindex >= previous->memberships )
		    {
		      ++shrunkerror;
		      if ( debug ) { fprintf(stderr, "previous index %i exceeds membemships %i\n", relindex, previous->memberships);}
		    }
		}
	      else
		{
		  relindex = i;
		}

	      // replace this by shrunk
	      if ( previous->link[relindex].next == this )
		{
		  previous->link[relindex].next=shrunk;
		  ++nowayback;
		}
	      else
		{
		  if (debug) { fprintf(stderr, "previous.next %i was not pointing on this %p != %p \n", relindex, previous->link[relindex].next, this);}
		  ++shrunkerror;
		}
	    }	  
	  
	  if ( next == NULL )
	    {
	      if ( shrunk->link[shrunkpos].memberof->tail == this )
		{
		  shrunk->link[shrunkpos].memberof->tail=shrunk;
		  ++nowayback;
		}
	      else
		{
		  if (debug) { fprintf(stderr, "tail of owning list mismatch\n" );}
		  ++shrunkerror;
		}
	    }
	  else
	    {
	      int relindex = 0;
	      if (allistelement_is_shrunk(next))
		{
		  relindex = indexset_getrelindex(&next->indexset,i);
		  if ( relindex >= next->memberships )
		    {
		      ++shrunkerror;
		      if ( debug ) { fprintf(stderr, "next index %i exceeds membemships %i\n", relindex, next->memberships);}
		    }
		}
	      else
		{
		  relindex = i;
		}
	      // replace this by shrunk
	      if ( next->link[relindex].previous == this )
		{
		  next->link[relindex].previous=shrunk;
		  ++nowayback;
		}
	      else
		{
		  if (debug) { fprintf(stderr, "next.previous %i was not pointing on this %p != %p \n", relindex, next->link[relindex].previous, this);}
		  ++shrunkerror;
		}
	    }
	  ++shrunkpos;
	}
      shrunk->flags|= ALLIST_SHRUNK;
      shrunk->memberships=shrunkpos;
    }
  if ( shrunkerror > 0 )
    {
      if ( shrunk != NULL )
	{
	  if ( nowayback == 0 )
	    {
	      free(shrunk);
	    }
	  // corrupted links remains to this allocated space, should not free
	  // corrupted people should not be freed.
	}
      shrunk = NULL;
    }
  if ( shrunkhealth != NULL )
    {
      shrunkhealth->shrunkerrors=shrunkerror;
      shrunkhealth->nowayback=nowayback;
    }
  return shrunk;      
}

/**
  create a context for a maximum of  memberships count of allists
 */
struct allistcontext * new_allistcontext(unsigned short memberships)
{
  int newlength = sizeof(struct allistcontext) + sizeof(struct allistof) * (memberships-1);
  struct allistcontext * allocated = malloc(newlength);
  if ( allocated != NULL )
    {
      bzero(allocated,newlength);
      allocated->membership_reservation=memberships;
    }
  return allocated;
}

/**
  create a new list into context, within context use first free membership
 */
struct allistof * new_allistof(struct allistcontext * context)
{
  unsigned int membership = context->next_membership;
  if ( membership < context->membership_reservation )
    {
      struct allistof * list=&context->list[membership];
      list->membership_id=membership;
      ++context->next_membership;
      return list;
    }
  return NULL;
}

/**
  find list with membership id within this context
 */
struct allistof * allistcontext_get_membership(struct allistcontext * context, unsigned short membership)
{
  if ( membership < context->next_membership )
    {     
      return &context->list[membership];
    }
  // either too big or not yet allocated.
  return NULL;
}

struct allistelement * allistcontext_new_allistelement(struct allistcontext * context, void * data)
{
  struct allistelement * element = new_allistelement(context->membership_reservation, data, 0);
  return element;
}

struct allistelement * allistelement_add_in(struct allistelement * element, struct allistof * list)
{
  struct allistof * previouslist = NULL;
  struct allistelement * tail = list->tail;

  if (element == NULL)
    {
      return NULL;
    }
  if ( list->membership_id > 64)
    {
      // extended add in TODO
      return NULL;
    }
  
  previouslist = element->link[allistelement_getrelindex(element,list->membership_id)].memberof;
  
  // was not a member of this list
  if ( previouslist == NULL )
    {      
      // empty list
      if (allistelement_is_shrunk(element) )
	{
	  // can't add an element already shrunk into a new list.
	  return NULL;
	}
      if ( (list->head == NULL) || (tail == NULL) )
	{
	  if (element->link[list->membership_id].next !=NULL)
	    {
	      // :-(
	      ++list->errors;
	    }
	  if (element->link[list->membership_id].previous !=NULL)
	    {
	      // :-(
	      ++list->errors;
	    }
	  if ( (list->head != NULL ) || (tail != NULL ))
	    {
	      // :-(
	      ++list->errors;
	    }
	  if (list->errors == 0)
	    {
	      list->head = element;
	      list->tail = element;
	      ++list->count;
	    }
	  else
	    {
	      return NULL;
	    }
	}
      else
	{	  
	  if (allistelement_is_shrunk(element))
	    {
	      // can't add an element already shrunk into a new list.
	      return NULL;
	    }
	  if (list->errors == 0)
	    {
	      int rel_index = allistelement_getrelindex(tail, list->membership_id);
	      if ( rel_index > list->membership_id)
		{
		  ++list->errors;
		  // don't continue index is wrong
		  return NULL;
		}
	      // add a new element in an existing list => add at end.
	      if ( tail->link[rel_index].next != NULL)
		{
		  // :-( there is already an element after end of list ...
		  ++list->errors;
		}
	      if ( ( tail->link[rel_index].memberof == NULL)
		   || (tail->link[rel_index].memberof != list) )
		{
		  ++list->errors;
		}
	      if (list->errors == 0)
		{
		  tail->link[rel_index].next=element;
		  element->link[list->membership_id].previous=tail;
		  list->tail=element;
		  ++list->count;
		}
	      else
		{
		  return NULL;
		}
	    }
	  else
	    {
	      return NULL;
	    }
	}
      element->link[list->membership_id].memberof=list;
    }
  else
    {
      if ( previouslist != list )
	{
	  // list membership_id conflicting.
	  ++list->errors;
	  return NULL;
	}
    } 
  return element;  
}

int allistelement_is_in(struct allistelement * element, struct allistof * list)
{
  int rel_index = allistelement_getrelindex(element, list->membership_id);
  if ( rel_index > list->membership_id )
    {
      return 0; // should be -1 or something indicating an error.
    }
  return element->link[rel_index].memberof == list;
}

struct allistelement * move_to_offset( struct allistelement * current,
		int offset,
		int absindex)
{
  if ( current == NULL )
    {
      return current;
    }     
  if ( offset > 0 )
    {
      for (int j=0; (current !=NULL) && (j< offset); j++)
	{	  
	  current=current->link[allistelement_getrelindex(current,absindex)].next;
	}
    }
  else if ( offset < 0 )
    {
      for (int j=0; (current !=NULL) && (j>offset); j--)
	{
	  current=current->link[allistelement_getrelindex(current,absindex)].previous;
	}
    }
  return current;
}

void * allist_for_each(struct allistof * list,
		       struct allistelement * start,
		       void * (*callback) (struct allistof * list, struct allistelement * element, struct allistelement * next, int count, void * param),
		       void * param,
		       int step,
		       int offset)
{
  void * result = NULL;
  struct allistelement * current = NULL;
  struct allistelement * walk = NULL;
  int absindex = 0;
  if ( list == NULL)
    {
      return NULL;
    }
  absindex = list->membership_id;
  if ( start == NULL )
    {
      if ( step < 0 )
	{
	  current = list->tail;
	}
      else
	{
	  current = list->head;
	}
    }
  else
    {
      current = start;
    }
  // move to offset
  current = move_to_offset(current, offset, absindex);
  for (int i=0; (current != NULL) && ( i<list->count) ; i++)
    {
      walk=current;
      
      current = move_to_offset(current, step, absindex);
      
      result = callback(list, walk, current, i, param);
      if ( result == NULL )
	{
	  break;
	}
      else
	{
	  param=result;
	}
      if ( walk == current )
	{
	  // walking to very same than the one previoulsy walked.
	  break;
	}
    }
  return param;
}
