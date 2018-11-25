#include "alabnf_matcher.h"
#include <stdio.h>
#include <strings.h>

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
	state->current_rule = &alabnf->root_rule;
	if ( state->current_rule != NULL )
	  {
	    state->current_node = state->current_rule->value;
	  }
	state->datablock_index=0;
	state->parent = NULL;
      }
      
    }
}

alabnf_character * alabnf_matcher_get_next_char(struct alabnf_matcher * matcher)
{
  struct alinputstream * stream = matcher->input;
  
  unsigned char uchar = alinputstream_readuchar(stream);
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

enum alabnf_match alabnf_match_character(struct alabnf_matcher * matcher,
				       alabnf_character * next_char)
{
  struct alabnf_matcher_state * state = matcher->current_state;

  if ( state != NULL )
    {
      struct alabnf_node * current_node = state->current_node;

      if (current_node == NULL )
	{
	  aldebug_printf(NULL,"[WARNING] no current_node in %s %s %i\n",__FILE__,__func__,__LINE__);
	  return ALABNF_MATCH_NONE;
	}

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
	      aldebug_printf(NULL,"[WARNING] current_node string type is a rule reference in %s:%s:%i\n",__FILE__,__func__,__LINE__);

	      // somehow should not happen since it should not be a string but a ALABNF_NT_RULE_REF
	    }
	  else
	    {
	      // ALABNF_ST_UNDEFINED 
	      aldebug_printf(NULL,"[WARNING] current_node string type is not a quoted string but %i in %s:%s:%i\n",current_node->content.string.type,__FILE__,__func__,__LINE__);
	    }
	}
      else if (current_node->type == ALABNF_NT_SEQUENCE)
	{
	  struct alabnf_sequence * sequence = &current_node->content.sequence;
	  struct alabnf_node * node = sequence->node;
	  if ( node != NULL )
	    {
	      // KLUDGE TOY
	      // stacking ... TODO
	      state->current_node = node;
	      state->next_sequence = sequence->next;
	      return ALABNF_MATCH_REMATCH;
	    }	  
	}
      else if (current_node->type == ALABNF_NT_RULE_REF)
	{
	  struct alabnf_rule_ref * rule_ref = &current_node->content.rule_ref;
	  struct alabnf_node * node = rule_ref->resolved;
	  if ( node != NULL )
	    {
	      // KLUDGE TOY
	      // stacking ... TODO
	      state->current_node = node;
	      return ALABNF_MATCH_REMATCH;
	    }
	  else
	    {
	      aldebug_printf(NULL,"[WARNING] unresolved rule_ref in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	    }
	  
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] current_node type is not a string but %i in %s:%s:%i\n",current_node->type,__FILE__,__func__,__LINE__);
	}
    }
  return ALABNF_MATCH_NONE;
}

void alabnf_match(struct alabnf_matcher * matcher)
{
  alabnf_character * next_char = NULL;
  enum alabnf_match match = ALABNF_MATCH_NONE;
  do {
    if ( match != ALABNF_MATCH_REMATCH )
      {
	next_char = alabnf_matcher_get_next_char(matcher);
      }
    match = alabnf_match_character(matcher,next_char);
    if (match == ALABNF_MATCH_CONTINUE)
    {
      // continue;
      if ( next_char != NULL )
	{
	  printf("%c",next_char->uchar);
	}
    }
    else if (match == ALABNF_MATCH_FULL)
    {      
      if ( next_char != NULL )
	{
	  printf("%c",next_char->uchar);
	}
      
      struct alabnf_matcher_state * state = matcher->current_state;

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
	  // todo play with parent alternative.
	  printf("matched !\n");
	  break;
	}
    }
    else if ( match != ALABNF_MATCH_REMATCH )
    {
      printf("'%c' match failed %i\n",next_char->uchar,match);
      break;
    }

  }
  while ( next_char != NULL );
    
}

