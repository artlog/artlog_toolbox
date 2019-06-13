#include "alinput_file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

// friend of alinput.c

int alinput_file_close(struct alinputstream * input)
{
  if ( input->type == ALINPUTSTREAM_TYPE_FD )
    {
      if ( input->fd >=0 )
	{
	  close(input->fd);
	  input->fd=-1;
	}
    }
  // else todo
  return 0;
}

enum al_global_error_code  alinput_file_open_init(struct alinputstream * input, const char * filename)
{
  if (input != NULL )
    {
      int fd = open(filename,O_RDONLY);  
      if ( fd >= 0)
	{
	  alinputstream_init(input,fd);
	  alinputstream_set_close_callback(input,alinput_file_close,NULL);
	  return AL_EC_OK;
	}
      else
	{
	  return AL_EC_FILE_ERROR;
	}
    }
  return AL_EC_INVALID_PARAMETER;
}

enum al_global_error_code  alinput_file_dir_filename_open_init(struct alinputstream * input,const char * dir, const char * filename)
{  
  // HARDCODED LIMIT
  char fullpath[4096];
  snprintf(fullpath,sizeof(fullpath),"%s/%s",dir,filename);
  return alinput_file_open_init(input,fullpath);
}
