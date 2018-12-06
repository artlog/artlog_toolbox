#include "alabnf_matcher.h"
#include <stdio.h>
#include <strings.h>

#include <stdlib.h>

const void * ALABNF_DEAD_CANARY = (void *) 0xdeadca01;

// to read alternatives ...
const int ALABNF_READBLOCKSIZE = 4096;

static char descr[30];
static char state_descr[255];

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

char * alabnf_matcher_get_state_descr(struct alabnf_matcher_state * state)
{
  if ( state != NULL)
    {
      snprintf(state_descr,255,"('%p' type %i current %p current_type %i index %i)",state,state->initial_node == NULL ? -1 : state->initial_node->type,state->current_node,state->current_node !=NULL ? state->current_node->type : -1,state->datablock_index);
    }
  else
    {
      snprintf(state_descr,255,"(NULL state)");
    }
  return state_descr;
}

#define ALABNF_MATCHER_DEBUG_TEXT_STATE(achar,text,state) aldebug_printf(NULL,"[DEBUG] %s %s state %s  in %s %s %i\n",text,alabnf_matcher_get_descr(achar),alabnf_matcher_get_state_descr(state),__FILE__,__func__,__LINE__)

struct alabnf_matcher_state * alabnf_matcher_state_alloc()
{
  struct alabnf_matcher_state * state = malloc(sizeof(*state));
  aldebug_printf(NULL,"[DEBUG] matcher state alloc %p at %s:%s:%i\n",state,__FILE__,__func__,__LINE__);
  return state;
}

void alabnf_matcher_state_free(struct alabnf_matcher_state * state)
{
  if ( state != NULL )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"matcher state free",state);
      if ( state->current_node == ALABNF_DEAD_CANARY  )
	{
	  aldebug_printf(NULL,"[FATAL] matcher state free canary hit in %s:%s:%i",__FILE__,__func__,__LINE__);
	  return;
	}
      if ( state->initial_node->type == ALABNF_NT_ALT )
	{
	  aldebug_printf(NULL,"[DEBUG] free inputstream  in  %s:%s:%i",__FILE__,__func__,__LINE__);
	  alinputstream_free_shared(state->input);
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
      aldebug_printf(NULL,"[DEBUG] init state %p parent %p node %p node->type %i\n",state,parent,node,(node == NULL) ? -1 : node->type);
      state->parent = parent;
      state->initial_node = node;
      state->current_node = node;
      state->next_sequence = NULL;
      state->datablock_index = 0;
      if ( parent != NULL )
	{
	  state->current_rule = parent->current_rule;
	}
      if (node == NULL )
	{
	  aldebug_printf(NULL,"[FATAL] init state %p parent %p node NULL\n",state,parent);
	}
    }
}

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
	state->input=matcher->input;
      }
      
    }
}

alabnf_character * alabnf_matcher_get_next_char(struct alabnf_matcher * matcher, struct alabnf_matcher_state * state)
{
  struct alinputstream * stream = state->input;
  
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

// if match occur then datablock_index of current state will increase.
// if match last part of datablock then return ALABNF_MATCH_FULL
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
	      return ALABNF_MATCH_FULL;
	    }
	  current_state->datablock_index=datablock_index;
	  return ALABNF_MATCH_CONTINUE;
	}
      else
	{
	  printf("'%c' != '%c'\n",datablock->data.ucharptr[datablock_index],next_char->uchar);
	}
    }
  else
    {
      printf("str length %i > %i\n",datablock_index,datablock->length);
    }
  
  return ALABNF_MATCH_NONE;
}


struct alabnf_matcher_state * alabnf_matcher_create_child_state(
								   struct alabnf_matcher_state * parent,
								   struct alabnf_node * node)
{
  // TODO ALLOC use matcher allocation ?
  struct alabnf_matcher_state * child_state = alabnf_matcher_state_alloc();
  // don't print it since not yet initialized can contian heavy garbage
  // ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"create child state", child_state);
  if ( child_state != NULL )
    {
      alabnf_matcher_init_state(child_state,parent,node);
    }
  return child_state;
}

// will stack parent alternative to point on next  and process current node one.
enum alabnf_match alabnf_matcher_process_alternative(alabnf_character * next_char,
						     struct alabnf_matcher * matcher,
						     struct alabnf_matcher_state * state,
						     struct alabnf_alternative * alternative)
  {
    ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"process alt",state);
    struct alabnf_node * node = alternative->node;
    // current_node is NULL meaning we have to walk next alternative for this state.
    state->current_node = NULL;
    // new alternative is next one.
    state->alt=alternative->alt;
    if ( node != NULL )
      {
	struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, node);
	// current state should remember stream position
	child_state->input = alinputstream_create_mark_shared(state->input,ALABNF_READBLOCKSIZE);
	child_state->input = state->input;
	matcher->current_state = child_state;
	return ALABNF_MATCH_REMATCH;
      }
    else
      {
	aldebug_printf(next_char,"[ERROR] null node in alternative in %s:%s:%i\n",__FILE__,__func__,__LINE__);
      }
    return ALABNF_MATCH_NONE;
  }

// will stack parent iterator to point on next  and process current node one.
enum alabnf_match alabnf_matcher_process_iterator(alabnf_character * next_char,
						     struct alabnf_matcher * matcher,
						     struct alabnf_matcher_state * state,
						     struct alabnf_iterator * iterator)
  {
    ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"process iterator",state);
    if ( ( iterator->max == ALABNF_INFINITE_ITERATION) || ( state->iteration < iterator->max ) )
      {
	struct alabnf_node * node = iterator->node;
	// current_node is NULL meaning we have to walk next iterator for this state.
	state->current_node = NULL;
	// new iterator is next one.
	state->iteration++;
	if ( node != NULL )
	  {
	    struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, node);
	    child_state->input = state->input;
	    matcher->current_state = child_state;
	    return ALABNF_MATCH_REMATCH;
	  }
	else
	  {
	    aldebug_printf(next_char,"[ERROR] null node in iterator in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	  }
      }
    return ALABNF_MATCH_NONE;
  }

enum alabnf_match alabnf_match_character(struct alabnf_matcher * matcher,
				       alabnf_character * next_char)
{
  struct alabnf_matcher_state * state = matcher->current_state;

  if (next_char == NULL)
    {
      // TO CHECK
      return ALABNF_MATCH_NONE;
    }
  
  if ( state != NULL )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match character",state);
      struct alabnf_node * current_node = state->current_node;
      struct alabnf_node * initial_node = state->initial_node;

      // current node NULL mean we have to progress depending on initial node.
      if (current_node == NULL )
	{
	  if (initial_node != NULL )
	    {
	      // progress within a sequence
	      if (initial_node->type == ALABNF_NT_SEQUENCE)
		{
		  // it means we are progressing  in a sequence
		  struct alabnf_sequence * sequence = NULL;
		  struct alabnf_node * node = NULL;
		  sequence = state->next_sequence;
		  if (sequence != NULL)
		    {
		      // get node for this sequence.
		      node = sequence->node;
		    }
		  else
		    {
		      // we are at end of this sequence, it is completed AND need a rematch / unstack
		      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"end of sequence -> unstack ",state);
		      state->current_node=NULL;
		      return ALABNF_MATCH_UNSTACK;      
		    }
		  state->current_node = node;
		  // walking between next element of sequence keep same state but update node and next_sequence.
		  state->next_sequence = sequence->next;
		  if ( node != NULL )
		    {
		      // will rematch with a non NULL current node.
		      return ALABNF_MATCH_REMATCH;
		    }
		  else
		    {
		      // node for sequence is NULL => looks like an invalid case
		      aldebug_printf(NULL,"[ERROR] node is NULL within a sequence in %s:%s:%i\n",
				     __FILE__,__func__,__LINE__);
		    }
		}
	      else if (initial_node->type == ALABNF_NT_ALT)
		{
		  // progress within alternatives.
		  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match next alt",state);
		  // related SET_ALTERNATIVE
		  // this is here that we gather next alternative, this is mandatory else we hit an infinite loop
		  struct alabnf_alternative * alternative = NULL; 
		  // will take next alternative
		  alternative = state->alt;
		  // alternative can be NULL for last one.	
		  if ( alternative != NULL )
		    {
		      return alabnf_matcher_process_alternative(next_char,matcher,state, alternative);
		    }
		  else
		    {
		      // else all alternatives have been evaluated => ALABNF_MATCH_NONE
		      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"all alternatives done",state);		  
		      // should rematch or unstack ... ?
		      return ALABNF_MATCH_NONE;
		    }
		}
	      else
		{
		  aldebug_printf(NULL,"[WARNING] no initial or current_node in %s %s %i\n",__FILE__,__func__,__LINE__);
		  return ALABNF_MATCH_NONE;
		}
	      // FIXME, done for alternative case when matched.
	      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"current node NULL -> unstack ",state);
	      return ALABNF_MATCH_UNSTACK;
	    }
	}

      // here current_node != NULL
      
      aldebug_printf(NULL,"[DEBUG] match char on initial node type %i current node type %i %p state %p\n",
		     initial_node->type, current_node->type, current_node, state);
      alabnf_dump_node(current_node);

      if (current_node->type == ALABNF_NT_ALT)
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match alt current ALABNF_NT_ALT",state);
	  if (initial_node != current_node)
	    {
	      // ex in sequence.
	      // move current alt node to be an initial state.
	      state->current_node = NULL;
	      struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, current_node);
	      // current state should remember stream position
	      // child_state->input = alinputstream_create_mark_shared(state->input,ALABNF_READBLOCKSIZE);
	      child_state->input = state->input;
	      matcher->current_state = child_state;
	      return ALABNF_MATCH_REMATCH;
	    }
	  // current_node is one node within flat list of alternatives if not first time
	  return alabnf_matcher_process_alternative(next_char,matcher,state, &current_node->content.alt);
	}
      else if (current_node->type == ALABNF_NT_ITERATOR)
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match ALABNF_NT_ITERATOR",state);
	  return alabnf_matcher_process_iterator(next_char,matcher,state, &current_node->content.iterator);
	}
      else if (current_node->type == ALABNF_NT_SEQUENCE)
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match current ALABNF_NT_SEQUENCE",state);
	  // it means we are entering a sequence
	  struct alabnf_sequence * sequence = NULL;
	  struct alabnf_node * node = NULL;
	  if ( initial_node == current_node )
	    {
	      // entering in a sequence as flat use next_sequence to progress in it when current_node is NULL.
	      sequence = &current_node->content.sequence;
	      node = sequence->node;
	      // next_sequence point on second element ?
	      state->next_sequence = sequence->next;
	      state->current_node = node;
	      if ( node == NULL )
		{
		  // node for sequence is NULL => looks like an invalid case
		  aldebug_printf(NULL,"[ERROR] node is NULL within a sequence in %s:%s:%i\n",
				 __FILE__,__func__,__LINE__);
		}
	    }
	  else
	    {
	      // node is a sequence ( unflatened sequence ) walk in depth first		      
	      struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, current_node);
	      child_state->input=state->input;
	      matcher->current_state = child_state;
	    }
	  return ALABNF_MATCH_REMATCH;
	}      
      else if (current_node->type == ALABNF_NT_STRING)
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
	      aldebug_printf(NULL,"[WARNING] current_node string type is a rule reference in %s:%s:%i\n",__FILE__,__func__,__LINE__);

	      // somehow should not happen since it should not be a string but a ALABNF_NT_RULE_REF
	    }
	  else
	    {
	      // ALABNF_ST_UNDEFINED 
	      aldebug_printf(NULL,"[WARNING] current_node string type is not a quoted string but %i in %s:%s:%i\n",current_node->content.string.type,__FILE__,__func__,__LINE__);
	    }
	}
      else if (current_node->type == ALABNF_NT_RANGE)
	{
	  struct alabnf_range * range = &current_node->content.range;
	  int value = (int) next_char->uchar;
	  if (( value >= range->start ) && ( value <= range->end ))
	    {
	      aldebug_printf(NULL,"[DEBUG] match ALABNF_NT_RANGE %i <=%i<=%i in %s:%s:%i\n",
			     range->start,value,range->end,
			     __FILE__,__func__,__LINE__);
	      return ALABNF_MATCH_FULL;
	    }
	  else
	    {
	      aldebug_printf(NULL,"[DEBUG] UNmatch ALABNF_NT_RANGE %i , %i ,%i in %s:%s:%i\n",
			     range->start,value,range->end,
			     __FILE__,__func__,__LINE__);

	    }
	}	
      else if (current_node->type == ALABNF_NT_RULE_REF)
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match current ALABNF_NT_RULE_REF",state);
	  struct alabnf_rule_ref * rule_ref = &current_node->content.rule_ref;
	  struct alabnf_node * node = rule_ref->resolved;
	  if ( node != NULL )
	    {
	      state->current_node = node;
	      return ALABNF_MATCH_REMATCH;
	    }
	  else
	    {
	      aldebug_printf(NULL,"[WARNING] unresolved rule_ref in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	      // WHAT TO DO HERE ?? skip ??
	      aldatablock * datablock = &rule_ref->keyblock;
	      aldebug_printf(NULL,"rulename="ALPASCALSTRFMT"\n",ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
	    }
	  
	}
      else if (current_node->type == ALABNF_NT_ALT)
	{
	  // WARNING this means initial node is NOT an alternative but current is.
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"current is alt while initial is not", state);
	  // FIXME...
	  // what should we do ? push a new alt state here ?
	  struct alabnf_alternative * alternative = &current_node->content.alt;
	  return alabnf_matcher_process_alternative(next_char,matcher,state, alternative);	  
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] current_node type is not a string but %i , initial type is %i in %s:%s:%i\n",current_node->type,initial_node->type,__FILE__,__func__,__LINE__);
	}
    }
  return ALABNF_MATCH_NONE;
}

// set state to parent state.
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
	  aldebug_printf(NULL,"[DEBUG] unstack state for initial node %i at %s:%s:%i\n",
			 node->type,
			 __FILE__,__func__,__LINE__);
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"unstack free",state);
	  alabnf_matcher_state_free(state);
	}
      // unstack
      state = parent;
      matcher->current_state = state;

      return 1;
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] parent point on itself at %s:%s:%i\n",__FILE__,__func__,__LINE__);
      return 0;
    }
}

int alabnf_matcher_unstack_until(
				 alabnf_character * next_char,
				 enum alabnf_node_type node_type,
				 struct alabnf_matcher * matcher,
				 struct alabnf_matcher_state * state,
				 struct alabnf_matcher_state * parent)
{  
  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"request unstack until",state);
  struct alabnf_node * node = state->initial_node;

  while ( (state != NULL ) && ( node->type != node_type ))
    {  

      // unstack
      if ( ! alabnf_matcher_unstack(matcher,state,parent))
	{
	  return 0;
	}
      state = matcher->current_state;
      if ( state != NULL )
	{
	  node = state->initial_node;
	  parent = state->parent;
	}
      else
	{
	  parent = NULL;
	}
    }
  if ( state != NULL )
    {
      aldebug_printf(NULL,"[DEBUG] unstack until node %p type %i found %s:%s:%i\n",
		     node,node->type,
		     __FILE__,__func__,__LINE__);
		     
    }
  return ( state != NULL );
}

int alabnf_matcher_unstack_while(
				 alabnf_character * next_char,
				 enum alabnf_node_type node_type,
				 struct alabnf_matcher * matcher,
				 struct alabnf_matcher_state * state,
				 struct alabnf_matcher_state * parent)
{  
  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"request unstack while",state);
  struct alabnf_node * node = state->initial_node;

  while ( (state != NULL ) && ( node->type == node_type ))
    {  

      aldebug_printf(NULL,"[DEBUG] unstack while node %p type %i %s:%s:%i\n",
		     node,node->type,
		     __FILE__,__func__,__LINE__);

      // unstack
      if ( ! alabnf_matcher_unstack(matcher,state,parent))
	{
	  return 0;
	}
      state = matcher->current_state;
      if ( state != NULL )
	{
	  node = state->initial_node;
	  
	  parent = state->parent;
	}
      else
	{
	  parent = NULL;
	}
    }
  if ( state != NULL )
    {
      aldebug_printf(NULL,"[DEBUG] FOUND unstack while node %p type %i found %s:%s:%i\n",
		     node,node->type,
		     __FILE__,__func__,__LINE__);
		     
    }
  return ( state != NULL );
}

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

int alabnf_matcher_state_has_next_type(struct alabnf_matcher_state * state, enum alabnf_node_type node_type)
{
  struct alabnf_node * initial = state->initial_node;

  if (initial->type == 	node_type)
    {
      return ( initial != state->current_node );
    }

  return 0;
}

int alabnf_matcher_state_has_next_sequence(struct alabnf_matcher_state * state)
{
  return alabnf_matcher_state_has_next_type(state,ALABNF_NT_SEQUENCE);
}

int alabnf_matcher_state_has_next_alternative(struct alabnf_matcher_state * state)
{
  return alabnf_matcher_state_has_next_type(state,ALABNF_NT_ALT);
}

// current failed need to find next alternatives
int alabnf_matcher_find_next_fail(
				 alabnf_character * current_char,
				 struct alabnf_matcher * matcher,
				 struct alabnf_matcher_state * state,
				 struct alabnf_matcher_state * parent)
{

  if ( alabnf_matcher_state_has_next_alternative(state) )
    {
      // force progression.
      state->current_node=NULL;
      return 1;
    }

  // unstack all alternatives, one alternative match all.
  state = matcher->current_state;
  if ( state != NULL )
    {
      parent = state->parent;
    }
  else
    {
      parent = NULL;
    }

  return alabnf_matcher_unstack_until(current_char, ALABNF_NT_ALT,matcher,state,parent);
}

// current did fully match, need to complete all alternatives
int alabnf_matcher_find_next_success(
				 alabnf_character * current_char,
				 struct alabnf_matcher * matcher,
				 struct alabnf_matcher_state * state,
				 struct alabnf_matcher_state * parent)
{

  if ( alabnf_matcher_state_has_next_sequence(state) )
    {
      // force progression.
      state->current_node=NULL;
      return 1;
    }

  // unstack current terminal
  if ( alabnf_matcher_state_is_terminal(state))
    {
      
      if ( ! alabnf_matcher_unstack(matcher,state,parent))
	{
	  return 0;
	}
    }

  // unstack all alternatives, one alternative match all.
  state = matcher->current_state;
  if ( state != NULL )
    {
      parent = state->parent;
    }
  else
    {
      parent = NULL;
    }

  return alabnf_matcher_unstack_while(current_char, ALABNF_NT_ALT,matcher,state,parent);
}

void alabnf_match(struct alabnf_matcher * matcher)
{
  alabnf_character * next_char = NULL;
  enum alabnf_match match = ALABNF_MATCH_NONE;
  struct alabnf_matcher_state * state = NULL;
  struct alabnf_matcher_state * parent = NULL;

  // protect against rematches infinite loop
  int rematches = 0;
  int maxrematches = 10;
  
  do {
    state = matcher->current_state;
    if ( state == NULL )
      {
	// did we complete 
	aldebug_printf(NULL,"[DEBUG] null state at %s:%s:%i",__FILE__,__func__,__LINE__);
	printf("readched top of the stack -> match \n");
	break;
      }
    
    if ( match == ALABNF_MATCH_REMATCH )
      {
	rematches ++;
	if (rematches > maxrematches )
	  {
	    aldebug_printf(NULL,"[FATAL] too many rematches at %s:%s:%i",
			   __FILE__,__func__,__LINE__);
	    break;
	  }
      }
    else
      {
	rematches = 0;
	next_char = alabnf_matcher_get_next_char(matcher,state);	
      }
    
    if ( next_char == NULL)
      {
	ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"NULL char",state);
	break;
      }
    match = alabnf_match_character(matcher,next_char);
    state = matcher->current_state;
    if ( state != NULL )
      {
	parent = state->parent;
      }
    else
      {
	aldebug_printf(NULL,"[FATAL] null state at %s:%s:%i",__FILE__,__func__,__LINE__);
	break;
      }

    if ( next_char != NULL )
      {
	aldebug_printf(NULL,"[DEBUG] match (%i) char '%c'\n", match ,next_char->uchar);
      }

    if (match == ALABNF_MATCH_CONTINUE)
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_CONTINUE",state);
      // continue;
    }
    else
    if (match == ALABNF_MATCH_UNSTACK)
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_UNSTACK",state);
      // current state was fully matched, need to check parents.
      if ( parent != NULL )
	{
	  printf("*");
	  struct alabnf_node * parent_node = parent->initial_node;
	  if (
	      ( parent_node->type == ALABNF_NT_ALT )
	      || ( parent_node->type == ALABNF_NT_SEQUENCE ) )
	    {
	      if ( ! alabnf_matcher_unstack(matcher,state,parent))
		{
		  break;
		}
	    }
	  else
	    {
	      aldebug_printf(NULL,"[FATAL] parent not a sequence or alternative but %i at %s:%s:%i",
			     parent_node->type,
			     __FILE__,__func__,__LINE__);
	      break;
	    }
	  // SHOULD we REMATCH ?
	}
      else
	{
	  printf("'%c' match COMPLETE %i\n",next_char->uchar,match);
	  break;
	}

    }
    else if (match == ALABNF_MATCH_FULL)
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_FULL",state);
      if ( next_char != NULL )
	{
	  printf("%c",next_char->uchar);
	}
      printf("+");
      
      state->datablock_index=0;

      alabnf_matcher_find_next_success(next_char,matcher,state,parent);
    }
    else if ( match == ALABNF_MATCH_NONE )
    {
      ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_NONE",state);
      // should handle unstacking of parent context
      // related SET_ALTERNATIVE
      alabnf_matcher_find_next_fail(next_char,matcher,state,parent);

      match = ALABNF_MATCH_REMATCH;
    }
    else if ( match == ALABNF_MATCH_REMATCH )
      {
	ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"ALABNF_MATCH_REMATCH",state);
      // rematch !
      }
    else
      {
	aldebug_printf(NULL,"[FATAL] unsupported alabnf_match value (%i) at %s:%s:%i\n",match,__FILE__,__func__,__LINE__);
      }

  }
  while ( next_char != NULL );  
  // todo free ....
}

