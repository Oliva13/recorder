#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <sys/wait.h>
#include "recorder.h"

typedef struct
{
	char **files;
	int  cnt_files;
	
}t_ftp_args;

static void cleanupHandler(void *args)
{
	int i;
	t_ftp_args *thargs = (t_ftp_args *)args;
	for(i = 0; i < thargs->cnt_files; i++)
	{
	    free(thargs->files[i]);
	}
	free(thargs->files);
	free(thargs);
	//printf("cleanup: freeing block at %p\n", args);
}

static void *recThr_send_ftp(void* args)
{
    t_ftp_args *thargs = (t_ftp_args *)args;
	int ret = 0, i = 0;
	short ftpPort = 21;
	char ftpServer[49] = "";
	char ftpUser[49] = "";
	char ftpPasswd[49] = "";
	char ftpFolder[130] = "";
   	char cmd[512];
	int file_not_exist = 0;
	
	pthread_detach(pthread_self());
	
	cfg_get_short("net:ftp:port",   &ftpPort);
	cfg_get_str("net:ftp:password", ftpPasswd, 49);
	cfg_get_str("net:ftp:username", ftpUser, 49);
	cfg_get_str("net:ftp:server",   ftpServer, 49);
	cfg_get_str("net:ftp:folder",   ftpFolder, 129);
			
	if (strcmp(ftpUser, "-") == 0 || strcmp(ftpPasswd, "-") == 0)
	{
        strcpy(ftpUser, "anonymous");
        strcpy(ftpPasswd, "ip@cam.com");
	}

	if (strcmp(ftpFolder, "-") == 0)
	{
		ftpFolder[0] = 0;
    }
	else
	{
		strcat(ftpFolder, "/");
	}

	//LOGI("thargs->file - %s\n", thargs->files[0]);
	//LOGI("thargs->cnt_files - %d\n", thargs->cnt_files);

	for(i = 0; i < thargs->cnt_files; i++)
	{
		//LOGI("file - %s\n", thargs->files[i]);
		ret = file_check(thargs->files[i]);
		if(ret)
		{
			if(file_not_exist == (thargs->cnt_files - 1))
			{
                LOGE("files not exist\n");
                cleanupHandler(thargs);
				return (void *)0;
			}
			file_not_exist++;
			continue;
		}
		
		if(strcmp(ftpServer, "-") != 0)
		{	
			memset(cmd, 0, 512);
			sprintf(cmd, "ftpput -u \"%s\" -p \"%s\" -P %d \"%s\" \"%s%s\" \"%s\" 2>/tmp/ftp.err", 
					ftpUser, ftpPasswd, ftpPort, ftpServer, ftpFolder, strrchr(thargs->files[i], '/') + 1, thargs->files[i]);
			system(cmd);
		}
		//LOGI("unlink - %s\n", thargs->files[i]);
		unlink(thargs->files[i]);
	}
	
    cleanupHandler(thargs);
	pthread_exit(NULL);
	return (void *)0;
}

int send_ftp_files(char *files[], int cnt_files)
{
	int i = 0;
	pthread_t recftpThread;
	t_ftp_args *ptr;
	
	//LOGE("Send ftp");

	ptr = malloc(sizeof(t_ftp_args));
    ptr->files = calloc(cnt_files, sizeof(char *));

	for(i = 0; i < cnt_files; i++)
	{
        ptr->files[i] = calloc(strlen(files[i])+1, sizeof(char));
	    strcpy(ptr->files[i], files[i]);
	    //printf("th1 - %s\n", ptr->files[i]);
	}
	ptr->cnt_files = cnt_files;


	//printf("th2 - %s\n",  ptr->files[0]);
	if (pthread_create(&recftpThread, NULL, recThr_send_ftp, ptr))
	{
		printf("recThr_send_ftp create fail\n");
        cleanupHandler(ptr);
		return FW_FAIL;
	}
	return FW_OK;
}

