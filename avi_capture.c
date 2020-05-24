#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/time.h>
#include <errno.h>
#include <dirent.h>
#include <stddef.h>
#include <libgen.h>
#include "recorder.h"

static unsigned long get_uptime(void)
{
	struct timespec tm;
	if (clock_gettime(CLOCK_MONOTONIC, &tm) != 0)
		return 0;
	return tm.tv_sec;
}

/*
int check_stream_avi(avi_status_t *status)
{
	if(status->type != 'I' || status->type != 'P')
	{
		return FW_FAIL;
	}
	return FW_OK;
}
*/

int get_frame_num_audio(int audio_idx, int64_t video_pts)
{
	int ret = 0;
	int i, number_frame;
	avi_status_t *p_audio;

	ret = avi_dir_stream(audio_idx, &p_audio);

	if(ret == 0 || p_audio == NULL)
	{
		return FW_FAIL;
	}
	//LOGW("find audio search_time - %lld", video_pts);
	for (i = 0; i < ret; i++)
	{
		if(video_pts > p_audio->pts)
		{
			break;
		}
		p_audio++;
	}

    LOGW("End audio %c %d %lld", p_audio->type, p_audio->number, p_audio->pts);

	number_frame = p_audio->number;

	if(p_audio)
	{
		deallocate((void *)&p_audio);
	}

	return number_frame;
}

int get_frame_num_video(int idx, int time_ms, avi_status_t *st_result, avi_status_t *max_result)
{
	int ret = 0;
	int i;
	int first_find_I = 0;
	avi_status_t *status;
	ret = avi_dir_stream(idx, &status);
	if(ret == 0 || status == NULL)
	{
		return FW_FAIL;
	}
	//ret = check_stream_avi(status);
	//if(ret == FW_FAIL)
	//{
	//	return FW_UNSUPPORTED_FILE_FORMAT;
	//}
	avi_status_t *prev_last_I, *prev_first_I;
	avi_status_t *curr_I = malloc(sizeof(avi_status_t));
	avi_status_t *p = status;

	int64_t search_time = p->pts - time_ms * 1000;

	//LOGW("find search_time - %lld", search_time);

	for (i = 0; i < ret; i++)
	{
		if(search_time < p->pts)
		{
			if(p->type == 'I')
			{
				memcpy(curr_I, p, sizeof(avi_status_t));
				prev_last_I = curr_I;
				if(!first_find_I)
				{
					prev_first_I = curr_I;
					//LOGW("find prevI end %c %d %lld", prev_first_I->type, prev_first_I->number, prev_first_I->pts);
					first_find_I = 1;
					memcpy(max_result, prev_first_I, sizeof(avi_status_t));
					//LOGW("max_result1 - %lld", max_result->pts);
				}
				//LOGW("find prev_last_I end %c %d %lld", prev_last_I->type, prev_last_I->number, prev_last_I->pts);
			}
		}
		else
		{
			if(p->type == 'I')
			{
				//LOGW("find I break %c %d %lld", p->type, p->number, p->pts);
				prev_last_I = p;
				memcpy(max_result, prev_last_I, sizeof(avi_status_t));
				//LOGW("max_result2 - %lld", max_result->pts);
				break;
			}
		}
		p++;
	}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//LOGW("End %c %d %lld", prev_last_I->type, prev_last_I->number, prev_last_I->pts);
	memcpy(st_result, prev_last_I, sizeof(avi_status_t)); // find prev_last_I
	free(status);
	free(curr_I);
	return FW_OK;
}

int init_audio(avi_param_t *p_avi)
{
	int v = 0;
	int audio_source = 0;
	
	if (cfg_get_int("io:audio_sources", &audio_source))
	{
		return 1;
	}
	
	if(audio_source)
	{	
		if (0 == cfg_get_int("audio:enabled", &v))
		{
			p_avi->audio_is_enable = v;
			LOGE("audio:enabled - %d", p_avi->audio_is_enable);
		}
		else
		{
			return 1;
		}
	
		if(p_avi->audio_is_enable)
		{
			if (0 == cfg_get_int("audio:encoder", &v))
			{
				switch (v)
				{
					case 0: v = 0x0001; break;
					case 1: v = 0x0007; break;
					case 2: v = 0x0045; break;
					case 3: v = 0x0002; break;
				}
				p_avi->audio_encoder = v;
				LOGE("audio:encoder - 0x%x", p_avi->audio_encoder);
			}
			else
			{
				return 1;
			}
	
			if (0 == cfg_get_int("audio:samplerate", &v))
			{
				switch (v)
				{
					case 0: v = 8000; break;
					case 1: v = 11025; break;
					case 2: v = 16000; break;
					case 3: v = 22050; break;
					case 4: v = 32000; break;
					case 5: v = 44100; break;
				}
				p_avi->audio_samplerate = v;
				LOGE("audio:samplerate - %d", p_avi->audio_samplerate);
			}
			else
			{
				return 1;
			}
		}
	}
	return 0;
}

void* RecordActionProc(void *arg)
{
    FW_RESULT_TYPE nResult = FW_OK;

	int lDurationBefore, lDurationAfter;
    int64_t lDurationAfter_pts;
    int lDuration = 0, lDuration_next = 0;

	FileWritingDest	pthrfwDest;

	avi_status_t *st_result = NULL, *max_result = NULL;

	int num_video_frame = 0;
	int num_stream = 0;
	unsigned long start, now;
	int res = 0;
	rec_video_t rec_param_video;
	char *lpFileName = NULL;
	char param[64], num[2];
   	avi_param_t *p_avi = NULL;
	struct thread_args *p = (struct thread_args *)arg;

	int vfNumber = 0;
	int vStream = 0;
	avi_frame_t *vFrame = NULL;
	int vNoData = 0;
	int afNumber = 0;
	int aStream = 0;
	avi_frame_t *aFrame = NULL;
	int aNoData = 0;
	int allsize = 0;
	int unit_size = 0;
	char start_code[4] = {0x0, 0x0, 0x0, 0x1};
	long long get_disk_size = 0;
	char stream_name[64];
	
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

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    lDurationBefore = p->nDurationBefore; //mc
    lDurationAfter = p->nDurationAfter; //mc
    num_stream = p->nStream;
	pthrfwDest = p->fwDest;

	nResult = check_stream(p->nStream);
	if(nResult == FW_FAIL)
	{
		nResult = FW_DISABLED_STREAM;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}
	
	check_stream_name(p->nStream, stream_name);

	//LOGI("check_stream_name avi!!! - %s", stream_name);

	if((kAviFileWritingFormat == p->fwFormat) && (strcmp("H.264", stream_name) != 0))
	{
		if(strcmp("H.265", stream_name) != 0)
		{
			nResult = FW_UNSUPPORTED_FILE_FORMAT;
			p->callback(p, p->hUserData, nResult, NULL, 0);
			goto exit;
		}
		//else
		//{
		//	p_avi->stream_name = 0xaa;
		//}
	}
	//else
	//{
	//	p_avi->stream_name = 0x55;
	//}
		
	lpFileName = malloc(LOCAL_PATH_LENGTH);
	if(lpFileName == NULL)
	{
		LOGE("Error: lpFileName == NULL");
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}

  	p_avi = (avi_param_t *)malloc(sizeof(avi_param_t));
	if(p_avi == NULL)
	{
		LOGE("Error: p_avi == NULL");
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}

    //LOGI("RecordActionProc");

	pthread_setspecific(saved_avi_param, (void *)p_avi);

    st_result = malloc(sizeof(avi_status_t));
	if(st_result == NULL)
	{
		LOGE("Error: malloc st_result");
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}
    max_result = malloc(sizeof(avi_status_t));
	if(max_result == NULL)
	{
		LOGE("Error: malloc max_result");
		deallocate((void *)&st_result);
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}

	//LOGW("lDurationBefore - %d", lDurationBefore);

    res = get_frame_num_video(num_stream, lDurationBefore, st_result, max_result);
    if(res)
    {
		LOGE("Error: get_frame_num_video");
		deallocate((void *)&st_result);
		deallocate((void *)&max_result);
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
    }

	aStream = CHNL_AVI_G711;
	afNumber = get_frame_num_audio(aStream, st_result->pts);
	if(afNumber == FW_FAIL)
    {
		LOGE("Error: get_frame_num_audio");
		deallocate((void *)&st_result);
		deallocate((void *)&max_result);
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
    }

	//LOGW("num_vido_frame  numframe - %d, pts - %lld", st_result->number, st_result->pts);
	//LOGE("======================================================================================================================");
	//system("ls /tvh/av/0");
	//LOGE("======================================================================================================================");
	//system("ls /tvh/av/3");
	//LOGE("======================================================================================================================");
	//LOGW("num_audio_frame - %d", afNumber);

	lDurationAfter_pts = (max_result->pts + (lDurationAfter * 1000));
	//LOGW("lDurationAfter - %d", lDurationAfter);
	//LOGW("lDurationAfter_pts - %lld", lDurationAfter_pts);
	num_video_frame = st_result->number;
	//LOGW("num_video_frame - %d", num_video_frame);

	vfNumber = num_video_frame;
	vStream = num_stream;
	int64_t save_prev_pts_i = 0;
	p->id_pthread = pthread_self();
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//add check_disk_space
	if((nResult = getfilename(lpFileName, p->id_pthread, pthrfwDest)))
	{
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}
	strcat(lpFileName, ".avi");
	memset(p_avi, 0, sizeof(avi_param_t));
	
	if(strcmp("H.264", stream_name) != 0)
	{
		if(!strcmp("H.265", stream_name))
		{
			p_avi->stream_name = 0xaa;
		}
	}
	else
	{
		p_avi->stream_name = 0x55;
	}
	
	p_avi->fOutFid = OpenOutputFile(lpFileName);
	if (p_avi->fOutFid == NULL)
	{
		LOGE("fOutFid == NULL");
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}

	p_avi->fNumSubsessions = 2;

	sprintf(num, "%d", num_stream);

	res = cfg_get_float(STREAM("framerate"), &rec_param_video.framerate);
	if(res != 0) 
	{
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}
	
	res = cfg_get_int(STREAM("width"),  &rec_param_video.width);
	if(res != 0) 
	{
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}
	
	res = cfg_get_int(STREAM("height"), &rec_param_video.height);
	if(res != 0) 
	{
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}

	//LOGE("rec_param_video.framerate - %f", rec_param_video.framerate);
	//LOGE("rec_param_video.width - %d", rec_param_video.width);
	//LOGE("rec_param_video.height - %d", rec_param_video.height);
	
	p_avi->fMovieFPS = rec_param_video.framerate;
	p_avi->fMovieWidth = rec_param_video.width;
	p_avi->fMovieHeight = rec_param_video.height;

	res = init_audio(p_avi);
	if(res)
	{
		nResult = FW_FAIL;
		p->callback(p, p->hUserData, nResult, NULL, 0);
		goto exit;
	}
	
	addFileHeader_AVI();

	start = get_uptime();
	
	vFrame = (avi_frame_t *)malloc(sizeof(avi_frame_t));
	aFrame = (avi_frame_t *)malloc(sizeof(avi_frame_t));
	
	memset(vFrame, 0, sizeof(avi_frame_t));
	memset(aFrame, 0, sizeof(avi_frame_t));
	
	//LOGE("1");	
	while(1)
	{
		//LOGE("777");
		res = avi_get_frame(vStream, vfNumber, vFrame);
		//LOGE("11");	
		if(res < 0 && res != -ENODATA)
		{
			LOGE("1.Failed to get video frame (%d)", res);
			nResult = FW_FAIL;
			//freeSubsessionBuffer(vFrame);
			//freeSubsessionBuffer(aFrame);
			res = completeOutputFile();
			CloseInputFile(p_avi->fOutFid);
			//LOGE("4");
			if(!res)
			{
				unlink(lpFileName);
			}
			p->callback(p, p->hUserData, nResult, res ? lpFileName : NULL, lDuration);
			goto exit;
		}
		else
		{
			if(res != -ENODATA)
			{
				//LOGE("5");
				start = get_uptime();
				unsigned char *data = vFrame->data;
				allsize = vFrame->size;
				data = vFrame->data;
				while(allsize)
				{
					memcpy(&unit_size, data, 4);
					memcpy(data, start_code, 4);
					data += (unit_size + 4); // next_unit
					allsize -= (unit_size + 4);
				}
				if(vFrame->type == 'I')
				{
					char *dname;
					char path[LOCAL_PATH_LENGTH];
					strcpy(path, lpFileName);
					dname = dirname(path);
					//LOGW("dirname - %s\n", dname);
					get_disk_size = get_diskfree_space(dname);
					if(get_disk_size < MIN_SIZE_DISK_SPACE)
					{						
						LOGW("Memory space not enough - %lld\n", get_disk_size);
						nResult = FW_INSUFFICIENT_SPACE;
						//freeSubsessionBuffer(vFrame);
						//freeSubsessionBuffer(aFrame);
						res = completeOutputFile();
						CloseInputFile(p_avi->fOutFid);
						if(!res)
						{
							unlink(lpFileName);
						}
						p->callback(p, p->hUserData, nResult, res ? lpFileName : NULL, lDuration);
						goto exit;
					}
				}
				if(vFrame->pts > lDurationAfter_pts && vFrame->type == 'I')
				{
					//LOGE("4");
					save_prev_pts_i = vFrame->pts;
					completeOutputFile();
					CloseInputFile(p_avi->fOutFid);
					lDuration_next = p->callback(p, p->hUserData, nResult, lpFileName, lDuration);
					if(lDuration_next == 0)
					{
						LOGI("lDuration_next == 0\n");
						//freeSubsessionBuffer(vFrame);
						//freeSubsessionBuffer(aFrame);
						goto exit;
					}
					pthread_mutex_lock(&p->thread_flag_mutex);
					if(p->thread_flag)
					{
						LOGW("1.avi pthdata->thread_flag - %d", p->thread_flag);
						pthread_mutex_unlock(&p->thread_flag_mutex);
						LOGW("1.avi main break!!!");
						//freeSubsessionBuffer(vFrame);
						//freeSubsessionBuffer(aFrame);
						goto exit;
					}
					pthread_mutex_unlock(&p->thread_flag_mutex);
					lDurationAfter_pts = (save_prev_pts_i + (lDuration_next * 1000));
					if((nResult = getfilename(lpFileName, p->id_pthread, pthrfwDest)))
					{
						LOGE("Error: getfilename");
						p->callback(p, p->hUserData, nResult, NULL, 0);
						//freeSubsessionBuffer(vFrame);
						//freeSubsessionBuffer(aFrame);
						goto exit;
					}
					strcat(lpFileName, ".avi");
					memset(p_avi, 0, sizeof(avi_param_t));
					p_avi->fOutFid = OpenOutputFile(lpFileName);
					if (p_avi->fOutFid == NULL)
					{
						LOGE("fOutFid == NULL");
						nResult = FW_FAIL;
						p->callback(p, p->hUserData, nResult, NULL, 0);
						//freeSubsessionBuffer(vFrame);
						//freeSubsessionBuffer(aFrame);
						goto exit;
					}
					
					if(strcmp("H.264", stream_name) != 0)
					{
						p_avi->stream_name = 0xaa;
					}
					else
					{
						p_avi->stream_name = 0x55;
					}
										
					p_avi->fNumSubsessions = 2;
					p_avi->fMovieFPS = rec_param_video.framerate;
					p_avi->fMovieWidth = rec_param_video.width;
					p_avi->fMovieHeight = rec_param_video.height;
					
					res = init_audio(p_avi);
					if(res)
					{
						nResult = FW_FAIL;
						p->callback(p, p->hUserData, nResult, NULL, 0);
						//freeSubsessionBuffer(vFrame);
						//freeSubsessionBuffer(aFrame);
						goto exit;
					}			
					addFileHeader_AVI();
					//LOGE("6");
				}
				pthread_mutex_lock(&p->thread_flag_mutex);
				if(p->thread_flag)
				{
					//LOGW("2.avi pthdata->thread_flag - %d", p->thread_flag);
					pthread_mutex_unlock(&p->thread_flag_mutex);
					useFrame(vFrame);
					//LOGW("2.avi main break!!!");
					//freeSubsessionBuffer(vFrame);
					break;
				}
				pthread_mutex_unlock(&p->thread_flag_mutex);
				//LOGE("7");
				useFrame(vFrame);
				freeSubsessionBuffer(vFrame);
				vfNumber++;
				vNoData = 0;
			}
			else
			{
				//LOGE("14 %d", i++);
				freeSubsessionBuffer(vFrame);
				vNoData = 1;
				//LOGE("888");
			}
		}

 	 if(p_avi->audio_is_enable)
 	 {	
		res = avi_get_frame(aStream, afNumber, aFrame);
		if(res < 0 && res != -ENODATA)
		{
			LOGE("1.Failed to get video frame (%d)", res);
			nResult = FW_FAIL;
			//freeSubsessionBuffer(aFrame);
			//freeSubsessionBuffer(vFrame);
			res = completeOutputFile();
			CloseInputFile(p_avi->fOutFid);
			if(!res)
			{
				unlink(lpFileName);
			}
			p->callback(p, p->hUserData, nResult, res ? lpFileName : NULL, lDuration);
			goto exit;
		}
		else
		{
			if(res != -ENODATA)
			{
				pthread_mutex_lock(&p->thread_flag_mutex);
				if(p->thread_flag)
				{
					LOGW("3.avi pthdata->thread_flag - %d", p->thread_flag);
					pthread_mutex_unlock(&p->thread_flag_mutex);
					useFrame(aFrame);
					LOGW("3.avi main break!!!");
					//freeSubsessionBuffer(aFrame);
					break;
				}
		//		LOGE("8");
				pthread_mutex_unlock(&p->thread_flag_mutex);
				useFrame(aFrame);
				freeSubsessionBuffer(aFrame);
				afNumber++;
				aNoData = 0;
		//		LOGE("9");
			}
			else
			{
				freeSubsessionBuffer(aFrame);
				aNoData = 1;
			}
		}
	}
	else
	{
		aNoData = 1;
	}
	
	if(aNoData && vNoData)
	{
		  now = get_uptime() - start;
	//	LOGE("12");
		  if(now >= 2)
	   	  {
			LOGE("Timeout - %ld", now);
			nResult = FW_DISABLED_STREAM;
			res = completeOutputFile();
			CloseInputFile(p_avi->fOutFid);
			if(!res)
			{
				unlink(lpFileName);
			}
			p->callback(p, p->hUserData, nResult, res ? lpFileName : NULL, lDuration);
			//freeSubsessionBuffer(vFrame);
			//freeSubsessionBuffer(aFrame);
			goto exit;
		  }
	   }
	   //LOGE("10");
	   usleep(5000);
	}
	//LOGW("exit avi pthdata pthdata->thread_flag - %d", p->thread_flag);
	completeOutputFile();
	CloseInputFile(p_avi->fOutFid);
	p->callback(p, p->hUserData, nResult, lpFileName, lDuration);

exit:
		
	if(vFrame != NULL)
		deallocate((void *)&vFrame);
	if(aFrame != NULL)
		deallocate((void *)&aFrame);
	if(st_result != NULL)
		deallocate((void *)&st_result);
	if(max_result != NULL)
		deallocate((void *)&max_result);
	if(lpFileName != NULL)
		deallocate((void *)&lpFileName);
	if(p_avi != NULL)
		deallocate((void *)&p_avi);
	return NULL;
}
