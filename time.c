#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <syslog.h>
#include <sys/time.h>
#include <errno.h>
#include "recorder.h"

int check_local_path(char *rec_local_path, FileWritingDest	pDest)
{
	int res = 0;
	char cfg_path[LOCAL_PATH_LENGTH];
	long long size = 0;

	memset(cfg_path, 0, LOCAL_PATH_LENGTH);
    cfg_get_str("rec:local:path", cfg_path, 255);

	memset(rec_local_path, 0, LOCAL_PATH_LENGTH);
	
	res = path_check(cfg_path); // To do  
	if(res == FW_UNKNOWN_LOCAL_STORAGE)
	{
		if(pDest == kFtpFileWritingDest || pDest == kEmailFileWritingDest)
		{
			sprintf(rec_local_path,"/tmp");
			//LOGW("check_local_path: rec_local_path is %s\n", rec_local_path);
		}
		else
		{
			return FW_UNKNOWN_LOCAL_STORAGE;
		}
	}
	else
	{
		sprintf(rec_local_path,"/mnt/%s", cfg_path);
		//LOGI("check_local_path: rec_local_path is %s \n", rec_local_path);
	}

	res = dir_check(rec_local_path);
	if(res == FW_FAIL)
	{
		if(pDest == kFtpFileWritingDest || pDest == kEmailFileWritingDest)
		{
			memset(rec_local_path, 0, LOCAL_PATH_LENGTH);
			sprintf(rec_local_path,"/tmp");
		}
		else
		{
			return 1;
		}
	}

	size = get_diskfree_space(rec_local_path); 
	if (size < MIN_SIZE_DISK_SPACE) //!!
	{
		LOGW("Memory space not enough\n");
		return FW_INSUFFICIENT_SPACE;
	}
	return 0;
}

int getfilename(char *pnfile, pthread_t id, FileWritingDest	pDest)
{
    struct tm *tmptm = NULL;
	int res = 0;
    char rec_local_path[LOCAL_PATH_LENGTH];
	time_t temptime;

    if(pnfile == NULL)
    {
    	return 1;
    }

    (void)time(&temptime);
    tmptm = localtime(&temptime);

	res = check_local_path(rec_local_path, pDest);
	if(res)
	{
		LOGE("check_local_path - %s\n", rec_local_path);
		return res;
	}

	sprintf(pnfile,"%s/%04d%02d%02d%02d%02d%02d_0x%x", rec_local_path, tmptm->tm_year + 1900, tmptm->tm_mon + 1, tmptm->tm_mday, tmptm->tm_hour, tmptm->tm_min, tmptm->tm_sec, (int)id);

	//LOGI("rec_local_path - %s\n", rec_local_path);
	//LOGI("Ok!!! name file is %s, %d\n", pnfile, pDest);

	return FW_OK;
}





