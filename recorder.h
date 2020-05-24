#ifndef _RECORDER_H_
#define _RECORDER_H_

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "avicrt.h"
#include "sembuf.h"
#include "aviutils.h"
#include "inputfile.h"
#include "boolean.h"
#include "avi.h"
#include "cfg.h"
#include "ixml.h"
#include "queue.h"
#include "FileWritingLibrary.h"
#include "jpeg.h"
#include "sd.h"
#include "time.h"
#include "mylog.h"
#include "avi_capture.h"
#include "smtp.h"
#include "ftp.h"
#include "timesheet.h"
#include "ipc.h"

#define NO_DATA_CMD		     "NO_DATA..."
#define CMD_LENGTH 	 		 11
#define LOCAL_PATH_LENGTH    256
#define MIN_SIZE_DISK_SPACE  8*1024 //Mb
#define FILE_NAME_LENGTH 	 64

struct thread_args
{
    FileWritingDest	    fwDest;
    int					nStream;
    FileWritingFormat   fwFormat;
    int					nDurationBefore;
    int					nDurationAfter;
    FW_HANDLE_TYPE		hUserData;
    FW_CALLBACK_TYPE	callback;
    pthread_cond_t		thread_flag_cv;
    pthread_mutex_t		thread_flag_mutex;
	pthread_t 			threadRecord;		
	int					thread_flag;
    int					id_pthread;
    FW_RESULT_TYPE 		nResult;
    const char			*lpFileName;
    int					nResultDuration;
    void				*data_pthread;
};

typedef struct
{
    float   framerate;
    int     fd;
    int     width;
    int     height;
    int     vfNumber;
    int     afNumber;
    int64_t durf_pts;
	struct  ipc_unit *pmq;

}rec_video_t;

#define STREAM(p) strcat(strcat(strcpy(param, "stream"), num), ":"p)
extern void initialize_flag(struct thread_args *ptr);
extern int threadquit;
void deallocate(void **buffer);

#endif
