#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include "recorder.h"

void freeSubsessionBuffer(avi_frame_t *pSubBuffer)
{
	//LOGE("free1 - %p", pSubBuffer);
	//if(!pSubBuffer)
	//{
	//	//LOGE("free11");
	//	return;
	//}
	//if(pSubBuffer->data) 
	//{
	//	LOGE("free2 - %p", pSubBuffer->data);
	//	deallocate((void *)&pSubBuffer->data);
	//}
	//LOGE("free4 - %p", pSubBuffer);
	//deallocate((void *)&pSubBuffer);
	////LOGE("free4");
	
	if (pSubBuffer) 
	{
		if (pSubBuffer->data) 
		{
			//LOGE("7878");
			free(pSubBuffer->data);
			//LOGE("7879");
		}
		memset(pSubBuffer, 0, sizeof(avi_frame_t));
	}
}


