#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>
#include <dirent.h>
#include "queue.h"
#include "inputfile.h"
#include "avi.h"
#include "avicrt.h"
#include "aviutils.h"
#include "sembuf.h"
#include "jpeg.h"
#include "sd.h"
#include "time.h"
#include "mylog.h"
#include "avi_capture.h"
#include "recorder.h"
#include "cfg.h"
#include "ixml.h"

extern record_t recLParams;
pthread_t cleanThread;

void initialize_flag(struct thread_args *ptr)
{
	pthread_mutex_init(&ptr->thread_flag_mutex, NULL);
	pthread_cond_init(&ptr->thread_flag_cv, NULL);
	ptr->thread_flag = 0;
}

FW_RESULT_TYPE InitializeFileWritingLibrary(void)
{
	record_t *pconf;
	int i = 0;
	
	printf("InitializeFileWritingLibrary\n");
	pthread_key_create(&saved_avi_param, (void *)NULL);

	subscribe_removeiffileold();
	subscribe_archiveduration();
	subscribe_removeifnospace();
	subscribe_reservedspace();
	
	for(i = 0; i < 4; i++)
	{
	  on_cfg_changed(i);
	}
	
	pconf = &recLParams;
	
	if (pthread_create(&cleanThread, NULL, timesheet, pconf))
    {
		LOGE("scheduleThread create fail\n");
		return FW_FAIL;
    }
    	
	return FW_OK;
}

FW_RESULT_TYPE UninitializeFileWritingLibrary(void)
{
	printf("UninitializeFileWritingLibrary\n");
	threadquit = 1; // exit pthread timesheet
	pthread_join(cleanThread, NULL);
	return FW_OK;
}

FW_RESULT_TYPE StartFileWriting(FileWritingDest		fwDest,
				int 								nStream,
				FileWritingFormat					fwFormat,
				int									nDurationBefore,
				int									nDurationAfter,
				FW_HANDLE_TYPE						hUserData,
				FW_CALLBACK_TYPE					callback,
				FW_HANDLE_TYPE						*phFileWritingId)
{
   
   
	   if(fwFormat == kAviFileWritingFormat)		   
	   {   
		   struct thread_args *avi_pthargs;
		   avi_pthargs = malloc(sizeof(struct thread_args));
		  
		   *phFileWritingId = (FW_HANDLE_TYPE)avi_pthargs;
		  
		   avi_pthargs->fwDest = fwDest;
		   avi_pthargs->nStream = nStream;
		   avi_pthargs->fwFormat = fwFormat;
		   avi_pthargs->nDurationBefore = nDurationBefore;
		   avi_pthargs->nDurationAfter = nDurationAfter;
		   avi_pthargs->hUserData = hUserData;
		   avi_pthargs->callback = callback;
		   
		   if (pthread_create(&avi_pthargs->threadRecord, NULL, &RecordActionProc, avi_pthargs))
		   {
				printf("Record createthread failed\n");
				return FW_FAIL;
		   }
  	       return FW_OK;
	   }
	   else
	   {
			if(fwFormat == kJpgFileWritingFormat)
			{
				struct thread_args *jpg_pthargs;
				jpg_pthargs = malloc(sizeof(struct thread_args));
			    *phFileWritingId = (FW_HANDLE_TYPE)jpg_pthargs;
		   	    jpg_pthargs->fwDest = fwDest;
				jpg_pthargs->nStream = nStream;
				jpg_pthargs->fwFormat = fwFormat;
				jpg_pthargs->nDurationBefore = nDurationBefore;
				jpg_pthargs->nDurationAfter = nDurationAfter;
				jpg_pthargs->hUserData = hUserData;
				jpg_pthargs->callback = callback;
				if (pthread_create(&jpg_pthargs->threadRecord, NULL, &RecordJpegActionProc, jpg_pthargs))
				{
					printf("Record createthread failed\n");
					return FW_FAIL;
				}
		        return FW_OK;
			}
     }
    
	 UninitializeFileWritingLibrary();
     return FW_FAIL;
};

FW_RESULT_TYPE StopFileWriting(FW_HANDLE_TYPE phFileWritingId)
{
	struct thread_args *ptr = (struct thread_args *)phFileWritingId;
	if(ptr != NULL)
	{
		printf("hFileWritingId - %p\n", ptr);
		ptr->thread_flag = 1;
			
		pthread_join(ptr->threadRecord, NULL);
		
		pthread_mutex_destroy(&ptr->thread_flag_mutex);
		pthread_cond_destroy(&ptr->thread_flag_cv);
		
		free(phFileWritingId);
		return FW_OK;
	}
	return FW_FAIL;
}
