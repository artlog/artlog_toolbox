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

#endif
