#include <stdio.h>
#include <sys/time.h> // gettimeofday 사용
#include <unistd.h>
#include <math.h> // floor 사용
#include <string.h> // string 관련 함수 사용
#include "ssu_sindex.h"

void ssu_sindex(){
	while (1){	
		char oper[10]; // 초기 명령어
		scanf("%s", oper);

		// exit 입력 시 종료
		if(strcmp(oper, "exit") == 0){
			break;
		}

		// help, 이외 명령어 입력 시 명령어 사용법 출력
		print_inst(); // 명령어 사용법
	}

	return;
}

void print_inst(){
	printf("Usage:\n");
	printf("  > find [FILENAME] [PATH]\n");
	printf("    >> [INDEX] [OPTION ... ]\n");
	printf("  > help\n");
	printf("  > exit\n\n");
	printf("  [OPTION ... ]\n");
	printf("   q : report only when files differ\n");
	printf("   s : report when two files are the same\n");
	printf("   i : ignore case differences in file contents\n");
	printf("   r : recursivly compare any subdirectories found\n");
}