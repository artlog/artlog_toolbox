#include "alabnf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* code created by a human brain */

/*
      NOTE:     ABNF strings are case-insensitive and
                  the character set for these strings is us-ascii.
*/

// maximum iteration of state machine for parsing (protection against inifinite loop )
// in relateive size of read characters
// 5 times number of chars in text...
const int ALABNF_MAX_INTERNAL_REMATCH_TIMES=5;

// max number char in one string
const int ALABNF_MAX_CHARS=100;
// const value indicating that iterator is infinite
const int ALABNF_INFINITE_ITERATION=-1;

#define ALABNF_DEBUG_UNEXPECTED_CHAR(c) aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in %s %s %i\n",c,c>32 ? c : '.',__FILE__,__func__,__LINE__);

#define ALABNF_DEBUG_TEXT_STATE(c,text,state_machine) aldebug_printf(NULL,"[DEBUG] %s ('%i','%c') at %i:%i  in %s %s %i\n",text,c,c>=32 ? c : '.' ,state_machine->lf_line,state_machine->current_indent,__FILE__,__func__,__LINE__);

void alabnf_start_string(struct alabnf_sm * state_machine, char c);
void alabnf_string(struct alabnf_sm * state_machine, char c);
void alabnf_dump_sequence(struct alabnf_sequence * start_sequence);
void alabnf_dump_alternative(struct alabnf_alternative * alternative);
void alabnf_dump_range(struct alabnf_range * range);
void alabnf_close_group(struct alabnf_sm * state_machine, char c);
void alabnf_close_name_string_continue(struct alabnf_sm * state_machine, char c);
void alabnf_close_name_string_rematch(struct alabnf_sm * state_machine, char c);
void alabnf_close_string_rematch(struct alabnf_sm * state_machine, char c);
void alabnf_close_string_continue(struct alabnf_sm * state_machine, char c);
// <a>*<b>element
void alabnf_iterator_string_start(struct alabnf_sm * state_machine, char c);
// any new element on left part ( ie after = )
void alabnf_start_string_ruledef(struct alabnf_sm * state_machine, char c);

struct alabnf_node * alabnf_collect_node_sequence(struct alabnf * alabnf, struct alabnf_node * collector, struct alabnf_node * new_node);
  
struct alabnf_node * alabnf_create_node(struct alabnf * alabnf, enum alabnf_node_type type)
{
  // use alabnf->context allocator.
  struct alabnf_node * node = (struct alabnf_node *) ALALLOC(alabnf->context.allocator, sizeof(struct alabnf_node));
  bzero(node,sizeof(*node));
  node->type = type;
  return node;
}

struct alabnf_node * alabnf_create_iterator_node(struct alabnf * alabnf, int min,int max,struct alabnf_node * node)
{
  struct alabnf_node * iterator_node=alabnf_create_node(alabnf,ALABNF_NT_ITERATOR);
  if ( iterator_node != NULL )
    {
      struct alabnf_iterator * iterator = &iterator_node->content.iterator;
      iterator->min=min;
      iterator->max=max;
      iterator->node=node;
      return iterator_node;
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] allocation of iterator failed\n");
    }
  return NULL;
}

struct alabnf_node * alabnf_create_sequence_node(struct alabnf * alabnf, struct alabnf_node * node, struct alabnf_sequence * next)
{
  struct alabnf_node * sequence_node=alabnf_create_node(alabnf,ALABNF_NT_SEQUENCE);
  if ( sequence_node != NULL )
    {
      struct alabnf_sequence * sequence = &sequence_node->content.sequence;
      sequence->node = node;
      sequence->next = next;
      return sequence_node;
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] allocation of sequence failed\n");
    }
  return NULL;
}

void alabnf_print_token(struct alhash_entry * mytoken)
{
    if ( mytoken != NULL )
    {
      if ( mytoken->key.data.ptr != NULL )
	{
	  struct alhash_datablock * datablock = &mytoken->key;
	  printf(ALPASCALSTRFMT" ",
		 ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
	}
    }

}

void alabnf_dump_iterator(struct alabnf_iterator * iterator)
{
  if ( iterator != NULL )
    {
      if ( iterator->min == 0 )
	{
	  // prefix
	  if ( iterator->max == 1 )
	    {
	      // optional
	      printf("[");
	    }
	  else if ( iterator->max == 0 )
	    {
	      // well empty whatever
	      printf("0");
	    }
	  else if ( iterator->max == ALABNF_INFINITE_ITERATION )
	    {
	      printf("*");		   
	    }
	  else
	    {
	      printf("%i*%i",iterator->min,iterator->max);

	    }
	  alabnf_dump_node(iterator->node);
	  // suffix
	  if ( iterator->max == 1 )
	    {
	      // optional
	      printf("]");
	    }

	}
      else
	{
	  if ( iterator->max == ALABNF_INFINITE_ITERATION )
	    {
	      printf("m%i*",iterator->min);
	    }
	  else
	    {
	      printf("m%i*M%i",iterator->min,iterator->max);
	    }
	  alabnf_dump_node(iterator->node);
	}
    }
}

void alabnf_dump_hex_string(aldatablock * string )
{
  printf("%%x");
  for (int i=0; i< string->length-1; i++)
    {
      printf("%x.",string->data.charptr[i]);
    }
  printf("%x",string->data.charptr[string->length-1]);
}

void alabnf_dump_rule_ref(struct alabnf_rule_ref * rule_ref)
{
  struct alhash_datablock * datablock = &rule_ref->keyblock;
  if ( datablock->data.ptr != NULL )
    {
      printf(ALPASCALSTRFMT,
	     ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
    }
  else
    {
      printf("unamed_rule_ref%p",rule_ref);
    }

}

void alabnf_dump_node(struct alabnf_node * node )
{
  if ( node != NULL )
    {
      if ( (unsigned long) node <  64L )
	{
	  // corrupted pointer (often address within a NULL struct )
	  aldebug_printf(NULL,"[FATAL] node pointer %p invalid in %s",node, __FILE__);
	  //return;
	}
      enum alabnf_node_type type = node->type;
      switch(type)
	{
	case ALABNF_NT_ITERATOR:
	  alabnf_dump_iterator(&node->content.iterator);
	  break;
	case ALABNF_NT_SEQUENCE:
	  alabnf_dump_sequence(&node->content.sequence);
	  break;
	case ALABNF_NT_STRING:
	  {
	    struct alhash_datablock * datablock = &node->content.string.strbloc;
	    enum alabnf_string_type string_type = node->content.string.type;
	    if ( string_type == ALABNF_ST_RULENAME )
	      {		
		// for debug : printf("'"ALPASCALSTRFMT"'",
		printf(ALPASCALSTRFMT,
		       ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
	      }
	    else if ( string_type == ALABNF_ST_QUOTED )
	      {			
		printf("\""ALPASCALSTRFMT"\"",
		       ALPASCALSTRARGS(datablock->length,datablock->data.charptr));

	      }
	    else if ( string_type == ALABNF_ST_UNDEFINED )
	      {
		printf("ALABNF_ST_UNDEFINED_");
		alabnf_dump_hex_string(datablock);
	      }
	    else
	      {
		// need a better printf to support non printable strings as hex,int, quoted ...
		alabnf_dump_hex_string(datablock);
	      }

	  }
	  break;
	case ALABNF_NT_ALT:
	  alabnf_dump_alternative(&node->content.alt);
	  break;
	case ALABNF_NT_RANGE:
	  alabnf_dump_range(&node->content.range);
	  break;
	case ALABNF_NT_RULE_REF:
	  alabnf_dump_rule_ref(&node->content.rule_ref);
	  break;
	default:
	  printf("unrecognized abnf type %i\n",type);
	}
    }
  else
    {
      // NULL
      aldebug_printf(NULL,"[ERROR] node pointer NULL in %s %s %i", __FILE__, __func__,__LINE__);
      printf("¤");
    }
}

void alabnf_dump_alternative(struct alabnf_alternative * start_alternative)
{
  struct alabnf_alternative * alternative=start_alternative;
  struct alabnf_alternative * next_alternative=NULL;
  // debug
  //printf("{");
  while (alternative != NULL)
    {
      next_alternative = alternative->alt;
      aldebug_printf(NULL,"node %p alt %p\n", alternative->node, next_alternative);
      // DEBUG only, to remove
      alabnf_dump_node(alternative->node);
      if (next_alternative != NULL)
	{
	  printf("/");
	}
      alternative=next_alternative;
    }
  // debug
  //printf("}");


}

void alabnf_dump_range(struct alabnf_range * range)
{
  // TODO
  printf("%%x%x-%x",range->start,range->end);
}

void alabnf_dump_sequence(struct alabnf_sequence * start_sequence)
{
  struct alabnf_sequence * sequence=start_sequence;
  struct alabnf_sequence * next_sequence=NULL;
  printf("(");
  while (sequence != NULL)
    {
      next_sequence = sequence->next;
      aldebug_printf(NULL,"\nnext %p node %p\n", next_sequence, sequence->node);
      // debug printf("§");
      alabnf_dump_node(sequence->node);
      // debug printf("$");
      if (next_sequence != NULL)
	{
	  printf(" ");
	}
      sequence=next_sequence;
    }
  printf(")");
}

void alabnf_dump_rule(struct alabnf_rule * rule)
{
  // TODO
}

struct alabnf_node * alabnf_create_node_string(
					       struct alabnf * alabnf, aldatablock * block,
					       enum alabnf_string_type type)
{
    struct alabnf_node * node =  alabnf_create_node(alabnf, ALABNF_NT_STRING);
    struct alabnf_string * node_string = &node->content.string;
    node_string->type=type;
    node_string->strbloc.data.charptr=al_copy_block(&alabnf->context.allocator.ringbuffer,block);
    node_string->strbloc.length=block->length;
    node_string->strbloc.type=block->type;

    if ( (unsigned long) node < 64L )
    {
      aldebug_printf(NULL,"[FATAL] invalid pointer node %p %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
      node = NULL;
    }

    return node;
}

struct alabnf_node * alabnf_create_alternative_node(struct alabnf * alabnf,
						    struct alabnf_node * node,
						    struct alabnf_alternative * alt)
{
    struct alabnf_node * alternative_node =  alabnf_create_node(alabnf, ALABNF_NT_ALT);
    struct alabnf_alternative * node_alternative = &alternative_node->content.alt;
    node_alternative->node = node;
    node_alternative->alt = alt;

    return alternative_node;
}

struct alabnf_node * alabnf_get_abnf_token(struct alhash_entry * entry)
{
  if ( entry->key.data.ptr == entry->value.data.ptr )
    {
      // Not yet abnfized, in fact should always be the case for terminals ( ie not rule refs ).
      return NULL;
    }

  // can happen with rule_ref name.
  aldebug_printf(NULL,"[DEBUG] existing token entry %p %s:%s:%i\n",entry, __FILE__,__func__,__LINE__);
  // was implemented this way but altering tokenizer token and mixing terminals with non terminal is a bad idea
  struct alabnf_node * node = (struct alabnf_node *) entry->value.data.ptr;

  if ( (unsigned long) node < 64L )
    {
      aldebug_printf(NULL,"[FATAL] invalid pointer node %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
      node = NULL;
    }

  if ( (node == NULL) || ( node->type == ALABNF_NT_INVALID ))
    {
      // ooops
      aldebug_printf(NULL,"[FATAL] invalid node %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
    }
  return node;
}

/* 
create a node string of type state_machine->string_type
*/
struct alabnf_node * alabnf_build_abnf_node(struct alabnf_sm * state_machine, struct alhash_entry * mytoken)
{
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);
  // should alter value to point on alabnf_string
  // we copy key to node->string by allocating/copying it on new context.
  struct alabnf_node * node = alabnf_create_node_string(alabnf,&mytoken->key, state_machine->string_type);
  if ((node == NULL) || ( node->type == ALABNF_NT_INVALID ))
    {
      // ooops
      aldebug_printf(NULL,"[FATAL] invalid node %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
    }
  return node;
}


void alabnf_start_ruleline(struct alabnf_sm * state_machine, char c)
{
  alabnf_start_string(state_machine, c);
}


void alabnf_state_machine_init(struct alabnf_sm *state_machine,   struct alinputstream * inputstream)
{
  bzero(state_machine,sizeof(*state_machine));
  state_machine->one_char_method = alabnf_start_ruleline;
  state_machine->next_action = ALABNF_PA_CONTINUE;
  state_machine->inputstream = inputstream;
  state_machine->rematch=0;
  state_machine->state=ALABNF_STATE_RULENAME;
  state_machine->string_type=ALABNF_ST_RULENAME;
  state_machine->stack=alstack_allocate();
  altokenizer_init(&state_machine->tokenizer);
}

void alabnf_state_machine_release(struct alabnf_sm * state_machine)
{
  altokenizer_release(&state_machine->tokenizer);  
};

// cumulated number is returned and reset.
int alabnf_flush_number(struct alabnf_number_sm * state_machine)
{
  int value = state_machine->cumulated;
  aldebug_printf(NULL,"\n%i 0x%x '%c'\n",value,value,( ( value > 32 ) && ( value < 128 )) ? (char) value : '.');
  state_machine->cumulated=0;
  state_machine->seen=0;

  return value;
}

void alabnf_add_char(struct alabnf_sm * state_machine, char token, char c)
{
  //aldebug_printf(NULL,"%c%c",token,c);
  altokenizer_add_char(&state_machine->tokenizer,token,c);

  int pending_chars = altokenizer_get_pending_chars(&state_machine->tokenizer);
  if ( pending_chars > ALABNF_MAX_CHARS )
    {
      aldebug_printf(NULL,"[FATAL] too big token pending_chars %i > %i in %s %s %i\n",pending_chars,ALABNF_MAX_CHARS,__FILE__,__func__,__LINE__);
      exit(1);
    }
}

void alabnf_handle_lf(struct alabnf_sm * state_machine)
{
  // lf , expect a new line
  aldebug_printf(NULL,"LF\n");
  state_machine->lf_line ++;
  
  // tentative to handle mutliple files parsing, reset rule if mutliple empty lines...
  if ( (state_machine->current_indent == 0 ) && (  state_machine->linebreak > 1 ) )
    {
      aldebug_printf(NULL,"[DEBUG] empty lines %i\n", state_machine->lf_line); 
      state_machine->linebreak = 0;
    }
  else
    {
      state_machine->linebreak ++;
    }
  state_machine->current_indent = 0;

}

void alabnf_handle_space(struct alabnf_sm * state_machine)
{
}

// cumulated number is converted to char and reset.
void alabnf_flush_number_to_char(struct alabnf_sm * state_machine)
{
  int value = alabnf_flush_number(&state_machine->number_sm);

  // if needed for range.
  if ( state_machine->number_sm.state == ALABNF_NSM_MAX_SET )
    {
      state_machine->number_sm.max=value;
    }
  else
    {
      state_machine->number_sm.min=value;
      alabnf_add_char(state_machine,'?',(char) value);
    }
}

// within a comment eat everything until a LF
void alabnf_comment(struct alabnf_sm * state_machine, char c)
{
  if ( c != 10 )
    {
      // remark eat CR too
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      if ( state_machine->state == ALABNF_STATE_RULENAME )
	{
	  aldebug_printf(NULL,"[ERROR] comment within rule name at %i:%i in %s %s %i\n",
			 state_machine->lf_line,state_machine->current_indent,
			 __FILE__,__func__,__LINE__);

	  state_machine->one_char_method = alabnf_start_string;
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      else
	{
	  // rematch ? sure ?
	  state_machine->next_action = ALABNF_PA_REMATCH;
	  state_machine->one_char_method = alabnf_start_string_ruledef;
	}
    }
}


void alabnf_chevron_start(struct alabnf_sm * state_machine, char c)
{
  if ( c != '>' )
    {
      state_machine->next_action = ALABNF_PA_REMATCH;
      state_machine->one_char_method = alabnf_string;
      state_machine->string_type=ALABNF_ST_RULENAME;
    }
  else
    {
      state_machine->next_action = ALABNF_PA_CONTINUE;
      state_machine->one_char_method = alabnf_start_string_ruledef;
    }
}

static int alabnf_is_closing_word(char c)
{
  // why not ')' ?
  return (( c == ' ') || ( c== '>' ));
}

static int alabnf_is_close_any(char c)
{
  return (( c == ')') || ( c== ']' ));
}

void alabnf_binary_string(struct alabnf_sm * state_machine, char c)
{
  if ( ( c >= '0' ) && ( c <= '1' ) )
    {
      state_machine->string_type=ALABNF_ST_HEX;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      if ( c == '.' )
	{
	  // concatenation
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      else if ( c == '-' )
	{
	  // TODO FIXME range
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	  return;
	}
      else
	{
	  state_machine->close_method = alabnf_close_string_rematch;
	  state_machine->next_action = ALABNF_PA_CLOSE;
	}
    }
}

void alabnf_decimal_string(struct alabnf_sm * state_machine, char c)
{
  struct alabnf_number_sm * number_sm = &state_machine->number_sm;
  if ( ( c >= '0' ) && ( c <= '9' ) )
    {
      state_machine->string_type=ALABNF_ST_DEC;
      int result = number_sm->cumulated*10 + (c-'0');
      if ( result < 256 )
	{
	  number_sm->cumulated = result;
	}
      number_sm->seen++;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      alabnf_flush_number_to_char(state_machine);
      if ( c == '.' )
	{
	  // concatenation
	 state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      else if ( c == '-' )
	{
	  // TODO FIXME range
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	  return;
	}
      else
	{
	  state_machine->close_method = alabnf_close_string_rematch;
	  state_machine->next_action = ALABNF_PA_CLOSE;
	}
    }
}

void alabnf_hexadecimal_string(struct alabnf_sm * state_machine, char c)
{
  struct alabnf_number_sm * number_sm = &state_machine->number_sm;
  // printf("\nhex %c\n",c);
  if (
      (( c >= '0' ) && ( c <= '9' ))
      )
    {
      state_machine->string_type=ALABNF_ST_HEX;
      number_sm->cumulated = number_sm->cumulated << 4 | (c-'0');
      number_sm->seen++;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
      if (
      (( c >= 'a' ) && ( c <= 'f' ))
      )
    {
      state_machine->string_type=ALABNF_ST_HEX;
      number_sm->cumulated = number_sm->cumulated << 4 | (0xa+c-'a');
      number_sm->seen++;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
    else
      if (
	(( c >= 'A' ) && ( c <= 'F' ))
      )
    {
      //
      state_machine->string_type=ALABNF_ST_HEX;
      number_sm->cumulated = number_sm->cumulated << 4 | (0xA+c-'A');
      number_sm->seen++;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
      else	
	{
	if ( c == '.' )
	  {
	    // concatenation
	    number_sm->state=ALABNF_NSM_START;
	    state_machine->next_action = ALABNF_PA_CONTINUE;
	    return;
	  }
	else
	if ( c == '-' )
	  {
	    // range
	    number_sm->state=ALABNF_NSM_MIN_SET;
	    state_machine->next_action = ALABNF_PA_CONTINUE;
	    return;
	  }
	else
	  {
	    // end of parsing for hexadecimal string / range.
	    if ( number_sm->state==ALABNF_NSM_MAX_SET )
	      {
		state_machine->string_type=ALABNF_ST_RANGE;		
	      }
	    state_machine->close_method = alabnf_close_string_rematch;
	    state_machine->next_action = ALABNF_PA_CLOSE;
	    return;
	  }
      }

  if (number_sm->seen > 1)
    {
      if ( number_sm->state==ALABNF_NSM_MIN_SET )
	{
	  number_sm->state=ALABNF_NSM_MAX_SET;
	}
      alabnf_flush_number_to_char(state_machine);
    }
}

// first char of string was recognized next can then be a digit
void alabnf_string_suffix_intern(struct alabnf_sm * state_machine, char c)
{
  if (
      (( c >= 'a' ) && ( c <= 'z' ))
      ||
      (( c >= 'A' ) && ( c <= 'Z' ))
      ||
      (( c == '-'))
      ||
      (( c >= '0' ) && ( c <= '9' ))
      )
    {
      alabnf_add_char(state_machine,'s',(char) c);      
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else    
    {
      if ( alabnf_is_closing_word(c) )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] '%i' '%c' in alabnf rule definition %s %s %i\n",c,c >=32 ? c : '.',__FILE__,__func__,__LINE__);	  
	  state_machine->close_method = alabnf_close_name_string_rematch;
	}
      state_machine->next_action = ALABNF_PA_CLOSE;
    }
}

void alabnf_name_string_intern(struct alabnf_sm * state_machine, char c)
{
  if (
      (( c >= 'a' ) && ( c <= 'z' ))
      ||
      (( c >= 'A' ) && ( c <= 'Z' ))
      ||
      (( c == '-'))
      )
    {
      alabnf_add_char(state_machine,'s',(char) c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      if (( c >= '0') && ( c <= '9' ))
	{
	  aldebug_printf(NULL,"[WARNING] '%i' '%c' numbers starting a string %s:%s:%i\n",
			 c,c >=32 ? c : '.',
			 __FILE__,__func__,__LINE__);
	}
      if ( alabnf_is_closing_word(c) )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{
	  if ( ! alabnf_is_close_any(c) )
	    {
	      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
	    }
	  state_machine->close_method = alabnf_close_name_string_rematch;
	}
      state_machine->next_action = ALABNF_PA_CLOSE;
    }
}

void alabnf_name_string_suffix(struct alabnf_sm * state_machine, char c)
{
  alabnf_string_suffix_intern(state_machine, c);
  if ( state_machine->next_action == ALABNF_PA_CLOSE )
    {
      if ( alabnf_is_closing_word(c) )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{

	  state_machine->close_method = alabnf_close_name_string_rematch;
	}
    }

}

void alabnf_name_string(struct alabnf_sm * state_machine, char c)
{
  alabnf_name_string_intern(state_machine, c);
  if ( state_machine->next_action == ALABNF_PA_CLOSE )
    {
      if ( alabnf_is_closing_word(c) )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in %s %s %i\n",c,c,__FILE__,__func__,__LINE__);
	  state_machine->close_method = alabnf_close_name_string_rematch;
	}
    }
  else
    {
      state_machine->one_char_method=alabnf_name_string_suffix;
    }

}


void alabnf_string_suffix(struct alabnf_sm * state_machine, char c)
{
  alabnf_string_suffix_intern(state_machine, c);
  if ( state_machine->next_action == ALABNF_PA_CLOSE )
    {
      if ( alabnf_is_closing_word(c) )
      	{
	  state_machine->close_method = alabnf_close_string_continue;
	}
      else
	{
	  ALABNF_DEBUG_UNEXPECTED_CHAR(c)
	  state_machine->close_method = alabnf_close_string_rematch;
	}
    }

}

void alabnf_string(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE(c,"string",state_machine);
  alabnf_name_string_intern(state_machine, c);
  if ( state_machine->next_action == ALABNF_PA_CLOSE )
    {
      if ( alabnf_is_closing_word(c) )
	{
	  state_machine->close_method = alabnf_close_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in %s %s %i\n",c,c,__FILE__,__func__,__LINE__);
	  state_machine->close_method = alabnf_close_string_rematch;
	}
    }
  else
    {
      state_machine->one_char_method=alabnf_string_suffix;
    }

}

// stack an empty sequence ( as group tag )
void albnf_stack_sequence(struct alabnf_sm * state_machine)
{
  struct alstack * stack = state_machine->stack;
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);

  ALABNF_DEBUG_TEXT_STATE('.',"stack_sequence",state_machine);
  
  struct al_token token;
  token.token=ALABNF_NT_SEQUENCE;

  // HACK to overcome  altokenizer_dict_add_string: Assertion `length!=0' failed
  // HACK REUSE iterator value
  alabnf_add_char(state_machine,'^',(char) state_machine->iterator_index);  
  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,'(');

  struct alabnf_node * sequence_node = alabnf_create_sequence_node(alabnf, NULL,NULL);
  alstack_push_ref(stack,sequence_node);
  
  state_machine->iterator_index ++;

}
  
void alabnf_group_start(struct alabnf_sm * state_machine, char c)
{
  albnf_stack_sequence(state_machine);
}

void alabnf_stack_iterator(struct alabnf_sm * state_machine,int min, int max)
{
  if ( ( min == 1 ) && ( max == 1 ))
    {
      aldebug_printf(NULL,"[WARNING] useless iterator min == max == 1 at %s:%s:%i",__FILE__,__func__,__LINE__);
    }
  struct alstack * stack = state_machine->stack;
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);

  ALABNF_DEBUG_TEXT_STATE('.',"stack_iterator",state_machine);
  
  struct al_token token;
  token.token=ALABNF_NT_ITERATOR;
  
  //HACK to overcome  altokenizer_dict_add_string: Assertion `length!=0' failed
  alabnf_add_char(state_machine,'^',(char) state_machine->iterator_index);  
  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,'^');

  struct alabnf_node * iterator_node = alabnf_create_iterator_node(alabnf,min,max,NULL);
  alstack_push_ref(stack,iterator_node);
 
  state_machine->iterator_index ++;
}

void alabnf_alternative_start(struct alabnf_sm * state_machine, char c)
{
 
  struct alstack * stack = state_machine->stack;
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);

  struct alstackelement * element=NULL;

  if ( alstack_used(stack) < 2 )
    {
      aldebug_printf(NULL,"[ERROR] starting an alternative with a stack containing only rulename %s %s %i\n",__FILE__,__func__,__LINE__);
      return;
    }

  element=alstack_pop(stack);
  if ( element != NULL )
    {
      struct alabnf_node * node = (struct alabnf_node *) element->reference;
  
      if ( node != NULL )
	{
	  struct al_token token;
	  token.token=ALABNF_NT_ALT;

	  //HACK to overcome  altokenizer_dict_add_string: Assertion `length!=0' failed
	  alabnf_add_char(state_machine,'/',(char) state_machine->iterator_index);  
	  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,'/');

	  struct alabnf_node * iterator_node = alabnf_create_alternative_node(alabnf,node,NULL);
	  alstack_push_ref(stack,iterator_node);

	  state_machine->iterator_index ++;
	}
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] alternative left stacked part was empty %s:%s:%i\n",__FILE__,__func__,__LINE__);
      exit(1);
    }
}

// ('('|'<') element  / iterator prefix is already handled
void alabnf_element_start(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE(c,"element_start",state_machine);
  state_machine->one_char_method=alabnf_element_start;
  if ( c == '(' )
    {
      alabnf_group_start(state_machine,c);
      state_machine->one_char_method=alabnf_start_string_ruledef;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else if ( c == '<' )
    {
      state_machine->one_char_method=alabnf_chevron_start;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else if ( c == ' ' )
    {
      // ignore spaces here.
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  // not smart, was detected on invalid abnf with + - non quoted.
  // FIXME should actualy really know exhaustive list of unacceptable chars here
  else if (  ( c == '+' ) || ( c == '-' ) )
    {
      ALABNF_DEBUG_UNEXPECTED_CHAR(c);      
      state_machine->next_action = ALABNF_PA_FAIL;
    }  
  else
    // TODO FIXME can be typed string
    {
      // string suffix since we directly call name_string_intern for first char
      state_machine->one_char_method = alabnf_string;
      state_machine->string_type=ALABNF_ST_RULENAME;

      state_machine->next_action = ALABNF_PA_REMATCH; 
    }
}

/* start of ITERATOR PARSING */

// <b>element
void alabnf_iterator_max(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE(c,"iterator_max",state_machine);
  if (
      (( c >= '0' ) && ( c <= '9' ))
      )
    {
      if ( state_machine->it_state != ALABNF_ISM_MAX_START )
	{
	  state_machine->it_state = ALABNF_ISM_MAX_START;
	  state_machine->it_max=(c-'0');
	}
      else
	{
	  state_machine->it_max=state_machine->it_max*10+(c-'0');
	}
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      alabnf_stack_iterator(state_machine,state_machine->it_min,state_machine->it_max);
      alabnf_element_start(state_machine,c);
    }
}

// a+*<b>element
void alabnf_iterator_min(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE(c,"iterator_min",state_machine);
  if (
      (( c >= '0' ) && ( c <= '9' ))
      )
    {
      state_machine->it_state = ALABNF_ISM_MIN_START;
      state_machine->it_min=state_machine->it_min*10+(c-'0');
    }
  else if ( c == '*')
    {
      if ( state_machine->it_state !=  ALABNF_ISM_MIN_START )
	{
	  state_machine->it_min=0;
	}
      state_machine->it_max=ALABNF_INFINITE_ITERATION;
      state_machine->it_state = ALABNF_ISM_MIN_SET;
      state_machine->one_char_method=alabnf_iterator_max;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      alabnf_stack_iterator(state_machine,state_machine->it_min,state_machine->it_max);
      alabnf_element_start(state_machine,c);
    }

}

// <a>*<b>element
void alabnf_iterator_string_start(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE(c,"iterator_string",state_machine);
  state_machine->it_state = ALABNF_ISM_START;
  state_machine->it_min=0;
  state_machine->it_max=0;
  if (
      (( c >= '0' ) && ( c <= '9' ))
      )
    {
      state_machine->it_state = ALABNF_ISM_MIN_START;
      state_machine->it_min=c-'0';
      state_machine->one_char_method=alabnf_iterator_min;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else if ( c == '*')
    {
      state_machine->it_state = ALABNF_ISM_MIN_SET;
      state_machine->it_min=0;
      state_machine->it_max=ALABNF_INFINITE_ITERATION;
      state_machine->one_char_method=alabnf_iterator_max;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {

      state_machine->it_min=1;
      state_machine->it_max=1;
      // no iterator
      // alabnf_stack_iterator(state_machine,state_machine->it_min,state_machine->it_max);
      alabnf_element_start(state_machine,c);
    }

}

/* end of ITERATOR PARSING */

void alabnf_start_typed_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;

  ALABNF_DEBUG_TEXT_STATE(c,"typed_string",state_machine);
  state_machine->number_sm.state=ALABNF_NSM_START;
  
  switch(c)
    {
    case 'b':
      state_machine->one_char_method=alabnf_binary_string;
      next_action = ALABNF_PA_CONTINUE;
      break;
    case 'd':
      state_machine->one_char_method=alabnf_decimal_string;
      next_action = ALABNF_PA_CONTINUE;
      break;
    case 'x':
      state_machine->one_char_method=alabnf_hexadecimal_string;
      next_action = ALABNF_PA_CONTINUE;
      break;
    default:
      next_action = ALABNF_PA_FAIL;
    }


  state_machine->next_action = next_action;
  
}

void alabnf_quoted_string(struct alabnf_sm * state_machine, char c)
{
  state_machine->string_type=ALABNF_ST_QUOTED;
  ALABNF_DEBUG_TEXT_STATE(c,"ALABNF_ST_QUOTED",state_machine);
  switch(c)
    {
    case '"':
      state_machine->close_method = alabnf_close_string_continue;
      state_machine->next_action = ALABNF_PA_CLOSE;
      break;
    default:
      alabnf_add_char(state_machine,'"',c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
}

// where alternative are glued
// TODO check iterator HERE too ? ( check callers ).
// return non NULL head if entry was merged
struct alabnf_node * alabnf_merge_alt_and_iterator_with_head(  struct alabnf * alabnf, struct alabnf_node * head_node, struct alabnf_node * node)
{
  if ( head_node != NULL )
    {
      aldebug_printf(NULL,"[INFO] head node found %p type %i node %p in %s:%s:%i\n",
		     head_node, head_node->type, node,
		     __FILE__,__func__,__LINE__);

      if ( head_node->type == ALABNF_NT_ALT )
	{

	  struct alabnf_alternative * alternative = &head_node->content.alt;
	  if ( alternative->alt == NULL )
	    {
	      // an empty slot : merge entry here
	      struct alabnf_node * merge_entry = alabnf_create_alternative_node(alabnf,node,NULL);
	      alternative->alt=&merge_entry->content.alt;
	      aldebug_printf(NULL,"[INFO] MERGE ALTERNATIVES %p alt %p\n", alternative, alternative->alt);
	      /*
	      printf("\n");
	      alabnf_dump_node(head_node);
	      */
	    }
	  else
	    {
	      aldebug_printf(NULL,"[INFO] SKIP ALTERNATIVES %p alt %p\n", alternative, alternative->alt);
	      head_node = NULL;
	    }
	}
      else if ( head_node->type == ALABNF_NT_ITERATOR )
	{
	  // CHECK if iterator computation can always be done there...
	  struct alabnf_iterator * iterator = &head_node->content.iterator;
	  aldebug_printf(NULL,"[DEBUG] MERGE ITERATOR %p node %p type %i %p at %s:%s:%i\n",
			 iterator->node, node, node->type, head_node,
			 __FILE__,__func__,__LINE__);
	  if ( iterator->node == NULL)
	    {
	      if ( node->type == ALABNF_NT_ITERATOR )
		{
		  // WEIRD two iterators to merge ?
		}
	      else if ( node->type == ALABNF_NT_ALT )
		{
		  // should convert alt in group ?
		}

	      // an empty alternative slot merge
	      // return head_node as it is and use node as iterator node.
	      iterator->node = node;
	    }
	  else
	    {
	      head_node = NULL;
	    }
	}
      else
	{
	  head_node = NULL;
	}
    }
  return head_node;
}

// return a node that is a sequence if collector is non null
struct alabnf_node * alabnf_collect_node_sequence(struct alabnf * alabnf, struct alabnf_node * collector, struct alabnf_node * new_node)
{  
  struct alabnf_sequence * sequence=NULL;
  struct alabnf_sequence * next_sequence=NULL;
  struct alabnf_node * node=NULL;

  aldebug_printf(NULL,"[DEBUG] in %s %s %i\n",__FILE__,__func__,__LINE__);
  
  if ( collector != NULL )
    {
      if ( collector->type == ALABNF_NT_SEQUENCE )
	{
	  next_sequence = &collector->content.sequence;
	}
      else
	{
	  // collector was a simple token and new_node is a token too ? really ?
	  next_sequence = NULL;

	  if ( ( new_node->type ==  ALABNF_NT_ALT )
	       || ( new_node->type ==  ALABNF_NT_ITERATOR ) )
	    {
	      struct alabnf_node * head_node = alabnf_merge_alt_and_iterator_with_head(alabnf, new_node, collector);
	      if ( head_node != NULL )
		{
		  node = head_node;
		  return node;
		}
	      // what about new_node here ?
	    }

	  node =  alabnf_create_node(alabnf, ALABNF_NT_SEQUENCE);
	  if ( node != NULL )
	    {
	      next_sequence = &node->content.sequence;
	      next_sequence->node = collector;
	      next_sequence->next = NULL;
	    }
	  else
	    {
	      aldebug_printf(NULL,"[FATAL] null allocated node in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	    }
	}
      node =  alabnf_create_node(alabnf, ALABNF_NT_SEQUENCE);
      if ( node != NULL )
	{
	  sequence = &node->content.sequence;
	  sequence->node = new_node;
	  sequence->next = next_sequence;
	  next_sequence=sequence;
	}
      else
	{
	  aldebug_printf(NULL,"[FATAL] null node in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	}
    }
  else
    {
      if ( new_node != NULL )
	{	  
	  if ( new_node->type !=  ALABNF_NT_SEQUENCE )
	    {
	      aldebug_printf(NULL,"[DEBUG] new node type %i in %s:%s:%i\n",new_node->type,__FILE__,__func__,__LINE__);
	      if ( new_node->type == ALABNF_NT_INVALID )
		{
		  aldebug_printf(NULL,"[FATAL] invalid new node %p in %s:%s:%i\n",new_node,__FILE__,__func__,__LINE__);
		}
	      node=new_node;
	    }
	  else
	    {
	      aldebug_printf(NULL,"[DEBUG] don't append to child sequence in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	      node =  alabnf_create_node(alabnf, ALABNF_NT_SEQUENCE);
	      if ( node != NULL )
		{
		  sequence = &node->content.sequence;
		  sequence->node = new_node;
		  sequence->next = NULL;
		}
	      else
		{
		  aldebug_printf(NULL,"[FATAL] null allocated node in %s:%s:%i\n",__FILE__,__func__,__LINE__);
		}
	    }
	}	
    }

  return node;
}


struct alabnf_node * alabnf_fetch_head_node(struct alstack * stack)
{
  struct alabnf_node * node = NULL;
  struct alstackelement * element=NULL;
  element=alstack_fetch(stack);
  if (element != NULL )
    {
      node = (struct alabnf_node *) element->reference;
    }
  return node;
}

// where alternative are glued
void alabnf_merge_with_head(  struct alabnf * alabnf, struct alstack * stack, struct alabnf_node * node)
{
  // TODO check iteratorS and alternativeS
  struct alabnf_node * head_node = alabnf_fetch_head_node(stack);
  if ( head_node != NULL )
    {
      head_node = alabnf_merge_alt_and_iterator_with_head(alabnf,head_node,node);
    }
  // if entry was not merged with head node
  if ( head_node == NULL )
    {
      alstack_push_ref(stack,node);
    }
}

// unstack until incomplete iterator found.
// stack full iterator with content as sequence
void alabnf_close_iterator(struct alabnf_sm * state_machine, char c)
{
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);
  struct alstack * stack = state_machine->stack;
  
  struct alstackelement * element=NULL;
  struct alabnf_node * collected_node = NULL;
  
  ALABNF_DEBUG_TEXT_STATE(c,"close_iterator",state_machine);
  
  if ( alstack_used(stack) < 2 )
    {
      aldebug_printf(NULL,"[ERROR] closing an iterator with a stack containing only rulename %s %s %i\n",__FILE__,__func__,__LINE__);
      return;
    }
  
  element=alstack_pop(stack);
  if (element != NULL )
    {
      struct alabnf_node * node = (struct alabnf_node *) element->reference;
  
      while ( node != NULL )
	{
	  // TOCHECK since rework
	  // FIXME uses state_machine->string_type which is not sync at this stage.
	  // struct alabnf_node * node = alabnf_build_abnf_node(state_machine,entry);
	  if ( (node != NULL ) && (node->type == ALABNF_NT_ITERATOR ) )
	    {
	      struct alabnf_iterator * iterator = &node->content.iterator;
	      if ( iterator->node == NULL )
		{
		  // iterator start found, can create iterator object and return
		  iterator->node = collected_node;

		  // should push an entry : this one has iterator as value.
		  alabnf_merge_with_head(alabnf,stack,node);
		  
		  // WELL... DONE not well done.
		  break;
		}
	      // else there is another iterator already resolved within this sequence
	      else
		{
		  collected_node = alabnf_collect_node_sequence(alabnf, collected_node, node);
		}
	    }
	  else
	    {	      
	      collected_node = alabnf_collect_node_sequence(alabnf, collected_node, node);
	    }

	  if ( collected_node == NULL )
	    {
	      aldebug_printf(NULL,"[FATAL] null collected_node\n");
	    }

	  if ( alstack_used(stack) > 1 )
	    {
	      element=alstack_pop(stack);
	      if ( element != NULL )
		{
		  node = (struct alabnf_node *) element->reference;
		}
	      else
		{
		  node = NULL;
		}
	    }
	  else
	    {
	      if ( alstack_used(stack) == 1 )
		{
		  // SHOULD WE push collected_node ???
		  alstack_push_ref(stack,collected_node);
		}
	      else
		{
		  aldebug_printf(NULL,"[ERROR] closing an iterator with a stack containing only rulename %s %s %i\n",__FILE__,__func__,__LINE__);
		}
	      node = NULL;
	    }
	      
	}
    }
}

// unstack until start sequence
// stack full iterator with content as sequence
void alabnf_close_sequence(struct alabnf_sm * state_machine, char c)
{
  struct alabnf * alabnf   = alabnf_state_machine_generated(state_machine);
  struct alstack * stack = state_machine->stack;
  
  struct alstackelement * element=NULL;
  struct alabnf_node * collected_node = NULL;

  if ( alstack_used(stack) < 2 )
    {
      aldebug_printf(NULL,"[ERROR] closing a sequence with a stack containing only rulename %s %s %i\n",__FILE__,__func__,__LINE__);
      return;
    }

  element=alstack_pop(stack);
  if (element != NULL )
    {
      struct alabnf_node * node = (struct alabnf_node *) element->reference;
      
      while ( node != NULL )
	{
	  // TOCHECK since rework
	  // FIXME uses state_machine->string_type which is not sync at this stage.
	  // struct alabnf_node * node = alabnf_build_abnf_node(state_machine,entry);
	  if ( (node != NULL ) && (node->type == ALABNF_NT_SEQUENCE ) )
	    {
	      struct alabnf_sequence * sequence = &node->content.sequence;
	      if ( sequence->node == NULL )
		{
		  // iterator start found, can create iterator object and return
		  sequence->node = collected_node;
		  // should push an entry : this one has sequence as value.
		  alabnf_merge_with_head(alabnf,stack,node);
		  // WELL... DONE not well done.
		  break;
		}
	      // else there is another sequence already resolved within this sequence
	      else
		{
		  collected_node = alabnf_collect_node_sequence(alabnf, collected_node, node);
		}
	    }
	  else
	    {	      
	      collected_node = alabnf_collect_node_sequence(alabnf, collected_node, node);
	    }

	  if ( collected_node == NULL )
	    {
	      aldebug_printf(NULL,"[FATAL] null collected_node\n");
	    }

	  if ( alstack_used(stack) > 1 )
	    {
	      element=alstack_pop(stack);
	      if ( element != NULL )
		{
		  node = (struct alabnf_node *) element->reference;
		}
	      else
		{
		  node = NULL;
		}
	    }
	  else
	    {
	      // considering that what remain on stack is rulename
	      if ( alstack_used(stack) == 1 )
		{
		  // SHOULD WE push collected_node ???
		  alstack_push_ref(stack,collected_node);
		}
	      else
		{
		  aldebug_printf(NULL,"[ERROR] closing a sequence with a stack of size %i containing no rulename %s %s %i\n", alstack_used(stack),__FILE__,__func__,__LINE__);
		  // HACK FIXME
		  alabnf_dump_node(collected_node);
		}
	      
	      node = NULL;
	    }
	}
    }
}


void alabnf_close_group(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE('.',"close_group",state_machine);
  alabnf_close_sequence(state_machine,c);
}

void alabnf_close_optional(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_iterator(state_machine,c);
}

void alabnf_close_any(struct alabnf_sm * state_machine, char c)
{
  switch(c)
    {
    case ']':
      alabnf_close_iterator(state_machine,c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case ')':
      alabnf_close_group(state_machine,c);
      state_machine->next_action = ALABNF_PA_CONTINUE;      
      break;
    }

  // TODO handle alternative and iterator
}

void alabnf_optional_start(struct alabnf_sm * state_machine, char c)
{
  // stack iterator 0,1 start
  alabnf_stack_iterator(state_machine,0,1);
}

// close rule consume full stack expecting last element to be rule name.
void alabnf_close_rule(struct alabnf_sm * state_machine)
{
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);
  
  struct alabnf_node * node =  alabnf_create_node(alabnf, ALABNF_NT_STRING);

  struct alstack * stack = state_machine->stack;
  int entries = alstack_used(stack);

  // should create rule = sequence of nodes
  struct alstackelement * element=NULL;
  struct alabnf_node * collector = NULL;
  
  aldebug_printf(NULL,"[INFO] parsing a rule in stack of %i tokens .\n",  entries );
  
  // we are unstacking so popping next before previous.
  for (int i=1; i<entries; i++)
    {
      element=alstack_pop(stack);
      if ( element != NULL )
	{
	  node = (struct alabnf_node *) element->reference;
	  if ( node == NULL )
	    {
	      aldebug_printf(NULL,"[WARNING] NULL node in %s:%s:%i\n",__FILE__,__func__,__LINE__);
	    }
	  else
	    {
	      aldebug_printf(NULL,"add node type %i in rule sequence\n", node->type);
	      collector = alabnf_collect_node_sequence(alabnf,collector,node);
	    }
	}
    }
  if ( alstack_used(stack) == 1 )
    {
      struct alstackelement * rule=alstack_pop(stack);
      if ( rule != NULL )
	{
	  aldebug_printf(NULL,"[INFO] in %s:%s:%i\n", __FILE__, __func__,__LINE__ );
	  struct alabnf_node * node = (struct alabnf_node *)  rule->reference;
	  if ( node != NULL )
	    {
	      if (node->type == ALABNF_NT_RULE_REF )
		{
		  struct alabnf_rule_ref * rule_ref = &node->content.rule_ref;
		  if ( rule_ref->resolved != NULL )
		    {
		      aldebug_printf(NULL,"[ERROR] redefinition of rule rule_ref in %s:%s:%i\n", __FILE__, __func__,__LINE__ );
		      // could create automagically an alternative... ALABNF_NT_ALT with rule_ref->resolved and collector
		    }
		  else
		    {
		      rule_ref->resolved=collector;
		    }
		}
	      else
		{
		  aldebug_printf(NULL,"[ERROR] rule definition should be a rule_ref in %s:%s:%i\n", __FILE__, __func__,__LINE__ );		  
		}

	      // FIXME TOY CODE
	      printf("\n");
	      // alabnf_print_token(mytoken);
	      alabnf_dump_node(node);
	      if ( collector != NULL )
		{
		  printf("= ");
		  alabnf_dump_node(collector);

		  // TODO collect other than initial rule ...
		  if ( alabnf->root_rule.value == NULL )
		    {
		      // FIXME dangerous no check of types
		      memcpy(&alabnf->root_rule.rule_name,&node->content.string,sizeof(node->content.string));
		      alabnf->root_rule.value = collector;
		    }

		}
	      printf("\n");
	    }


	}
      else
	{
	  aldebug_printf(NULL,"[FATAL] parsing a NULL rule token in stack of tokens %i . %s %s %i\n",  alstack_used(stack),__FILE__,__func__,__LINE__ );
	}
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] parsing a rule without a rule in stack of tokens %i . %s %s %i\n",  alstack_used(stack), __FILE__,__func__,__LINE__ );
    }

}

void alabnf_new_rule(struct alabnf_sm * state_machine)
{
  alabnf_close_rule(state_machine);
  state_machine->rule_number++;
  aldebug_printf(NULL,"[DEBUG] rule # %i, indent %i/%i/%i at %s/%s/%i\n", state_machine->rule_number,
	 state_machine->current_indent,
		 state_machine->initial_indent,
		 state_machine->linebreak,
		 __FILE__,__func__,__LINE__);

  state_machine->one_char_method=alabnf_name_string;
  state_machine->state = ALABNF_STATE_RULENAME;
  state_machine->string_type=ALABNF_ST_RULENAME;
}

// any new element on left part ( ie after = )
void alabnf_start_string_ruledef(struct alabnf_sm * state_machine, char c)
{
  ALABNF_DEBUG_TEXT_STATE(c,"string_ruledef",state_machine);
  switch(c)
    {
    case '%':
      state_machine->one_char_method=alabnf_start_typed_string;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case ' ':
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '=':
      aldebug_printf(NULL,"[ERROR] unexpected '=' in alabnf rule definition %s %s %i\n", __FILE__, __func__, __LINE__);
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      state_machine->one_char_method=alabnf_quoted_string;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      alabnf_alternative_start(state_machine,c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '[':
      alabnf_optional_start(state_machine,c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '(':
      alabnf_group_start(state_machine,c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 13:
      // cr, expect lf now
      aldebug_printf(NULL,"CR\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;      
      break;
    case 10:
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case ';':
      state_machine->next_action = ALABNF_PA_CONTINUE;
      state_machine->one_char_method = alabnf_comment;
      break;
    case '<':
      state_machine->next_action = ALABNF_PA_CONTINUE;
      state_machine->one_char_method = alabnf_chevron_start;
      break;      
    default:
      state_machine->next_action = ALABNF_PA_REMATCH;
      if ( state_machine->current_indent <= state_machine->initial_indent )
	{
	  alabnf_new_rule(state_machine);
	}
      else
	{
	  aldebug_printf(NULL,"[DEBUG] CONTINUE rule # %i, indent %i/%i\n",
			 state_machine->rule_number,
			 state_machine->current_indent,
			 state_machine->initial_indent);
	  state_machine->one_char_method=alabnf_iterator_string_start;
	}

    }

}

void alabnf_start_string(struct alabnf_sm * state_machine, char c)
{
  switch(c)
    {
    case ' ':
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '=':
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 13:
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 10:
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    default:
      state_machine->one_char_method=alabnf_name_string;
      state_machine->next_action = ALABNF_PA_REMATCH;
    }

}

void alabnf_expect_equal(struct alabnf_sm * state_machine, char c)
{

  switch(c)
    {
    case ' ':      
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '=':
      if ( state_machine->state == ALABNF_STATE_RULENAME )
	{
	  state_machine->one_char_method=alabnf_start_string_ruledef;
	  state_machine->initial_indent=state_machine->current_indent;
	  state_machine->state=ALABNF_STATE_RULEDEF;
	}
      else
	{
	  ALABNF_DEBUG_UNEXPECTED_CHAR(c)
	}
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 13:
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 10:
      ALABNF_DEBUG_UNEXPECTED_CHAR(c)
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    default:
      aldebug_printf(NULL,"[ERROR] unexpected '%i' in %s %s %i expect equals \n",c, __FILE__, __func__, __LINE__);
      state_machine->next_action = ALABNF_PA_REMATCH;
    }

}

void alabnf_stack_rule_ref(struct alabnf_sm * state_machine, struct alhash_entry * mytoken)
{
  struct alabnf_node * node = alabnf_get_abnf_token(mytoken);

  ALABNF_DEBUG_TEXT_STATE('.',"stack_rule_ref",state_machine);
  
  struct alabnf_rule_ref * rule_ref = NULL;
  if ( node == NULL )
    {
      struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);

      node = alabnf_create_node(alabnf,ALABNF_NT_RULE_REF);
      if ((node == NULL) || ( node->type != ALABNF_NT_RULE_REF ))
	{
	  // ooops
	  aldebug_printf(NULL,"[FATAL] invalid node %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
	}
      else	
	{
	  rule_ref = &node->content.rule_ref;
	  memcpy(&rule_ref->keyblock,&mytoken->key,sizeof(rule_ref->keyblock));
	  // IS it REALLY changing content of hash table ? YES
	  mytoken->value.data.ptr=node;
	  mytoken->value.length=sizeof(*node);
	  mytoken->value.type=ALTYPE_OPAQUE;
	}      
    }
  else
    {
      // poor lazzy protection
      if ( (unsigned long) node < 64L )
	{
	  aldebug_printf(NULL,"[FATAL] invalid pointer node %p\n",node);
	  // FIXME HARD EXIT
	  exit(1);
	}

      // should be a rule ref
      if ( node->type == ALABNF_NT_RULE_REF )
	{
	  rule_ref = &node->content.rule_ref;
	  if ( rule_ref->resolved == NULL )
	    {
	      // well.. unresolved...
	    }
	}
      else
	{
	  aldebug_printf(NULL,"[FATAL] expecting a rule ref got %i in %s:%s:%i\n",node->type,__FILE__,__func__,__LINE__);
	}
    }
  
  alstack_push_ref(state_machine->stack,node);
}

void alabnf_stack_range(struct alabnf_sm * state_machine)
{
  struct alabnf_node * node = NULL;
  struct alabnf_range * range = NULL;
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);

  node = alabnf_create_node(alabnf,ALABNF_NT_RANGE);
  if ((node == NULL) || ( node->type != ALABNF_NT_RANGE ))
    {
      // ooops
      aldebug_printf(NULL,"[FATAL] invalid node %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
    }
  else	
    {
      range = &node->content.range;
      range->start = state_machine->number_sm.min;
      range->end = state_machine->number_sm.max;
    }  
  alstack_push_ref(state_machine->stack,node);
}

void alabnf_close_string(struct alabnf_sm * state_machine, char c)
{
  int pending_chars = altokenizer_get_pending_chars(&state_machine->tokenizer);
  if ( pending_chars > 0 )
    {
      if ( pending_chars > ALABNF_MAX_CHARS )
	{
	  aldebug_printf(NULL,"[FATAL] too big token pending_chars %i > %i in %s %s %i\n",pending_chars,ALABNF_MAX_CHARS,__FILE__,__func__,__LINE__);
	  exit(1);
	}

      if ( state_machine->string_type == ALABNF_ST_RANGE )
	{
	  aldebug_printf(NULL,"[DEBUG] close string for range in %s %s %i\n",__FILE__,__func__,__LINE__);

	  // flush pending chars they have been collected in number_sm min,max by other means.
	  altokenizer_reset_buffer_pos(&state_machine->tokenizer);

	  // Exception rulename will create a range entry.
	  alabnf_stack_range(state_machine);
	}
      else
	{
	  struct al_token token;
	  token.token=ALABNF_NT_STRING;
      
	  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,c);

	  if ( state_machine->string_type == ALABNF_ST_RULENAME )
	    {
	      aldebug_printf(NULL,"[DEBUG] close string token %p for rule_ref in %s %s %i\n",mytoken,__FILE__,__func__,__LINE__);
	  
	      // Exception rulename will create a rule_ref entry.
	      alabnf_stack_rule_ref(state_machine, mytoken);
	    }
	  else 
	    {
	      aldebug_printf(NULL,"[DEBUG] close string token %p in %s %s %i\n",mytoken,__FILE__,__func__,__LINE__);
	  
	      // WARNING allocation done on generated part...
	      struct alabnf_node * node = alabnf_build_abnf_node(state_machine, mytoken);

	      if ( (unsigned long) node < 64L )
		{
		  aldebug_printf(NULL,"[FATAL] invalid pointer node %p\n",node);
		  exit(1);
		}
	      alstack_push_ref(state_machine->stack,node);
	    }
	}
    }     

  // string was closed, need to start a new one to know its type.
  state_machine->string_type=ALABNF_ST_UNDEFINED;

  // next state
  if ( state_machine->state == ALABNF_STATE_RULENAME )
    {
      // HOW can it be ???
      state_machine->one_char_method=alabnf_start_string;
    }
  else
    {
      state_machine->one_char_method=alabnf_start_string_ruledef;
    }

}


void alabnf_close_name_string(struct alabnf_sm * state_machine, char c)
{
  if ( state_machine->state == ALABNF_STATE_RULENAME )
    {
      state_machine->one_char_method=alabnf_expect_equal;
    }
  else    
    {
      // UHU error
      aldebug_printf(NULL,"[ERROR] unexpected state %i for alabnf_close_name_string\n", state_machine->state );
      state_machine->one_char_method=alabnf_start_string_ruledef;
    }
  struct al_token token;
  token.token=ALABNF_NT_STRING;

  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,c);
  aldebug_printf(NULL,"[DEBUG] close name string %p\n",mytoken);

  alabnf_stack_rule_ref(state_machine, mytoken);
  
  // string was closed, need to start a new one to know its type.
  state_machine->string_type=ALABNF_ST_UNDEFINED;

}

void alabnf_close_string_rematch(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_string(state_machine, c);
  state_machine->next_action = ALABNF_PA_REMATCH;

  // do closing of any '])'	      
  alabnf_close_any(state_machine,c);
}

void alabnf_close_name_string_rematch(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_name_string(state_machine, c);
  state_machine->next_action = ALABNF_PA_REMATCH;
}

void alabnf_close_string_continue(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_string(state_machine, c);
  state_machine->next_action = ALABNF_PA_CONTINUE;
}

// closes a rulename
void alabnf_close_name_string_continue(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_name_string(state_machine, c);
  state_machine->next_action = ALABNF_PA_CONTINUE;
}

char alabnf_eat_char(struct alabnf_sm * state_machine)
{
  char c = (char) alinputstream_readuchar(state_machine->inputstream);
  if ( state_machine->inputstream->eof == 0 )
    {
      state_machine->characters ++;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      state_machine->next_action = ALABNF_PA_FAIL;
    }
  return c;
}

void alabnf_state_machine_run(struct alabnf_sm * state_machine)
{
  enum alabnf_parser_action action = state_machine->next_action;
  char c = 0;
  int seen_eof = 0;
  
  do 
    {
      state_machine->state_loop ++;      
      // capture the character to analyze
      if ( state_machine->next_action == ALABNF_PA_REMATCH )
	{
	  if ( action == ALABNF_PA_REMATCH )
	    {
	      aldebug_printf(NULL,"[WARNING] two successive rematch %p at (line,column) (%i,%i) \n",
			     state_machine->one_char_method,
			     state_machine->lf_line,
			     state_machine->current_indent);

	      if ( state_machine->characters * ALABNF_MAX_INTERNAL_REMATCH_TIMES < state_machine->state_loop )
		{
		  aldebug_printf(NULL,"[FATAL] states loop %i too big compared to stream parsed char %i at  (line,column) (%i,%i)\n",
				 state_machine->state_loop,
				 state_machine->characters,
				 state_machine->lf_line,
				 state_machine->current_indent);

		  exit(1);
		}
	      
	    }
	  char r = state_machine->rematch;
	  if ( c != r )
	    {
	      aldebug_printf(NULL,"[FATAL] rematch diff %i != %i at (line,column) (%i,%i)\n",
			     c,r,
			     state_machine->lf_line,
			     state_machine->current_indent);
	      exit(1);
	    }	    
	  aldebug_printf(NULL,"[DEBUG] rematch %p %x '%c' at (line,column) (%i,%i)\n",
			 state_machine->one_char_method,
			 c,c>32 ? c:'?',
			 state_machine->lf_line,
			 state_machine->current_indent
			 );
	}
      else
	{
	  c = alabnf_eat_char(state_machine);
	  // quick fix for infinite loop in last comment
	  if ( c == 0 )
	    {
	      seen_eof ++;
	      if ( seen_eof > 100 )
		{
		  aldebug_printf(NULL,"[FATAL] eof char 0 seen %i times. would need a proper code fix. break.\n",seen_eof);
		  break;		  
		}
		 
	    }
	  else
	    {
	      seen_eof = 0;
	    }
	  state_machine->rematch=c;
	  switch(c)
	    {
	    case 13:
	      break;
	    case 10:
	      alabnf_handle_lf(state_machine);
	      break;
	    default:
	      state_machine->current_indent++;
	    }
	  
	}
      action = state_machine->next_action;

      // analyze character
      if ( state_machine->next_action != ALABNF_PA_FAIL )
	{
	  alabnf_one_char_method one_char_method = state_machine->one_char_method;
	  if ( one_char_method != NULL )
	    {
	      one_char_method(state_machine, c);
	      if ( state_machine->next_action == ALABNF_PA_CLOSE )
		{
		  if ( c != 0 )
		    {
		      alabnf_close_method close_method = state_machine->close_method;
		      aldebug_printf(NULL,"[DEBUG] CLOSE %p %x '%c' at (line,column) (%i,%i)\n",
				     close_method,
				     c,c>32 ? c:'?',
				     state_machine->lf_line,
				     state_machine->current_indent
			 );

		      close_method(state_machine,c);
		      if ( state_machine->next_action == ALABNF_PA_CLOSE )
			{
			  aldebug_printf(NULL,"[WARNING] cascading CLOSE\n");
			}
		    }
		  else
		    {
		      // should be EOF ?
		      state_machine->next_action = ALABNF_PA_FAIL;
		    }
		}
	    }
	  else
	    {
	      state_machine->next_action = ALABNF_PA_FAIL;
	    }
	}
    }
  while (  state_machine->next_action != ALABNF_PA_FAIL );

  // last rule ...
  alabnf_close_rule(state_machine);

}

struct alabnf * alabnf_state_machine_generated(struct alabnf_sm *state_machine)
{
  struct alabnf * alabnf = state_machine->generated;
  if ( alabnf == NULL )
    {
      // FIXME malloc at least should be freed
      alabnf=calloc(1,sizeof(*alabnf));
      alhash_context_init(&alabnf->context,64,1024,200);
      state_machine->generated=alabnf;
      alabnf->root_rule.value=NULL;
    }

  return alabnf;
}
