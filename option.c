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
	char *readLine, *readCmpLine;
	
	// FILE *fp = fopen(filelist[0].path, "r"); // 원본 파일
	// FILE *cmpFp = fopen(filelist[cmpIdx].path, "r"); // 비교할 파일

	FILE *fp = fopen("a.txt", "r"); // 테스트 원본 파일
	FILE *cmpFp = fopen("b.txt", "r"); // 테스트 비교할 파일	

	int lineIdx = 0; // 원본 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인

	// 의사코드
	// 원본 비교시작줄 나올때까지 find - ok
	// 바로 나오면 스킵하고 안나오면 비교파일 다음줄 탐색 - ok
	// 찾다 나오면 추가 처리 - ok
	// 비교 파일 끝까지 안나오면 원본 줄 끝까지 반복 -> 가장 비교본시작줄과 가까운 곳 찾기
	// 1. 비교시작줄과 일치 -> 삭제 - ok
	// 2. 일치x -> (원본 파일 끝까지 돌려서 가장 시작줄과 가까운 곳까지)수정

	// 원본 끝줄까지 반복해도 안나오고 비교파일 끝난거 아니면 수정, 끝났으면 삭제
	// 반복하다 나온 줄이 비교파일 비교시작 줄이면 그동안 반복 삭제 처리

	// 방법2
	// 두 파일 최장 공통 부분 수열 구해서
	// LCS 사이 + - 섞여있음 -> 수정
	// + -> 추가, - -> 삭제

	while (!feof(fp)){
		int startFtell = ftell(fp); // 원본파일 다시 탐색시 시작할 라인(읽기 전)
		readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
		lineIdx++;

		int cmpStartFtell = ftell(cmpFp); // 비교파일 다시 탐색시 시작할 라인(읽기 전)
		readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;

		int nearOriIdx = BUF_SIZE; // 비교본 시작라인 가장 가까운 원본 줄
		int nearOriFtell = BUF_SIZE; // 가장 가까운 원본 라인
		int nearCmpIdx = BUF_SIZE; // 가장 가까운 비교 줄
		int nearCmpFtell = BUF_SIZE; // 가장 가까운 비교 라인

		if(strcmp(readLine, readCmpLine) != 0){ // 다르면 나올때까지 비교파일 탐색
			int startLine = lineIdx; // 원본파일 시작 라인
			int cmpStartLine = cmpLineIdx; // 비교파일 시작 라인(읽은 후)
			bool doOriSeek = true;

			// 원본파일 비교 시작줄 나올떄까지 탐색
			while (!feof(cmpFp)){
				readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
				cmpLineIdx++;

				// 추가 처리 : 탐색하다 같은 줄 나온 경우
				if(strcmp(readLine, readCmpLine) == 0){
					// 상황별 추가 포맷 출력
					if(cmpStartLine == (cmpLineIdx - 1))
						printf("%da%d\n", lineIdx - 1, cmpStartLine);
					else 
						printf("%da%d,%d\n", lineIdx - 1, cmpStartLine, cmpLineIdx - 1);

					// 추가된 파일 내용 출력
					fseek(cmpFp, cmpStartFtell, SEEK_SET); // 비교 시작 위치로 이동
					for(int i = cmpStartLine; i < cmpLineIdx; i++){
						printf("> ");
						readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
						printf("%s", readCmpLine);
					}
					readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 처리했으므로 한 줄 추가
					doOriSeek = false; // 삭제, 수정비교 x
					break;
				}
			}


			// 비교 파일 끝까지 안나오면 원본 줄 끝까지 반복 -> 가장 비교본시작줄과 가까운 곳 찾기
			// 1. 비교시작줄과 일치 -> 삭제 - ok
			// 2. 일치x -> (원본 파일 끝까지 돌려서 가장 시작줄과 가까운 곳까지)수정

			// 삭제, 수정 : 비교 파일 끝까지 안나오면 원본 줄 끝까지 반복 -> 가장 비교본시작줄과 가까운 곳 찾기
			while (!feof(fp) && doOriSeek){
				readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
				lineIdx++;

				fseek(cmpFp, cmpStartFtell, SEEK_SET); // 비교 시작 위치로 이동
				cmpLineIdx = cmpStartLine - 1; // 비교 시작 라인(읽기전)으로 초기화

				// 비교시작 라인부터 탐색
				while (!feof(cmpFp)){
					readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
					cmpLineIdx++;

					// 문장 일치시
					if(strcmp(readLine, readCmpLine) == 0){
						// 삭제 처리 : 일치한 문장 라인이 비교시작 라인일경우
						if(cmpLineIdx == cmpStartLine){
							//상황별 삭제 포맷 출력
							if(startLine == (lineIdx - 1))
								printf("%dd%d\n", startLine, cmpLineIdx - 1);
							else
								printf("%d,%dd%d\n", startLine, lineIdx - 1, cmpLineIdx - 1);
							// 삭제된 내용 출력
							fseek(fp, startFtell, SEEK_SET); // 원본 비교시작 라인으로 이동
							for(int i = startLine; i < lineIdx; i++){
								printf("< ");
								readLine = fgets(line, BUF_SIZE, fp); // 비교파일 한 줄 읽기
								printf("%s", readLine);
							}
							readLine = fgets(line, BUF_SIZE, fp); // 처리했으므로 한 줄 추가
							// 원본 반복 탈출
							doOriSeek = false;
							break;

						}
						// 수정 처리
						else{ // 비교본 시작라인과 가장 가까운 원본라인 구하기
							// 공백일경우, 공백이 아닌 경우도 일치해야함
							if(strcmp(readLine, "\n") == 0){
								int bufFtell = ftell(fp);
								int cmpBufFtell = ftell(cmpFp);
								// 공백 아닐때까지 반복
								while (!feof(fp) && !feof(cmpFp)){
									readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
									readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기

									if(strcmp(readLine, readCmpLine) == 0 && strcmp(readLine, "\n") != 0){
										nearCmpIdx = cmpLineIdx;
										nearCmpFtell = ftell(cmpFp);
										nearOriIdx = lineIdx;
										nearOriFtell = ftell(fp);					
									}
								}
								fseek(fp, bufFtell, SEEK_SET);
								fseek(cmpFp, cmpBufFtell, SEEK_SET);
							}
							else if(cmpLineIdx < nearCmpIdx){ // 더 가까울 경우 인덱스, 라인 저장
								nearCmpIdx = cmpLineIdx;
								nearCmpFtell = ftell(cmpFp);
								nearOriIdx = lineIdx;
								nearOriFtell = ftell(fp);
							}
						}
					}
				}
			}
			// 수정 처리
			if(doOriSeek){
				cmpLineIdx = nearCmpIdx;
				lineIdx = nearOriIdx;

                // 상황별 수정 포맷 출력
                if(startLine == (lineIdx - 1) && cmpStartLine == (cmpLineIdx - 1))
                    printf("%dc%d\n", startLine, cmpStartLine);
                else if(startLine == (lineIdx - 1) && cmpStartLine != (cmpLineIdx - 1))
                    printf("%dc%d,%d\n", startLine, cmpStartLine, cmpLineIdx - 1);
                else if(startLine != (lineIdx - 1) && cmpStartLine == (cmpLineIdx - 1))
                    printf("%d,%dc%d\n", startLine, lineIdx - 1, cmpStartLine);
                else printf("%d,%dc%d,%d\n", startLine, lineIdx - 1, cmpStartLine, cmpLineIdx - 1);			
			}

                        //  // 수정 내용 출력 : 원본
                        //  fseek(fp, startFtell, SEEK_SET); // 원본 비교시작 라인으로 이동
                        //  for(int i = startLine; i < lineIdx; i++){
                        //      printf("< ");
                        //      readLine = fgets(line, BUF_SIZE, fp); // 비교파일 한 줄 읽기
                        //      printf("%s", readLine);
                        //  }
                        //  readLine = fgets(line, BUF_SIZE, fp); // 처리했으므로 한 줄 추가
                        //  printf("---\n");
                        //  // 수정 내용 출력 : 비교
                        //  fseek(cmpFp, cmpStartFtell, SEEK_SET); // 비교 시작 위치로 이동
                        //  for(int i = cmpStartLine; i < cmpLineIdx; i++){
                        //      printf("> ");
                        //      readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
                        //      printf("%s", readCmpLine);
                        //  }
                        //  readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 처리했으므로 한 줄 추가
                        // }



		}
		// else printf("same!!\n");

	}

	fclose(fp);
	fclose(cmpFp);
}