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
#include "smtp.h"
#include "cfg.h"
#include "ixml.h"
#include "recorder.h"
#include <unistd.h>

typedef struct
{
	char **files;
	int  cnt_files;
}t_smtp_args;

static void cleanupSmtpHandler(void *args)
{
	int i;
	t_smtp_args *thargs = (t_smtp_args *)args;
	for(i = 0; i < thargs->cnt_files; i++)
	{
		free(thargs->files[i]);
	}
	free(thargs->files);
	free(thargs);
	//printf("cleanup: freeing block at %p\n", args);
}

static void *recThr_send_smtp(void* args)
{
   t_smtp_args *thargs = (t_smtp_args *)args;	
   int ret = 0, i = 0;
   
   char sender[49] = "";
   char receiver[49] = "";
   char password[49] = "";
   char username[49] = "";
   short port = 25;
   char servername[49] = "";
 
   char subject[49] = "";
   char auth = 0;
   
   char fname[64];
   char sendmail_cmd[512];
   char makemime_cmd[4096];
   
   char env_cmd[4608];
   
   int file_not_exist = 0;
   
   pthread_detach(pthread_self());
   
   cfg_get_str("net:smtp:sender", sender, 49);
   cfg_get_str("net:smtp:receiver", receiver, 49);
   cfg_get_str("net:smtp:password", password, 49);
   cfg_get_str("net:smtp:username", username, 49);
   cfg_get_short("net:smtp:port", &port);
   cfg_get_str("net:smtp:server", servername, 49);
   cfg_get_char("net:smtp:security", &auth);
   cfg_get_str("net:hostname", subject, 49);
 
   	if((strcmp(servername, "-") == 0) || (strcmp(sender, "-") == 0) || (strcmp(receiver, "-") == 0) || \
		 (strcmp(password, "-") == 0) || (strcmp(username, "-") == 0) ||  (strcmp(servername, "-") == 0))
	{
		goto param_null;
	}
		 
	sprintf (makemime_cmd, "makemime -a \"Subject: %s\" ", subject);
	for(i = 0; i < thargs->cnt_files; i++)
	{
		ret = file_check(thargs->files[i]);
		if(ret)
		{
		   LOGE("file  %s not exist\n", thargs->files[i]);
		   if(file_not_exist == (thargs->cnt_files - 1))
		   {
			   LOGE("files not exist\n");
			   goto end;
		   }
		   file_not_exist++;
		   continue;
		}
		memset(fname, 0, 64);
		sprintf(fname, "%s ", thargs->files[i]);
	 	strcat(makemime_cmd, fname);
	 }
	 
	 if (auth)
	 {
			sprintf(sendmail_cmd, "sendmail -f \"%s\" -H 'exec openssl s_client -quiet -cert /root/.authenticate/private/smtp-starttls.pem -pass pass:appropho -tls1 -starttls smtp -connect %s:%d' -au\"%s\" -ap\"%s\" \"%s\" > /tmp/sendmail.err 2>&1", sender, servername, port, username, password, receiver);
	 }
	 else
	 {
	 	 //printf("sendmail -f \"%s\" -t \"%s\" -S \"%s:%d\"  -au\"%s\" -ap\"%s\"\n", sender, receiver, servername, port, sender, password);
		 sprintf(sendmail_cmd, "sendmail -f \"%s\" -t \"%s\" -S \"%s:%d\"  -au\"%s\" -ap\"%s\"", sender, receiver, servername, port, username, password);
	 }
	
	 sprintf(env_cmd, "%s 2>/tmp/makemime.err | %s 2>/tmp/sendmail.err", makemime_cmd, sendmail_cmd);
	 //LOGE("%s\n", env_cmd);
	 system(env_cmd);

param_null:		 
	 
	 file_not_exist = 0;

	 for(i = 0; i < thargs->cnt_files; i++)
	 {
		 ret = file_check(thargs->files[i]);
		 if(ret)
		 {
//			 LOGE("file  %s not exist\n", thargs->files[i]);
			 if(file_not_exist == (thargs->cnt_files - 1))
			 {
				 LOGE("files not exist\n");
				 goto end;
			 }
			 file_not_exist++;
			 continue;
		 }
		unlink(thargs->files[i]);
    }	
end:	
    cleanupSmtpHandler(thargs);
	pthread_exit(NULL);
	return (void *)0;
}

int send_smtp_files(char *files[], int cnt_files)
{
	int i = 0;
	pthread_t recsmtpThread;
	t_smtp_args *ptr;
	ptr = malloc(sizeof(t_smtp_args));
	ptr->files = calloc(cnt_files, sizeof(char *));
	for(i = 0; i < cnt_files; i++)
	{
		ptr->files[i] = calloc(strlen(files[i])+1, sizeof(char));
		strcpy(ptr->files[i], files[i]);
//		printf("th1 - %s\n", ptr->files[i]);
	}
	ptr->cnt_files = cnt_files;
//	printf("th2 - %s\n",  ptr->files[0]);
	if (pthread_create(&recsmtpThread, NULL, recThr_send_smtp, ptr))
	{
//		printf("recThr_send_smtp create fail\n");
		cleanupSmtpHandler(ptr);
		return FW_FAIL;
	}
	return FW_OK;
}







