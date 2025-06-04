#ifndef _ALCOMMON_H_
#define _ALCOMMON_H_

// could be qint64 in Qt, see cstdint ...
typedef long long alint64_t;
typedef unsigned long long aluint64_t;

#define ALC_FLAG_IS_SET(x,flag) (x & flag) != 0
#define FLAG_IS_SET ALC_FLAG_IS_SET

// still ctx is provided for compatiblity, not used...
void memory_shortage(void * ctx);

#define ALC_SET_FLAG(x,flag) {x |= flag;}

// thanks to this format ... print non NULL terminated string
#define ALPASCALSTRFMT "%.*s"
#define ALPASCALSTRARGS(length,strptr) length,strptr

enum al_global_error_code {
  // ! request was not yet implemented
  AL_EC_NYI=-1,
  //! error comes form some input ( might derive from structure hierarch or links ).
  AL_EC_INVALID_INPUT=-2,
  //! more precise than invalid input, error is in one parameter
  AL_EC_INVALID_PARAMETER=-3,
  // ! error while accessing a file
  AL_EC_FILE_ERROR=-4,
  // ! eof reached stopping current action
  AL_EC_EOF=-5,
  // ! request to access memory out of bound ( often within a block )
  AL_EC_OOB=-6,
  // this should not happen but will...
  AL_EC_BUG=-7,
  // expected value has no corresponding index
  AL_EC_INDEX_NOT_FOUND=-8,
  // some initialization is missing
  AL_EC_INIT_MISSING=-9,
  AL_EC_OK=1,
  AL_EC_FALSE=0,
  // neither true or false, process should continue to know it
  AL_EC_CONTINUE=2,
};

#endif
