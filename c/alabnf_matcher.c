#include "alabnf_matcher.h"
#include <stdio.h>

void alabnf_match_init(struct alabnf_matcher * matcher,
		       struct alabnf * alabnf,
		       struct alinputstream * input)
{
  if ( matcher != NULL )
    {
      matcher->abnf_syntax = alabnf;
      matcher->input=input;
      matcher->current_state = &matcher->root_state;
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
	  return ALABNF_MATCH_NONE;
	}

      if (current_node->type == ALABNF_NT_STRING)
	{
	  if (current_node->content.string.type == ALABNF_ST_QUOTED)
	    {
	      return abnf_match_datablock_character(matcher, &current_node->content.string.strbloc,next_char);
	    }
	}
    }
  return ALABNF_MATCH_NONE;
}

void alabnf_match(struct alabnf_matcher * matcher)
{
  alabnf_character * next_char = NULL;
  do {
    next_char = alabnf_matcher_get_next_char(matcher);
    if ( alabnf_match_character(matcher,next_char) != ALABNF_MATCH_CONTINUE )
      {
	break;
      }
  }
  while ( next_char != NULL );
    
}

