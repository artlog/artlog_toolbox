#ifndef ALDEBUG_OUTPUT_HEADER_
#define ALDEBUG_OUTPUT_HEADER_

#include "aloutput.h"

struct aldebugconfig {
  int muted;
  struct aloutputstream output;
};

extern struct aldebugconfig aldebug_default;

#define DBGSTREAM &aldebug_default

// output_uri specify where to output, NULL is stderr
void aldebug_start(char * ouput_uri);

void aldebug_end();

// temporary mute any debug
void aldebug_mute();

// unmute globally
void aldebug_unmute();

void aldebug_printf(struct aldebugconfig * debugconfig, const char *format, ...);

struct aloutputstream * aldebug_get_output(struct aldebugconfig * debugconfig);

#endif // #ifndef ALDEBUG_OUTPUT_HEADER_
