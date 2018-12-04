#include "alabnf_matcher.h"
#include <stdio.h>
#include <strings.h>

#include <stdlib.h>

const void * ALABNF_DEAD_CANARY = (void *) 0xdeadca01;

// to read alternatives ...
const int ALABNF_READBLOCKSIZE = 4096;

static char descr[30];

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

#define ALABNF_MATCHER_DEBUG_TEXT_STATE(achar,text,state) aldebug_printf(NULL,"[DEBUG] %s %s state %p  in %s %s %i\n",text,alabnf_matcher_get_descr(achar),state,__FILE__,__func__,__LINE__)

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
	  aldebug_printf(NULL,"[FATAL] matcher state free canary hit at %s:%s:%i",__FILE__,__func__,__LINE__);
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
      state->parent = parent;
      state->initial_node = node;
      state->current_node = node;
      state->next_sequence = NULL;
      state->datablock_index = 0;
      if ( parent != NULL )
	{
	  state->current_rule = parent->current_rule;
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
  ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"create child state", child_state);
  if ( child_state != NULL )
    {
      alabnf_matcher_init_state(child_state,parent,node);
    }
  return child_state;
}

enum alabnf_match alabnf_matcher_process_alternative(struct alabnf_matcher * matcher, struct alabnf_matcher_state * state,struct alabnf_alternative * alternative)
  {
    ALABNF_MATCHER_DEBUG_TEXT_STATE(NULL,"process alt",state);
    struct alabnf_node * node = alternative->node;
    // remark initial node is kept as it is as ALABNF_NT_ALT, then not set.
    state->current_node = node;
    if ( node != NULL )
      {
	// new alternative is next one.
	state->alt=alternative->alt;
	struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, node);
	// current state should remember stream position
	// child_state->input = alinputstream_create_mark_shared(state->input,ALABNF_READBLOCKSIZE);
	child_state->input = state->input;
	matcher->current_state = child_state;
	return ALABNF_MATCH_REMATCH;
      }
    else
      {
	aldebug_printf(NULL,"[ERROR] null node in alternative in %s:%s:%i\n",__FILE__,__func__,__LINE__);
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

      if (current_node == NULL )
	{
	  if (initial_node != NULL )
	    {
	      // FIXME, done for alternative case when matched.
	      return ALABNF_MATCH_UNSTACK;
	    }
	  else
	    {
	      aldebug_printf(NULL,"[WARNING] no initial or current_node in %s %s %i\n",__FILE__,__func__,__LINE__);
	      return ALABNF_MATCH_NONE;
	    }
	}

      aldebug_printf(NULL,"[DEBUG] match char on node type %i %p state %p\n",current_node->type, current_node, state);
      alabnf_dump_node(current_node);

      if (initial_node->type == ALABNF_NT_ALT)
	{
	  ALABNF_MATCHER_DEBUG_TEXT_STATE(next_char,"match alt",state);
	  // current_node is one node within flat list of alternatives if not first time

	  // related SET_ALTERNATIVE
	  // this is here that we gather next alternative, this is mandatory else we hit an infinite loop
	  struct alabnf_alternative * alternative = NULL; 
	  if ( state->alt == NULL )
	    {
	      // really first time we open this state with this alt node
	      if ( current_node == initial_node )
		{
		  // take first alternative attached to current_node
		  alternative = &initial_node->content.alt;
		}
	      // else alternative is NULL
	    }
	  else
	    {
	      // will take next alternative
	      alternative = state->alt;
	      // alternative can be NULL for last one.
	    }
	  if ( alternative != NULL )
	    {
	      return alabnf_matcher_process_alternative(matcher,state, alternative);
	    }
	  // else all alternatives have been evaluated => ALABNF_MATCH_NONE	  
	}
      else if (current_node->type == ALABNF_NT_SEQUENCE)
	{
	  struct alabnf_sequence * sequence = &current_node->content.sequence;
	  struct alabnf_node * node = sequence->node;
	  if ( node != NULL )
	    {
	      if ( node->type == ALABNF_NT_SEQUENCE)
		{
		  // first node is a sequence ( unflatened sequence )
		  struct alabnf_matcher_state * child_state = alabnf_matcher_create_child_state(state, node);
		  child_state->input=state->input;
		  matcher->current_state = child_state;
		}
	      else
		{
		  // walking between next element of sequence keep same state but update node and next_sequence.
		  state->current_node = node;
		  state->next_sequence = sequence->next;
		}
	      return ALABNF_MATCH_REMATCH;
	    }	  
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
	      return ALABNF_MATCH_FULL;
	    }
	}	
      else if (current_node->type == ALABNF_NT_RULE_REF)
	{
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
	  return alabnf_matcher_process_alternative(matcher,state, alternative);	  
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] current_node type is not a string but %i , initial type is %i in %s:%s:%i\n",current_node->type,initial_node->type,__FILE__,__func__,__LINE__);
	}
    }
  return ALABNF_MATCH_NONE;
}

void alabnf_match(struct alabnf_matcher * matcher)
{
  alabnf_character * next_char = NULL;
  enum alabnf_match match = ALABNF_MATCH_NONE;
  struct alabnf_matcher_state * state = NULL;
  struct alabnf_matcher_state * parent = NULL;
  do {
    state = matcher->current_state;
    if ( match != ALABNF_MATCH_REMATCH )
      {
	next_char = alabnf_matcher_get_next_char(matcher,state);
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
      // continue;
    }
    else
    if (match == ALABNF_MATCH_UNSTACK)
    {
      // current state was fully matched, need to check parents.
      if ( parent != NULL )
	{	      
	  if ( state != parent )
	    {
	      printf("*");
	      struct alabnf_node * parent_node = parent->initial_node;
	      if ( parent_node->type == ALABNF_NT_ALT )
		{
		  // related SET_ALTERNATIVE
		  // unstack
		  // CHECKME looks fragile, required for memory leak protection
		  if ( state != &matcher->root_state )
		    {
		      alabnf_matcher_state_free(state);
		    }
		  state = parent;
		  matcher->current_state = state;
		}
	    }
	  else
	    {
	      aldebug_printf(NULL,"[FATAL] parent point on itself at %s:%s:%i",__FILE__,__func__,__LINE__);
	      break;
	    }
	}
      else
	{
	  printf("'%c' match COMPLETE %i\n",next_char->uchar,match);
	  break;
	}

    }
    else if (match == ALABNF_MATCH_FULL)
    {      
      if ( next_char != NULL )
	{
	  printf("%c",next_char->uchar);
	}
      printf("+");

      state->datablock_index=0;
      // should check we consumed full sequence of root ...
      // currently broken
      if ( state->next_sequence != NULL )
	{
	  printf("#");
	  state->current_node=state->next_sequence->node;
	  state->next_sequence=state->next_sequence->next;
	}
      else
	{
	  state->current_node=NULL;
	  state->next_sequence=NULL;
	}
      if (  state->current_node == NULL )
	{
	  // todo play with parent alternative or unflattened sequence
	  printf("matched !\n");

	  if ( parent != NULL )
	    {	      
	      struct alabnf_node * parent_node = parent->initial_node;
	      if ( parent_node->type == ALABNF_NT_ALT )
		{
		  // parent is an alternative but one did altready matched
		  // => don't check other alternative, consider alternative itself as resolved
		  // DOES THIS work ?
		  printf("\\");
		  parent->current_node = NULL;
		}
	      else if ( parent_node->type == ALABNF_NT_SEQUENCE )
		{
		  if ( state != parent )
		    {
		      printf("^");
		      // CHECKME looks fragile, required for memory leak protection
		      if ( state != &matcher->root_state )
			{
			  alabnf_matcher_state_free(state);
			}
		      // unstack
		      state = parent;
		      matcher->current_state = state;
		    }
		  else
		    {
		      aldebug_printf(NULL,"[FATAL] parent point on itself at %s:%s:%i\n",__FILE__,__func__,__LINE__);
		      break;
		    }
		}
	      else
		{
		  aldebug_printf(NULL,"[FATAL] unexpected parent type (%i) on itself at %s:%s:%i\n",parent_node->type,__FILE__,__func__,__LINE__);
		  break;
		}
	    }
	  else
	    {
	      break;
	    }
	}
    }
    else if ( match == ALABNF_MATCH_NONE )
    {
      // should handle unstacking of parent context
      // 1. is there an alternative ?

      if ( parent != NULL )
	{	      
	  printf("|");
	  if ( state != parent )
	    {
	      printf("^");	      
	      struct alabnf_node * parent_node = parent->initial_node;
	      if ( parent_node->type == ALABNF_NT_ALT )
		{
		  // related SET_ALTERNATIVE
		  // unstack
		  // CHECKME looks fragile, required for memory leak protection
		  if ( state != &matcher->root_state )
		    {
		      alabnf_matcher_state_free(state);
		    }
		  state = parent;
		  matcher->current_state = state;
		}
	    }
	  else
	    {
	      aldebug_printf(NULL,"[FATAL] parent point on itself at %s:%s:%i",__FILE__,__func__,__LINE__);
	      break;
	    }
	}
      else
	{
	  printf("'%c' match failed %i\n",next_char->uchar,match);
	  break;
	}
    }
    else if ( match == ALABNF_MATCH_REMATCH )
      {
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

