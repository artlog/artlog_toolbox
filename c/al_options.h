#ifndef _AL_OPTIONS_H_
#define _AL_OPTIONS_H_

/**
keep options of a program
 **/

#include "aljson.h"
#include "alhash.h"

struct al_options {
  struct alhash_table table;
  alstrings_ringbuffer_pointer ringbuffer;
};

// create options from arguments
struct al_options * al_create_options(int argc, char ** argv);

// get one named option by key
struct alhash_datablock * al_option_get(struct al_options * options, char * key);

// add one option
void al_option_add(struct al_options * options, char * key, char * value);

// free options created with al_create_options.
void al_options_release(struct al_options * options);

#endif
