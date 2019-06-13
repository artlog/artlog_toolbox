#ifndef ALSAVE_HEADER_
#define ALSAVE_HEADER_

struct alsavecontext {
  char dir[4096];
  char prefix[256];
  char extension[256];
  int index_min;
  int index_max;
  int maxsave;
  char lastfile[4608];
};

/** set with new debug and returns old settings
 */
int alsave_set_debug(int debug);

int alsave_init_context(struct alsavecontext * context, const char * dir, const char* prefix, const char * extension);

int alsave_shift_file_name(struct alsavecontext * savecontext);

/** return EL_EC_OK if file exist and can be open in read mode */
enum al_global_error_code alsave_file_exists(char * template);

/* set prefix with a nul terminated string */
int alsave_set_prefix(struct alsavecontext * savecontext, const char * prefix);

/* return full filename constructed from dir prefix index and extension */
char * alsave_get_fullfilename(struct alsavecontext * savecontext);
  
#endif
