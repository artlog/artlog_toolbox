#include <stdio.h>
#include <assert.h>

#include "alcommon.h"
#include "todo.h"
#include "c_tokenizer.h"

TOKEN_DECLARE_TOKENIZER(DQUOTE,'"');
TOKEN_DECLARE_TOKENIZER(SQUOTE,'\'');

  
// match everything until */
struct al_token * c_tokenizer_eat_up_to_end_of_comment(struct json_ctx * ctx, void * data)
{
  int match = 0;

  // potential match that can unmatch
  char m = 0;
  char c = ctx->next_char(ctx, data);

  // main loop char by char, delegate to sub parser in many cases.
  while (c != 0)  {
    switch(c)
      {
      case '*':
	if ( match == 1 )
	  {
	    ctx->add_char(ctx,c,m);
	  }
	m=c;
	match=1;
	break;
      case '/':
	if (match==1)
	  {
	    JSON_TOKEN(COMMENT);
	  }
	else
	  {
	    match=0;
	  }
	break;
      default:
	if ( match == 1 )
	  {
	    ctx->add_char(ctx,c,m);
	  }
	match=0;
	break;
      }
    ctx->add_char(ctx,c,c);
    c = ctx->next_char(ctx, data);
  }
  if ( c != 0 )
    {
      // suspicious case
      printf("// EOF in comment due to '%c' \n",c);
    }
  JSON_TOKEN(EOF);
}

struct al_token * c_tokenizer_eat_up_word(struct json_ctx * ctx, void * data)
{
  char c = ctx->next_char(ctx, data);
  int text = 0;

  while (c != 0)
  {
    if ( (( c >= 'a' ) && ( c <= 'z' ))
	 || (( c >= 'A' ) && ( c <= 'Z' ))
	 || ( c == '_' ) )
      {
	text=1;
	ctx->add_char(ctx,c,c);
      }
    else
      {
	if ( ( text > 0 ) &&
	     (    ( ( c>='0' ) && (c<='9') )
	     )
	   )
	  {
	    ctx->add_char(ctx,c,c);
	  }
	else
	  {
	    ctx->pushback_char(ctx,data,c);
	    JSON_TOKEN(WORD);
	  }
      }
    c = ctx->next_char(ctx, data);
  }
  if ( c != 0 )
    {
      // suspicious case
      printf("// EOF in word due to '%c' \n",c);
    }
  JSON_TOKEN(EOF);

}

// match until end of line ( cr of lf )
struct al_token * c_tokenizer_eat_up_to_end_of_line(struct json_ctx * ctx, void * data)
{

  char c = ctx->next_char(ctx, data);

  // main loop char by char, delegate to sub parser in many cases.
  while (c != 0)  {
      switch(c)
      {
      case 0x0a: // lf
      case 0x0c: // cr
	JSON_TOKEN(COMMENT);
      }
      ctx->add_char(ctx,c,c);
      c = ctx->next_char(ctx, data);
  }
  if ( c != 0 )
    {
      // suspicious case
      printf("// EOF in end of line due to '%c' \n",c);
    }
  JSON_TOKEN(EOF);
}

struct al_token * c_pragma_handler(struct json_ctx * ctx, void * data)
{
  struct al_token * token =  c_tokenizer_eat_up_to_end_of_line(ctx,data);
  JSON_TOKEN(PRAGMA);
}

// expect to enter here while a first '/' has been hit...
struct al_token * c_tokenizer_potential_comment(struct json_ctx * ctx, void * data)
{
  char c = ctx->next_char(ctx, data);

  // main loop char by char, delegate to sub parser in many cases.
  if  (c != 0)  {
  
    // ignored characters when not in string or comment tokenizer
    if ((ctx->internal_flags & JSON_FLAG_IGNORE) == 0 )
      {
	switch(c)
	  {
	  case '/':
	    // // read all until end of line...
	    c_tokenizer_eat_up_to_end_of_line(ctx,data);
	    JSON_TOKEN(COMMENT);
	    break;
	  case '*':
	    // /* read up to '*/'
	    c_tokenizer_eat_up_to_end_of_comment(ctx,data);
	    JSON_TOKEN(COMMENT);
	    break;
	  default:
	    ctx->pushback_char(ctx,data,c);
	    JSON_TOKEN(PUSHBACK);
	  }
      }
  }
  if ( c != 0 )
    {
      // suspicious case
      printf("// EOF in potential commen due to '%c' \n",c);
    }
  JSON_TOKEN(EOF);

}

struct al_token * c_tokenizer_starting_with_minus(struct json_ctx * ctx, void * data)
{
    char c = ctx->next_char(ctx, data);
    switch (c)  {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return tokenizer_NUMBER(ctx,c,data);
      break;
    case '>':
      JSON_TOKEN(RIGHT_ARROW);
      break;
    case '-':
      JSON_TOKEN(DECREMENT);
      break;
    case '=':
      JSON_TOKEN(SUBTRACT);
      break;
    default:
      ctx->pushback_char(ctx,data,c);
      JSON_TOKEN(MINUS);
    }
}

struct al_token * c_tokenizer_starting_with_and(struct json_ctx * ctx, void * data)
{
    char c = ctx->next_char(ctx, data);
    switch (c)  {
    case 0:
      JSON_TOKEN(EQUAL);
      break;
    case '&':
      ctx->add_char(ctx,'&',c);
      JSON_TOKEN(LOGICAL_AND);
      break;
    default:
      ctx->pushback_char(ctx,data,c);
      JSON_TOKEN(AMPERSAND);
      break;
    }
}

struct al_token * c_tokenizer_starting_with_exclamation(struct json_ctx * ctx, void * data)
{
    char c = ctx->next_char(ctx, data);
    switch (c)  {
    case 0:
      JSON_TOKEN(EOF);
      break;
    case '=':
      ctx->add_char(ctx,'!',c);
      JSON_TOKEN(COMPARE_DIFFERENT);
      break;
    default:
      ctx->pushback_char(ctx,data,c);
      JSON_TOKEN(EXCLAMATION);
      break;
    }
}

struct al_token * c_tokenizer_starting_with_plus(struct json_ctx * ctx, void * data)
{
    char c = ctx->next_char(ctx, data);
    switch (c)  {
    case 0:
      JSON_TOKEN(EOF);
      break;
    case '=':
      // ctx->add_char(ctx,'+',c);
      JSON_TOKEN(ADD);
      break;
    case '+':
      // ctx->add_char(ctx,'+',c);
      JSON_TOKEN(INCREMENT);
      break;
    default:
      ctx->pushback_char(ctx,data,c);
      JSON_TOKEN(PLUS);
      break;
    }
}

struct al_token * c_tokenizer_starting_with_or(struct json_ctx * ctx, void * data)
{
    char c = ctx->next_char(ctx, data);
    switch (c)  {
    case 0:
      JSON_TOKEN(EQUAL);
      break;
    case '|':
      ctx->add_char(ctx,'|',c);
      JSON_TOKEN(LOGICAL_OR);
      break;
    default:
      ctx->pushback_char(ctx,data,c);
      JSON_TOKEN(PIPE);
      break;
    }
}

struct al_token * c_tokenizer_starting_with_equal(struct json_ctx * ctx, void * data)
{
    char c = ctx->next_char(ctx, data);
    switch (c)  {
    case 0:
      JSON_TOKEN(EQUAL);
      break;
    case '=':
      // ctx->add_char(ctx,'=',c);
      JSON_TOKEN(COMPARE_EQUAL);
      break;
    default:
      ctx->pushback_char(ctx,data,c);
      JSON_TOKEN(EQUAL);
      break;
    }
}

struct al_token * c_tokenizer(struct json_ctx * ctx, void * data)
{
  char c = ctx->next_char(ctx, data);
  ctx->internal_flags &= !JSON_FLAG_IGNORE;

  // main loop char by char, delegate to sub parser in many cases.
  while (c != 0)  {
    if ( FLAG_IS_SET(ctx->debug_level,2) )
      {
	switch(c)
	  {
	  case '{':
	  case '}':
	  case '[':
	  case ']':
	  case '"':
	  case '\'':
	  case ',':
	  case ':':
	  case '?':
	    printf("%c[%x]\n",c,c);
	    puts("\n-------");
	    break;
	  default:
	    printf("%c",c);
	  }
      }

    // ignored characters when not in string or comment tokenizer
    switch(c)
      {
      case ' ':
      case 0x0a: // lf
      case 0x0c: // cr
      case 9: // tab
	// ignore
	if ( FLAG_IS_SET(ctx->debug_level,1) )
	  {
	    if ( ! FLAG_IS_SET(ctx->internal_flags , JSON_FLAG_IGNORE)  )
	      {
		printf("<ignore %c", c);
	      }

	    else
	      {
		printf("%c", c);
	      }
	  }
	ctx->internal_flags |= JSON_FLAG_IGNORE;
	break;
      default:
	if ( FLAG_IS_SET(ctx->debug_level,1) )
	  {
	    if ( FLAG_IS_SET(ctx->internal_flags, JSON_FLAG_IGNORE) )
	      {
		printf("ignore>");
	      }
	  }
	ctx->internal_flags &= !JSON_FLAG_IGNORE;
      };

    if ((ctx->internal_flags & JSON_FLAG_IGNORE) == 0 )
      {
	switch(c)
	  {
	  case '#':
	    return c_pragma_handler(ctx,data);
	  case '{':
	    JSON_TOKEN(OPEN_BRACE);
	    break;
	  case '}':
	    JSON_TOKEN(CLOSE_BRACE);
	    break;
	  case '(':
	    JSON_TOKEN(OPEN_PARENTHESIS);
	    break;
	  case ')':
	    JSON_TOKEN(CLOSE_PARENTHESIS);
	    break;
	  case '[':
	    JSON_TOKEN(OPEN_BRACKET);
	    break;
	  case ']':
	    JSON_TOKEN(CLOSE_BRACKET);
	    break;
	  case '"':
	    return tokenizer_DQUOTE(ctx,data);
	    // else should be handled by ':', ',' or '}' or ']' ie any close.
	    break;
	  case ',':
	    JSON_TOKEN(COMMA);
	    break;
	  case '?':
	    JSON_TOKEN(QUESTION);
	    break;
	  case ':':
	    JSON_TOKEN(COLON);
	    break;
	  case '\'':
	    return tokenizer_SQUOTE(ctx,data);
	    break;
	  case '\\':
	    // - multiline ( + crlf )
	    todo("multiline \\");
	    break;
	  case '/':
	    // many possibilities with comments
	    // /*
	    // //
	    return c_tokenizer_potential_comment(ctx,data);
	    break;
	  case ';':
	    JSON_TOKEN(SEMI_COLON);
	    break;
	  case '-':
	    // NOT THAT EASY ... -> is not number ...
	    return c_tokenizer_starting_with_minus(ctx,data);
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
	    return tokenizer_NUMBER(ctx,c,data);
	    break;
	  case '&':
	    return c_tokenizer_starting_with_and(ctx,data);
	    break;
	  case '|':
	    return c_tokenizer_starting_with_or(ctx,data);
	  case '.':
	    JSON_TOKEN(DOT);
	    break;
	  case '*':
	    JSON_TOKEN(STAR);
	  case '>':
	    JSON_TOKEN(SUPERIOR);
	    break;
	  case '<':
	    JSON_TOKEN(INFERIOR);
	    break;
	  case '=':
	    return c_tokenizer_starting_with_equal(ctx,data);
	  case '%':
	    JSON_TOKEN(PERCENT);
	    break;
	  case '!':
	    return c_tokenizer_starting_with_exclamation(ctx, data);
	  case '+':
	    return c_tokenizer_starting_with_plus(ctx, data);
	  case 0:
	    // force failure programming error
	    assert(c!=0);
	    break;
	  default: // 'normal' word
	    ctx->add_char(ctx,c,c);
	    return c_tokenizer_eat_up_word(ctx, data);
	  }
      }
    
    c=ctx->next_char(ctx, data);
  }
  
  JSON_TOKEN(EOF);
}
