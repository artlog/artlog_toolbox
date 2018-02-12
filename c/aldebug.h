#ifndef __ALDEBUG_H__
#define __ALDEBUG_H__

/**
support xxx_set_debug(object,flags) and xxx_is_debug(object,flag);
 */
#include "alcommon.h"

// add a member ( or a global varable )
#define ALDEBUG_DEFINE_FLAG(x) int x;

#define ALDEBUG_DECLARE_FUNCTIONS(member_type, prefix)	\
int prefix ##_set_debug(member_type * m, int flags); \
int prefix ##_is_debug(member_type * m,int flag);\



#define ALDEBUG_DEFINE_FUNCTIONS(member_type, prefix, debug)	\
int prefix ##_set_debug(member_type * m,int flags) \
{ \
  if ( m != NULL ) \
    { \
      int previous=m->debug; \
      m->debug=flags; \
      return previous; \
    } \
  else { return 0; } \
} \
int prefix ##_is_debug(member_type * m,int flag) \
{ \
  return ALC_FLAG_IS_SET(m->debug,flag); \
}

#define ALDEBUG_DEFINE_STRUCT_FUNCTIONS(struct_type) ALDEBUG_DEFINE_FUNCTIONS(struct struct_type, struct_type, debug)

#define ALDEBUG_SET_DEBUG(object, prefix, flags) prefix ##_set_debug(object,flags);

#define ALDEBUG_IF_DEBUG(object, prefix, debug)	\
  if ( prefix ##_is_debug( object, 1))

void aldebug_printf(void * debugconfig, const char *format, ...);

#endif // #ifndef __ALDEBUG_H__
