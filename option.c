#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "option.h"

void option(int fileOrDir, struct fileLists *fileList, int listSize){
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
			perror("index 입력이 없음");
		}
		else{
			int idx = atoi(index_option[0]);
			if(idx > listSize - 1 || idx == 0){ // 입력 INDEX가 배열 초과할 경우 에러
				perror("index 존재 x");
			}
			else if(index_option[1] != NULL){ // 옵션 있는 경우

			}
			else{
				if(fileOrDir == 1){ // 파일인 경우 파일 비교 실행
					
					printf("idx : %d\n", idx);
					cmp_file(index_option, fileList);
					break;
				}

				break;
			}			
		}
	}
}

// 파일 비교(옵션 x)
void cmp_file(char *index_option[IDXOPTION_SIZE], struct fileLists *filelist){
	char line[BUF_SIZE];
	char *pLine;
	FILE *in = fopen("test.txt", "r");
	while (!feof(in)){
		pLine = fgets(line, BUF_SIZE, in);
		printf("%s", pLine);
	}
	fclose(in);
}