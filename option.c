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
			int cmpIdx = atoi(index_option[0]); // 입력 INDEX
			if(cmpIdx > listSize - 1 || cmpIdx == 0){ // 입력 INDEX가 배열 초과할 경우 에러
				perror("index 존재 x");
			}
			else if(index_option[1] != NULL){ // 옵션 있는 경우

			}
			else{
				if(fileOrDir == 1){ // 파일인 경우 파일 비교 실행
					cmp_file(cmpIdx, fileList);
					break;
				}

				break;
			}			
		}
	}
}

// 파일 비교(옵션 x)
void cmp_file(int cmpIdx, struct fileLists *filelist){
	char line[BUF_SIZE], cmpLine[BUF_SIZE];
	char *getLine, *getCmpLine;
	
	// FILE *fp = fopen(filelist[0].path, "r"); // 원본 파일
	// FILE *cmpFp = fopen(filelist[cmpIdx].path, "r"); // 비교할 파일

	FILE *fp = fopen("a.txt", "r"); // 테스트 원본 파일
	FILE *cmpFp = fopen("b.txt", "r"); // 테스트 비교할 파일	

	int lineIdx = 0; // 원본 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인

	// 의사코드
	// 원본 비교시작줄 나올때까지 find
	// 바로 나오면 스킵하고 안나오면 비교파일 다음줄 탐색
	// 찾다 나오면 추가 처리
	// 비교 파일 끝까지 안나오면 원본 다음 줄 탐색(반복)
	// 원본 끝줄까지 반복해도 안나오고 비교파일 끝난거 아니면 수정, 끝났으면 삭제
	// 반복하다 나온 줄이 비교파일 비교시작 줄이면 그동안 반복 삭제 처리
	// 아니면(몇 줄 건너뛰어서 나오면) 그 부분 change 처리

	while (!feof(fp)){
		getLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
		// printf("%s", getLine);
		lineIdx++;

		int saveCmpLineIdx = ftell(cmpFp); // 비교파일 다시 탐색시 시작할 라인 
		getCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		// printf("%s", getCmpLine);
		cmpLineIdx++;

		if(strcmp(getLine, getCmpLine) != 0){ // 다르면 나올때까지 비교파일 탐색
			int startLineIdx = cmpLineIdx; // 시작 라인		
			while (!feof(cmpFp)){
				getCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
				cmpLineIdx++;

				// 추가 처리 : 탐색하다 같은 줄 나온 경우
				if(strcmp(getLine, getCmpLine) == 0){
					printf("%da%d,%d\n", lineIdx - 1, startLineIdx, cmpLineIdx - 1); // 추가 포맷 출력
					// 추가된 파일 내용 출력
					fseek(cmpFp, saveCmpLineIdx, SEEK_SET); // 비교 시작 위치로 이동
					for(int i = startLineIdx; i < cmpLineIdx; i++){
						printf("> ");
						getCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
						printf("%s", getCmpLine);
					}
					getCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 처리했으므로 한 줄 추가
					break;
				}
			}
		}
		// else printf("same!!\n");

	}

	fclose(fp);
	fclose(cmpFp);

	// while (!feof(fp)){
	// 		getLine = fgets(line, BUF_SIZE, fp);
	// 		printf("%s", getLine);
	// }	
	// printf("--------------\n");
	// while (!feof(cmpFp)){
	// 		getCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp);
	// 		printf("%s", getCmpLine);
	// }
}