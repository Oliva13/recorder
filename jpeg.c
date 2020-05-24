#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include "recorder.h"

int check_stream_jpg(avi_status_t *status)
{
	if(status->type != 'J')
	{
		return FW_FAIL;
	}
	return FW_OK;
}

int get_nframe_jpeg_before(int jpeg_idx, int time_ms, avi_status_t *p_jpeg)
{
	int ret = FW_OK;
	int i;
	avi_status_t *status;

	ret = avi_dir_stream(jpeg_idx, &status);
	if(ret == 0)
		return FW_FAIL;

	avi_status_t *p = status;

	ret = check_stream_jpg(p);
	if(ret == FW_FAIL)
	{
		return FW_UNSUPPORTED_FILE_FORMAT;
	}

	int64_t search_time = p->pts - (time_ms * 1000);

	LOGW("find jpeg search_time - %lld", search_time);
	for (i = 0; i < ret; i++)
	{
		if(search_time > p->pts)
		{
			break;
		}
		p++;
	}

	memcpy(p_jpeg, p, sizeof(avi_status_t));

	free(status);

	LOGW("End jpeg type - %c number - %d jpeg_pts - %lld", p_jpeg->type, p_jpeg->number, p_jpeg->pts);

	return ret;
}

int getJPGframe(int JpgNumStream, int JpgfNumber, avi_frame_t *JpgFrame)
{
	int wait_for_frame = 0;
	int ret = 0;

	while(1)
	{
		//memset(&JpgFrame, 0, sizeof(JpgFrame));
		ret = avi_get_frame(JpgNumStream, JpgfNumber, JpgFrame);
		if (ret == -ENODATA)
		{
			//LOGE("ENODATA\n");
			if (wait_for_frame > 800)
			{
				LOGE("get_video_frame: Failed wait_for_frame (%d)\n", ret);
				wait_for_frame = 0;
				return FW_FAIL;
			}
			wait_for_frame += 1;
			usleep(2500);
			continue;
		}
		if(ret < 0)
		{
			LOGE("Failed to get video frame (%d)", ret);
			return FW_FAIL;
		}
		else
		{
			wait_for_frame = 0;
			break;
		}
	}
	return FW_OK;
}

int write_jpeg_frame(avi_frame_t *JpgFrame, char *lpFileName, FileWritingDest pDest)
{
	//int jpg_header_found = 0;
	int mjpegsize = 0;
	int size = 0;
	unsigned char *currdata, *ptr;
	pthread_t id = 0;
	FILE* jpgOutFid;

	mjpegsize = JpgFrame->size;
	currdata = JpgFrame->data;

	id = pthread_self();
	//
	// To do check_disk_space
	//
	if(getfilename(lpFileName, id, pDest))
	{
		//LOGE("Error: getfilename");
		return FW_FAIL;
	}

	strcat(lpFileName, ".jpeg");
	jpgOutFid = OpenOutputFile(lpFileName);
	if (jpgOutFid == NULL)
	{
		LOGE("fOutFid == NULL");
		return FW_FILE_WRITE_ERROR;
	}

	ptr = currdata;

	while(ptr < currdata + mjpegsize)
	{
		size = *(int*)ptr;
		ptr += 4;
		fwrite(ptr, size, 1, jpgOutFid);
		ptr += size;
	}

	CloseOutputFile(jpgOutFid);
	return FW_OK;
}

//To do join for error phtread
void* RecordJpegActionProc(void *arg)
{
	FW_RESULT_TYPE nResult = FW_OK;

	struct thread_args *p = (struct thread_args *)arg;

	avi_status_t *st_result = NULL;
    char *lpFileName = NULL;

	avi_frame_t *JpgFrame = NULL;

	int lDurationBefore;
	int64_t IntervalAfter_pts;
    int lDuration, Interval_next;

    int num_jpeg_frame;
	FileWritingDest pthrfwDest;

	int ret = 0;
	int num_stream = 0;
    char stream_name[64];

    lDurationBefore = p->nDurationBefore;
    num_stream 		= p->nStream;
	pthrfwDest      = p->fwDest;
	lDuration = 0;
    Interval_next = 0;
    num_jpeg_frame = 0;
	
	while(ipc_wait("ENC") != 0) 
	{
		if(p->thread_flag)
		{
			nResult = FW_FAIL;
			p->callback(p, p->hUserData, nResult, NULL, lDuration);
			return NULL;
		}
	}
	
	initialize_flag(p);

	ret = check_stream(p->nStream);
	if(ret == FW_FAIL)
	{
		nResult = FW_DISABLED_STREAM;
		goto exit;
	}

	check_stream_name(p->nStream, stream_name);
	//LOGE("check_stream_name jpg!!! - %s", stream_name);

	if((kJpgFileWritingFormat == p->fwFormat) && (strcmp("MJPEG", stream_name) != 0))
	{
		nResult = FW_UNSUPPORTED_FILE_FORMAT;
		goto exit;
	}

	lpFileName = malloc(LOCAL_PATH_LENGTH);
	if(lpFileName == NULL)
	{
		LOGE("Error: lpFileName == NULL");
		nResult = FW_FAIL;
		goto exit;
	}

	st_result = malloc(sizeof(avi_status_t));  // To do check error

	// find first numFrm before
    nResult = get_nframe_jpeg_before(num_stream, lDurationBefore, st_result);
	if (nResult == FW_FAIL || nResult == FW_UNSUPPORTED_FILE_FORMAT)
	{
		free(st_result);
		goto exit;
	}

	//LOGW("get num_jpeg_frame numframe - %d, pts - %lld", st_result->number, st_result->pts);

    IntervalAfter_pts = st_result->pts;
	num_jpeg_frame = st_result->number;

    free(st_result);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int JpgfNumber = num_jpeg_frame;
	int JpgNumStream = num_stream;

    printf("========================================================================\n");

	JpgFrame = (avi_frame_t *)malloc(sizeof(avi_frame_t));
	memset(JpgFrame, 0, sizeof(avi_frame_t));
	
	while(1)
	{
		ret = getJPGframe(JpgNumStream, JpgfNumber, JpgFrame);
		if(ret < 0)
		{
			nResult = ret;
			free(JpgFrame);
			goto exit;
		}
	
		if(JpgFrame->pts >= IntervalAfter_pts)
		{
			//LOGW("DurationAfter_pts - %lld", IntervalAfter_pts);
		    ret = write_jpeg_frame(JpgFrame, lpFileName, pthrfwDest);
			if(ret)
			{
				//LOGW("Error: FW_DISABLED_STREAM2");
				nResult = ret;
				freeSubsessionBuffer(JpgFrame);
				goto exit;
			}

			Interval_next = p->callback(p, p->hUserData, nResult, lpFileName, lDuration);
			if(!Interval_next)
			{
				freeSubsessionBuffer(JpgFrame);
				goto ok;
			}
			//printf("nDuration_next - %d\n", Interval_next);
			IntervalAfter_pts = (IntervalAfter_pts + (Interval_next * 1000));
			//printf("IntervalAfter_pts - %lld\n", IntervalAfter_pts);
		}
		freeSubsessionBuffer(JpgFrame);
		JpgfNumber++;
		pthread_mutex_lock (&p->thread_flag_mutex);
		if(p->thread_flag)
		{
			LOGW("pthdata cond_signal - %d", p->thread_flag);
			pthread_cond_signal(&p->thread_flag_cv);
			pthread_mutex_unlock(&p->thread_flag_mutex);
			LOGW("main break!!!");
			goto ok;
		}
		pthread_mutex_unlock(&p->thread_flag_mutex);
	}

exit:
	p->callback(p, p->hUserData, nResult, NULL, 0);
ok:
	if(lpFileName != NULL)
		deallocate((void *)&lpFileName);
	return NULL;
}

