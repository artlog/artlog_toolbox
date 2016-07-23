/**
   A multiple list of elements that could belong to multiple list.
   every element knows its membership.
*/

struct allistof;
struct allistelement;

struct allistlink {
  struct allistelement * next;
  struct allistelement * previous;
  void * data; // specific data for this membership
  struct allistof* memberof;
};

struct indexset {
  unsigned long long set;
  unsigned char count;
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
  struct indexset indexset; // what indexes in global membership it belongs to
  /**
    when a call to shrink is done, it is no more possible to add membership
    all links will be used and shrunk will be set to 1.
    at creation shrunk is 0, and there is a link for every possible membership
    some links can be NULL
   */
  int shrunk; 
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
struct allistcontext * new_allistcontext(unsigned short memberships);

/**
  create a new list into context, within context use first free membership
 */
struct allistof * new_allistof(struct allistcontext * context);

/**
  find list with membership id within this context
 */
struct allistof * allistcontext_get_membership(struct allistcontext * context, unsigned short membership);

struct allistelement * allistcontext_new_allistelement(struct allistcontext * context, void * data);

struct allistelement * allistelement_add_in(struct allistelement * element, struct allistof * list);

/**
return non 0 value is element is in list.
 */
int allistelement_is_in(struct allistelement * element, struct allistof * list);

/*
 shrink allistelement to take a minimal size
 WARNING this rewrite full linkage ( next previous head tail ).
 */
struct allistelement * allistelement_shrink(struct allistelement * this, struct shrunkinfo * shrunkinfo);

int allist_set_debug( int d);
