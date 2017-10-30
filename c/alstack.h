#ifndef __ALSTACK_HEADER__
#define __ALSTACK_HEADER__

/* alstack stack contract to cover reference or long */

#define ALSTACK_CHUNK 4096

struct alstackelement {
  void * reference;
};

enum alstack_flag {
  ALSTACK_FULL_FLAG = 1, // chunk is full
  ALSTACK_NEXT_FLAG = 2, // next chunk is allocated and valid
  ALSTACK_DYNAMIC_FLAG = 4, // this is a dynamic allocation
  ALSTACK_VALID_FLAG = 8, // this is a valid stack ( should always be set ).
  ALSTACK_PREVIOUS_FLAG = 16 // previous chunk exists and is valid.
};
  
struct alstack {
  int size; // maximum number of elements in this chunk
  int index; // current FREE index in this element array chunk
  int flags;
  int reserved; // to make people argue why is there a reserved field.
  struct alstack * next; // relevant if ALSTACK_NEXT_FLAG is set
  struct alstack * previous; // relevant if ALSTACK_PREVIOUS_FLAG is set
  struct alstackelement element[ALSTACK_CHUNK];
};

/*
setup a alstack, content of alstack is considered irrelevant (ie can be zeroed)
init_flags defines wether this is a static or dynamic stack ( initial flags)
*/
struct alstack * alstack_init(struct alstack * stack, int init_flags);

/*
allocate dynamically first chunk for alstack
will call init on alstack
*/
struct alstack * alstack_allocate();

/* push a pointer to an element on the stack */
struct alstackelement * alstack_push_ref(struct alstack * stack, void * reference );

/* pop an element from the stack */
struct alstackelement * alstack_pop(struct alstack * stack);

/* pop all elements and call a function on them 
return number of elements poped
*/
int alstack_popall(struct alstack * stack, int (*callback)(struct alstackelement * stackelement));

/* destroy stack ( and all next stack up to tail ) 
return number of element unlinked
*/
int alstack_destroy(struct alstack * stack, int (*unlink_element)(struct alstackelement * stackelement));

#endif
