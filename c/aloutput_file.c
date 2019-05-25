#include "aloutput_file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// friend of aloutput.c

void aloutput_file_close(struct aloutputstream * output)
{
  if ( output->target == ALOUTPUT_TARGET_FD )
    {
      if ( output->fd >=0 )
	{
	  close(output->fd);
	  output->fd=-1;
	}
    }
}

enum al_global_error_code  aloutput_file_open_init(struct aloutputstream * output, const char * filename)
{
  if (output != NULL )
    {
      int fd = open(filename,O_WRONLY|O_CREAT);
      if ( fd >= 0)
	{
	  aloutputstream_fd_init(output,fd);
	  aloutputstream_set_close_callback(output,aloutput_file_close,NULL);
	  return AL_EC_OK;
	}
      else
	{
	  return AL_EC_FILE_ERROR;
	}
    }
  return AL_EC_INVALID_PARAMETER;
}
