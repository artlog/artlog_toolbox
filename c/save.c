#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "save.h"

int save_debug=0;

int save_set_debug(int debug)
{
  int previous_debug=save_debug;
  save_debug=debug;
  return previous_debug;
}

int save_file_exists(char * template)
{
  FILE * test=fopen(template,"r");
  if ( test == NULL )
    {
      if (errno == ENOENT) {
	if (save_debug > 0) {printf("file %s doesn't exist\n", template);}
	return -1;
      }
      else if ( errno == EINVAL )
	{
	  fprintf(stderr,"weird invalid argument, contact developper (in function %s:%i)\n",__func__,__LINE__);
	  return -2;
	}
      else
	{
	  printf("File exists but can't be openned errno=%i %s\n",errno,strerror(errno));
	  return 1;
	}
    }
  else
    {
      if (save_debug>0) {printf("file %s already exists\n", template);}
      fclose(test);
      return 0;
    }
  return -3;
}

int save_init_context(struct savecontext * context, char * dir, char* prefix, char * extension)
{
  strncpy(context->dir,dir,sizeof(context->dir));
  strncpy(context->prefix,prefix,sizeof(context->prefix));
  strncpy(context->extension,extension,sizeof(context->extension));
  context->lastfile[0]=0;
  context->maxsave=100;
  context->index_min=context->maxsave;
  context->index_max=0;

  return 0;
}

/**
 * every file with a prefix prefix.[0..9]+.extension will be rename with a superior index if room is needed.
 * what about prefix.extension ( ie without 0 between prefix and extension ) ?
 */
int save_shift_file_name(struct savecontext * savecontext)
{
  char fullpath[4096];
  DIR *currentdir;
  struct dirent *fileentry;
  // biggest index acceptable.
  int maxsave = savecontext->maxsave;

  char * prefix = savecontext->prefix;
  char * extension = savecontext->extension;

  printf("search for freefile %s.%s within %s\n", prefix, extension, savecontext->dir);
  currentdir = opendir (savecontext->dir);
  if ( currentdir != NULL )
    {
      char template[512];
      char * freefile = NULL;

      // don't go over 500 characters for filename, protect template allocation, well above 256 chars of filename.
      if (strlen(prefix)+strlen(extension) > 500)
	{
	  return -1;
	}
      
      // don't let prefix or extension with % , simple protection against template messup
      if (strchr(prefix,'%')) {
	return -1;
      }
      if (strchr(extension,'%')) {
	return -1;
      }

      sprintf(template,"%s.%s",prefix,extension);
      {
	sprintf(fullpath,"%s/%s",savecontext->dir,template);
	// file exists ?
	if ( save_file_exists(fullpath) == -1)
	  {
	    // file does not exist : GOOD, free file is found !
	    freefile=template;
	  }	
      }

      // need to move file to have a place.
      if ( freefile == NULL )
	{
	  char oldpath[256];
	  char newpath[256];
	  int dir_fd=dirfd(currentdir);
	  int count = 0; // number of entries matching template found
	  int index_max = 0;
	  int index_min = maxsave;
	  int freeindex=0;
	  
	  // prefix.[0..9]+.extension
	  sprintf(template,"%s.%%i.%s",prefix,extension);
	  while ( (fileentry = readdir(currentdir)) != NULL )
	    {
	      int index;
	      if (sscanf(fileentry->d_name,template,&index) == 1)
		{
		  if ( save_debug >0 ) {
		    printf("FOUND index %i in\n",index);
		    printf("%s %i==",template, index);
		    printf(template, index);
		    printf("==%s\n", fileentry->d_name);
		  }
		  sprintf(newpath,template,index);
		  // protect against sscanf early match ( seems to disregards extension )
		  // ex: "non matching sscanf selection cube.9.record != cube.9.cubemodel"
		  if ( strcmp(newpath,fileentry->d_name) != 0 )
		    {
		      if ( save_debug > 0 )
			{
			  printf("non matching sscanf selection %s != %s\n", newpath,fileentry->d_name);
			}
		      continue;
		    }

		  if ( index > index_max )
		    {
		      index_max = index;
		    }
		  if ( index < index_min )
		    {
		      index_min = index;
		    }
		  if ( index > maxsave )
		    {
		      printf("index %i > %i should be removed\n",index,maxsave);
		    }
		  count ++;
		}
	    }
	  freeindex=index_max+1;
	  if ( save_debug > 1 )
	    {
	      if ( count < (index_max-index_min+1) )
		{
		  printf("matching files count %i => not all index are used between %i and %i\n", count, index_min, index_max);
		}
	      else if ( count > (index_max-index_min+1) )
		{
		  printf("matching files count %i => index reused between %i and %i\n", count, index_min, index_max);
		}
	    }
	  if ( index_min < 2 )
	    {
	      
	      // .0 or .1 are used ... too bad...
	      sprintf(template,"%s.%i.%s", prefix,(index_max+1),extension);
	      freefile = template;

	      // move all index up ...
	      
	      // find first hole if any
	      if ( count < (index_max-index_min+1) )
		{
		  for (int i=1; i<index_max; i++)
		    {
		      sprintf(oldpath,"%s.%i.%s", prefix,i,extension);
		      sprintf(fullpath,"%s/%s",savecontext->dir,oldpath);
		      // file exists
		      if ( save_file_exists(fullpath) == -1 )
			{
			  freeindex = i;
			  break;
			}
		    }
		}

	      for (int i=freeindex-1; i>0; i--)
		{		
		  sprintf(oldpath,"%s.%i.%s", prefix,i,extension);		
		  sprintf(newpath,"%s.%i.%s", prefix,i+1,extension);
		  if (save_debug > 0) {printf("rename %s->%s\n", oldpath, newpath);}
		  if ( renameat(dir_fd, oldpath,
				dir_fd, newpath) != 0 )
		    {
		      fprintf(stderr,"error while renaming %s->%s  [%i] %s index min=%i\n",oldpath, newpath, errno, strerror(errno), index_min);
		      break;
		    }
		}
	    }
	  else
	    {
	      printf("index_min %i index_max %i\n",index_min,index_max);
	    }

	  // and finaly move away prefix.extension file.
	  sprintf(oldpath,"%s.%s", prefix,extension);
	  freefile = oldpath;
	  sprintf(newpath,"%s.%i.%s", prefix,1,extension);
	  if ( save_debug > 0 ) { printf("rename %s->%s\n", oldpath, newpath);}
	  if ( renameat(dir_fd, oldpath,
			dir_fd, newpath) != 0 )
	    {
	      fprintf(stderr,"error while renaming %s->%s  [%i] %s\n",oldpath, newpath, errno, strerror(errno));
	    }

	}

      if ( freefile != NULL )
	{
	  printf("freefile %s\n", freefile);	  
	}

      closedir (currentdir);

      return 0;
    }
  else
    {
      fprintf(stderr,"can't open current dir ./ [%i] \n", errno);
      return -1;
    }
}
