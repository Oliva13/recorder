#ifndef _AVI_H_
#define _AVI_H_

#include "sembuf.h"

struct AVIIndexRecord
{
	struct AVIIndexRecord *fNext;
	unsigned fChunkId;
	unsigned fFlags;
	unsigned fOffset;
	unsigned fSize;
} *newIndexRecord;

void addIndexRecord(struct AVIIndexRecord *newIndexRecord); 
int completeOutputFile(void);
int useFrame(avi_frame_t *buffer);

#endif

