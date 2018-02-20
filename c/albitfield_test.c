#include "albitfieldreader.h"
#include "albitfieldwriter.h"
#include "al_options.h"
#include "aldebug.h"

void usage()
{
  printf("test bitfield writer/reader\n");
  printf(" will copy input to output with bit chunks\n");
  printf("outfile: file to create\n");
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
      FILE * fout = fopen(opt1->data.charptr, "w");
      if ( fout != NULL )
	{
	  struct bitfieldwriter writer;
	  struct alhash_datablock * opt2 = al_option_get(options,"infile");
	  FILE * file = NULL;
	  if ( opt2 == NULL )
	    {
	      file = fopen(argv[0],"r");
	    }
	  else
	    {
	      file = fopen(opt2->data.charptr,"r");
	    }
	  aloutputstream_init(&output,fout);
	  bitfieldwriter_init(&writer);
	  bitfieldwriter_setoutputstream(&writer,&output);
	  if ( file != NULL )
	    {     
	      alinputstream_init(&input,fileno(file));
	      int bits = 32;
	      struct bitfieldreader bfreader;
	      fieldreader_init(&bfreader);
	      fieldreader_setinput(&bfreader,&input);
	      int field = 0, lastfield = 0;
	      int total_bits = 0;
	      while ( bitfieldreader_is_eof(&bfreader) == 0 )
		{
		  field = fieldreader_read(&bfreader,bits);
		  aldebug_printf(NULL,"%i ", bits);
		  if ( bitfieldreader_is_eof(&bfreader) == 1 )
		    {
		      // special case at end where input can be unaligned.
		      bits = bitfieldreader_get_readbits(&bfreader);
		      aldebug_printf(NULL,"last field %08x total bits %i, last bits %i\n", field, total_bits, bits);
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
	      aldebug_printf(NULL,"total bits %i, bytes %i", total_bits, total_bits/8);
	      fclose(file);
	    }
	  aloutputstream_close(&output);
	}
    } 
  al_options_release(options);
}
  
