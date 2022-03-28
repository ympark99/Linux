#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssu_sdup.h"
#include "ssu_help.h"

void ssu_sdup(){
	while (1){
		char *oper = malloc(sizeof(char) * BUF_SIZE);
		printf("20182615> "); // 프롬프트 출력
		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}
		
		char *splitOper[OPER_LEN] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		while (ptr != NULL){
			if(idx < OPER_LEN)	splitOper[idx] = ptr;
			idx++;
			ptr = strtok(NULL, " ");
		}

		// fmd5 명령 시
		if(splitOper[0] != NULL && strcmp(splitOper[0], "fmd5") == 0){
			// 명령어 인자 틀리면 에러 처리
			if(idx != OPER_LEN){
				printf("error\n");
			}
			else{ // ssu_find-md5.c 실행
				// find_first(splitOper);
			}
		}		
		// exit 입력 시 종료
		else if(splitOper[0] != NULL && strcmp(oper, "exit") == 0){
			// exit 뒤 인자 붙으면 help와 동일한 결과 출력
			if(splitOper[1] != NULL){
				ssu_help();
			}
			else break;
		}	
		else if(splitOper[0] != NULL){ // 엔터키 입력 아닌 경우 명령어 사용법 출력
			ssu_help();
		}
		free(oper);
	}
}