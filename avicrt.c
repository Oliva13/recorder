#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include "recorder.h"

void addIndexRecord(struct AVIIndexRecord *newIndexRecord) 
{
	avi_param_t *p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param);
	
	if (p_avi->fIndexRecordsHead == NULL) 
	{
		p_avi->fIndexRecordsHead = newIndexRecord;
	} 
	else 
	{
		p_avi->fIndexRecordsTail->fNext = newIndexRecord;
	}
	p_avi->fIndexRecordsTail = newIndexRecord;
	p_avi->fIndexRecordsTail->fNext = NULL;
	p_avi->fNumIndexRecords++; //check!!!
}

int useFrame(avi_frame_t *buffer)
{
	struct AVIIndexRecord *newIndexRecord = NULL;
	int testIndex = 0;
	unsigned char *frameSource = buffer->data;
	unsigned frameSize = buffer->size;

	avi_param_t *p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param);

	if (buffer->type == AVI_AUDIO_FRAME) 
	{
		if (p_avi->fPrevAudioPts)
		{
			int uSecondsDiff = buffer->pts - p_avi->fPrevAudioPts;
			unsigned bytePerSeconds = buffer->size * 1000000.0 / uSecondsDiff;
			if (bytePerSeconds > p_avi->fAudioMaxBytesPerSecond)
			{
				p_avi->fAudioMaxBytesPerSecond = bytePerSeconds;
			}
		}
		p_avi->fPrevAudioPts = buffer->pts;
	}	
	else
	{
		if (p_avi->fPrevVideoPts)
		{
			int uSecondsDiff = buffer->pts - p_avi->fPrevVideoPts;
			unsigned bytePerSeconds = buffer->size * 1000000.0 / uSecondsDiff;
			if (bytePerSeconds > p_avi->fVideoMaxBytesPerSecond)
			{
            	p_avi->fVideoMaxBytesPerSecond = bytePerSeconds;
			}
		}
		p_avi->fPrevVideoPts = buffer->pts;
	}
	// Add an index record for this frame:
	newIndexRecord = malloc(sizeof(struct AVIIndexRecord));
	if(buffer->type == AVI_AUDIO_FRAME)
	{
		p_avi->fAVISubsessionTag = fourChar('0', '1', 'w', 'b');
	}
	else
	{
		p_avi->fAVISubsessionTag = fourChar('0', '0', 'd', 'c');
	}
	newIndexRecord->fChunkId = p_avi->fAVISubsessionTag;
	testIndex = (buffer->type == AVI_AUDIO_FRAME ? 0 : 4);
	newIndexRecord->fFlags = frameSource[testIndex] == 0x67 ? 0x10 : 0;
	newIndexRecord->fOffset = 4 + p_avi->fNumBytesWritten;
	newIndexRecord->fSize = frameSize; 

	addIndexRecord(newIndexRecord);
	
	// Write the data into the file:
	p_avi->fNumBytesWritten += addWord(p_avi->fAVISubsessionTag);
	p_avi->fNumBytesWritten += addWord(frameSize);

	fwrite(frameSource, 1, frameSize, p_avi->fOutFid);
	p_avi->fNumBytesWritten += frameSize;
	// Pad to an even length:
	if (frameSize%2 != 0) 
	{
		p_avi->fNumBytesWritten += addByte(0);
	}
	
	if(buffer->type == AVI_AUDIO_FRAME)
	{
		p_avi->fNumAudioFrames++;
	}
	else
	{
		p_avi->fNumVideoFrames++;
	}
	return FW_OK;
}

int completeOutputFile(void) 
{
	unsigned maxBytesPerSecond = 0;
	struct AVIIndexRecord *indexRecord = NULL;
	struct AVIIndexRecord *qRecord = NULL;
	int recIndex = 0;
	
	avi_param_t *p_avi = (avi_param_t *)pthread_getspecific(saved_avi_param); // to do search p_avi

	maxBytesPerSecond += p_avi->fAudioMaxBytesPerSecond + p_avi->fVideoMaxBytesPerSecond; 
	setWord(p_avi->fVideoSTRHFrameCountPosition, p_avi->fNumVideoFrames);
	if(p_avi->fNumAudioFrames)
	{
		 setWord(p_avi->fAudioSTRHFrameCountPosition, p_avi->fNumAudioFrames);
	}
	//// Global fields:
	add4ByteString("idx1");
	addWord(p_avi->fNumIndexRecords*4*4); // the size of all of the index records, which come next:
	for (indexRecord = p_avi->fIndexRecordsHead; indexRecord != NULL; indexRecord = qRecord) 
	{
		addWord(indexRecord->fChunkId);
		addWord(indexRecord->fFlags);
		addWord(indexRecord->fOffset);
		addWord(indexRecord->fSize);
		qRecord = indexRecord->fNext;
		deallocate((void *)&indexRecord);
		recIndex = 1;
	}
	p_avi->fRIFFSizeValue += (p_avi->fNumBytesWritten + (p_avi->fNumIndexRecords*4*4));
	setWord(p_avi->fRIFFSizePosition, p_avi->fRIFFSizeValue);
	setWord(p_avi->fAVIHMaxBytesPerSecondPosition, maxBytesPerSecond);
	setWord(p_avi->fAVIHFrameCountPosition, p_avi->fNumVideoFrames > 0 ? p_avi->fNumVideoFrames : p_avi->fNumAudioFrames);
	p_avi->fMoviSizeValue += p_avi->fNumBytesWritten;
	setWord(p_avi->fMoviSizePosition,  p_avi->fMoviSizeValue);
	return recIndex;
}
