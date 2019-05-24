#include "alhash_output.h"
#include "al_options.h"

void al_option_dump_output(struct al_options * options, struct aloutputstream * output)
{

  aloutputstream_printf_1k(output, "argsnumber=%i\n", options->argsnumber); 
  
  alhash_walk_table(&options->context.dict,alhash_output_walk_simple_callback_cast_outputstream,(void *) output);
}
