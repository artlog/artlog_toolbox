#include "alpathfile.h"
#include "aldebug_output.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

struct alpathfile_options * alpathfile_get_default_options()
{
  return NULL;
}

/**
options use alpathfile_get_default_options()
pathanme0 full path NUL terminated string
 **/
alpathfile_reply_set alpathfile_check_exists(struct alpathfile_options * options, const char * pathname0 )
{
  alpathfile_reply_set reply = 0;
  const char* pathfilename = pathname0;
  struct stat sb;

  if (stat(pathfilename, &sb) == 0)
    {
      switch (sb.st_mode & S_IFMT) {
      case S_IFBLK: // printf("block device\n");
	reply |=  ALPATHFILE_REPLY_IS_OTHER;
	break;
      case S_IFCHR:  // printf("character device\n");
	reply |=  ALPATHFILE_REPLY_IS_OTHER;
	break;
      case S_IFDIR:  // printf("directory\n");
	reply |=  ALPATHFILE_REPLY_IS_DIR;
	break;
      case S_IFIFO:  // printf("FIFO/pipe\n");
	reply |=  ALPATHFILE_REPLY_IS_OTHER;
	break;
      case S_IFLNK:  // printf("symlink\n");
	// FIXME ? should find on what it points
	reply |=  ALPATHFILE_REPLY_IS_LINK;
	break;
      case S_IFREG:  printf("regular file\n");
	reply |=  ALPATHFILE_REPLY_IS_FILE;
	break;
      case S_IFSOCK: // intf("socket\n");
	reply |=  ALPATHFILE_REPLY_IS_OTHER;
	break;
      default:      //  printf("unknown?\n");
	reply |=  ALPATHFILE_REPLY_IS_OTHER;
	break;
      }
    }
  else
    {
      if ( errno == ENOENT )
	{
	  reply = ALPATHFILE_REPLY_NO_ENTRY;
	}
      else
	{
	  reply = ALPATHFILE_REPLY_ERROR;
	}
    }
  return reply;
}

enum al_global_error_code  alpathfile_is_directory(const char * pathname0 )
{
  alpathfile_reply_set reply =  alpathfile_check_exists(alpathfile_get_default_options(),pathname0);
  return ALC_FLAG_IS_SET(reply,ALPATHFILE_REPLY_IS_DIR)  ?
    AL_EC_OK :
    ( ( reply == ALPATHFILE_REPLY_ERROR ) ?
      AL_EC_FILE_ERROR : AL_EC_FALSE ) ;
}

enum al_global_error_code  alpathfile_is_file(const char * pathname0 )
{
  alpathfile_reply_set reply =  alpathfile_check_exists(alpathfile_get_default_options(),pathname0);
  return ALC_FLAG_IS_SET(reply,ALPATHFILE_REPLY_IS_FILE) ?
    AL_EC_OK :
    ( ( reply == ALPATHFILE_REPLY_ERROR ) ?
      AL_EC_FILE_ERROR : AL_EC_FALSE );
}

enum al_global_error_code  alpathfile_exists(const char * pathname0 )
{
  alpathfile_reply_set reply =  alpathfile_check_exists(alpathfile_get_default_options(),pathname0);
  return (reply ==  ALPATHFILE_REPLY_NO_ENTRY ) ?
    AL_EC_FALSE :
    ( ( reply == ALPATHFILE_REPLY_ERROR ) ?
      AL_EC_FILE_ERROR : AL_EC_OK );
}

enum al_global_error_code alpathfile_can_open(const char * pathname0)
{
  const char * template = pathname0;
  
  FILE * test=fopen(template,"r");
  if ( test == NULL )
    {
      if (errno == ENOENT) {
	return AL_EC_FALSE;
      }
      else if ( errno == EINVAL )
	{
	  aldebug_printf(NULL,"weird invalid argument, contact developper (in function %s:%i)\n",__func__,__LINE__);
	  return AL_EC_INVALID_INPUT;
	}
      else
	{
	  printf("File exists but can't be opened errno=%i %s\n",errno,strerror(errno));
	  return  AL_EC_FILE_ERROR;
	}
    }
  else
    {
      fclose(test);
      return  AL_EC_OK;
    }
  return AL_EC_NYI;
}
