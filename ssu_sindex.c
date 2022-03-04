#include <stdio.h>
#include <unistd.h>
#include <string.h> // string 관련 함수 사용
#include <stdlib.h> // realpath 사용
#include "ssu_sindex.h"

void ssu_sindex(){
	while (1){	
		char oper[OPER_SIZE]; // 초기 명령어
		printf("20182615> "); // 프롬프트 출력
		fgets(oper, OPER_SIZE, stdin); // 명령어 입력

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}
		
		// find 명령 시 find함수 실행
		if(isFind(oper)){
			memmove(oper, oper + 4, strlen(oper)); // find 문자열 제거
			find(oper);
		}
		// exit 입력 시 종료
		else if(strcmp(oper, "exit\n") == 0){
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

// find 명령어 확인 함수
bool isFind(char oper[]){
	for(int i = 0; i < 4; i++){
		if(oper[0] == 'f' && oper[1] == 'i' && oper[2] == 'n' && oper[3] == 'd') return true;
	}
	return false;
}

void find(char oper[]){
	char *findOper[OPER_SIZE] = {NULL, }; // FILENAME PATH등 find 이후 명령어 저장
	
	char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

	int idx = 0;
	while (ptr != NULL){
		if(idx < OPER_SIZE)	findOper[idx] = ptr;
		idx++;
		ptr = strtok(NULL, " ");
	}

	for(int i = 0; i < 10; i++){
		printf("%s\n", findOper[i]);
	}

	if(findOper[0] == ' ' || findOper[1] == ' '){
		printf("error\n");
		return;
	}
}