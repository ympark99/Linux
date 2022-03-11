#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "option.h"

void option(int fileOrDir, struct fileLists *fileList){
	while(1){
        printf(">> ");

        char oper[BUF_SIZE];

		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}
		
		char *index_option[IDXOPTION_SIZE] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		// [0] : INDEX , [1] : OPTION or NULL
		while (ptr != NULL){
			if(idx < IDXOPTION_SIZE) index_option[idx] = ptr;
			idx++;
			ptr = strtok(NULL, " ");
		}

		// INDEX 입력 없을 경우
		if(index_option[0] == NULL){
			perror("index 입력이 없거나 존재x");
		}
		else if(index_option[1] != NULL){ // 옵션 있는 경우

		}
		else{
			if(fileOrDir == 1){ // 파일인 경우
				cmp_file(index_option);
				break;
			}

			break;
		}
	}
}

// 파일 비교(옵션 x)
void cmp_file(char *index_option[IDXOPTION_SIZE]){

}