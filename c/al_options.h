#ifndef _AL_OPTIONS_H_
#define _AL_OPTIONS_H_

#include "json.h"
#include "alhash.h"

struct al_options {
  struct alhash_table table;
  struct token_char_buffer buffer;
};

struct al_options * al_create_options(int argc, char ** argv);

struct alhash_datablock * al_option_get(struct al_options * options, char * key);

void al_option_add(struct al_options * options, char * key, char * value);

// free options created with al_create_options.
void al_options_release(struct al_options * options);

#endif
