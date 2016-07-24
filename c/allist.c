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
  if ( pabs >= INDEXSET_COUNT )
    {
      fprintf(stderr,"[ERROR] wrong membership abs for indexset %i %s:%i", pabs, __func__,__LINE__);
      return 0;
    }
  return ( (indexset->set & (1L << pabs)) != 0 );
}

int indexset_reset(struct indexset * indexset, int pabs)
{
  if ( pabs >= INDEXSET_COUNT )
    {
      fprintf(stderr,"[ERROR] wrong membership abs for indexset %i %s:%s", pabs, __func__,__LINE__);
      return 0;
    }
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
  if ( pabs >= INDEXSET_COUNT )
    {
      fprintf(stderr,"[ERROR] wrong membership abs for indexset %i %s:%s", pabs, __func__,__LINE__);
      return 0;
    }
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

int indexset_count(struct indexset * indexset)
{
  int p1=0;
  int p2=0;
  p1 = popcount((unsigned int) (indexset->set >>32));
  p2 = popcount((unsigned int) indexset->set) ;
  return p1 + p2;

}

/** from an absolute index get relative index 
 * this is used if shrunk
 * == number of bits set to 1 bellow position ( minus one : index starts at 0 )
 */
int indexset_getrelindex(struct indexset * indexset, int pabs)
{
  unsigned long long set = indexset->set;
  if ( debug>1) {fprintf(stderr,"get relindex %p %i set 0x%lx\n",indexset, pabs, set);}
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
    p1 = popcount( ((unsigned int) (set >>32)) & ( ((unsigned int) 0xffffffff) >> (63-pabs) ));
    p2 = popcount((unsigned int) set) ;
  } else {
    p1 = popcount(set & (((unsigned int)(0xffffffff)) >> (31-pabs))) ;
  }
  if (debug>2) {     fprintf(stderr,"indexset %lx %i %i %i \n", indexset->set, pabs, p1,p2); }
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
  fprintf(where,"indexset count %i set 0x%lx\n",indexset->count,  indexset->set);
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

int allistelement_release(struct allistelement * this)
{
  if ( this != NULL )
    {
      if ( (this->flags & ALLIST_MALLOC) != 0 )
	{
	  // make it clearly wrong to prevent reuse and help debugging
	  this->memberships=-1;
	  this->data= (void *) 0xdeadbeefL;
	  this->extlink= (void *) 0xdeadbeefL;
	  this->link[0].next= (void *) 0xdeadbeefL;
	  this->link[0].previous= (void *) 0xdeadbeefL;
	  free(this);
	  return 0;
	}
      else
	{
	  return -1;
	}
    }
  else
    {
      return -2;
    }
}
int allistelement_is_ext(struct allistelement * element, int membership)
{
  // TODO should check in element
  return ( membership > INDEXSET_COUNT );
}

int allistelement_has_ext(struct allistelement * element)
{
  return (element->flags & ALLIST_EXT) != 0;
}

int allistelement_is_shrunk(struct allistelement * element)
{
  if ( element == NULL )
    {
      fprintf(stderr, "NULL element in %s:%i \n", __func__,__LINE__);
      return -1;
    }
  return (element->flags & ALLIST_SHRUNK) != 0;
}
  
int allistelement_getrelindex( struct allistelement * current, int absindex)
{
  int relindex = 0;
  if ( current == NULL )
    {
      fprintf(stderr, "NULL element in %s:%i \n", __func__,__LINE__);
      return -1;
    }  
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

int allistelement_get_memberships_ext(struct allistelement * this)
{
  struct allistextlink * ext = this->extlink;
  int extmemberships = 0;
  while ( ext != NULL )
    {
      int count =0;
      for (int i=0; i<INDEXSET_COUNT; i++)
	{
	  if ( ext->link[i].memberof != NULL )
	    {
	      ++count;
	    }
	}
      if ( count != indexset_count( &ext->indexset))
	{
	  fprintf(stderr, "indexset mismatch %s:%i %i!=%i 0x%lx\n", __func__,__LINE__,count,indexset_count( &ext->indexset),ext->indexset.set);
	  for (int i=0; i<INDEXSET_COUNT; i++)
	    {
	      if ( ext->link[i].memberof != NULL )
		{
		  fprintf(stderr, "[DEBUG] ext %p first %i\n", ext, ext->first);
		  fprintf(stderr, "[DEBUG] indexset membership %p %i\n", ext->link[i].memberof, ext->link[i].memberof->membership_id);
		  fprintf(stderr, "[DEBUG] this %p %p\n", this, this->data);
		}
	    }
	}
      extmemberships+=count;
      ext = ext->nextextlink;
    }      
  return extmemberships;
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


/**
  create a context for a maximum of  memberships count of allists
 */
struct allistcontext * new_allistcontext(int memberships)
{
  int newlength = sizeof(struct allistcontext) + sizeof(struct allistof) * (memberships-1);
  struct allistcontext * allocated = calloc(1, newlength);
  if ( allocated != NULL )
    {
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
struct allistof * allistcontext_get_membership(struct allistcontext * context, int membership)
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

struct allistextlink * allistelement_get_extlink_ext(struct allistelement * element, struct allistof * list)
{
  int membership = list->membership_id;
  int error = 0;
  if ( (element->flags & ALLIST_EXT) != 0 )
    {
      struct allistextlink * ext = element->extlink;
      while ( ext != NULL )
	{
	  if ( ( membership >= ext->first )  && ( membership < (ext->first + INDEXSET_COUNT) ))
	    {
	      // found right ext chunk
	      return ext;
	    }
	  ext = ext->nextextlink;
	}      
      return NULL;
    }
  return NULL;
}
  
struct allistlink * allistelement_get_link_ext(struct allistelement * element, struct allistof * list)
{  
  int membership = list->membership_id;
  struct allistextlink * ext = allistelement_get_extlink_ext(element,list);
  if ( ext != NULL )
    {
      if ( ( membership >= ext->first )  && ( membership < ( ext->first + INDEXSET_COUNT) ) )
	{
	  // found right ext chunk
	  struct allistlink * link = &ext->link[ membership - ext->first];
	  if ( link->memberof == NULL )
	    {
	      return NULL;
	    }
	  if ( link->memberof == list )
	    {
	      return link;
	    }
	  else
	    {
	      // error
	      if (debug) {fprintf(stderr,"ext membership of element %p set to another list %p!=%p \n", element, list, link->memberof);}
	      ++ list->errors;
	    }
	}
    }
  else
    {
      if (debug) {fprintf(stderr,"requested ext link for element  %p list %p , no membership\n", element, list);}
    }
  return NULL;
}

int allistelement_is_in_ext(struct allistelement * element, struct allistof * list)
{
  return (allistelement_get_link_ext(element, list) != NULL);
}

struct allistlink * get_link( struct allistelement * current,struct allistof * list)
{
  if ( allistelement_is_ext(current, list->membership_id) )
    {
      return allistelement_get_link_ext(current, list);
    }
  else
    {
      int absindex = list->membership_id;
      return &current->link[allistelement_getrelindex(current,absindex)];
    }
}

struct allistelement * walk_one( struct allistelement * current,
			     struct allistof * list,
			     int direction)
{
  struct allistlink * link = get_link(current, list);
  if ( link != NULL )
    {
      if ( direction > 0 )
	{
	  current=link->next;
	}
      else if ( direction < 0)
	{
	  current=link->previous;
	}
    }
  else
    {
      current = NULL;
    }
  return current;
}

/** add  in extended part 
 */
struct allistelement * allistelement_add_in_ext(struct allistelement * element, struct allistof * list)
{
  struct allistlink * link = NULL;
  struct allistextlink * ext = allistelement_get_extlink_ext(element, list);
  int membership = list->membership_id;
  if (ext == NULL)
    {
      // should add it
      // allocate a new and link it at head
      ext = calloc(1, sizeof( struct allistextlink));
      if ( ext != NULL )
	{
	  ext->nextextlink=element->extlink;
	  element->extlink=ext;
	  element->flags|=ALLIST_EXT;
	  ext->first=membership - (membership % INDEXSET_COUNT);
	  if (debug) {fprintf(stderr,"allocate ext link %p element.data %p first %i membership %i\n", ext, element->data, ext->first, membership);}
	}
      else
	{
	  if (debug) { fprintf(stderr,"memory allocation failure for allistextlink %i bytes. memory shortage !\n", sizeof( struct allistextlink));}
	  return NULL;
	}
    }

  if ( ext != NULL )
    {
      if ( ( membership >= ext->first )  && ( membership < (ext->first + INDEXSET_COUNT)) )
	{
	  // found right ext chunk
	  struct allistlink * link = &ext->link[ membership - ext->first];	  
	  if ( link->memberof == NULL )
	    {
	      if ( link->next != NULL )
		{
		  if (debug) { fprintf(stderr,"[ERROR] element %p in list %p has no membership but a next set\n", element, list, link->next);}
		}
	      if (debug) { fprintf(stderr,"add ext %p in list %p list->tail %p \n", ext, list, list->tail);}
	      // should add it.
	      if ( list->tail != NULL )
		{
		  struct allistelement * previous = list->tail;
		  struct allistlink * plink = NULL;
		  if ( list->head == NULL)
		    {
		      fprintf(stderr,"[ERROR] list %p with tail %p set but with a head null\n", list, list->tail);
		      ++ list->errors;
		      list->head = element;
		    }
		  //  struct allistlink * plink = get_link(previous, list);
		  if ( allistelement_is_ext(previous, list->membership_id) )
		    {
		      if (allistelement_has_ext(previous))
			{
			  plink = allistelement_get_link_ext(previous, list);
			}
		      else if ( allistelement_is_shrunk(previous))
			{
			  plink = &previous->link[allistelement_getrelindex(previous,list->membership_id)];
			}
		      else
			{
			  fprintf(stderr,"[ERROR] element %p in list %p for membership > %i neither extended or shrunk \n", previous, list, INDEXSET_COUNT);
			}
		    }
		  else
		    {
		      plink = &previous->link[allistelement_getrelindex(previous,list->membership_id)];
		    }
		  if ( plink != NULL )
		    {
		      if ( plink->next == NULL )
			{
			  plink->next=element;
			}
		      else
			{
			  fprintf(stderr,"[ERROR] list %p with tail %p set and tail next %p non null\n", list, list->tail, plink->next);
			  ++ list->errors;
			}		      
		    }
		  else
		    {
		      if (debug) {
			fprintf(stderr,"[WARNING] plink NULL  ext %p in list %p\n", ext, list);
			fprintf(stderr,"[DEBUG] ext first %i in list previous.flags %x \n", ext->first, previous->flags);
			fprintf(stderr,"[DEBUG] list membership %i ext ? %i\n", list->membership_id, allistelement_is_ext(previous, list->membership_id));
		      }
		    }
		  link->previous=list->tail;
		}
	      else
		{
		  // if tail is null then list is empty, head should be null too
		  if ( list->head != NULL)
		    {
		      fprintf(stderr,"list %p with null tail but with a head set %p\n", list, list->head);
		      ++ list->errors;
		      // leave it in dangling state...
		    }
		  else
		    {
		      list->head=element;
		    }
		}		
	      indexset_set(&ext->indexset,(membership - ext->first));
	      list->tail=element;
	      link->memberof=list;	      
	      ++list->count;
	    }
	  if ( link->memberof == list )
	    {
	      return element;
	    }
	  else
	    {
	      // error
	      if (debug) {fprintf(stderr,"[ERROR] element %p ext %p already in list %p membership %i\n",element, ext, list, membership);}
	      ++ list->errors;
	    }
	}
      else
	{
	  if (debug) {fprintf(stderr,"[ERROR] ext %p first %i in wrong membership %i\n",ext, ext->first, membership);}
	}
    }
  return NULL;
}
  
struct allistelement * allistelement_add_in(struct allistelement * element, struct allistof * list)
{
  struct allistof * previouslist = NULL;
  struct allistelement * tail = list->tail;

  if (list->errors > 0 )
    {
      if (debug) {fprintf(stderr,"[WARNING] can't add an element %p in list %p have errors %i\n", element, list, list->errors);}
      return NULL;
    }
  if (element == NULL)
    {
      return NULL;
    }
  if ( allistelement_is_ext( element, list->membership_id))
    {
      if (debug) {fprintf(stderr,"add_in_ext element  %p membership %i \n", element, list->membership_id);}
      return allistelement_add_in_ext(element,list);
    }
  
  previouslist = element->link[allistelement_getrelindex(element,list->membership_id)].memberof;
  
  // was not a member of this list
  if ( previouslist == NULL )
    {      
      // empty list
      if (allistelement_is_shrunk(element) )
	{
	  // can't add an element already shrunk into a new list.
	  if (debug) {fprintf(stderr,"[WARNING] can't add an element %p already shrunk into a new list %p.\n", element, list);}
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
  int membership = list->membership_id;
  if ( allistelement_is_ext(element, membership) )
    {
      int rel_index = allistelement_getrelindex(element, list->membership_id);
      if ( rel_index > list->membership_id )
	{
	  return 0; // should be -1 or something indicating an error.
	}
      return element->link[rel_index].memberof == list;
    }
  else
    {
      return allistelement_is_in_ext(element, list);
    }
}


struct allistelement * walk_steps( struct allistelement * current,
		int offset,
		struct allistof * list)
{
  if ( current == NULL )
    {
      return current;
    }
  if ( offset > 0 )
    {
      for (int j=0; (current !=NULL) && (j< offset); j++)
	{	  
	  current=walk_one(current,list,offset);
	}
    }
  else if ( offset < 0 )
    {
      for (int j=0; (current !=NULL) && (j>offset); j--)
	{
	  current=walk_one(current,list,offset);
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
  if ( list == NULL)
    {
      return NULL;
    }
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
  current = walk_steps(current, offset, list);
  for (int i=0; (current != NULL) && ( i<list->count) ; i++)
    {
      walk=current;
      
      current = walk_steps(current, step, list);
      
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

/*
 shrink allistelement to take a minimal size
 WARNING this rewrite full linkage ( next previous head tail ).
 */
struct allistelement * allistelement_shrink(struct allistelement * this, struct shrunkinfo * shrunkhealth)
{
  int shrunkerror = 0;
  int nowayback = 0;
  int extmemberships = 0;

  if ( this == NULL)
    {
      fprintf(stderr, "NULL element in %s:%i \n", __func__,__LINE__);
      return NULL;
    }
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

  if (allistelement_has_ext(this))
    {
      extmemberships =  allistelement_get_memberships_ext(this);;
      if ( debug) { fprintf(stderr,"shrink of element that has extended memberships . %p %i.\n", this, extmemberships);}
    }
  // current limit of indexset
  if ( allistelement_is_ext(this, memberships) )
    {
      // TODO support ext
      if ( debug) { fprintf(stderr,"no shrink if requiring extended memberships . %p %i TODO \n", this, memberships);}
      return this;
    }

  int oldlength = sizeof(*this) + sizeof(this->link) * (memberships-1);
  int newlength = oldlength + sizeof(this->link) * extmemberships;

  struct allistelement * shrunk = malloc(newlength);
  if ( shrunk != NULL )
    {
      int shrunkpos = 0;
      memcpy(shrunk,this,newlength);
      // reset pointers to shrunk
      // i is absolute ( in this ) that is >= relative
      // shrunkpos is relative, in shrunk
      if ( debug > 1 ) {fprintf(stderr,"this %p shrunk %p memberships %i\n",this, shrunk, this->memberships);}	  
      for (int i=0; (i < this->memberships) && (shrunkerror==0); i++)
	{
	  if ( this->link[i].memberof != NULL )
	    {
	      if ( i < INDEXSET_COUNT )
		{
		  if ( indexset_set(&shrunk->indexset, i) == 0)
		    {
		      // was already set
		      if (debug) {fprintf(stderr,"indexset already set for %i",i);}
		    }
		}
	    }
	  else
	    {
	      if ( i < INDEXSET_COUNT )
		{
		  if ( indexset_reset(&shrunk->indexset, i) != 0 )
		    {
		      // was set
		      if (debug) {fprintf(stderr,"indexset set for %i ( should be 0) ",i);}
		    }
		}
	      // we are not interested.
	      continue;
	    }
	  if ( debug > 1 ) {fprintf(stderr,"this %p shrunk %p indexset %lu \n",this, shrunk, shrunk->indexset.set);}
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

	  if (( next !=NULL) && (allistelement_has_ext(next)))
	    {
	      if (debug) {fprintf(stderr,"next %p is extended\n", next );}
	    }

	  if (( previous !=NULL) && (allistelement_has_ext(previous)))
	    {
	      if (debug) {fprintf(stderr,"previous %p is extended\n", previous );}
	    }

	  if ( this->link[i].next != next )
	    {
	      if (debug) {fprintf(stderr,"shrunk %p and this %p next differs\n", this->link[i].next, next );}
	      ++ shrunkerror;
	    }

	  if ( this->link[i].previous != previous )
	    {
	      if (debug) {fprintf(stderr,"shrunk %p and this %p previous differs\n", this->link[i].previous , previous );}
	      ++ shrunkerror;
	    }

	  if (debug)
	    {
	      fprintf(stderr,"%i, %i, %p %p\n",i, shrunkpos, shrunk->data, shrunk->link[shrunkpos].memberof);
	      fprintf(stderr,"Next %p %p\n", next, (next !=NULL) ? next->data : NULL);
	      fprintf(stderr,"Previous %p %p\n", previous, (previous !=NULL) ? previous->data : NULL);
	    }
	  ++shrunkpos;
	}

      memberships = shrunkpos;

      // should copy used extended links.
      if ( (shrunkpos < (memberships + extmemberships)) && (shrunkerror==0) )
	{
	    struct allistextlink * ext = this->extlink;
	    int count =0;
	    while ( ext != NULL )
	      {
		for (int i=0; i<INDEXSET_COUNT; i++)
		  {
		    if ( ext->link[i].memberof != NULL )
		      {
			++count;
			memcpy(&shrunk->link[shrunkpos], &ext->link[i], sizeof(shrunk->link[1]));
			++shrunkpos;
		      }
		  }
		ext = ext->nextextlink;
	      }
	    if ( shrunkpos != (memberships + extmemberships) )
	      {
		fprintf(stderr,"[ERROR] unexpected membership size %s:i %i!=%i %i %i",__func__,__LINE__,shrunkpos,(memberships + extmemberships), count, extmemberships);
		shrunkerror++;		
	      }	    
	}

      // free ext links
      if ( shrunkerror == 0 )
	{
      	    struct allistextlink * ext = this->extlink;
	    struct allistextlink * next = this->extlink;
	    while ( ext != NULL )
	      {
		next = ext->nextextlink;		
		++nowayback;
		ext->nextextlink = (void *) 0xdeadbebf;
		free(ext);
		ext=next;
	      }
	}

      // get all links to 'this' element and change them to 'shrunk'
      for (shrunkpos=0; (shrunkpos < (memberships + extmemberships) ) && (shrunkerror==0); shrunkpos++)
	{

	  int i = shrunk->link[shrunkpos].memberof->membership_id;
	  struct allistelement * next = shrunk->link[shrunkpos].next;
	  struct allistelement * previous = shrunk->link[shrunkpos].previous;

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
		  if (debug) { fprintf(stderr, "head of owning list mismatch %p != %p %p \n", shrunk->link[shrunkpos].memberof->head, this, shrunk );}
		  ++shrunkerror;
		}	     
	    }
	  else
	    {
	      struct allistlink * plink = get_link(previous,shrunk->link[shrunkpos].memberof);
	      if ( plink != NULL )
		{
		  // replace this by shrunk
		  if ( plink->next == this )
		    {
		      plink->next=shrunk;
		      ++nowayback;
		    }
		  else
		    {
		      if (debug) { fprintf(stderr, "previous.next membership was not pointing on this %p != %p \n", i, plink->next, this);}
		      ++shrunkerror;
		    }		  
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
		  if (debug) { fprintf(stderr, "tail of owning list mismatch  %p != %p %p \n", shrunk->link[shrunkpos].memberof->tail, this, shrunk );}
		  ++shrunkerror;
		}
	    }
	  else
	    {
	      struct allistlink * nlink = get_link(next,shrunk->link[shrunkpos].memberof);
	      if ( nlink != NULL )
		{
		  // replace this by shrunk
		  if ( nlink->previous == this )
		    {
		      nlink->previous=shrunk;
		      ++nowayback;
		    }
		  else
		    {
		      if (debug) { fprintf(stderr, "next.previous %i was not pointing on this %p != %p \n", i, nlink->next, this);}
		      ++shrunkerror;
		    }		  
		}
	    }
	}
      shrunk->flags|= ALLIST_SHRUNK;
      shrunk->flags&= ~ALLIST_EXT;
      shrunk->memberships=shrunkpos;
      shrunk->extlink=NULL;
    }
  if ( shrunkerror > 0 )
    {
      if ( shrunk != NULL )
	{
	  if ( nowayback == 0 )
	    {
	      free(shrunk);
	    }
	  // corrupted people should not be freed.
	  if ( debug ) { fprintf(stderr,"corrupted links remains to this allocated space, should not free %p\n", shrunk);}
	}
      shrunk = NULL;
    }
  if ( shrunkhealth != NULL )
    {
      shrunkhealth->shrunkerrors=shrunkerror;
      shrunkhealth->nowayback=nowayback;
    }
  if ( shrunk != NULL )
    {
      if ( debug > 1 ) {fprintf(stderr,"RETURN this %p shrunk %p indexset %lu \n",this, shrunk, shrunk->indexset.set);}
    }
  return shrunk;      
}
