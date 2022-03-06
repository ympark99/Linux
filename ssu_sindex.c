#include <stdio.h>
#include <unistd.h>
#include <string.h> // string 관련 함수 사용
#include <stdlib.h> // realpath 사용
#include "ssu_sindex.h"

void ssu_sindex(){
	while (1){
		char *oper = malloc(sizeof(char) * OPER_SIZE);
		printf("20182615> "); // 프롬프트 출력
		fgets(oper, OPER_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}
		
		char *findOper[FINDOPER_SIZE] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		while (ptr != NULL){
			if(idx < FINDOPER_SIZE)	findOper[idx] = ptr;
			idx++;
			ptr = strtok(NULL, " ");
		}

		// find 명령 시 find 함수 실행
		if(findOper[0] != NULL && strcmp(findOper[0], "find") == 0){
			// 명령어 인자 틀리면 에러 처리
			if(idx != FINDOPER_SIZE){
				printf("error\n");
			}
			else{
				find(findOper);
			}
		}		
		// exit 입력 시 종료
		else if(findOper[0] != NULL && strcmp(oper, "exit") == 0){
			// exit 뒤 인자 붙으면 help와 동일한 결과 출력
			if(findOper[1] != NULL){
				print_inst();
			}
			else{
				printf("Prompt End\n");
				break;
			}
		}	
		else if(findOper[0] != NULL){ // 엔터키 입력 아닌 경우 명령어 사용법 출력
			print_inst(); // 명령어 사용법
		}

		free(oper);
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

// find 함수
void find(char *findOper[]){
	// 상대경로인 경우 절대 경로로 변환
	if(findOper[2][0] != '/'){
		char buf[OPER_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[2], buf) == NULL){
			perror("realpath error"); // to do :전역변수 errno에 설정
			return;
		}
		printf("real path is %s\n", buf);
	}
}