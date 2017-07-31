#ifndef _ALCOMMON_H_
#define _ALCOMMON_H_

#define FLAG_IS_SET(x,flag) (x & flag) != 0

// still ctx is provided for compatiblity, not used...
void memory_shortage(void * ctx);

#endif
