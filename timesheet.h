#ifndef _RMOLDFILES_H_
#define _RMOLDFILES_H_

#include <stdio.h>
#include "recorder.h"

typedef struct
{
	int  				removeIfFileOld;
	int  				removeIfNoSpace;
	int  				arhiveDuration;
	int 				reservedSpace;
	int  				changeStream; 

}record_t;

void *timesheet(void* args);

#endif
