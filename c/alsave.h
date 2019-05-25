#ifndef ALSAVE_HEADER_
#define ALSAVE_HEADER_

struct alsavecontext {
  char dir[4096];
  char prefix[256];
  char extension[256];
  int index_min;
  int index_max;
  int maxsave;
  char lastfile[256];
};

/** set with new debug and returns old settings
 */
int alsave_set_debug(int debug);

int alsave_init_context(struct alsavecontext * context, const char * dir, const char* prefix, const char * extension);

int alsave_shift_file_name(struct alsavecontext * savecontext);

/** return 0 if file exist and can be open in read mode */
int alsave_file_exists(char * template);

#endif
