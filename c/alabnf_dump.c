#include "alabnf_dump.h"

#include <stddef.h>
#include "aldebug_output.h"

/** alabnf output functions*/
void alabnf_dump_sequence(struct aloutputstream * output,struct alabnf_sequence * start_sequence);
void alabnf_dump_alternative(struct aloutputstream * output,struct alabnf_alternative * alternative);
void alabnf_dump_range(struct aloutputstream * output,struct alabnf_range * range);

void alabnf_print_token(struct aloutputstream * output, struct alhash_entry * mytoken)
{
    if ( mytoken != NULL )
    {
      if ( mytoken->key.data.ptr != NULL )
	{
	  struct alhash_datablock * datablock = &mytoken->key;
	  aloutputstream_printf_1k(output,ALPASCALSTRFMT" ",
		 ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
	}
    }

}

void alabnf_dump_iterator(struct aloutputstream * output,struct alabnf_iterator * iterator)
{
  if ( iterator != NULL )
    {
      if ( iterator->min == 0 )
	{
	  // prefix
	  if ( iterator->max == 1 )
	    {
	      // optional
	      aloutputstream_printf_1k(output,"[");
	    }
	  else if ( iterator->max == 0 )
	    {
	      // well empty whatever
	      aloutputstream_printf_1k(output,"0");
	    }
	  else if ( iterator->max == ALABNF_INFINITE_ITERATION )
	    {
	      aloutputstream_printf_1k(output,"*");		   
	    }
	  else
	    {
	      aloutputstream_printf_1k(output,"%i*%i",iterator->min,iterator->max);

	    }
	  alabnf_dump_node(output,iterator->node);
	  // suffix
	  if ( iterator->max == 1 )
	    {
	      // optional
	      aloutputstream_printf_1k(output,"]");
	    }
	}
      else
	{
	  if ( iterator->max == ALABNF_INFINITE_ITERATION )
	    {
	      aloutputstream_printf_1k(output,"m%i*",iterator->min);
	    }
	  else
	    {
	      aloutputstream_printf_1k(output,"m%i*M%i",iterator->min,iterator->max);
	    }
	  alabnf_dump_node(output,iterator->node);
	}
    }
}

void alabnf_dump_hex_string(struct aloutputstream * output,aldatablock * string )
{
  aloutputstream_printf_1k(output,"%%x");
  for (int i=0; i< string->length-1; i++)
    {
      aloutputstream_printf_1k(output,"%hhx.",string->data.charptr[i]);
    }
  aloutputstream_printf_1k(output,"%hhx",string->data.charptr[string->length-1]);
}

void alabnf_dump_nameblock(struct aloutputstream * output,struct alhash_datablock * datablock)
{
  if ( datablock->data.charptr != NULL )
    {
      aloutputstream_printf_1k(output,ALPASCALSTRFMT,
	     ALPASCALSTRARGS(datablock->length,datablock->data.charptr));
    }
  else
    {
      aloutputstream_printf_1k(output,"#unamed(%p)", datablock);
    }

}

void alabnf_dump_quoted_nameblock(struct aloutputstream * output,struct alhash_datablock * datablock,char start, char end)
{
  if ( datablock->data.charptr != NULL )
    {
      aloutputstream_printf_1k(output,"%c"ALPASCALSTRFMT"%c",
			       start,
			       ALPASCALSTRARGS(datablock->length,datablock->data.charptr),
			       end);
    }
  else
    {
      aloutputstream_printf_1k(output,"%c%c", start,end);
    }

}

void alabnf_dump_string(struct aloutputstream * output, struct alabnf_string * string )
{
  struct alhash_datablock * datablock = &string->strbloc;
  enum alabnf_string_type string_type = string->type;

  if ( string_type == ALABNF_ST_RULENAME )
    {
      alabnf_dump_nameblock(output,datablock);
    }
  else if ( string_type == ALABNF_ST_QUOTED )
    {
      alabnf_dump_quoted_nameblock(output,datablock,'"','"');
    }
  else if ( string_type == ALABNF_ST_UNDEFINED )
    {
      aloutputstream_printf_1k(output,"ALABNF_ST_UNDEFINED_");
      alabnf_dump_hex_string(output,datablock);
    }
  else if ( string_type == ALABNF_ST_HEX )
    {
      alabnf_dump_hex_string(output,datablock);
    }
  else
    {
      // need a better printf to support non printable strings as hex,int, quoted ...
      aloutputstream_printf_1k(output,"string_type(%i):",string_type);
      alabnf_dump_hex_string(output,datablock);
    }

}

void alabnf_dump_rule_ref(struct aloutputstream * output,struct alabnf_rule_ref * rule_ref)
{  
  struct alhash_datablock * datablock = &rule_ref->keyblock;
  alabnf_dump_nameblock(output,datablock);
}

void alabnf_dump_node(struct aloutputstream * output,struct alabnf_node * node )
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
	  alabnf_dump_iterator(output,&node->content.iterator);
	  break;
	case ALABNF_NT_SEQUENCE:
	  alabnf_dump_sequence(output,&node->content.sequence);
	  break;
	case ALABNF_NT_STRING:
	  alabnf_dump_string(output, &node->content.string);
	  break;
	case ALABNF_NT_ALT:
	  alabnf_dump_alternative(output,&node->content.alt);
	  break;
	case ALABNF_NT_RANGE:
	  alabnf_dump_range(output,&node->content.range);
	  break;
	case ALABNF_NT_RULE_REF:
	  alabnf_dump_rule_ref(output,&node->content.rule_ref);
	  break;
	default:
	  aldebug_printf(NULL,"[ERROR] unrecognized abnf type %i\n",type);
	}
    }
  else
    {
      // NULL
      aldebug_printf(NULL,"[ERROR] node pointer NULL in %s %s %i", __FILE__, __func__,__LINE__);
      aloutputstream_printf_1k(output,"¤");
    }
}

void alabnf_dump_alternative(struct aloutputstream * output,struct alabnf_alternative * start_alternative)
{
  struct alabnf_alternative * alternative=start_alternative;
  struct alabnf_alternative * next_alternative=NULL;
  // debug
  aloutputstream_printf_1k(output,"{");
  while (alternative != NULL)
    {
      next_alternative = alternative->alt;
      aldebug_printf(NULL,"node %p alt %p\n", alternative->node, next_alternative);
      // DEBUG only, to remove
      alabnf_dump_node(output,alternative->node);
      if (next_alternative != NULL)
	{
	  aloutputstream_printf_1k(output,"/");
	}
      alternative=next_alternative;
    }
  // debug
  aloutputstream_printf_1k(output,"}");


}

void alabnf_dump_range(struct aloutputstream * output,struct alabnf_range * range)
{
  // TODO
  aloutputstream_printf_1k(output,"%%x%x-%x",range->start,range->end);
}

void alabnf_dump_sequence(struct aloutputstream * output,struct alabnf_sequence * start_sequence)
{
  struct alabnf_sequence * sequence=start_sequence;
  struct alabnf_sequence * next_sequence=NULL;
  aloutputstream_printf_1k(output,"(");
  while (sequence != NULL)
    {
      next_sequence = sequence->next;
      aldebug_printf(NULL,"\nnext %p node %p\n", next_sequence, sequence->node);
      // debug printf("§");
      alabnf_dump_node(output,sequence->node);
      // debug printf("$");
      if (next_sequence != NULL)
	{
	  aloutputstream_printf_1k(output," ");
	}
      sequence=next_sequence;
    }
  aloutputstream_printf_1k(output,")");
}

void alabnf_dump_rule(struct aloutputstream * output,struct alabnf_rule * rule)
{
  // todo rule=
  if ( rule != NULL )
    {
      alabnf_dump_string(output,&rule->rule_name);
      aloutputstream_printf_1k(output," = ");
      alabnf_dump_node(output,rule->value);
      aloutputstream_printf_1k(output,"\n");
    }
}
