#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "option.h"

//todo : 마지막줄 공백인 경우 처리 (fopen != NULL 등 이용)

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
			else{
				if(fileOrDir == 1) // 파일인 경우 파일 비교 실행
					// todo: 옵션 중복가능이므로 index_optiond으로 넘겨주기
					index_option[1] == NULL? cmp_file(cmpIdx, fileList) : cmp_fileOption(cmpIdx, fileList, index_option[1]);
				else if(fileOrDir == 2)
					cmp_dir(cmpIdx, fileList, index_option[1]); // 디렉토리 비교 실행(입력 INDEX, 파일 리스트, 입력 옵션)
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

	// todo : 입력한 path로 변경
	FILE *fp = fopen("test/e.txt", "r"); // 테스트 원본 파일
	FILE *cmpFp = fopen("test/f.txt", "r"); // 테스트 비교할 파일	
	int lineIdx = 0; // 원본 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인

	while (!feof(fp)){
		int startFtell = ftell(fp); // 원본파일 다시 탐색시 시작할 라인(읽기 전)
		readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
		lineIdx++;

		int cmpStartFtell = ftell(cmpFp); // 비교파일 다시 탐색시 시작할 라인(읽기 전)
		readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;

		int nearOriIdx = BUF_SIZE; // 비교본 시작라인 가장 가까운 원본 줄
		int nearCmpIdx = BUF_SIZE; // 가장 가까운 비교 줄

		bool isEdit = false; // 수정하는지 확인하는 함수(마지막 제외)

		if(strcmp(readLine, readCmpLine) != 0){ // 다르면 나올때까지 비교파일 탐색
			int startLine = lineIdx; // 원본파일 시작 라인(읽은 후)
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
										nearOriIdx = lineIdx;				
									}
								}
								fseek(fp, bufFtell, SEEK_SET);
								fseek(cmpFp, cmpBufFtell, SEEK_SET);
							}
							else if(cmpLineIdx < nearCmpIdx){ // 더 가까울 경우 인덱스, 라인 저장
								nearCmpIdx = cmpLineIdx;
								nearOriIdx = lineIdx;
							}
							isEdit = true;
						}
					}
				}
			}
			// 수정 처리			
			if(doOriSeek){
				if(!isEdit && (feof(fp) && feof(cmpFp))){ // 파일 전체 끝난경우, 라인 저장
					nearCmpIdx = cmpLineIdx + 1;
					nearOriIdx = lineIdx + 1;
				}
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

                // 수정 내용 출력 : 원본
				fseek(fp, startFtell, SEEK_SET); // 원본 비교시작 라인으로 이동
                for(int i = startLine; i < lineIdx; i++){
					if(readLine == NULL) break;
                    printf("< ");
                    readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
                    printf("%s", readLine);
                }
                readLine = fgets(line, BUF_SIZE, fp); // 처리했으므로 한 줄 추가
				if(feof(fp)) printf("\n\\ No newline at end of file\n"); // 마지막 줄인경우 출력
                printf("---\n");
                // 수정 내용 출력 : 비교
                fseek(cmpFp, cmpStartFtell, SEEK_SET); // 비교 시작 위치로 이동
                for(int i = cmpStartLine; i < cmpLineIdx; i++){
					if(readCmpLine == NULL) break;
                    printf("> ");
                    readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
                    printf("%s", readCmpLine);
                }
                readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 처리했으므로 한 줄 추가
				if(feof(cmpFp)) printf("\n\\ No newline at end of file\n"); // 마지막 줄인경우 출력
			}			
		}
	}
	// todo: 마지막 줄 처리 맞는지 점검, 추가 테스트
	fclose(fp);
	fclose(cmpFp);
}

// 파일비교(옵션 O)
void cmp_fileOption(int cmpIdx, struct fileLists *filelist, char *options){
	char oriPath[BUF_SIZE]; // 원본파일 path
	strcpy(oriPath, filelist[0].path);

	char cmpPath[BUF_SIZE]; // 비교파일 path
	strcpy(cmpPath, filelist[cmpIdx].path);

	// 다른 인덱스 찾기
	int diffIdx = 0; // 다른 인덱스
	for(int i = 0; i < strlen(oriPath); i++){
		if(oriPath[i] != cmpPath[i]){
			diffIdx = i;
			break;
		}
	}
	memmove(oriPath, oriPath + diffIdx, strlen(oriPath)); // 하위파일로 잘라주기
	memmove(cmpPath, cmpPath + diffIdx, strlen(cmpPath)); // 하위파일로 잘라주기

	char line[BUF_SIZE], cmpLine[BUF_SIZE];
	char *readLine, *readCmpLine;
	
	FILE *fp = fopen(filelist[0].path, "r"); // 원본 파일
	FILE *cmpFp = fopen(filelist[cmpIdx].path, "r"); // 비교할 파일

	int lineIdx = 0; // 원본 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인

	while (!feof(fp) && !feof(cmpFp)){
		readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
		lineIdx++;

		readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;

		if((strcmp(readLine, readCmpLine) != 0) && (strcmp(options, "q") == 0)){ // 내용 다르고 q 옵션일경우 출력 후 함수 종료
			printf("Files %s and %s differ\n", oriPath, cmpPath);
			return;
		}
	}
	printf("Files %s and %s are identical\n", oriPath, cmpPath);

	fclose(fp);
	fclose(cmpFp);
}
void cmp_dir(int cmpIdx, struct fileLists *fileList, char *options){
	
}
// // 디렉토리 비교
// void cmp_dir(int cmpIdx, struct fileLists *filelist, char *options){
//     struct dirent **oriList, **cmpList;
//     int oriCnt, cmpCnt; // 원본 리스트 개수, 비교본 리스트 개수

// 	char subOriPath[BUF_SIZE]; // 원본파일 path (하위파일 잘라주기 용)
// 	strcpy(subOriPath, filelist[0].path);

// 	char subCmpPath[BUF_SIZE]; // 비교파일 path (하위파일 잘라주기 용)
// 	strcpy(subCmpPath, filelist[cmpIdx].path);

// 	// 다른 인덱스 찾기
// 	int diffIdx = 0; // 다른 인덱스
// 	for(int i = 0; i < strlen(subOriPath); i++){
// 		if(subOriPath[i] != subCmpPath[i]){
// 			diffIdx = i;
// 			break;
// 		}
// 	}
// 	memmove(subOriPath, subOriPath + diffIdx, strlen(subOriPath)); // 하위파일로 잘라주기
// 	memmove(subCmpPath, subCmpPath + diffIdx, strlen(subCmpPath)); // 하위파일로 잘라주기


// 	char oriPath[BUF_SIZE], cmpPath[BUF_SIZE]; // 원본 비교 경로(하위파일 합치기 용)
// 	strcpy(oriPath, filelist[0].path);
// 	strcpy(cmpPath, filelist[cmpIdx].path);

// 	// 에러 있을 경우 에러 처리
// 	if((oriCnt = scandir(filelist[0].path, &oriList, NULL, alphasort)) == -1){
// 		fprintf(stderr, "%s Directory Scan Error : %s \n", filelist[0].path, strerror(errno)); // todo : errno 설정
// 		return;
// 	}
// 	if((cmpCnt = scandir(filelist[cmpIdx].path, &cmpList, NULL, alphasort)) == -1){
// 		fprintf(stderr, "%s Directory Scan Error : %s \n", filelist[cmpIdx].path, strerror(errno)); // todo : errno 설정
// 		return;
// 	}

// 	for(int i = 0; i < oriCnt; i++){
//         // 현재디렉토리, 이전디렉토리 무시
//         if ((!strcmp(oriList[i]->d_name, ".")) || (!strcmp(oriList[i]->d_name, ".."))){
//             continue;
//         }

// 		// 원본 경로 + '/하위파일'
// 		strcat(oriPath, "/");
// 		strcat(oriPath, oriList[i]->d_name);		
// 		// 하위파일 자른 경로 + '/하위파일'
// 		strcat(subOriPath, "/");
// 		strcat(subOriPath, oriList[i]->d_name);

// 		// 비교본 하나씩 비교
// 		for(int j = 0; j < cmpCnt; j++){
// 			if ((!strcmp(cmpList[j]->d_name, ".")) || (!strcmp(cmpList[j]->d_name, ".."))){
// 				continue;
// 			}
// 			// 비교 경로 + '/하위파일'
// 			strcat(cmpPath, "/");
// 			strcat(cmpPath, oriList[j]->d_name);
// 			// 하위파일 자른 경로 + '/하위파일'
// 			strcat(subCmpPath, "/");
// 			strcat(subCmpPath, oriList[j]->d_name);

// 			// 이름 동일한경우
// 			if(strcmp(oriList[i]->d_name, cmpList[j]->d_name) == 0){
// 				// 파일 종류 다른 경우
// 				if(get_fileOrDir(oriPath) != get_fileOrDir(cmpPath)){
// 					printf("File %s is a %s while file %s is a %s\n", subOriPath, getfileStr(get_fileOrDir(oriPath)), subCmpPath, getfileStr(get_fileOrDir(cmpPath)));
// 					break;
// 				}
// 				// 내용 다른 경우 
// 				// todo : r 옵션
// 				// cmp_file(cmpIdx, filelist); // 파일 비교
// 			}
// 		}
// 		printf("-------\n");
// 	}
// }

int get_fileOrDir(char *path){
	struct stat st;
	int fileOrDir = 0;
	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("stat error");
		return -1;
	}

	// 파일 형식
	switch (st.st_mode & S_IFMT){
		case S_IFREG:
			fileOrDir = 1;
			break;
		case S_IFDIR:
			fileOrDir = 2;
			break;
	}
	return fileOrDir;
}

char *getfileStr(int fileOrDir){
	if(fileOrDir == 1) return "regular file";
	return "directory";
}