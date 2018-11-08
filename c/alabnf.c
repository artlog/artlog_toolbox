#include "alabnf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* code created by a human brain */

/*
      NOTE:     ABNF strings are case-insensitive and
                  the character set for these strings is us-ascii.
*/

const int ALBNF_MAX_CHARS=100;

void alabnf_start_string(struct alabnf_sm * state_machine, char c);
void alabnf_dump_node(struct alabnf_node * node );
void alabnf_dump_sequence(struct alabnf_sequence * start_sequence);
void alabnf_dump_alternative(struct alabnf_alternative * alternative);
void alabnf_dump_range(struct alabnf_range * range);
void alabnf_start_string_ruledef(struct alabnf_sm * state_machine, char c);
void alabnf_close_group(struct alabnf_sm * state_machine, char c);
void alabnf_close_name_string_continue(struct alabnf_sm * state_machine, char c);
void alabnf_close_name_string_rematch(struct alabnf_sm * state_machine, char c);
void alabnf_close_string_rematch(struct alabnf_sm * state_machine, char c);
void alabnf_close_string_continue(struct alabnf_sm * state_machine, char c);

struct alabnf_node * alabnf_create_node(struct alabnf * alabnf, enum alabnf_node_type type)
{
  // use alabnf->context allocator.
  struct alabnf_node * node = (struct alabnf_node *) ALALLOC(alabnf->context.allocator, sizeof(struct alabnf_node));
  bzero(node,sizeof(*node));
  node->type = type;
  /*
  switch(type)
    {
    case ALABNF_NT_ITERATOR:
      break;
    case ALABNF_NT_SEQUENCE:
      break;
    case ALABNF_NT_STRING:
      break;
    case ALABNF_NT_ALT:
      break;
    case ALABNF_NT_RANGE:
      break;
    }
  */
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
  struct alabnf_node * sequence_node=alabnf_create_node(alabnf,ALABNF_NT_ITERATOR);
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
      if (( iterator->min == 0 ) && ( iterator->max == 1 ))
	{
	  // optional
	  printf("[");
	  alabnf_dump_node(iterator->node);
	  printf("]");
	}
      else
	{
	  printf("%i %i",iterator->min,iterator->max);
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
		printf("ERROR_UNDEF_");
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
	default:
	  printf("unrecognized abnf type %i\n",type);
	}
    }
}

void alabnf_dump_alternative(struct alabnf_alternative * start_alternative)
{
  struct alabnf_alternative * alternative=start_alternative;
  struct alabnf_alternative * next_alternative=NULL;
  while (alternative != NULL)
    {
      next_alternative = alternative->alt;
      aldebug_printf(NULL,"\nnext %p node %p\n", next_alternative, alternative->node);
      alabnf_dump_node(alternative->node);
      if (next_alternative != NULL)
	{
	  printf("/");
	}
      else
	{
	  // DEBUG only to remove
	  printf("|");
	}
      alternative=next_alternative;
    }

}

void alabnf_dump_range(struct alabnf_range * range)
{
  // TODO
  printf("TODO alabnf_dump_range");
}

void alabnf_dump_sequence(struct alabnf_sequence * start_sequence)
{
  struct alabnf_sequence * sequence=start_sequence;
  struct alabnf_sequence * next_sequence=NULL;
  while (sequence != NULL)
    {
      next_sequence = sequence->next;
      aldebug_printf(NULL,"\nnext %p node %p\n", next_sequence, sequence->node);
      printf("(");
      alabnf_dump_node(sequence->node);
      if (next_sequence != NULL)
	{
	  printf(" ");
	}
      printf(")");
      sequence=next_sequence;
    }
}

void alabnf_dump_rule(struct alabnf_rule * rule)
{
  // TODO
}

struct alabnf_node * alabnf_create_node_string(struct alabnf * alabnf, aldatablock * block, enum alabnf_string_type type)
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
      // Not yet abnfized
      return NULL;
    }
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

// can alter both state_machine and mytoken value
struct alabnf_node * alabnf_build_abnf_node(struct alabnf_sm * state_machine, struct alhash_entry * mytoken)
{
  struct alabnf_node * node = alabnf_get_abnf_token(mytoken);
  if ( node == NULL )
    {
    struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);
    // should alter value to point on alabnf_string
    // we copy value to node->string by allocating/copying it on new context.
    node = alabnf_create_node_string(alabnf,&mytoken->value, state_machine->string_type);    
    if ((node == NULL) || ( node->type == ALABNF_NT_INVALID ))
    {
      // ooops
      aldebug_printf(NULL,"[FATAL] invalid node %p %s:%s:%i\n",node, __FILE__,__func__,__LINE__);
    }

    // IS it REALLY changing content of hash table ? YES
    mytoken->value.data.ptr=node;
    mytoken->value.length=sizeof(*node);
    mytoken->value.type=ALTYPE_OPAQUE;
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
  if ( pending_chars > ALBNF_MAX_CHARS )
    {
      aldebug_printf(NULL,"[FATAL] too big token pending_chars %i > %i in %s %s %i\n",pending_chars,ALBNF_MAX_CHARS,__FILE__,__func__,__LINE__);
      exit(1);
    }
}

// cumulated number is converted to char and reset.
void alabnf_flush_number_to_char(struct alabnf_sm * state_machine)
{
  int value = alabnf_flush_number(&state_machine->number_sm);  

  alabnf_add_char(state_machine,'?',(char) value);
}

void alabnf_comment(struct alabnf_sm * state_machine, char c)
{
  if (( c != 10 ) && ( c != 13 ))
    {
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      state_machine->next_action = ALABNF_PA_REMATCH;
      state_machine->one_char_method = alabnf_start_string_ruledef;
    }
}


void alabnf_chevron(struct alabnf_sm * state_machine, char c)
{
  // FIXME
  if ( c != '>' )
    {
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      state_machine->next_action = ALABNF_PA_CONTINUE;
      state_machine->one_char_method = alabnf_start_string_ruledef;
    }
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
      number_sm->cumulated = number_sm->cumulated << 4 | (0xA+c-'A');
      number_sm->seen++;
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
      else	
	{
	if ( c == '.' )
	  {
	    // concatenation
	    state_machine->next_action = ALABNF_PA_CONTINUE;
	    return;
	  }
	else
	if ( c == '-' )
	  {
	    // TODO FIXME range
	    state_machine->next_action = ALABNF_PA_CONTINUE;
	    return;
	  }
	else
	  {
	    state_machine->close_method = alabnf_close_string_rematch;
	    state_machine->next_action = ALABNF_PA_CLOSE;
	    return;
	  }
      }

  if (number_sm->seen > 1)
    {
      alabnf_flush_number_to_char(state_machine);
    }
}

// first char of string was recognized
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
      if ( c == ' ' )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] '%i' '%c' in albnf rule definition\n",c,c >=32 ? c : '.');	  
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
      if ( c == ' ' )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[WARNING] '%i' '%c' in albnf rule definition\n",c,c >=32 ? c : '.');	  
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
        if ( c == ' ' )
	{
	  state_machine->close_method = alabnf_close_name_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in %s %s %i\n",c,c,__FILE__,__func__,__LINE__);
	  state_machine->close_method = alabnf_close_name_string_rematch;
	}
    }

}

void alabnf_name_string(struct alabnf_sm * state_machine, char c)
{
  alabnf_name_string_intern(state_machine, c);
  if ( state_machine->next_action == ALABNF_PA_CLOSE )
    {
        if ( c == ' ' )
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
        if ( c == ' ' )
	{
	  state_machine->close_method = alabnf_close_string_continue;
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in albnf rule definition\n",c,c);	  
	  state_machine->close_method = alabnf_close_string_rematch;
	}
    }

}

void alabnf_string(struct alabnf_sm * state_machine, char c)
{
  alabnf_name_string_intern(state_machine, c);
  if ( state_machine->next_action == ALABNF_PA_CLOSE )
    {
        if ( c == ' ' )
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

  struct al_token token;
  token.token=ALABNF_NT_SEQUENCE;
  // FIXME non-terminals are colliding with terminals
  // BROKEN need a way to create a new token
  // HACK to overcome  altokenizer_dict_add_string: Assertion `length!=0' failed
  // HACK REUSE iterator value
  alabnf_add_char(state_machine,'^',(char) state_machine->iterator_index);  
  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,'(');
  alstack_push_ref(stack,mytoken);

  struct alabnf_node * sequence_node = alabnf_create_sequence_node(alabnf, NULL,NULL);
  
  // IS it REALLY changing content of hash table ? YES
  mytoken->value.data.ptr=sequence_node;
  mytoken->value.length=sizeof(*sequence_node);
  mytoken->value.type=ALTYPE_OPAQUE;  
  
  state_machine->iterator_index ++;

}
  
void alabnf_group_start(struct alabnf_sm * state_machine, char c)
{
  albnf_stack_sequence(state_machine);
}

void alabnf_stack_iterator(struct alabnf_sm * state_machine,int min, int max)
{
  struct alstack * stack = state_machine->stack;
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);
  // NONONO should push an entry !
  // alstack_push_ref(stack,iterator);
  struct al_token token;
  token.token=ALABNF_NT_ITERATOR;
  // FIXME non-terminals are colliding with terminals
  // BROKEN need a way to create a new token
  //HACK to overcome  altokenizer_dict_add_string: Assertion `length!=0' failed
  alabnf_add_char(state_machine,'^',(char) state_machine->iterator_index);  
  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,'^');
  alstack_push_ref(stack,mytoken);

  struct alabnf_node * iterator_node = alabnf_create_iterator_node(alabnf,min,max,NULL);
  // IS it REALLY changing content of hash table ? YES
  mytoken->value.data.ptr=iterator_node;
  mytoken->value.length=sizeof(*iterator_node);
  mytoken->value.type=ALTYPE_OPAQUE;  
  
  state_machine->iterator_index ++;
}

void alabnf_alternative_start(struct alabnf_sm * state_machine, char c)
{
 
  struct alstack * stack = state_machine->stack;
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);

  struct alstackelement * element=NULL;
  struct alabnf_node * collected_node = NULL;

  element=alstack_pop(stack);
  if ( element != NULL )
    {
      struct alhash_entry * entry = (struct alhash_entry *) element->reference;
  
      if ( entry != NULL )
	{
	  // FIXME uses state_machine->string_type which is not sync at this stage.
	  struct alabnf_node * node = alabnf_build_abnf_node(state_machine,entry);

	  struct al_token token;
	  token.token=ALABNF_NT_ALT;
	  // FIXME non-terminals are colliding with terminals
	  // BROKEN need a way to create a new token
	  //HACK to overcome  altokenizer_dict_add_string: Assertion `length!=0' failed
	  alabnf_add_char(state_machine,'/',(char) state_machine->iterator_index);  
	  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,'/');
	  alstack_push_ref(stack,mytoken);

	  struct alabnf_node * iterator_node = alabnf_create_alternative_node(alabnf,node,NULL);
	  // IS it REALLY changing content of hash table ? YES
	  mytoken->value.data.ptr=iterator_node;
	  mytoken->value.length=sizeof(*iterator_node);
	  mytoken->value.type=ALTYPE_OPAQUE;  
  
	  state_machine->iterator_index ++;
	}
    }
}

void alabnf_iterator_string(struct alabnf_sm * state_machine, char c)
{
  if (
      (( c >= '0' ) && ( c <= '9' ))
      ||
      ( c == '*')
      )
    {
      // TODO iterator
      aldebug_printf(NULL,"ITERATOR %i\n",c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      // TODO should push iterator token on stack
      if ( c == '(' )
	{
	  alabnf_group_start(state_machine,c);
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      else if ( c == '<' )
	{
	  state_machine->one_char_method=alabnf_chevron;
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      // TODO FIXME can be typed string
      else
	{
	  state_machine->one_char_method = alabnf_string;
	  state_machine->string_type=ALABNF_ST_RULENAME;
	  // next_action not set by xxx_intern
	  alabnf_name_string_intern(state_machine, c);
	  if ( state_machine->next_action == ALABNF_PA_CLOSE )
	    {
	      if ( c == ' ' )
		{
		  state_machine->close_method = alabnf_close_string_continue;
		}
	      else
		{
		  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in %s %s %i\n",c,c,__FILE__,__func__,__LINE__);
		  state_machine->close_method = alabnf_close_string_rematch;
		}
	    }
	}
    }

}


void alabnf_start_typed_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;

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

// return a node that is a sequence if collector is non null
struct alabnf_node * alabnf_collect_node_sequence(struct alabnf * alabnf, struct alabnf_node * collector, struct alabnf_node * new_node)
{  
  struct alabnf_sequence * sequence=NULL;
  struct alabnf_sequence * next_sequence=NULL;
  struct alabnf_node * node=NULL;

  if ( collector != NULL )
    {
      if ( collector->type == ALABNF_NT_SEQUENCE )
	{
	  next_sequence = &collector->content.sequence;
	}
      else
	{
	  next_sequence = NULL;
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
	      if ( new_node->type == ALABNF_NT_INVALID )
		{
		  aldebug_printf(NULL,"[FATAL] invalid new node %p in %s %s %i\n",new_node,__FILE__,__func__,__LINE__);
		}
	      node=new_node;
	    }
	  else
	    {
	      // don't append to child sequence
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
      struct alhash_entry * entry = (struct alhash_entry *) element->reference;
      if ( entry != NULL )
	{
	  node = alabnf_get_abnf_token(entry);
	}
    }
  return node;
}

// assuming node is entry value.
void alabnf_merge_with_head(  struct alabnf * alabnf, struct alstack * stack, struct alhash_entry * entry, struct alabnf_node * node)
{
  // TODO check iteratorS and alternativeS
  struct alabnf_node * head_node = alabnf_fetch_head_node(stack);
  if ( head_node != NULL )
    {
      aldebug_printf(NULL,"[INFO] head node found %p type %i\n", head_node, head_node->type);
      if ( head_node->type == ALABNF_NT_ALT )
	{

	  struct alabnf_alternative * alternative = &head_node->content.alt;
	  if ( alternative->alt == NULL )
	    {
	      // an empty slot : merge entry here
	      struct alabnf_node * merge_entry = alabnf_create_alternative_node(alabnf,node,NULL);
	      alternative->alt=&merge_entry->content.alt;
	      aldebug_printf(NULL,"[INFO] MERGE ALTERNATIVES %p %p\n", alternative, alternative->alt);
	      exit(1);
	    }
	  else
	    {
	      aldebug_printf(NULL,"[INFO] SKIP ALTERNATIVES %p %p\n", alternative, alternative->alt);
	      head_node = NULL;
	    }
	}
    }
  // if entry was not merged with head node
  if ( head_node == NULL )
    {
      alstack_push_ref(stack,entry);
    }
}

// unstack until incomplete iterator found.
// stack full iterator with content as sequence
void alabnf_close_iterator(struct alabnf_sm * state_machine, char c)
{
  struct alabnf * alabnf   = alabnf_state_machine_generated(state_machine);
  struct alstack * stack = state_machine->stack;
  
  struct alstackelement * element=NULL;
  struct alabnf_node * collected_node = NULL;

  element=alstack_pop(stack);
  if (element != NULL )
    {
      struct alhash_entry * entry = (struct alhash_entry *) element->reference;
  
      while ( entry != NULL )
	{
	  // FIXME uses state_machine->string_type which is not sync at this stage.
	  struct alabnf_node * node = alabnf_build_abnf_node(state_machine,entry);
	  if ( (node != NULL ) && (node->type == ALABNF_NT_ITERATOR ) )
	    {
	      struct alabnf_iterator * iterator = &node->content.iterator;
	      if ( iterator->node == NULL )
		{
		  // iterator start found, can create iterator object and return
		  iterator->node = collected_node;
		  // should push an entry : this one has iterator as value.
		  alabnf_merge_with_head(alabnf,stack,entry,node);
		  
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

	  element=alstack_pop(stack);
	  if ( element != NULL )
	    {
	      entry = (struct alhash_entry *) element->reference;
	    }
	  else
	    {
	      entry = NULL;
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

  element=alstack_pop(stack);
  if (element != NULL )
    {
      struct alhash_entry * entry = (struct alhash_entry *) element->reference;
  
      while ( entry != NULL )
	{
	  // FIXME uses state_machine->string_type which is not sync at this stage.
	  struct alabnf_node * node = alabnf_build_abnf_node(state_machine,entry);
	  if ( (node != NULL ) && (node->type == ALABNF_NT_SEQUENCE ) )
	    {
	      struct alabnf_sequence * sequence = &node->content.sequence;
	      if ( sequence->node == NULL )
		{
		  // iterator start found, can create iterator object and return
		  sequence->node = collected_node;
		  // should push an entry : this one has sequence as value.
		  alabnf_merge_with_head(alabnf,stack,entry,node);
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

	  element=alstack_pop(stack);
	  if ( element != NULL )
	    {
	      entry = (struct alhash_entry *) element->reference;
	    }
	  else
	    {
	      entry = NULL;
	    }
	}
    }
}


void alabnf_close_group(struct alabnf_sm * state_machine, char c)
{
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
  // TODO collect rule...
  struct alabnf * alabnf = alabnf_state_machine_generated(state_machine);
  
  struct alabnf_node * node =  alabnf_create_node(alabnf, ALABNF_NT_STRING);

  struct alstack * stack = state_machine->stack;
  int entries = alstack_used(stack);

  // should create rule = sequence of nodes
  struct alstackelement * element=NULL;
  struct alabnf_sequence * sequence=NULL;
  struct alabnf_sequence * next_sequence=NULL;
  // we are unstacking so popping next before previous.
  for (int i=1; i<entries; i++)
    {
      struct alabnf_node * node_sequence;
      element=alstack_pop(stack);
      if ( element != NULL )
	{
	  struct alhash_entry * entry = (struct alhash_entry *) element->reference;

	  // FIXME uses state_machine->string_type which is not sync at this stage.
	  node = alabnf_get_abnf_token(entry);
	  if ( node == NULL )
	    {
	      aldebug_printf(NULL,"[WARNING] all nodes were not resolved for token %p",entry);
	    }
	  {
	      node =  alabnf_build_abnf_node(state_machine,entry);

	      // TODO use 
	      // node_sequence = alabnf_collect_node_sequence(struct alabnf_node * collector, struct alabnf_node * new_node)
	      {
		node_sequence =  alabnf_create_node(alabnf, ALABNF_NT_SEQUENCE);
		if (node_sequence != NULL)
		  {
		    sequence = &node_sequence->content.sequence;
		    sequence->node = node;
		    sequence->next = next_sequence;
		    next_sequence=sequence;
		  }
		else
		  {
		    aldebug_printf(NULL,"[FATAL] null node sequence in %s",__FILE__);
		  }
	      }
	  }
	}
    }
  if ( alstack_used(stack) == 1 )
    {
      struct alstackelement * rule=alstack_pop(stack);
      if ( rule != NULL )
	{
	  struct alhash_entry * mytoken = (struct alhash_entry *)  rule->reference;
	  if ( mytoken != NULL )
	    {
	      // FIXME TOY CODE
	      printf("\n");
	      alabnf_print_token(mytoken);
	      if ( sequence != NULL )
		{
		  printf("= ");
		  alabnf_dump_sequence(sequence);
		}
	      printf("\n");
	    }
	}
    }
  else
    {
      aldebug_printf(NULL,"[FATAL] parsing a rule without a rule in stack of tokens %i .\n",  alstack_used(stack) );
    }
}

void alabnf_new_rule(struct alabnf_sm * state_machine)
{
  alabnf_close_rule(state_machine);
  state_machine->rule_number++;
  aldebug_printf(NULL,"rule # %i, indent %i/%i", state_machine->rule_number,
	 state_machine->current_indent,
	 state_machine->initial_indent);
  state_machine->one_char_method=alabnf_name_string;
  state_machine->state = ALABNF_STATE_RULENAME;
}

void alabnf_start_string_ruledef(struct alabnf_sm * state_machine, char c)
{

  switch(c)
    {
    case '%':
      state_machine->one_char_method=alabnf_start_typed_string;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case ' ':
      //if ( state_machine->linebreak != 0 )
	{
	  aldebug_printf(NULL,"*");
	  state_machine->current_indent++;
	}
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
      // lf , expect a new line
      aldebug_printf(NULL,"LF\n");
      state_machine->lf_line ++;
      state_machine->current_indent = 0;
      state_machine->linebreak = 1;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case ';':
      state_machine->next_action = ALABNF_PA_CONTINUE;
      state_machine->one_char_method = alabnf_comment;
      break;
    case '<':
      state_machine->next_action = ALABNF_PA_CONTINUE;
      state_machine->one_char_method = alabnf_chevron;
      break;      
    default:
      state_machine->next_action = ALABNF_PA_REMATCH;
      if ( state_machine->current_indent <= state_machine->initial_indent )
	{
	  alabnf_new_rule(state_machine);	  
	}
      else
	{
	  aldebug_printf(NULL,"CONTINUE rule # %i, indent %i/%i\n", state_machine->rule_number,
			 state_machine->current_indent,
			 state_machine->initial_indent);
	  state_machine->one_char_method=alabnf_iterator_string;
	}

    }

}

void alabnf_start_string(struct alabnf_sm * state_machine, char c)
{
  switch(c)
    {
    case ' ':      
      state_machine->current_indent ++;
      if ( state_machine->rule_number == 0 )
	{
	  state_machine->initial_indent = state_machine->current_indent;
	}
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '=':
      aldebug_printf(NULL,"[ERROR] unexpected '=' in albnf start string\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      aldebug_printf(NULL,"[ERROR] unexpected '\"' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      aldebug_printf(NULL,"[ERROR] unexpected '/' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      aldebug_printf(NULL,"[ERROR] unexpected '.' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 13:
      aldebug_printf(NULL,"[ERROR] unexpected cr in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 10:
      aldebug_printf(NULL,"[ERROR] unexpected lf in albnf rule definition\n");
      state_machine->lf_line ++;
      state_machine->current_indent = 0;
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
	  state_machine->state=ALABNF_STATE_RULEDEF;
	}
      else
	{
	  aldebug_printf(NULL,"[ERROR] unexpected '=' in albnf rule definition\n");
	}
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      aldebug_printf(NULL,"[ERROR] unexpected '\"' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      aldebug_printf(NULL,"[ERROR] unexpected '/' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      aldebug_printf(NULL,"[ERROR] unexpected '.' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 13:
      aldebug_printf(NULL,"[ERROR] unexpected cr in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case 10:
      aldebug_printf(NULL,"[ERROR] unexpected lf in albnf rule definition\n");
      state_machine->lf_line ++;
      state_machine->current_indent = 0;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    default:
      aldebug_printf(NULL,"[ERROR] unexpected '%i' in %s %s %i expect equals \n",c, __FILE__, __func__, __LINE__);
      state_machine->next_action = ALABNF_PA_REMATCH;
    }

}


void alabnf_close_string(struct alabnf_sm * state_machine, char c)
{
  int pending_chars = altokenizer_get_pending_chars(&state_machine->tokenizer);
  if ( pending_chars > 0 )
    {
      if ( pending_chars > ALBNF_MAX_CHARS )
	{
	  aldebug_printf(NULL,"[FATAL] too big token pending_chars %i > %i in %s %s %i\n",pending_chars,ALBNF_MAX_CHARS,__FILE__,__func__,__LINE__);
	  exit(1);
	}
      struct al_token token;
      token.token=ALABNF_NT_STRING;
      // FIXME non-terminals are colliding with terminals
      struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,c);
      // alabnf_print_token(mytoken);
      alstack_push_ref(state_machine->stack,mytoken);
      aldebug_printf(NULL,"close string token %p in %s %s %i\n",mytoken,__FILE__,__func__,__LINE__);

      // this is here that mytoken->value can be set to something
      // struct alabnf_string would be a good idea
      // WARNING allocation should be on generated part...
      struct alabnf_node * node = alabnf_build_abnf_node(state_machine, mytoken);

      if ( (unsigned long) node < 64L )
	{
	  aldebug_printf(NULL,"[FATAL] invalid pointer node %p\n",node);
	  exit(1);
	}
    }     

  // string was closed, need to start a new one to know its type.
  state_machine->string_type=ALABNF_ST_UNDEFINED;

  // next state
  if ( state_machine->state == ALABNF_STATE_RULENAME )
    {
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
  // FIXME non-terminals are colliding with terminals
  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,c);
  // alabnf_print_token(mytoken);
  alstack_push_ref(state_machine->stack,mytoken);
  aldebug_printf(NULL,"close name string %p\n",mytoken);

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

  do 
    {
      // capture the character to analyze
      if ( state_machine->next_action == ALABNF_PA_REMATCH )
	{
	  if ( action == ALABNF_PA_REMATCH )
	    {
	      aldebug_printf(NULL,"[FATAL] two successive rematch\n");
	      exit(1);
	    }
	  char r = state_machine->rematch;
	  if ( c != r )
	    {
	      aldebug_printf(NULL,"[FATAL] rematch diff %i != %i\n",c,r);
	      exit(1);
	    }	    
	  aldebug_printf(NULL,"rematch %x '%c'\n",c,c>32 ? c:'?');
	}
      else
	{
	  c = alabnf_eat_char(state_machine);
	  state_machine->rematch=c;
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
		      // aldebug_printf(NULL, "\nCLOSE '%i'\n", c);
		      alabnf_close_method close_method = state_machine->close_method;
		      close_method(state_machine,c);
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
    }

  return alabnf;
}
