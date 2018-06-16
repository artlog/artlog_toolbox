#ifndef _ALCOMMON_H_
#define _ALCOMMON_H_


#define ALC_FLAG_IS_SET(x,flag) (x & flag) != 0
#define FLAG_IS_SET ALC_FLAG_IS_SET

// still ctx is provided for compatiblity, not used...
void memory_shortage(void * ctx);

#define ALC_SET_FLAG(x,flag) {x |= flag;}

// thanks to this format ... print non NULL terminated string
#define ALPASCALSTRFMT "%.*s"
#define ALPASCALSTRARGS(length,strptr) length,strptr

enum al_global_error_code {
  AL_EC_NYI=-1,
  //! error comes form some input ( might derive from structure hierarch or links ).
  AL_EC_INVALID_INPUT=-2,
  //! more precise than invalid input, error is in one parameter
  AL_EC_INVALID_PARAMETER=-3,
  AL_EC_FILE_ERROR=-4,
  AL_EC_OK=1,
  AL_EC_FALSE=0
};

#endif
