#include <stdio.h>
#include <sys/vfs.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "recorder.h"

record_t recLParams;

void on_cfg_changed(int pd)
{
	switch(pd)
	{
		case 0:
		{
			int removeIfFileOld = 0;
			cfg_get_int("rec:local:removeiffileold", &removeIfFileOld);
			recLParams.removeIfFileOld = removeIfFileOld;
			//LOGE("local removeiffileold changed!!! - %d", recLParams.removeIfFileOld);
		}
		break;
		case 1:
		{
			int arhiveDuration = 0;
			cfg_get_int("rec:local:archiveduration", &arhiveDuration);
			recLParams.arhiveDuration = arhiveDuration;
			//LOGE("local archiveduration changed!!! - %d", recLParams.arhiveDuration);
		}
		break;
		case 2:
		{	
			int removeIfNoSpace = 0;
			cfg_get_int("rec:local:removeifnospace", &removeIfNoSpace);
			recLParams.removeIfNoSpace = removeIfNoSpace;
			//LOGE("local removeifnospace changed!!! - %d", recLParams.removeIfNoSpace);
		}
		break;
		case 3:
		{
			int reservedSpace = 0;
			cfg_get_int("rec:local:reservedspace", &reservedSpace);
			recLParams.reservedSpace = reservedSpace;
			//LOGE("local reservedspace changed!!! - %d", recLParams.reservedSpace);
		}
		break;
	}
}

//void init_record_t_params(void)
//{
//pthread_mutex_init(&recLParams.pmutex, NULL);
//}

void subscribe_removeiffileold(void)
{
	cfg_subscribe("rec:local:removeiffileold", on_cfg_changed, 0);
}

void subscribe_archiveduration(void)
{
	cfg_subscribe("rec:local:archiveduration", on_cfg_changed, 1);
}

void subscribe_removeifnospace(void)
{
	cfg_subscribe("rec:local:removeifnospace", on_cfg_changed, 2);
}

void subscribe_reservedspace(void)
{
	cfg_subscribe("rec:local:reservedspace", on_cfg_changed, 3);
}

void subscribe_path(void)
{
	cfg_subscribe("rec:local:path", on_cfg_changed, 4);
}

int check_stream(int ch)
{
	char curr_stream[64];
	int enabled = 1;
	sprintf(curr_stream, "stream%d:enabled", ch);
	cfg_get_int(curr_stream, &enabled);
	if (enabled)
	{
		LOGI("ENB: %s", curr_stream);
		return FW_OK;
	}
	else
	{
		LOGI("DIS: %s", curr_stream);
		return FW_FAIL;
	}
}

void check_stream_name(int ch, char *stream_name)
{
	char curr_stream[64];
	sprintf(curr_stream, "stream%d:encoder", ch);
	cfg_get_name(curr_stream, stream_name, 64);
}

int path_check(const char *path)
{
	if(strcmp(path, "-") == 0)
	{
		//LOGE("%s path is not specified", path);
		return FW_UNKNOWN_LOCAL_STORAGE;
	}
	return 0;
}

int dir_check(const char *path)
{
	struct stat stats;

	if (stat(path, &stats))
		return FW_FAIL;

	if (!S_ISDIR (stats.st_mode))
	{
		LOGE("%s is not a directory", path);
		return FW_FAIL;
	}
	return 0;
}

int file_check(char *file)
{
	if(access(file, F_OK) != -1 )
	{
		// file exists
		return 0;
	}
	return 1;
}

long long get_diskfree_space(const char *p)
{
	struct statfs disk_statfs;
	if (statfs(p, &disk_statfs) < 0)
		return 0;
	return ((long long)disk_statfs.f_bsize * (long long)disk_statfs.f_bfree / 1024);
}
