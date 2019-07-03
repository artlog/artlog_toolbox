#include "alcommon.h"
#include "aldebug_output.h"
#include "al_options.h"
#include "alinput_file.h"
#include "aloutput_file.h"
#include "alcbor_decoder.h"
#include "aljson_dump.h"

#include <stddef.h>

int char_to_int(char a)
{
  int x = (a > '9') ? 10 + a - 'a'  : a - '0';
  return x;
}

// to move to input
unsigned char hex_to_byte(char a, char b)
{

  unsigned char byte = (unsigned char) ( 16 * char_to_int(a) + char_to_int(b) ) ;
  return byte;
}


void usage()
{
  aldebug_printf(DBGSTREAM,"Not yet implemented\n");
  aldebug_printf(DBGSTREAM,"https://tools.ietf.org/html/rfc7049\n");
  aldebug_printf(DBGSTREAM,"https://en.wikipedia.org/wiki/CBOR\n");

  aldebug_printf(DBGSTREAM,"hexstring=<hex string> to be converted to raw bytes ( version 0)");
  aldebug_printf(DBGSTREAM,"infile=<cbor input file>\n");
  aldebug_printf(DBGSTREAM,"outfile=<json output file to be created>\n");
}

// CBOR encoded data is seen as a stream of data items

int main(int argc, char ** argv )
{
  aldebug_start((void *) 0);

  struct al_options * options = al_options_create(argc,argv);
  struct alhash_datablock * infiledata = al_option_get(options,"infile");
  struct alhash_datablock * hexstring = al_option_get(options,"hexstring");
  struct alhash_datablock * outfiledata = al_option_get(options,"outfile");

  if ( hexstring != NULL )
    {
      if ( outfiledata != NULL )
	{
	  struct aloutputstream output;
	  if ( aloutput_file_open_init(&output, outfiledata->data.charptr) == AL_EC_OK )
	    {
	      for (int i =0; i < hexstring->length - 2 ; i+=2 )
		{
		  char a = hexstring->data.charptr[i];
		  char b = hexstring->data.charptr[i+1];
		  unsigned char byte = hex_to_byte(a,b);
		  // aldebug_printf(DBGSTREAM,"%c%c=%x,",a,b,byte);
		  aloutputstream_write_byte(&output,byte);
		}
	      aloutputstream_close(&output);
	    }
	}
    }
  else
  if ( infiledata != NULL )
    {
      alcbor_pc cbor_context;
      struct alinputstream input;
      
      if ( alinput_file_open_init(&input, infiledata->data.charptr) == AL_EC_OK )
	{
	if ( outfiledata != NULL )
	  {
	    struct aloutputstream output;
	    if ( aloutput_file_open_init(&output, outfiledata->data.charptr) == AL_EC_OK )
	      {
		alcbor_parsing_context_init(&cbor_context,&input,&output);
		// now ready to parse ?
		// TODO
		alcbor_parse(&cbor_context);
		struct json_object * root = alcbor_get_json_root(&cbor_context);
		if ( root != NULL )
		  {
		    struct print_ctx print_ctx;
		    struct json_parser_ctx * parser_ctx = alcbor_get_json_context(&cbor_context);

		    aljson_print_ctx_init(&print_ctx);
		    print_ctx.outfile=&output;
		    // should dump it to output
		    aljson_dump_object( parser_ctx, root, &print_ctx);
		  }		    
		alcbor_parsing_context_release(&cbor_context);
		aloutputstream_close(&output);
	      }
	    else
	      {
		aldebug_printf(DBGSTREAM,"[ERROR] can't create file to generate '" ALPASCALSTRFMT "'\n",
			       ALPASCALSTRARGS(infiledata->length,outfiledata->data.charptr));
	      }	  	  
	    alinputstream_close(&input);
	  }
	else
	  {
	    usage();
	  }
	}
      else
	{
	  aldebug_printf(DBGSTREAM,"[ERROR] file to parse '" ALPASCALSTRFMT "' not found \n",
	     ALPASCALSTRARGS(infiledata->length,infiledata->data.charptr));
	  usage();
	}
     
    }
  else
    {
      usage();
    }
  aldebug_end();
  return 0;
}
