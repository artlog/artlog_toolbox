#ifndef _AL_OPTIONS_H_
#define _AL_OPTIONS_H_

/**
keep options of a program 
can be retrieved by c string key as c strings
 **/

#include "aljson.h"
#include "alhash.h"
#include "aldebug.h"

struct al_options {
  ALDEBUG_DEFINE_FLAG(debug);
  struct alparser_ctx context;
};

ALDEBUG_DECLARE_FUNCTIONS(struct al_options, al_options);

// create options from arguments
struct al_options * al_options_create(int argc, char ** argv);

// get one named option by key
struct alhash_datablock * al_option_get(struct al_options * options,const char * key);

// add one option
void al_option_add(struct al_options * options,const char * key,const char * value);

// free options created with al_create_options.
void al_options_release(struct al_options * options);

#endif
