#include <stddef.h>

#include "albitfieldreader.h"
#include "albitfieldwriter.h"
#include "al_options.h"
#include "aldebug.h"
#include "aldebug_output.h"
#include "alinput_file.h"
#include "aloutput_file.h"

void usage()
{
  aldebug_printf(DBGSTREAM,"test bitfield writer/reader\n");
  aldebug_printf(DBGSTREAM," will copy input to output with bit chunks\n");
  aldebug_printf(DBGSTREAM,"outfile: file to create\n");
}

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);
  al_options_set_debug(options,0);
  usage();

  struct alinputstream input;
  struct aloutputstream output;
  struct alhash_datablock * opt1 = al_option_get(options,"outfile");
  if ( opt1 != NULL )
    {
      enum al_global_error_code outputerr =  aloutput_file_open_init(&output,opt1->data.charptr);
      if ( outputerr == AL_EC_OK )
	{
	  struct bitfieldwriter writer;
	  struct alhash_datablock * opt2 = al_option_get(options,"infile");
	  enum al_global_error_code inputerr = AL_EC_OK;
	  if ( opt2 == NULL )
	    {
	      inputerr=alinput_file_open_init(&input,argv[0]);
	    }
	  else
	    {
	      inputerr=alinput_file_open_init(&input,opt2->data.charptr);
	    }
	  bitfieldwriter_init(&writer);
	  bitfieldwriter_setoutputstream(&writer,&output);
	  if ( inputerr == AL_EC_OK )
	    {     
	      int bits = 32;
	      struct bitfieldreader bfreader;
	      fieldreader_init(&bfreader);
	      fieldreader_setinput(&bfreader,&input);
	      int field = 0, lastfield = 0;
	      int total_bits = 0;
	      while ( bitfieldreader_is_eof(&bfreader) == 0 )
		{
		  field = fieldreader_read(&bfreader,bits);
		  aldebug_printf(DBGSTREAM,"%i ", bits);
		  if ( bitfieldreader_is_eof(&bfreader) == 1 )
		    {
		      // special case at end where input can be unaligned.
		      bits = bitfieldreader_get_readbits(&bfreader);
		      aldebug_printf(DBGSTREAM,"last field %08x total bits %i, last bits %i\n", field, total_bits, bits);
		      if ( bits == 0 )
			{
			  break;
			}
		    }		  
		  bitfieldwriter_write(&writer,field,bits);
		  total_bits+=bits;
		  bits = ( lastfield & 0x1F ) + 1;
		  lastfield = ( lastfield + 1 )  ^ field;
		}
	      bitfieldwriter_padtobyte(&writer);
	      aldebug_printf(DBGSTREAM,"total bits %i, bytes %i", total_bits, total_bits/8);
	      alinputstream_close(&input);
	    }
	  aloutputstream_close(&output);
	}
    } 
  al_options_release(options);
}
  
