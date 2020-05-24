#include <stdio.h>
#include <unistd.h>
#include "recorder.h"


int func_callback_avi(FW_HANDLE_TYPE hFileWritingId, FW_HANDLE_TYPE hUserData,
                      FW_RESULT_TYPE nResult, const char *lpFileName, int nDuration)
{
    printf("func_callback_avi, result: %d, file: %s, durat: %d\n",
           nResult,
           lpFileName,
           nDuration);

	//struct thread_args *ptr = (struct thread_args *)hFileWritingId;
	//if(ptr != NULL)
	//{
	//	pthread_mutex_lock (&ptr->thread_flag_mutex);
	//	if(!ptr->thread_flag)
	//	{
			//LOGW("send_ftp_files flags - %d", ptr->thread_flag);
			//LOGW("send_ftp_files start!\n");
			//send_ftp_files((char**)&lpFileName, 1);
			//LOGW("send_ftp_files - Ok!\n");
			//LOGW("send_smtp_files start!\n");
//			send_smtp_files((char**)&lpFileName, 1);
			//LOGW("send_smtp_files - Ok!\n");

	  //   }
	//	pthread_mutex_unlock(&ptr->thread_flag_mutex);
	//}
    return 5000; //mc
}


int func_callback_avi1(FW_HANDLE_TYPE hFileWritingId, FW_HANDLE_TYPE hUserData, FW_RESULT_TYPE nResult, const char *lpFileName, int nDuration)
{
		printf("func_callback_avi, result: %d, file: %s, durat: %d\n", nResult, lpFileName, nDuration);

		if(nResult < 0)
		{
			//StopFileWriting(hFileWritingId);
			return 0;
		}
		send_ftp_files((char**)&lpFileName, 1);
		LOGE("send_ftp_files - Ok!\n");
		return 5000; //mc
		//return 300000; //mc
}

/*
int func_callback_avi2(FW_HANDLE_TYPE hFileWritingId, FW_HANDLE_TYPE hUserData, FW_RESULT_TYPE nResult, const char *lpFileName, int nDuration)
{
	printf("func_callback_avi1\n");

	return 3000; //mc
}

int func_callback_avi3(FW_HANDLE_TYPE hFileWritingId, FW_HANDLE_TYPE hUserData, FW_RESULT_TYPE nResult, const char *lpFileName, int nDuration)
{
	printf("func_callback_avi1\n");

	return 3000; //mc
}


int func_callback_jpg(FW_HANDLE_TYPE hFileWritingId, FW_HANDLE_TYPE hUserData, FW_RESULT_TYPE nResult, const char *lpFileName, int nDuration)
{
	printf("func_callback_jpg\n");

	return 5000; //mc
}

int func_callback_jpg1(FW_HANDLE_TYPE hFileWritingId, FW_HANDLE_TYPE hUserData, FW_RESULT_TYPE nResult, const char *lpFileName, int nDuration)
{
	printf("func_callback_jpg\n");

	return 3000; //mc
}
*/

struct user_data
{
    int data;
};

#if defined(SCHEDULER_ENABLED)
int debug_main( int argc, char *argv[])
#else
int main( int argc, char *argv[])
#endif
{
   printf("My start...\n");
   int  nDurationBefore;
   int	nDurationAfter;

   //int  nDurationBefore2;
   //int	nDurationAfter2;

   FW_HANDLE_TYPE	hUserData;
   FW_HANDLE_TYPE	phAVIFileWritingId;
   FW_HANDLE_TYPE	phAVIFileWritingId1;
   //FW_HANDLE_TYPE	phAVIFileWritingId2;
   //FW_HANDLE_TYPE	phAVIFileWritingId3;
   //FW_HANDLE_TYPE	phJPGFileWritingId;
   //FW_HANDLE_TYPE	phJPGFileWritingId1;

   struct user_data udata;
   struct user_data udata1;
   //struct user_data udata2;
   //struct user_data udata3;
   //struct user_data udata4;
   //struct user_data udata5;

   InitializeFileWritingLibrary();

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Create AVI  1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   udata.data = 0x55885599;
   hUserData = &udata;
   nDurationBefore = 0;
   nDurationAfter = 5000;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   //StartFileWriting(kLocalStorageFileWritingDest, 2, kJpgFileWritingFormat,
   StartFileWriting(kLocalStorageFileWritingDest, 0, kAviFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_avi, &phAVIFileWritingId);

   //StartFileWriting(kEmailFileWritingDest, 0, kAviFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_avi, &phAVIFileWritingId);

   udata1.data = 0x55885588;
   hUserData = &udata1;
   nDurationBefore = 0;
   nDurationAfter = 5000;

//   StartFileWriting(kLocalStorageFileWritingDest, 0, kAviFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_avi1, &phAVIFileWritingId1);

   StartFileWriting(kFtpFileWritingDest, 0, kAviFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_avi1, &phAVIFileWritingId1);

   /* udata1.data = 0x55885588;
   hUserData = &udata1;
   nDurationBefore2 = 3000;
   nDurationAfter2 = 2000;
   //int n = 250000;
   //while(n)
   //{
   //	   n--;
   //}
//   sleep(1);
//   StartFileWriting(kLocalStorageFileWritingDest, 1, kAviFileWritingFormat, nDurationBefore2, nDurationAfter2, hUserData, func_callback_avi1, &phAVIFileWritingId1);

   udata2.data = 0x55885588;
   hUserData = &udata2;
   nDurationBefore = 3000;
   nDurationAfter = 3000;
   StartFileWriting(kLocalStorageFileWritingDest, 1, kAviFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_avi2, &phAVIFileWritingId2);
   sleep(1);
   udata3.data = 0x55885588;
   hUserData = &udata3;
   nDurationBefore2 = 3000;
   nDurationAfter2 = 4000;
   StartFileWriting(kLocalStorageFileWritingDest, 1, kAviFileWritingFormat, nDurationBefore2, nDurationAfter2, hUserData, func_callback_avi3, &phAVIFileWritingId3);
   //sleep(1);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Create JPG
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   udata4.data = 0x88555588;
   hUserData = &udata4;
   nDurationBefore = 3000;
   nDurationAfter = 5000;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   StartFileWriting(kLocalStorageFileWritingDest, 2, kJpgFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_jpg, &phJPGFileWritingId);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   udata5.data = 0x88555588;
   hUserData = &udata5;
   nDurationBefore = 3000;
   nDurationAfter = 3000;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   StartFileWriting(kLocalStorageFileWritingDest, 2, kJpgFileWritingFormat, nDurationBefore, nDurationAfter, hUserData, func_callback_jpg, &phJPGFileWritingId1);
*/
/*
   printf("sheduler_start\n");
   subscribe_removeiffileold();
   subscribe_archiveduration();
   subscribe_removeifnospace();
   subscribe_reservedspace();
   subscribe_path();
  */
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   //sleep(620);

   sleep(100);

   LOGE("StopFileWriting avi!!!\n");

   StopFileWriting(phAVIFileWritingId);
   StopFileWriting(phAVIFileWritingId1);


   // StopFileWriting(phAVIFileWritingId2);
  // StopFileWriting(phAVIFileWritingId3);
  // StopFileWriting(phJPGFileWritingId);
  // StopFileWriting(phJPGFileWritingId1);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //printf("sheduler_continue\n");
   //sleep(1);
	 //check_stream(2);
	 //check_stream_name(2);

   //int cnt = 4, ret =0;
   //char *files[256] = {"file.txt", "file1.txt", "file2.txt", "file3.txt"};
   //ret = send_smtp_files((char **)&files,  cnt);
   //printf("send_smtp_files: %d\n", ret);

   //ret = send_ftp_files((char **)&files,  cnt);
   //printf("send_ftp_files: %d\n", ret);

 // get_sd_space("/home/alex/nfs");

  LOGE("sheduler_end\n");

 //  UninitializeFileWritingLibrary();
   return 0;
};


