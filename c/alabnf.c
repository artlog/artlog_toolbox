#include "alabnf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* code created by a human brain */

/*
      NOTE:     ABNF strings are case-insensitive and
                  the character set for these strings is us-ascii.
*/


struct alabnf_node * alabnf_create_node(struct alabnf * alabnf, enum alabnf_node_type type)
{
  // use alabnf->context allocator.
  struct alabnf_node * node = (struct alabnf_node *) ALALLOC(alabnf->context.allocator, sizeof(struct alabnf_node));
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

void alabnf_dump_sequence(struct alabnf_sequence * start_sequence);

void alabnf_dump_iterator(struct alabnf_iterator * iterator);

void alabnf_dump_alternative(struct alabnf_alternative * alternative);

void alabnf_dump_range(struct alabnf_range * range);

void alabnf_dump_node(struct alabnf_node * node )
{
  if ( node != NULL )
    {
      enum alabnf_node_type type = node->type;
      switch(type)
	{
	case ALABNF_NT_ITERATOR:
	  break;
	case ALABNF_NT_SEQUENCE:
	  alabnf_dump_sequence(&node->content.sequence);
	  break;
	case ALABNF_NT_STRING:
	  {
	    struct alhash_datablock * datablock = &node->content.string.strbloc;
	    if ( node->content.string.type == ALABNF_ST_RULENAME )
	      {		
		printf(ALPASCALSTRFMT,
		       ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
	      }
	    else
	      {
		// need a better printf to support non printable strings as hex,int, quoted ...
		printf("\""ALPASCALSTRFMT"\"",
		       ALPASCALSTRARGS(datablock->length,datablock->data.charptr));

	      }

	  }
	  break;
	case ALABNF_NT_ALT:
	  break;
	case ALABNF_NT_RANGE:
	  break;
	}
    }
}

void alabnf_dump_iterator(struct alabnf_iterator * iterator)
{
  // TODO
  printf("TODO alabnf_dump_iterator");
}

void alabnf_dump_alternative(struct alabnf_alternative * alternative)
{
  // TODO
  printf("TODO alabnf_dump_alternative");
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
      alabnf_dump_node(sequence->node);
      if (next_sequence != NULL)
	{
	  printf(" ");
	}
      sequence=next_sequence;
    }
}

void alabnf_dump_rule(struct alabnf_rule * rule)
{
  // TODO
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
  // we are unstaking so popping next before previous.
  for (int i=1; i<entries; i++)
    {
      struct alabnf_node * node_sequence;
      element=alstack_pop(stack);
      node =  alabnf_create_node(alabnf, ALABNF_NT_STRING);
      struct alhash_entry * entry = (struct alhash_entry *) element->reference;

      // we copy value to node->string by allocating/copying it on new context.
      {
	struct alabnf_string * node_string = &node->content.string;
	// FIXME should detect correct type ( hex,dec,bin .. ) RULENAME reference.
	node_string->type=ALABNF_ST_RULENAME;
	node_string->strbloc.data.charptr=al_copy_block(&alabnf->context.allocator.ringbuffer,&entry->value);
	node_string->strbloc.length=entry->value.length;
	node_string->strbloc.type=ALTYPE_OPAQUE;	
      }
     
      node_sequence =  alabnf_create_node(alabnf, ALABNF_NT_SEQUENCE);
      sequence = &node_sequence->content.sequence;
      sequence->node = node;
      sequence->next = next_sequence;
      next_sequence=sequence;
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
      aldebug_printf(NULL,"[FATAL] parsing a rule without a rule in stack of tokens.\n");
    }
}

void alabnf_start_string(struct alabnf_sm * state_machine, char c);

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

void alabnf_close_string_rematch(struct alabnf_sm * state_machine, char c);
void alabnf_close_string_continue(struct alabnf_sm * state_machine, char c);

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
  aldebug_printf(NULL,"%c",c);
  altokenizer_add_char(&state_machine->tokenizer,token,c);
}

// cumulated number is converted to char and reset.
void alabnf_flush_number_to_char(struct alabnf_sm * state_machine)
{
  int value = alabnf_flush_number(&state_machine->number_sm);  

  alabnf_add_char(state_machine,'?',(char) value);
}

void alabnf_start_string_ruledef(struct alabnf_sm * state_machine, char c);

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
  enum alabnf_parser_action next_action;
  if ( ( c >= '0' ) && ( c <= '1' ) )
    {
      //
      next_action = ALABNF_PA_CONTINUE;
    }
  else
    {
      if ( c == '.' )
	{
	  // concatenation
	  next_action = ALABNF_PA_CONTINUE;
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
	  next_action = ALABNF_PA_CLOSE;
	}
    }
  state_machine->next_action = next_action;
}

void alabnf_decimal_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;
  struct alabnf_number_sm * number_sm = &state_machine->number_sm;
  if ( ( c >= '0' ) && ( c <= '9' ) )
    {
      //
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
      // number_sm->cumulated |= (c-'0') << (4-number_sm->seen *4);
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


void alabnf_close_name_string_continue(struct alabnf_sm * state_machine, char c);

void alabnf_close_name_string_rematch(struct alabnf_sm * state_machine, char c);

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
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in albnf rule definition\n",c,c);	  
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
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in albnf rule definition\n",c,c);	  
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
	  aldebug_printf(NULL,"[ERROR] unexpected '%i' '%c' in albnf rule definition\n",c,c);	  
	  state_machine->close_method = alabnf_close_string_rematch;
	}
    }
  else
    {
      state_machine->one_char_method=alabnf_string_suffix;
    }

}

void alabnf_group(struct alabnf_sm * state_machine, char c)
{
  // TODO should we stack a state machine ?
  switch(c)
    {
    case ')':
      state_machine->close_method = alabnf_close_string_continue;
      state_machine->next_action = ALABNF_PA_CLOSE;
      break;      
    default:
      alabnf_add_char(state_machine,'"',c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
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
      if ( c == '(' )
	{
	  state_machine->one_char_method=alabnf_group;
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      else if ( c == '<' )
	{
	  state_machine->one_char_method=alabnf_chevron;
	  state_machine->next_action = ALABNF_PA_CONTINUE;
	}
      else
	{
	  state_machine->one_char_method = alabnf_string;
	  alabnf_name_string_intern(state_machine, c);
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

void alabnf_optional(struct alabnf_sm * state_machine, char c)
{
  // TODO should we stack a state machine ?
  switch(c)
    {
    case ']':
      state_machine->close_method = alabnf_close_string_continue;
      state_machine->next_action = ALABNF_PA_CLOSE;
      break;      
    default:
      alabnf_add_char(state_machine,'"',c);
      state_machine->next_action = ALABNF_PA_CONTINUE;
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
	  state_machine->current_indent++;
	}
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '=':
      aldebug_printf(NULL,"[ERROR] unexpected '=' in albnf rule definition\n");
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      // TODO
      state_machine->one_char_method=alabnf_quoted_string;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      // TODO alternatives
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '[':
      state_machine->one_char_method=alabnf_optional;
      state_machine->next_action = ALABNF_PA_CONTINUE;
      break;
    case '(':
      state_machine->one_char_method=alabnf_group;
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
      // TODO restart to a new rule
      if ( state_machine->current_indent <= state_machine->initial_indent )
	{
	  alabnf_new_rule(state_machine);	  
	}
      else
	{
	  aldebug_printf(NULL,"CONTINUE rule # %i, indent %i/%i", state_machine->rule_number,
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
      aldebug_printf(NULL,"[ERROR] unexpected '%i' in albnf expect equals\n",c);
      state_machine->next_action = ALABNF_PA_REMATCH;
    }

}

void alabnf_close_string(struct alabnf_sm * state_machine, char c)
{
  if ( state_machine->state == ALABNF_STATE_RULENAME )
    {
      state_machine->one_char_method=alabnf_start_string;
    }
  else
    {
      state_machine->one_char_method=alabnf_start_string_ruledef;
    }
  struct al_token token;
  token.token=ALABNF_NT_STRING;
  struct alhash_entry * mytoken = altokenizer_make_token(&state_machine->tokenizer,&token,c);
  alabnf_print_token(mytoken);
  alstack_push_ref(state_machine->stack,mytoken);
  aldebug_printf(NULL,"close string token %p\n",mytoken);
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
  alabnf_print_token(mytoken);
  alstack_push_ref(state_machine->stack,mytoken);
  aldebug_printf(NULL,"close name string %p\n",mytoken);
}

void alabnf_close_string_rematch(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_string(state_machine, c);
  state_machine->next_action = ALABNF_PA_REMATCH;
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
  // HACK care about end of line
  if (( c== 10 ) || (c==13))
    {
      aldebug_printf(NULL, "YARGL\n");
      state_machine->next_action = ALABNF_PA_REMATCH;  
    }
  else
    {
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
}

// closes a rulename
void alabnf_close_name_string_continue(struct alabnf_sm * state_machine, char c)
{
  alabnf_close_name_string(state_machine, c);
  state_machine->next_action = ALABNF_PA_CONTINUE;
  // HACK care about end of line
  if (( c== 10 ) || (c==13))
    {
      aldebug_printf(NULL,"YARGL\n");
      state_machine->next_action = ALABNF_PA_REMATCH;  
    }
  else
    {
      state_machine->next_action = ALABNF_PA_CONTINUE;
    }
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
      alabnf=malloc(sizeof(*alabnf));
      alhash_context_init(&alabnf->context,64,1024,200);
      state_machine->generated=alabnf;
    }

  return alabnf;
}
