#include <stdio.h>
#include <stdlib.h>
#include "ssu_help.h"

void ssu_help(){
	printf("Usage:\n");
	printf("  > fmd5/fsha1  [FILE_EXTENSION] -l [MINSIZE] -h [MAXSIZE] -d [TARGET_DIRECTORY] -t [THREAD_NUM]\n");
	printf("     >>  delete -l [SET_INDEX] -d [OPTARG] -i -f -t\n");
	printf("  > trash -c [CATEGORY] -o [ORDER]\n");
	printf("  > restore [RESTORE_INDEX]\n");
	printf("  > help\n");
	printf("  > exit\n\n");
}