#include "alcommon.h"
#include "aldebug_output.h"
#include "al_options.h"
#include "alinput_file.h"
#include "aloutput_file.h"
#include "alcbor_decoder.h"
#include "aljson_dump.h"
#include "alcbor_encoder.h"
#include "aljson_import_internal.h"
#include "alformat.h"

#include <stddef.h>

const char * CBOR_MAIN_VERSION = "0.3";

void usage()
{
  aldebug_printf(DBGSTREAM,"version %s\nconvert cbor <-> json  Not fully implemented\n", CBOR_MAIN_VERSION);
  aldebug_printf(DBGSTREAM,"https://tools.ietf.org/html/rfc7049\n\
https://en.wikipedia.org/wiki/CBOR\n\
hexstring=<hex string> to be converted to raw bytes in outfile ( in this case cbor)\n\
inform=json|cbor format of infile\n\
infile=<cbor or json input file>\n\
outfile=<json or cbor output file to be created>\n");
}

// CBOR encoded data is seen as a stream of data items

int main(int argc, char ** argv )
{
  aldebug_start((void *) 0);

  struct al_options * options = al_options_create(argc,argv);
  aldatablock * inform = al_option_get(options,"inform");
  aldatablock * infiledata = al_option_get(options,"infile");
  aldatablock * hexstring = al_option_get(options,"hexstring");
  aldatablock * outfiledata = al_option_get(options,"outfile");

  // default to cbor
  enum al_format inform_format=AL_FMT_CBOR;

  
  if ( hexstring != NULL )
    {
      // convert from hexstring to bytes ...
      // why is it done here ??
      if ( outfiledata != NULL )
	{
	  struct aloutputstream output;
	  if ( aloutput_file_open_init(&output, outfiledata->data.charptr) == AL_EC_OK )
	    {
	      for (int i =0; i < hexstring->length - 2 ; i+=2 )
		{
		  char a = hexstring->data.charptr[i];
		  char b = hexstring->data.charptr[i+1];
		  unsigned char byte = alstrings_hex_to_byte(a,b);
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
		if (inform != NULL)
		  {
		    if ( inform->data.charptr != NULL )
		      {
			char * txt = inform->data.charptr;

			inform_format = alstring_prefix(txt,"json",4,AL_FMT_JSON,AL_FMT_UNSET);
			if ( inform_format == AL_FMT_UNSET )
			  {
			    inform_format = alstring_prefix(txt,"cbor",4,AL_FMT_CBOR,AL_FMT_UNSET);
			  }		    
		      }
		  }

		if ( inform_format == AL_FMT_JSON )
		  {	      
		    // parse json and convert it to cbor
		    struct json_parser_ctx json_context;
		    json_token_ctx json_tokenizer;
		    struct print_ctx print_context;
		      
		    aljson_init(&json_context,&json_tokenizer,&print_context);
		    aljson_print_ctx_set_format(&print_context, ALJSON_PRINT_FLAT);
		      
		    struct json_object * root=NULL;
		    struct json_import_context_data data = { .inputstream=&input, .flags=0, .debug=0, .last=0 };
		    root=parse_level(&json_context,&data,root);

		    if ( root != NULL )
		      {
			struct alcbor_encoder encoder;
			alcbor_encoder_init(&encoder, &output);		      
			alcbor_encoder_json_object(root,&encoder.output_context);
		      }
		    else
		      {
			aldebug_printf(DBGSTREAM,"[ERROR] file to parse '" ALPASCALSTRFMT "' not recognized as json \n",
				       ALPASCALSTRARGS(infiledata->length,infiledata->data.charptr));

		      }
		  }
		else
		  {
		    // parse cbor and convert it to json
		    alcbor_parsing_context_init(&cbor_context,&input,&output);
		    alcbor_parse(&cbor_context);
		    struct json_object * root = alcbor_get_json_root(&cbor_context);
		    if ( root != NULL )
		      {
			struct print_ctx print_ctx;
			struct json_parser_ctx * parser_ctx = alcbor_get_json_context(&cbor_context);
			  
			aljson_print_ctx_init(&print_ctx);
			aljson_print_ctx_set_format(&print_ctx,ALJSON_PRINT_FLAT);
			print_ctx.outfile=&output;
			// should dump it to output
			aljson_dump_object(root, &print_ctx);
			// add a final \n.
			print_ctx.printf(&print_ctx,"\n");
		      }		    
		    alcbor_parsing_context_release(&cbor_context);
		  }
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
