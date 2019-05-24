#ifndef ALHASH_OUTPUT_HEADER_
#define ALHASH_OUTPUT_HEADER_

#include "alhash.h"
#include "aloutput.h"

/**
data will be casted to struct aloutputstream 

usage : 

alhash_walk_table(table, alhash_output_dump_entry_callback_cast_outputstream, (void *) output );

// where output is
struct aloutputstream * output;

**/
int alhash_output_dump_entry_callback_cast_outputstream(struct alhash_entry * entry, void * data, int index);


/**
data will be casted to struct aloutputstream 

usage : 

alhash_walk_table(table, alhash_output_walk_simple_callback_cast_outputstream, (void *) output );

// where output is
struct aloutputstream * output;

**/
int alhash_output_walk_simple_callback_cast_outputstream (struct alhash_entry * entry, void * data, int index);

#endif

