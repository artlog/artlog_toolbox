/**
   A multiple list of elements that could belong to multiple list.
   every element knows its membership.
*/

#define INDEXSET_COUNT 64

struct allistof;
struct allistelement;

enum allistelement_flags {
  ALLIST_SHRUNK = 1,
  ALLIST_EXT = 2, // has extended links
  ALLIST_MALLOC = 4 // was created with malloc, can be shrunk in-situ
};

struct allistlink {
  struct allistelement * next;
  struct allistelement * previous;
  void * data; // specific data for this membership
  struct allistof* memberof;
};

/**
 membership from 0 to INDEXSET_COUNT -1
 */
struct indexset {
  unsigned char count;
  unsigned long long set;
};

/**
 supports memberships id links from offset to offset + INDEXSET_COUNT -1
*/
struct allistextlink {
  int first; // should be a multiple of INDEXSET_COUNT
  struct indexset indexset;
  struct allistextlink * nextextlink;
  struct allistlink link[INDEXSET_COUNT];
};

struct allistelement {
  /**
    maximum number of memberhsip this element can have
    this is number of elements in link.
   */
  int memberships;
  void * data;
  /**
     indexset global index can be found from
     link[i].memberof->membership_id
     or
     indexset_getabsindex(indexset,i);
  */
  struct indexset indexset; // what indexes in global membership it belongs to, currently limited to first 64 memberships
  /**
    when a call to shrink is done, it is no more possible to add membership
    all links will be used and flag ALLIST_SHRUNK will be set.
    at creation shrunk is 0, and there is a link for every possible membership
    some links can be NULL
    
    ALLIST_EXT allows to indicate extlink is in use
   */
  unsigned short flags;
  struct allistextlink * extlink;
  struct allistlink link[1]; // link on next element in membership growable in fact [memberships];
};

/*
 a list of members
*/ 
struct allistof {
  struct allistelement * head; // head of list
  struct allistelement * tail; // tail of list
  int count; // number of elements in this list
  int membership_id; // this list link membership absolute index
  int errors; // number of errors ( one is enough, corrupted data )
};

struct allistcontext {
  unsigned int membership_reservation;
  unsigned int next_membership;
  struct allistof list[1]; // growable
};

struct shrunkinfo
{
  int shrunkerrors; // if set it means shrunk failure
  int nowayback; // if set it means pointer outside have been modified
};

/**
  create a context for a maximum of  memberships count of alllists
 */
struct allistcontext * new_allistcontext(int memberships);

/**
  create a new list into context, within context use first free membership
 */
struct allistof * new_allistof(struct allistcontext * context);

/**
  find list with membership id within this context
 */
struct allistof * allistcontext_get_membership(struct allistcontext * context, int membership);

struct allistelement * allistcontext_new_allistelement(struct allistcontext * context, void * data);

struct allistelement * allistelement_add_in(struct allistelement * element, struct allistof * list);

/**
return non 0 value is element is in list.
 */
int allistelement_is_in(struct allistelement * element, struct allistof * list);

/*
 shrink allistelement to take a minimal size
 WARNING this rewrite full linkage ( next previous head tail ).
 if element is shrunk then previous should be released.
 */
struct allistelement * allistelement_shrink(struct allistelement * this, struct shrunkinfo * shrunkinfo);

int allistelement_release(struct allistelement * this);

int allist_set_debug( int d);


int indexset_get(struct  indexset * indexset, int pabs);

int indexset_reset(struct indexset * indexset, int pabs);

int indexset_set(struct indexset * indexset, int pabs);

int allistelement_get_all_memberships(struct allistelement * this);

/** for each element of list call callback 
  first call is with param, next call are with previous result of callback
  if callback return NULL then for each stops
  start element where to start WARNING it MUST be withing list, if NULL, head or tail of list will be used.
  step : > 0 forward from head
         < 0 backward from tail
  offset : 0 starts at element
           > 0 starts at nth element
           < start at previousth element
*/
void * allist_for_each(struct allistof * list,
		       struct allistelement * start,
		       void * (*callback) (struct allistof * list, struct allistelement * element, struct allistelement * next, int count, void * param),
		       void * param,
		       int step,
		       int offset
		       );
