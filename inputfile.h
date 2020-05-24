#ifndef _INPUT_FILE_H_
#define _INPUT_FILE_H_

#include <sys/types.h>
#include <stdio.h>
#include  "boolean.h"

FILE* OpenOutputFile(char const* fileName);
void CloseOutputFile(FILE* fid);
FILE* OpenInputFile(char const* fileName);
void CloseInputFile(FILE* fid);
Boolean FileIsSeekable(FILE *fid);
int64_t SeekFile64(FILE *fid, int64_t offset, int whence);
u_int64_t GetFileSize(char const* fileName, FILE* fid);
int64_t TellFile64(FILE *fid);

#endif
