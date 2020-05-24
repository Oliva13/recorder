#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include "recorder.h"

FILE* OpenOutputFile(char const* fileName) 
{
	FILE* fid;
	// Check for special case 'file names': "stdout" and "stderr"
	if (strcmp(fileName, "stdout") == 0) 
	{
		fid = stdout;
	} 
	else 
		if (strcmp(fileName, "stderr") == 0) 
		{
			fid = stderr;
		}
		else
		{
			fid = fopen(fileName, "wb");
		}

		if (fid == NULL) 
		{
			LOGE("unable to open file %s, errno - %d\n", fileName, errno);
		}
		return fid;
}

void CloseOutputFile(FILE *fid) 
{
	if (fid != NULL && fid != stdout && fid != stderr) 
		fclose(fid);
}

FILE* OpenInputFile(char const* fileName)
{
  FILE* fid;
  // Check for a special case file name: "stdin"
  if (strcmp(fileName, "stdin") == 0) 
  {
    fid = stdin;
  }
  else
  {
    fid = fopen(fileName, "rb");
    if (fid == NULL)
    {
      LOGE("unable to open file %s\n", fileName);
    }
  }
  return fid;
}

void deallocate(void **buf)
{
	free(*buf);
	*buf = NULL;
}

void CloseInputFile(FILE *fid)
{
  // Don't close 'stdin', in case we want to use it again later.
  if (fid != NULL && fid != stdin) 
  {
     fclose(fid);
  }
}

int64_t SeekFile64(FILE *fid, int64_t offset, int whence)
{
  if (fid == NULL) 
	return -1;
  clearerr(fid);
  fflush(fid);
  return fseeko(fid, (off_t)(offset), whence);
}

int64_t TellFile64(FILE *fid)
{
  if (fid == NULL) 
	return -1;
  clearerr(fid);
  fflush(fid);
  return ftello(fid);
}

Boolean FileIsSeekable(FILE *fid)
{
  if (SeekFile64(fid, 1, SEEK_CUR) < 0)
  {
    return False;
  }
  SeekFile64(fid, -1, SEEK_CUR); // seek back to where we were
  return True;
}

u_int64_t GetFileSize(char const* fileName, FILE* fid)
{
  u_int64_t fileSize = 0; // by default

  if (fid != stdin)
  {
      if (fid != NULL && SeekFile64(fid, 0, SEEK_END) >= 0)
      {
			fileSize = (u_int64_t)TellFile64(fid);
			if (fileSize == (u_int64_t)-1) 
				fileSize = 0; // TellFile64() failed
			SeekFile64(fid, 0, SEEK_SET);
      }
  }
  return fileSize;
}

