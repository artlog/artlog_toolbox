#include "alabnf.h"
#include "al_options.h"
#include "alinput.h"

#include <stdio.h>
#include <stdlib.h>


// b binary
// d decimal
// x hexadecioal

/*
      NOTE:     ABNF strings are case-insensitive and
                  the character set for these strings is us-ascii.
*/

struct alabnf_sm;


typedef void (*alabnf_one_char_method) (struct alabnf_sm * state_machine, char c);
typedef void (*alabnf_close_method) (struct alabnf_sm * state_machine, char c);

enum alabnf_parser_action {
  ALABNF_PA_CONTINUE,
  ALABNF_PA_REMATCH,
  ALABNF_PA_CLOSE,
  ALABNF_PA_FAIL
};

struct alabnf_sm {
  void * todo;
  alabnf_one_char_method one_char_method;
  alabnf_close_method close_method;
  enum alabnf_parser_action next_action;
  struct alinputstream * inputstream;
  char rematch;
};

void alabnf_start_string(struct alabnf_sm * state_machine, char c);
  
void alabnf_state_machine_init(struct alabnf_sm *state_machine,   struct alinputstream * inputstream)
{
  state_machine->one_char_method = alabnf_start_string;
  state_machine->next_action = ALABNF_PA_CONTINUE;
  state_machine->inputstream = inputstream;
  state_machine->rematch=0;
}

void alabnf_close_string(struct alabnf_sm * state_machine, char c);

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
      else
	{
	  state_machine->close_method = alabnf_close_string;
	  next_action = ALABNF_PA_CLOSE;
	}
    }
  state_machine->next_action = next_action;
}

void alabnf_decimal_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;
  if ( ( c >= '0' ) && ( c <= '9' ) )
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
      else
	{
	  state_machine->close_method = alabnf_close_string;
	  next_action = ALABNF_PA_CLOSE;
	}
    }
  state_machine->next_action = next_action;
}

void alabnf_hexadecimal_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;
    if (
	(( c >= '0' ) && ( c <= '9' ))
	||
	(( c >= 'a' ) && ( c <= 'f' ))
	||
	(( c >= 'A' ) && ( c <= 'F' ))
	)
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
	else
	  {
	    state_machine->close_method = alabnf_close_string;
	    next_action = ALABNF_PA_CLOSE;
	  }
      }
    state_machine->next_action = next_action;
}

void alabnf_name_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;
  if (
      (( c >= 'a' ) && ( c <= 'z' ))
      ||
      (( c >= 'A' ) && ( c <= 'Z' ))
      )
    {
      //
      next_action = ALABNF_PA_CONTINUE;
    }
    else
      {
	state_machine->close_method = alabnf_close_string;
	next_action = ALABNF_PA_CLOSE;
      }
    state_machine->next_action = next_action;
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

void alabnf_start_string(struct alabnf_sm * state_machine, char c);

void alabnf_quoted_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;

  switch(c)
    {
    case '"':
      // todo should close current string...
      state_machine->one_char_method=alabnf_start_string;
      next_action = ALABNF_PA_CONTINUE;
      break;      
    default:
      next_action = ALABNF_PA_REMATCH;
    }

  state_machine->next_action = next_action;
  
}

void alabnf_start_string(struct alabnf_sm * state_machine, char c)
{
  enum alabnf_parser_action next_action;

  switch(c)
    {
    case '%':
      state_machine->one_char_method=alabnf_start_typed_string;
      next_action = ALABNF_PA_CONTINUE;
      break;
    case ' ':
      next_action = ALABNF_PA_CONTINUE;
      break;
    case '=':
      // TODO
      next_action = ALABNF_PA_CONTINUE;
      break;
    case '"':
      // TODO
      state_machine->one_char_method=alabnf_quoted_string;
      next_action = ALABNF_PA_CONTINUE;
      break;
    case '/':
      // TODO alternatives
      next_action = ALABNF_PA_CONTINUE;
      break;
    case '.':
      // TODO
      next_action = ALABNF_PA_CONTINUE;
      break;
    default:
      state_machine->one_char_method=alabnf_name_string;
      next_action = ALABNF_PA_REMATCH;
    }

  state_machine->next_action = next_action;
  
}

void alabnf_close_string(struct alabnf_sm * state_machine, char c)
{
  state_machine->one_char_method = alabnf_start_string;
  state_machine->next_action = ALABNF_PA_REMATCH;
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
	      aldebug_printf(NULL,"[FATAL] rematch a 0\n");
	      exit(1);
	    }	    
	  aldebug_printf(NULL,"rematch %c\n",c);
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
	      if ( state_machine->next_action == ALABNF_PA_CONTINUE )
		{
		  printf("%c",c);
		}
	      if ( state_machine->next_action == ALABNF_PA_CLOSE )
		{
		  if ( state_machine->rematch == 0 )
		    {
		      aldebug_printf(NULL,"[FATAL] rematch a 0\n");
		      exit(1);		     
		    }
		  printf( "\nCLOSE '%c'\n", c);
		  alabnf_close_method close_method = state_machine->close_method;
		  close_method(state_machine,c);
		}
	    }
	  else
	    {
	      state_machine->next_action = ALABNF_PA_FAIL;
	    }
	}
    }
  while (  action != ALABNF_PA_FAIL );

}

void alabnf_normalize_name(char * in, char ** out)
{
  *out=in;
}


int main(int argc, char ** argv)
{

  struct alabnf_sm   state_machine;

  struct al_options * options = al_options_create(argc,argv);
  struct alhash_datablock * infiledata = al_option_get(options,"infile");

  struct alinputstream main_inputstream;
  struct alinputstream * inputstream = NULL;

  if ( infiledata != NULL )
    {
      printf("file to parse '" ALPASCALSTRFMT "'\n",
	     ALPASCALSTRARGS(infiledata->length,infiledata->data.charptr));
      FILE * file = fopen((char *)infiledata->data.ptr, "r");
      if ( file == NULL )
	{
	  aldebug_printf(NULL,"[ERROR] fail to open '%s'\n",infiledata->data.charptr);
	}
      else
	{
	  alinputstream_init(&main_inputstream, fileno (file));
	  inputstream=&main_inputstream;

	  alabnf_state_machine_init(&state_machine,inputstream);
	  alabnf_state_machine_run(&state_machine);
	}
    }
  else
    {
      aldebug_printf(NULL,"[ERROR] missing argument infile= file to parse.");
    }
    
  aldebug_printf(NULL,"[TODO]\n");
  
}
