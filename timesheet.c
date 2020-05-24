#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <unistd.h>
#include "recorder.h"

int threadquit = 0;

int filter(const struct dirent *entry);

#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)

int str_is_alnum(const char *str)
{
    size_t length;
	int i = 0;
    length = strlen(str);
    if(length != 14)
    {
		return 1;
    }
    for(i = 0; i < 14; i++)
    {
		if(!is_digit(str[i]))
		{
            return 1;
		}
    }
    return 0;
}

int filter(const struct dirent *entry)
{
    const char *name = entry->d_name;
    char *strfile = NULL;
    char *strpattern = NULL;
    if(strcmp(".", name) == 0 || strcmp("..", name) == 0)
    {
		return 0;
    }
    else
    {
		strpattern = strdup(name);
		strfile = strchr(strpattern,'_');
		if(strfile != NULL)
		{
			*strfile ='\0';
			if(!str_is_alnum(strpattern))
			{
				free(strpattern);
				return 1;
			}
		}
		free(strpattern);
		return 0;
    }
}

void *timesheet(void* args)
{
	record_t *record = (record_t *)args;
	char lastDateName[40];
	struct tm *tm;
	time_t tnow;
	int duration;
	int files_count = 0;
	int i = 0, res = 0;
	char file_name_del[256];
	struct dirent **namelist;
	char  pthrPath[256];
	char  currentPath[256];

	int index = 0;

	memset(currentPath, 0, 256);
	memset(pthrPath, 0, 256);

	//LOGI("pthread timesheet started...");

	while(threadquit == 0)
	{
		res = check_local_path(pthrPath, kLocalStorageFileWritingDest);
		if(res != 0 && res != FW_INSUFFICIENT_SPACE)
		{
		   //LOGE("timesheet: Error: path_check!!!\n");
		  sleep(1);
		  continue;
		}

		  //printf("removeIfFileOld - %d\n", record->removeIfFileOld);
		  //printf("removeIfNoSpace - %d\n", record->removeIfNoSpace);
		  //printf("reservedSpace   - %d\n", record->reservedSpace);
		  //printf("arhiveDuration  - %d\n", record->arhiveDuration);

		//LOGE("files_count - %d\n", files_count);

		if(!strcmp(pthrPath, currentPath))
		{
			strcpy(currentPath, pthrPath);
			if(files_count)
			{
				for(i = index; i < files_count; i++)
				{
					free(namelist[i]);
				}
				files_count = 0;
				index = 0;
				free(namelist);
			}
		}

		if(record->removeIfFileOld || record->removeIfNoSpace)
		{
			//LOGE("RemoveIfFileOld or RemoveIfNoSpace\n");
			if(files_count == index)
			{
				if(files_count)
				{
					for(i = index; i < files_count; i++)
					{
						free(namelist[i]);
					}
					files_count = 0;
					index = 0;
					free(namelist);
				}

				//LOGE("===================== scandir ========================= \n");

				files_count = scandir(pthrPath, &namelist, &filter, alphasort);
				if (files_count < 0)
				{
					LOGE("Error: scandir\n");
					files_count = 0;
					sleep(1);
					continue;
				}

				if(!files_count)
				{
					//LOGW("New files not found!!!\n");
					free(namelist);
					sleep(1);
					continue;
				}
			}
		}
		if (record->removeIfFileOld)
		{
			//LOGE("Remove old files from archive\n");
			time(&tnow);
			duration = record->arhiveDuration*60*24*60;
			if (tnow > duration)
			{
				tnow -= duration;
			}
			else
			{
				tnow = 0;
			}
			tm = localtime(&tnow);
			sprintf(lastDateName, "%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
			//LOGE("lastDateName - %s\n", lastDateName);

			if(files_count)
			{
				for(i = index; i < files_count; i++)
				{
					if(strncmp(namelist[i]->d_name, lastDateName, 14) < 0)
					{
						sprintf(file_name_del, "%s/%s", pthrPath, namelist[i]->d_name);
						unlink(file_name_del);
						LOGE("Time out: index - %d, file_name_del - %s\n", index, file_name_del);
						free(namelist[i]);
						index++;
					}
					else
					{
						break;
					}
				}

			}
		}

		if (record->removeIfNoSpace)
		{
			//LOGE("Remove files if no space\n");
			long long size = 0;
			if(files_count)
			{
				for(i = index; i < files_count; i++)
				{
					size = get_diskfree_space(pthrPath);

					//LOGE("DiskSpace: size - %lld\n", size);

					if(size < ((long long)record->reservedSpace*1024))
					{
						sprintf(file_name_del, "%s/%s", pthrPath, namelist[i]->d_name);
						unlink(file_name_del);
						free(namelist[i]);
						index++;
					}
					else
					{
						break;
					}
				}
			}
		}
		sleep(2);
	}
	return (void *)0;
}


