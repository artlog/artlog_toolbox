#include "alabnf_matcher.h"
#include <stdio.h>
#include <strings.h>
#include "aldebug_output.h"

#include <stdlib.h>


/**

provide a alabnf_match funciton that wil lparse a file according to given (already parsed) ABNF syntax

**/

const void * ALABNF_DEAD_CANARY = (void *) 0xdeadca01;

// to read alternatives ...
const int ALABNF_READBLOCKSIZE = 4096;

static char descr[30];
static char state_descr[255];

#define ALABNF_MATCHER_LOG_TEXT_STATE(loglevel,achar,text,state) aldebug_printf(DBGSTREAM,"%s %s %s state %s  in %s %s %i\n",loglevel,text,alabnf_matcher_get_descr(achar),alabnf_matcher_get_state_descr(state),__FILE__,__func__,__LINE__)


#define ALABNF_MATCHER_DEBUG_TEXT_STATE(achar,text,state) ALABNF_MATCHER_LOG_TEXT_STATE("[DEBUG]",achar,text,state)

#define ALABNF_MATCHER_FATAL_TEXT_STATE(achar,text,state) ALABNF_MATCHER_LOG_TEXT_STATE("[FATAL]",achar,text,state)

#define ALABNF_MATCHER_LIMIT_STEPS(matcher,state) \
  {\
	matcher->steps++;\
	if ( matcher->steps > matcher->maxsteps )\
	  {\
            ALABNF_MATCHER_FATAL_TEXT_STATE(NULL,"TOO MANY STEPS",state); \
	    exit(1);\
	  }\
  }

int alabnf_fill_rule_info(char * rule_descr, int max, struct alabnf_rule * current_rule)
{
  int prefix=0;
  if ( current_rule != NULL )
    {
      aldatablock * datablock = &current_rule->rule_name.strbloc;
      if (current_rule->rule_name.type == ALABNF_ST_RULENAME )
	{		
	  prefix=snprintf(rule_descr,max, ALPASCALSTRFMT,
			  ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
	}
      else
	{
	  prefix=snprintf(rule_descr,max,"#ERR_T%iS%i",current_rule->rule_name.type,datablock->length);
	}	  	  
    }

  return prefix;
}

// forward declaration
struct alabnf_matcher_state * alabnf_matcher_create_child_state(
								   struct alabnf_matcher_state * parent,
								   struct alabnf_node * node);


int alabnf_matcher_is_terminal(struct alabnf_node * node)
{
  if ( node != NULL )
    {
      switch (node->type)
	{
	case ALABNF_NT_INVALID:
	case ALABNF_NT_STRING:
	case ALABNF_NT_RANGE:
	  return 1;
	case ALABNF_NT_RULE_REF:
	case ALABNF_NT_ITERATOR:
	case ALABNF_NT_SEQUENCE:
	case ALABNF_NT_ALT:
	default:
	  return 0;
	}
    }
  return 1;

}

int alabnf_matcher_state_is_terminal(struct alabnf_matcher_state * state)
{
  struct alabnf_node * initial = state->initial_node;

  return alabnf_matcher_is_terminal(initial);
}

char * alabnf_matcher_get_descr(alabnf_character * achar)
{
  if ( achar != NULL)
    {
      char c = achar->uchar;
      snprintf(descr,30,"('%i','%c')",c,c>=32 ? c : '.');
    }
  else
    {
      snprintf(descr,30,"(NULL char)");
    }
  return descr;
}

char * alabnf_matcher_get_state_descr(const struct alabnf_matcher_state * state)
{
  if ( state != NULL)
    {
      int prefix = alabnf_fill_rule_info(state_descr,100,state->current_rule);
      snprintf(state_descr+prefix,255-prefix,"('%p' T%i IT%i current %p current_type %i index %i)",state,state->type,state->initial_node == NULL ? -1 : state->initial_node->type,state->current_node,state->current_node !=NULL ? state->current_node->type : -1,state->datablock_index);
    }
  else
    {
      snprintf(state_descr,255,"(NULL state)");
    }
  return state_descr;
}


void alabnf_set_matcher_state_input(struct alabnf_matcher * matcher, struct alabnf_matcher_state * state)  
{
  ALABNF_MATCHER_DEBUG_TEXT_STATE(&matcher->tempchar1,"set matcher state",state);
  
  matcher->current_state = state;
  if ( state != NULL )
    {
      matcher->current_input = state->input;
    }
}

struct alabnf_matcher_state * alabnf_matcher_state_alloc()
{
  struct alabnf_matcher_state * state = malloc(sizeof(*state));
  aldebug_printf(DBGSTREAM,"[DEBUG] matcher state alloc %p at %s:%s:%i\n",state,__FILE__,__func__,__LINE__);
  return state;
}

void alabnf_matcher_state_free(struct alabnf_matcher_state * state)
{
  if ( state != NULL )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"matcher state free",state);
      if ( state->current_node == ALABNF_DEAD_CANARY  )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] matcher state free canary hit in %s:%s:%i",__FILE__,__func__,__LINE__);
	  return;
	}
      struct alinputstream * stream = state->input;      
      if ( (stream != NULL) && ( stream->type == ALINPUTSTREAM_TYPE_SHARED_CHILD ) )
	{
	  aldebug_printf(DBGSTREAM,"[DEBUG] free inputstream %p  in  %s:%s:%i",
			 stream,
			 __FILE__,__func__,__LINE__);
	  // FIXME DEACTIVATED
	  // alinputstream_free_shared(stream);
	}
      state->current_node = ALABNF_DEAD_CANARY;
      free(state);
    }
}

void alabnf_matcher_init_state(
			       struct alabnf_matcher_state * state,
			       struct alabnf_matcher_state * parent,
			       struct alabnf_node * node)
{
  if ( state != NULL )
    {
      aldebug_printf(DBGSTREAM,"[DEBUG] init state %p parent %p node %p node->type %i\n",state,parent,node,(node == NULL) ? -1 : node->type);
      // right type will be set by alabnf_specialize
      state->type=ALABNF_MATCHER_ST_UNSET;
      state->parent = parent;
      state->initial_node = node;
      state->current_node = node;
      state->next_sequence = NULL;
      state->datablock_index = 0;
      if ( parent != NULL )
	{
	  state->current_rule = parent->current_rule;
	}
      else
	{
	  state->current_rule = NULL;
	}
      if (node == NULL )
	{
	  aldebug_printf(DBGSTREAM,"[FATAL] init state %p parent %p node NULL\n",state,parent);
	}
    }
}


// set current state to parent state.
int alabnf_matcher_unstack(struct alabnf_matcher * matcher,
			   struct alabnf_matcher_state * state,
			   struct alabnf_matcher_state * parent)
{
  if ( state != parent )
    {
      printf("^");      
      // CHECKME looks fragile, required for memory leak protection
      if ( state != &matcher->root_state )
	{
	  struct alabnf_node * node = state->initial_node;
	  aldebug_printf(DBGSTREAM,"[DEBUG] unstack state for initial node %i at %s:%s:%i\n",
			 node->type,
			 __FILE__,__func__,__LINE__);
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"unstack free",state);
	  alabnf_matcher_state_free(state);
	}
      // unstack
      alabnf_set_matcher_state_input(matcher,parent);

      return 1;
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[FATAL] parent point on itself at %s:%s:%i\n",__FILE__,__func__,__LINE__);
      return 0;
    }
}

// will stack parent alternative to point on next  and process current node one.
// return new state for alternative NULL on failure ( invalid alternative )
struct alabnf_matcher_state *
alabnf_matcher_process_next_alternative(
				       struct alabnf_matcher_state * state,
				       struct alabnf_alternative * alternative
				       )
  {
    ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"process alt",state);

    if ( state->type != ALABNF_MATCHER_ST_OR )
      {
	ALABNF_MATCHER_FATAL_TEXT_STATE(NULL,"state %p non ALABNF_MATCHER_ST_OR",state);

	return NULL;
      }

    struct alabnf_node * node = alternative->node;
    // current_node is NULL meaning we have to walk next alternative for this state.
    state->current_node = node;
    // new alternative is next one.
    state->alt=alternative->alt;
    if ( node != NULL )
      {
	struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, node);
	// current state should remember stream position
	child_state->input = alinputstream_create_mark_shared(state->input,ALABNF_READBLOCKSIZE);
	return child_state;
      }
    else
      {
	aldebug_printf(DBGSTREAM,"[FATAL] null node within alternative in %s:%s:%i\n",__FILE__,__func__,__LINE__);
      }
    return NULL;
  }

struct alabnf_matcher_state *
alabnf_matcher_process_next_sequence(
				       struct alabnf_matcher_state * state,
				       struct alabnf_sequence * sequence
				       )
  {
    ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"process sequence",state);

    if ( state->type != ALABNF_MATCHER_ST_AND )
      {
	ALABNF_MATCHER_FATAL_TEXT_STATE(NULL,"state %p non ALABNF_MATCHER_ST_AND",state);
	return NULL;
      }

    struct alabnf_node * node = sequence->node;
    // current_node is NULL meaning we have to walk next sequence for this state.
    state->current_node = node;
    // new sequence is next one.
    state->next_sequence=sequence->next;
    if ( node != NULL )
      {
	struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, node);
	// child use very same stream than parent
	child_state->input = state->input;
	return child_state;
      }
    else
      {
	ALABNF_MATCHER_FATAL_TEXT_STATE(NULL,"null node within sequence",state);
      }
    return NULL;
  }


struct alabnf_matcher_state *
alabnf_matcher_specialize_alternative(struct alabnf_matcher_state * state,
				      struct alabnf_node * node)
{
  struct alabnf_alternative * alternative = &node->content.alt;
  state->type = ALABNF_MATCHER_ST_OR;
  struct alabnf_matcher_state * child_state = alabnf_matcher_process_next_alternative(state,alternative);
  return child_state;
}

struct alabnf_matcher_state *
alabnf_matcher_specialize_sequence(struct alabnf_matcher_state * state,
				   struct alabnf_node * node)
{
  struct alabnf_sequence * sequence = &node->content.sequence;
  state->type = ALABNF_MATCHER_ST_AND;
  struct alabnf_matcher_state * child_state = alabnf_matcher_process_next_sequence(state,sequence);
  return child_state;
}

struct alabnf_matcher_state *
alabnf_matcher_specialize_iterator(struct alabnf_matcher_state * state,
				   struct alabnf_node * node)
{
  struct alabnf_iterator * iterator = &node->content.iterator;
  state->type = ALABNF_MATCHER_ST_IT;
  state->iteration = 0;
  
  struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, iterator->node);
  // current state should remember stream position
  child_state->input = alinputstream_create_mark_shared(state->input,ALABNF_READBLOCKSIZE);

  return child_state;
}

struct alabnf_matcher_state *
alabnf_matcher_specialize_reference(struct alabnf_matcher_state * state,
				   struct alabnf_node * node)
{
  struct alabnf_rule_ref * rule_ref = &node->content.rule_ref;
  state->type = ALABNF_MATCHER_ST_REF;
  state->iteration = 0;

  struct alabnf_node * resolved = rule_ref->resolved;
  if ( resolved != NULL )
    {
      // create a child
      struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, resolved);
      // share same input stream.
      child_state->input = state->input;
      return child_state;
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[FATAL] unresolved rule_ref in %s:%s:%i\n",__FILE__,__func__,__LINE__);
      // WHAT TO DO HERE ?? skip ??
      // Looks like it is a FATAL error, all references should be resolved.
      aldatablock * datablock = &rule_ref->keyblock;
      aldebug_printf(DBGSTREAM,"rulename="ALPASCALSTRFMT"\n",ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
      return NULL;
    }

}

// if specialize fails can return NULL
struct alabnf_matcher_state *
alabnf_matcher_specialize(struct alabnf_matcher_state * state)
{
  if ( state->type == ALABNF_MATCHER_ST_UNSET )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"specialize",state);
      struct alabnf_node * node = state->initial_node;
      if ( node != NULL )
	{
	  switch( node-> type )
	    {
	    case ALABNF_NT_ALT:
	      state=alabnf_matcher_specialize_alternative(state,node);
	      break;
	    case ALABNF_NT_SEQUENCE:
	      state=alabnf_matcher_specialize_sequence(state,node);
	      break;
	    case ALABNF_NT_ITERATOR:
	      state=alabnf_matcher_specialize_iterator(state,node);
	      break;
	    case ALABNF_NT_RULE_REF:
	      state=alabnf_matcher_specialize_reference(state,node);
	      break;
	    default:
	      state->type = ALABNF_MATCHER_ST_NODE;
	    }
	}
      else
	{
	  ALABNF_MATCHER_FATAL_TEXT_STATE(NULL,"unset state without initial node",state);
	}
    }
  ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"specialized",state);
  return state;
}

// unstack a child of any kind to be a alternative parent.
enum alabnf_match alabnf_matcher_unstack_child_alt( struct alabnf_matcher * matcher,
					    struct alabnf_matcher_state * child,
					    struct alabnf_matcher_state * parent,
					    enum alabnf_match match
					    )
{
  if ( match == ALABNF_MATCH_SUCCESS_UNSTACK )
    {
      // very important to realign current stream with child result
      alinputstream_align_shared_with_child(parent->input,child->input);
      alabnf_matcher_unstack(matcher,child,parent);
      return match;
    }
  else if ( match == ALABNF_MATCH_FAIL_UNSTACK )
    {
      alabnf_matcher_unstack(matcher,child,parent);
      
      struct alabnf_matcher_state * state = parent;
      // find next alt
      // progress within alternatives.
      ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"match next alt",state);
      // related SET_ALTERNATIVE
      // this is here that we gather next alternative, this is mandatory else we hit an infinite loop
      struct alabnf_alternative * alternative = NULL;
      // will take next alternative
      alternative = state->alt;
      // alternative can be NULL for last one.	
      if ( alternative != NULL )
	{
	  struct alabnf_matcher_state * child_state = alabnf_matcher_process_next_alternative(state, alternative);
	  if (child_state != NULL )
	    {
	      alabnf_set_matcher_state_input(matcher,child_state);
	      return ALABNF_MATCH_CONTINUE;
	    }
	  else
	    {
	      return ALABNF_MATCH_ERROR;
	    }
	}
      else
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"all alternatives have been evaluated",state);
	  // ==> infinite loop
	  return match;
	}

    }  
  return ALABNF_MATCH_ERROR;
}

// unstack a child of any kind to be a sequence parent.
enum alabnf_match alabnf_matcher_unstack_child_sequence( struct alabnf_matcher * matcher,
						 struct alabnf_matcher_state * child,
						 struct alabnf_matcher_state * parent,
						 enum alabnf_match match
						 )
{
  if ( match == ALABNF_MATCH_FAIL_UNSTACK )
    {
      alabnf_matcher_unstack(matcher,child,parent);
      return match;
    }
  else if ( match == ALABNF_MATCH_SUCCESS_UNSTACK )
    {
      // TO BE DONE at any successfull unstack ?
      // very important to realign current stream with child result
      // alinputstream_align_shared_with_child(parent->input,child->input);
      alabnf_matcher_unstack(matcher,child,parent);

      struct alabnf_matcher_state * state = parent;
      // find next alt
      // progress within alternatives.
      ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"match next sequence",state);

      struct alabnf_sequence * sequence = state->next_sequence;
     
      // alternative can be NULL for last one.	
      if ( sequence != NULL )
	{
	  struct alabnf_matcher_state * child_state = alabnf_matcher_process_next_sequence(state, sequence);
	  if (child_state != NULL )
	    {
	      alabnf_set_matcher_state_input(matcher,child_state);
	      return ALABNF_MATCH_CONTINUE;
	    }
	  else
	    {
	      return ALABNF_MATCH_ERROR;
	    }
	}
      else
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"sequence nodes fully evaluted",state);
	  return match;
	}

    }  
  return ALABNF_MATCH_ERROR;
}

// unstack a child of any kind to be an iterator parent.
enum alabnf_match alabnf_matcher_unstack_child_iterator( struct alabnf_matcher * matcher,
						 struct alabnf_matcher_state * child,
						 struct alabnf_matcher_state * parent,
						 enum alabnf_match match
						 )
{
  struct alabnf_node * node = parent->initial_node;
  struct alabnf_iterator * iterator = &node->content.iterator;

  if ( match == ALABNF_MATCH_SUCCESS_UNSTACK )
    {
      // very important to realign current stream with child result
      alinputstream_align_shared_with_child(parent->input,child->input);
      ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"iterator aligned parent with child",parent);
      alabnf_matcher_unstack(matcher,child,parent);
      // iterate      
      parent->iteration ++;
      if ( ( iterator->max == ALABNF_INFINITE_ITERATION) || ( parent->iteration < iterator->max ) )
	{
	  struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(parent, iterator->node);
	  if (child_state != NULL )
	    {
	      // FIXME does infinite loop
	      // current state should remember stream position
	      child_state->input = alinputstream_create_mark_shared(parent->input,ALABNF_READBLOCKSIZE);
	      aldebug_printf(DBGSTREAM,"%p %p NEXT iteration %i  < iterator->max %i stream %p\n", parent, child_state, parent->iteration, iterator->max, child_state->input);
	      alabnf_set_matcher_state_input(matcher,child_state);
	      return ALABNF_MATCH_CONTINUE;
	    }
	  else
	    {
	      return ALABNF_MATCH_ERROR;
	    }
	}
      // matched maximum iteration
      aldebug_printf(DBGSTREAM,"%p iteration FULLY COMPLETED  %i  >= iterator->max %i\n", parent, parent->iteration, iterator->max);
      return match;
    }
  else if ( match == ALABNF_MATCH_FAIL_UNSTACK )
    {
      alabnf_matcher_unstack(matcher,child,parent);
      if ( parent->iteration >= iterator->min )
	{
	  // did match minimal iteration => ok
	  aldebug_printf(DBGSTREAM,"%p iteration PARTIALY COMPLETED  %i  >= iterator->min %i\n", parent, parent->iteration, iterator->min);
	  return ALABNF_MATCH_SUCCESS_UNSTACK;
	}
      return match;
    }  
  return ALABNF_MATCH_ERROR;
}


// unstack a child of any kind to be a reference parent.
enum alabnf_match alabnf_matcher_unstack_child_reference( struct alabnf_matcher * matcher,
						 struct alabnf_matcher_state * child,
						 struct alabnf_matcher_state * parent,
						 enum alabnf_match match
						 )
{
  struct alabnf_node * node = parent->initial_node;
  struct alabnf_rule_ref * rule_ref = &node->content.rule_ref;
  alabnf_matcher_unstack(matcher,child,parent);
  
  if ( match == ALABNF_MATCH_SUCCESS_UNSTACK )
    {
      aldatablock * datablock = &rule_ref->keyblock;
      aldebug_printf(DBGSTREAM,"rule match "ALPASCALSTRFMT"\n",ALPASCALSTRARGS(datablock->length,datablock->data.charptr));      
      return match;
    }
  else
    {
      return match;
    }

}

enum alabnf_match alabnf_matcher_unstack_child( struct alabnf_matcher * matcher,
						struct alabnf_matcher_state * child,
						struct alabnf_matcher_state * parent,
						enum alabnf_match match
						)
{
  if ( parent != NULL)
    {
      switch( parent->type )
	{
	case ALABNF_MATCHER_ST_OR:
	  return alabnf_matcher_unstack_child_alt(matcher,child,parent,match);
	  break;
	case ALABNF_MATCHER_ST_AND:
	  return alabnf_matcher_unstack_child_sequence(matcher,child,parent,match);
	  break;
	case ALABNF_MATCHER_ST_IT:
	  return alabnf_matcher_unstack_child_iterator(matcher,child,parent,match);
	  break;
	case ALABNF_MATCHER_ST_REF:
	  return alabnf_matcher_unstack_child_reference(matcher,child,parent,match);
	  break;
	default:
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(&matcher->tempchar1,"MATCH ERROR",parent);       
	  return ALABNF_MATCH_ERROR;
	}
    }
  else
    {      
      aldebug_printf(DBGSTREAM,"[FATAL] FIXME , what to do with state ... should dispose it ? MEMORY LEAK");      
      matcher->current_state = NULL;
      return match;
    }
}
 
// USED externaly ( from matcher main )
void alabnf_match_init(struct alabnf_matcher * matcher,
		       struct alabnf * alabnf,
		       struct alinputstream * input)
{
  if ( matcher != NULL )
    {
      // make sure it is clean.
      bzero(matcher,sizeof(*matcher));
      matcher->abnf_syntax = alabnf;
      matcher->input=input;
      matcher->current_state = &matcher->root_state;

      struct alabnf_matcher_state * state = matcher->current_state;
      // should setup root state
      {
	struct alabnf_node * node = alabnf->root_rule.value;
	
	alabnf_matcher_init_state(state,NULL,node);
	
	state->current_rule = &alabnf->root_rule;

	{
	  char descr[255];
	  int prefix = alabnf_fill_rule_info(descr, 255, state->current_rule);
	  descr[prefix]=0;
	  aldebug_printf(DBGSTREAM,"[DEBUG]  init root rule %s\n",descr);
	}

	state->input=matcher->input;

      }
    }
}

alabnf_character * alabnf_matcher_get_next_char(struct alabnf_matcher * matcher, struct alabnf_matcher_state * state)
{
  if ( state->type !=  ALABNF_MATCHER_ST_NODE )
    {
      aldebug_printf(DBGSTREAM,"[ERROR] get next char on state %p non ALABNF_MATCHER_ST_NODE type %i in %s:%s:%i\n",
		     state,state->type,
		     __FILE__,__func__,__LINE__);
      // never read a OR stream
      // it is used asparent stream reference for stream backtracking.

      // BUT what if it is not a OR stream but an IT or a REf ?
      
      state->tempchar2.uchar = 0;
      return &state->tempchar2;
    }
  else
    {
      struct alinputstream * stream = state->input;
      if ( stream != NULL )
	{
	  aldebug_printf(DBGSTREAM,"[DEBUG] get next char on stream %p type %i\n", stream, stream->type);  
	  unsigned char uchar = alinputstream_shared_readuchar(stream);
	  if ( uchar == 0 )
	    {
	      return NULL;
	    }
	  else
	    {      
	      matcher->tempchar1.uchar = uchar;
	      return &matcher->tempchar1;
	    }
	}
      else
	{
	  ALABNF_MATCHER_FATAL_TEXT_STATE(NULL,"null node within sequence",state);
	  exit(1);
	}
    }
}

// if match occur then datablock_index of current state will increase.
// if match last part of datablock then return ALABNF_MATCH_SUCCESS_UNSTACK
enum alabnf_match abnf_match_datablock_character(struct alabnf_matcher * matcher,
						 aldatablock * datablock,	 
						 alabnf_character * next_char)
{      
  struct alabnf_matcher_state * current_state = matcher->current_state;
  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"MATCH DATABLOCK",current_state); 
  int datablock_index = current_state->datablock_index;
  if ( datablock_index < datablock->length )
    {
      if (datablock->data.ucharptr[datablock_index] == next_char->uchar)
	{
	  datablock_index++;
	  if ( datablock_index == datablock->length )
	    {
	      return ALABNF_MATCH_SUCCESS_UNSTACK;
	    }
	  current_state->datablock_index=datablock_index;
	  return ALABNF_MATCH_CONTINUE;
	}
      else
	{
	  unsigned uchar = next_char->uchar;
	  aldebug_printf(DBGSTREAM,"[DEBUG] '%c' != '%c'\n",datablock->data.ucharptr[datablock_index],uchar >= 32 ? uchar : '.');
	}
    }
  else
    {
      aldebug_printf(DBGSTREAM,"[DEBUG] str length %i > %i\n",datablock_index,datablock->length);
    }
  
  return ALABNF_MATCH_FAIL_UNSTACK;
}


struct alabnf_matcher_state * alabnf_matcher_create_child_state(
								   struct alabnf_matcher_state * parent,
								   struct alabnf_node * node)
{
  // TODO ALLOC use matcher allocation ?
  struct alabnf_matcher_state * child_state = alabnf_matcher_state_alloc();
  // don't print it since not yet initialized can contain heavy garbage
  // ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"create child state", child_state);
  if ( child_state != NULL )
    {
      alabnf_matcher_init_state(child_state,parent,node);
    }
  return child_state;
}

enum alabnf_match alabnf_match_character(struct alabnf_matcher * matcher,
					 struct alabnf_matcher_state * state,
					 alabnf_character * next_char)
{

  if ( state != NULL )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match character",state);

      struct alabnf_node * current_node = state->current_node;
      struct alabnf_node * initial_node = state->initial_node;

      // process only NODE type
      switch (state->type)
	{
	case ALABNF_MATCHER_ST_NODE:
	  break;
	case ALABNF_MATCHER_ST_REF:
	  // why is it a reference and not a node ?
	  // Is it normal or not ?
	  ALABNF_MATCHER_FATAL_TEXT_STATE(next_char,"reference type unsupported",state);
	  return ALABNF_MATCH_ERROR;
	  // unsupported types
	case ALABNF_MATCHER_ST_OR:
	case ALABNF_MATCHER_ST_AND:
	case ALABNF_MATCHER_ST_IT:
	default:
	  ALABNF_MATCHER_FATAL_TEXT_STATE(next_char,"type unsupported",state);
	  return ALABNF_MATCH_ERROR;
	}

      // fully deprecated since alabnf_matcher_state_type usage, should process only NODE type here
      if (initial_node != NULL )
	{
	  if (initial_node->type == ALABNF_NT_SEQUENCE)
	    {
	      // progress within a sequence
	      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match initial sequence wrong type",state);
		  
	      return ALABNF_MATCH_ERROR;
	    }
	  else if (initial_node->type == ALABNF_NT_ALT)
	    {
	      // progress within alternatives.
	      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match initial alt wrong type",state);
		  
	      return ALABNF_MATCH_ERROR;
	    }
	  else if (initial_node->type == ALABNF_NT_ITERATOR)
	    {
	      // progress within iterator
	      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match initial iterator wrong type",state);
		  
	      return ALABNF_MATCH_ERROR;
	    }

	  if ( current_node == NULL )
	    {
	      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"current node NULL -> unstack ",state);
	      return ALABNF_MATCH_FAIL_UNSTACK;
	    }
	}


      // here current_node != NULL but next_char can be NULL

      
      if (next_char == NULL)
	{
	  // TO CHECK
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"next char  NULL ALABNF_MATCH_NONE ",state);
	  return ALABNF_MATCH_NONE;
	}  
      
      aldebug_printf(DBGSTREAM,"[DEBUG] match char on initial node type %i current node type %i %p state %p\n",
		     initial_node->type, current_node->type, current_node, state);
      // alabnf_dump_node(current_node);

      if (current_node->type == ALABNF_NT_STRING)
	{
	  enum alabnf_string_type string_type = current_node->content.string.type;
	  if (
	      ( string_type == ALABNF_ST_QUOTED)
	      || ( string_type == ALABNF_ST_HEX)
	      || ( string_type == ALABNF_ST_DEC)
	      || ( string_type == ALABNF_ST_BIN) )
	    {
	      return abnf_match_datablock_character(matcher, &current_node->content.string.strbloc,next_char);
	    }
	  else if ( string_type ==  ALABNF_ST_RULENAME )	    
	    {
	      aldebug_printf(DBGSTREAM,"[ERROR] current_node string type is a rule reference in %s:%s:%i\n",__FILE__,__func__,__LINE__);

	      // somehow should not happen since it should not be a string but a ALABNF_NT_RULE_REF
	      return ALABNF_MATCH_ERROR;
	    }
	  else
	    {
	      // ALABNF_ST_UNDEFINED 
	      aldebug_printf(DBGSTREAM,"[ERROR] current_node string type is not a quoted string but %i in %s:%s:%i\n",
			     current_node->content.string.type,
			     __FILE__,__func__,__LINE__);
	      return ALABNF_MATCH_ERROR;
	    }
	}
      else if (current_node->type == ALABNF_NT_RANGE)
	{
	  struct alabnf_range * range = &current_node->content.range;
	  int value = (int) next_char->uchar;
	  if (( value >= range->start ) && ( value <= range->end ))
	    {
	      aldebug_printf(DBGSTREAM,"[DEBUG] match ALABNF_NT_RANGE %i <=%i<=%i in %s:%s:%i\n",
			     range->start,value,range->end,
			     __FILE__,__func__,__LINE__);
	      return ALABNF_MATCH_SUCCESS_UNSTACK;
	    }
	  else
	    {
	      aldebug_printf(DBGSTREAM,"[DEBUG] UNmatch ALABNF_NT_RANGE %i , %i ,%i in %s:%s:%i\n",
			     range->start,value,range->end,
			     __FILE__,__func__,__LINE__);
	      return ALABNF_MATCH_FAIL_UNSTACK;

	    }
	}	
      else if (current_node->type == ALABNF_NT_RULE_REF)
	{
	  // DONE in specialization turn should not be hit
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"matching current ALABNF_NT_RULE_REF type ERROR",state);
	  return ALABNF_MATCH_NONE;
	  
	}
      else
	{
	  aldebug_printf(DBGSTREAM,"[WARNING] current_node type %i is not supported , initial type is %i in %s:%s:%i\n",current_node->type,initial_node->type,__FILE__,__func__,__LINE__);
	}
    }
  else
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"state NULL",state);
    }
  return ALABNF_MATCH_NONE;
}

// process alternative, sequence, iterator specialization
struct alabnf_matcher_state * alabnf_matcher_resolve_state_node(struct alabnf_matcher * matcher)
{
  struct alabnf_matcher_state * state =  matcher->current_state;  
  while ( ( state != NULL ) && ( state->type == ALABNF_MATCHER_ST_UNSET ) )
      {
	state = alabnf_matcher_specialize(state);
	ALABNF_MATCHER_LIMIT_STEPS(matcher,state);
      }
  return state;
}

void alabnf_match(struct alabnf_matcher * matcher)
{
  alabnf_character * next_char = NULL;
  enum alabnf_match match = ALABNF_MATCH_CONTINUE;
  struct alabnf_matcher_state * state =  matcher->current_state;
  struct alabnf_matcher_state * parent = NULL;

  // protect against rematches infinite loop
  matcher->rematches = 0;
  matcher->maxrematches = 10;
  
  do {

    state = alabnf_matcher_resolve_state_node(matcher);
    if ( state == NULL )
      {
	aldebug_printf(DBGSTREAM,"[FATAL] null state at %s:%s:%i",__FILE__,__func__,__LINE__);
	break;
      }
    alabnf_set_matcher_state_input(matcher,state);
    if ( match == ALABNF_MATCH_REMATCH )
      {
	matcher->rematches ++;
	if (matcher->rematches > matcher->maxrematches )
	  {
	    aldebug_printf(DBGSTREAM,"[FATAL] too many rematches at %s:%s:%i",
			   __FILE__,__func__,__LINE__);
	    break;
	  }
      }
    else
      {
	matcher->rematches = 0;
	next_char = alabnf_matcher_get_next_char(matcher,state);
      }
    ALABNF_MATCHER_LIMIT_STEPS(matcher,state);
    match = alabnf_match_character(matcher,state,next_char);
    matcher->last_match = match;
    
    state = matcher->current_state;
    
    if ( state != NULL )
      {
	parent = state->parent;
      }
    else
      {
	aldebug_printf(DBGSTREAM,"[FATAL] null state at %s:%s:%i",__FILE__,__func__,__LINE__);
	break;
      }

    if ( next_char != NULL )
      {
	unsigned uchar = next_char->uchar;
	aldebug_printf(DBGSTREAM,"[DEBUG] match (%i) char '%c'\n", match ,uchar >= 32 ? uchar : '.');
      }

    while ( (match == ALABNF_MATCH_FAIL_UNSTACK)
	    || (match == ALABNF_MATCH_SUCCESS_UNSTACK) )
      {	
	ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_UNSTACK",state);

	match = alabnf_matcher_unstack_child(matcher,state,parent,match);

	// need to get current matcher that since state might have been freed.
	state = matcher->current_state;
	ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_UNSTACK done",state);

	// unstacking state can create an unresolved brother
	state = alabnf_matcher_resolve_state_node(matcher);	
	alabnf_set_matcher_state_input(matcher,state);
	
	if ( state != NULL )
	  {
	    parent = state->parent;
	  }
	else	    
	  {
	    ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"COMPLETED state null",state);
	    if ( match ==  ALABNF_MATCH_SUCCESS_UNSTACK )
	      {
		match = ALABNF_MATCH_FULL;
	      }	      
	    else
	      {
		match = ALABNF_MATCH_NONE;
	      }
	    break;
	  }

	

      }

    if (match == ALABNF_MATCH_CONTINUE)
    {
      // this means same state need multiple characters
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_CONTINUE",state);
    }
    else if ( match == ALABNF_MATCH_NONE )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_NONE",state);
      printf("FAIL\n");
      break;
    }
    else if ( match == ALABNF_MATCH_FULL )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_FULL",state);
      printf("SUCCESS\n");
      break;
    }
    else if ( match == ALABNF_MATCH_REMATCH )
      {
	ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_REMATCH",state);
      // rematch !
      }
    else if ( match == ALABNF_MATCH_ERROR )
      {
	ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_ERROR",state);
	aldebug_printf(DBGSTREAM,"[FATAL] internal error at %s:%s:%i\n",
		       __FILE__,__func__,__LINE__);
	break;
      }
    else
      {
	aldebug_printf(DBGSTREAM,"[FATAL] unsupported alabnf_match value (%i) at %s:%s:%i\n",match,__FILE__,__func__,__LINE__);
	break;
      }

    state = matcher->current_state;
    if ( state == NULL )
      {
	// did we complete 
	aldebug_printf(DBGSTREAM,"[DEBUG] null state at %s:%s:%i",__FILE__,__func__,__LINE__);
	printf("readched top of the stack -> match \n");
	break;
      }

    if ( ( match == ALABNF_MATCH_REMATCH ) && ( next_char == NULL ) )
      {
	match = ALABNF_MATCH_CONTINUE;
      }
  }
  while ( state != NULL );  
  // todo free ....

  // should consume whole stream 
  struct alinputstream * stream = matcher->input;
  if ( ! alinputstream_iseof(stream) )
    {
      printf("unmatched chars (please hit Ctrl+D)\n");
      /* with stdin wihtin terminal it waits for keystroke new line or Ctrl+D */
      while ( ! alinputstream_iseof(stream) )
	{
	  unsigned char uchar = alinputstream_readuchar(stream);
	  printf("%c",uchar);
	}
    }
}

