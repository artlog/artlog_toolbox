#include <stddef.h>
#include <stdlib.h>

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
  aldebug_printf(DBGSTREAM,"infile: file to read, if not given will default to current binary.\n");
  aldebug_printf(DBGSTREAM,"outfile: file to create\n");
}

int main(int argc, char ** argv)
{
  struct al_options * options = al_options_create(argc,argv);

  aldebug_start(NULL);
  al_options_set_debug(options,0);

  struct alinputstream input;
  struct aloutputstream output;
  aldatablock * debug = al_option_get(options,"debug");
  if ( debug == NULL )
    {
      aldebug_mute();
    }
  usage();

  aldatablock * shuffleopt = al_option_get(options,"shuffle");
  int shuffle = 0;
  if (shuffleopt != NULL )
    {
      shuffle = atoi(shuffleopt->data.charptr);
      aldebug_printf(DBGSTREAM,"use shuffle bits %i\n", shuffle);
    }

  aldatablock * opt1 = al_option_get(options,"outfile");
  if ( opt1 != NULL )
    {
      enum al_global_error_code outputerr =  aloutput_file_open_init(&output,opt1->data.charptr);
      if ( outputerr == AL_EC_OK )
      {
        struct bitfieldwriter writer;
        aldatablock * opt2 = al_option_get(options,"infile");
        enum al_global_error_code inputerr = AL_EC_OK;
        if ( opt2 == NULL )
          {
            // use current binary name as source
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
            int bits = shuffle > 0 ? shuffle % 32 : 32;
            shuffle = ( shuffle == 0 );
            struct bitfieldreader bfreader;
            fieldreader_init(&bfreader);
            fieldreader_setinput(&bfreader,&input);
            int field = 0, lastfield = 0;
            int total_bits = 0;
            while ( bitfieldreader_is_eof(&bfreader) == 0 )
            {
              field = fieldreader_read(&bfreader,bits);
              if ( bitfieldreader_is_eof(&bfreader) == 1 )
                {
                  // special case at end where input can be unaligned.
                  bits = bitfieldreader_get_readbits(&bfreader);
                  // seems number of bits read on last is wrong.
                  aldebug_printf(DBGSTREAM,"last field %08x total bits %i, last bits %i\n", field, total_bits, bits);
                  if ( bits == 0 )
                  {
                    break;
                  }
                }
              bitfieldwriter_write(&writer,field,bits);
              total_bits+=bits;
              // not random but somehow shuffled by content
              // number of bits is never the same
              if ( shuffle )
                {
                  bits = ( lastfield & 0x1F ) + 1;
                  lastfield = ( lastfield + 1 )  ^ field;
                }
              // else bits=shuffle % 32;
            }
            // calling this does create wrong length ... to check
            // currently input tests file are byte aligned, so output will be aligned too
            // then this pad to byte should to nothing, but does ...
            // this is more a problem at read that does not return correct numbre of bits.
            // bitfieldwriter_padtobyte(&writer);
            aldebug_printf(DBGSTREAM,"total bits %i, bytes %i", total_bits, total_bits/8);
            alinputstream_close(&input);
          }
        aloutputstream_close(&output);
      }
    }
  al_options_release(options);
  aldebug_end();
}

