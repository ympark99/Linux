#include <stdio.h>
#include <stdlib.h>
#include "ssu_help.h"

int main(int argc, char *argv[]){
	ssu_help();
	exit(0);
}

void ssu_help(){
	printf("Usage:\n");
	printf("  > fmd5/fsha1  [FILE_EXTENSION] [MINSIZE] [MAXSIZE] [TARGET_DIRECTORY]\n");
	printf("     >>  [SET_INDEX] [OPTION ... ]\n");
	printf("         [OPTION ... ]\n");
	printf("         d [LIST_IDX] : delete [LIST_IDX] file\n");
	printf("         i : ask for confirmation before delete\n");
	printf("         f : force delete except the recently modified file\n");
	printf("         t : force move to Trash except the recently modified file\n");
	printf("   [IDX] a : delete the [IDX] file for all sets\n");
	printf("  > help\n");
	printf("  > exit\n\n");
}