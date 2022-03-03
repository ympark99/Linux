#include <stdio.h>
#include <unistd.h>
#include <string.h> // string 관련 함수 사용
#include "ssu_sindex.h"

void ssu_sindex(){
	while (1){	
		char oper[2000]; // 초기 명령어
		printf("20182615> ");
		// scanf("%s", oper);
		// gets(oper);
		fgets(oper, sizeof(oper), stdin);

		// exit 입력 시 종료
		if(strcmp(oper, "exit\n") == 0){
			printf("Prompt End\n");
			break;
		}
		else if(oper[0] != '\n'){ // 엔터키 입력 아닌 경우 명령어 사용법 출력
			print_inst(); // 명령어 사용법
		}
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