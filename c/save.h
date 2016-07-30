struct savecontext {
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
int save_set_debug(int debug);

int save_init_context(struct savecontext * context, char * dir, char* prefix, char * extension);

int save_shift_file_name(struct savecontext * savecontext);
