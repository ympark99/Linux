#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "option.h"

//todo : 마지막줄 공백인 경우 처리 (fopen != NULL 등 이용)
//todo : 디렉토리 -> 파일 비교시 여러줄 수정 처리됨
//todo : cmpfile diff 출력 경로 : 그냥 findoper[2]넘기면 될듯

//todo : 파일 마지막줄 공백

void option(int fileOrDir, struct fileLists *fileList, int listSize, char *inputPath){
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
		bool nextStep = true; // 계속 진행할지 판단
		// [0] : INDEX , [1] ~ [4] : OPTION or NULL
		while (ptr != NULL){
			if(idx < IDXOPTION_SIZE) index_option[idx] = ptr;
			else{
				perror("option 입력 초과");
				nextStep = false;
				break;
			}
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
				bool inputOptions[OPTION_SIZE] = {false, }; // q s i r 옵션 여부 확인 배열
				for(int i = 1; i < IDXOPTION_SIZE; i++){
					if(index_option[i] == NULL) continue;
					if(strcmp(index_option[i], "q") == 0)
						inputOptions[0] = true;
					else if(strcmp(index_option[i], "s") == 0)
						inputOptions[1] = true;
					else if(strcmp(index_option[i], "i") == 0)
						inputOptions[2] = true;
					else if(strcmp(index_option[i], "r") == 0)
						inputOptions[3] = true;
					else{
						perror("OPTION 존재 x");// 이상한 값 입력시
						nextStep = false;
						break;
					}
				}

				if(nextStep){
					if(fileOrDir == 1) // 파일인 경우 파일 비교 실행
						index_option[1] == NULL? cmp_file(fileList[0].path, fileList[cmpIdx].path, false) : cmp_fileOption(fileList[0].path, fileList[cmpIdx].path, inputOptions, false, NULL);
					else if(fileOrDir == 2){
						// inputPath 절대경로이므로 디렉토리이름으로 바꿔주기
						char* ptr = strrchr(inputPath, '/'); // 합쳤던 /하위파일명 포인터 연결
						strcpy(inputPath, ptr);
						memmove(inputPath, inputPath + 1, strlen(inputPath)); // 맨앞'/'제거						
						cmp_dir(inputPath, fileList[0].path, fileList[cmpIdx].path, inputOptions); // 디렉토리 비교 실행(입력 INDEX, ixd 0 path, 비교 path, 입력 옵션)
					}
					break;
				}
			}
		}
	}
}

// 파일 비교
void cmp_file(char *oriPath, char *cmpPath, bool sameAlpha){
	char line[BUF_SIZE], cmpLine[BUF_SIZE];
	char *readLine, *readCmpLine;
	FILE *fp = fopen(oriPath, "r"); // 원본 파일
	FILE *cmpFp = fopen(cmpPath, "r"); // 비교할 파일

	// FILE *fp = fopen("test/e.txt", "r"); // 테스트 원본 파일
	// FILE *cmpFp = fopen("test/f.txt", "r"); // 테스트 비교할 파일
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
		
		if(cmp_str(readLine, readCmpLine, sameAlpha) != 0){ // 다르면 나올때까지 비교파일 탐색
			int startLine = lineIdx; // 원본파일 시작 라인(읽은 후)
			int cmpStartLine = cmpLineIdx; // 비교파일 시작 라인(읽은 후)
			bool doOriSeek = true;

			// 원본파일 비교 시작줄 나올떄까지 탐색
			while (!feof(cmpFp)){
				readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
				cmpLineIdx++;

				// 추가 처리 : 탐색하다 같은 줄 나온 경우
				if(cmp_str(readLine, readCmpLine, sameAlpha) == 0){
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
					if(cmp_str(readLine, readCmpLine, sameAlpha) == 0){
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

									if(cmp_str(readLine, readCmpLine, sameAlpha) == 0 && strcmp(readLine, "\n") != 0){
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
	fclose(fp);
	fclose(cmpFp);
}

// 파일, 디렉토리 옵션 (fromDir : 0 -> ssu_index에서, 1 -> 디렉토리 함수에서)
void cmp_fileOption(char *oriPath, char *cmpPath, bool options[OPTION_SIZE], bool fromDir, char *printPath){
	bool isSame = true; // 같은지 확인
	char subOriPath[BUF_SIZE]; // 원본파일 path
	strcpy(subOriPath, oriPath);

	char subCmpPath[BUF_SIZE]; // 비교파일 path
	strcpy(subCmpPath, cmpPath);

	// 다른 인덱스 찾기
	int diffIdx = 0; // 다른 인덱스
	for(int i = 0; i < strlen(subOriPath); i++){
		if(subOriPath[i] != subCmpPath[i]){
			diffIdx = i;
			break;
		}
	}
	memmove(subOriPath, subOriPath + diffIdx, strlen(subOriPath)); // 하위파일로 잘라주기
	memmove(subCmpPath, subCmpPath + diffIdx, strlen(subCmpPath)); // 하위파일로 잘라주기

	char line[BUF_SIZE], cmpLine[BUF_SIZE];
	char *readLine, *readCmpLine;
	
	FILE *fp = fopen(oriPath, "r"); // 원본 파일
	FILE *cmpFp = fopen(cmpPath, "r"); // 비교할 파일

	int lineIdx = 0; // 원본 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인

	while (!feof(fp) && !feof(cmpFp)){
		readLine = fgets(line, BUF_SIZE, fp); // 원본파일 한 줄 읽기
		lineIdx++;

		readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;

		if(cmp_str(readLine, readCmpLine, options[2]) != 0){ // 내용 다를경우
			// 디렉토리에서 호출했을 경우 출력 후 리턴
			if(fromDir){
				// r 옵션
				if(options[3]){
					if(options[0]) printf("Files %s and %s differ\n", printPath, subCmpPath); // r, q 옵션
					else if(options[2]) printf("diff -i -r %s %s\n", printPath, subCmpPath); // r, i 옵션		
					else printf("diff %s %s\n", printPath, subCmpPath); // r 옵션			
				}
				else{
					if(options[0]) printf("Files %s and %s differ\n", printPath, subCmpPath); // q 옵션
					else if(options[2]) printf("diff -i %s %s\n", printPath, subCmpPath); // i 옵션
					else printf("diff %s %s\n", printPath, subCmpPath); // 옵션 x
				}
				return;
			}
			// 파일 옵션인경우
			// q 옵션일경우 출력 후 리턴
			if(options[0]){
				printf("Files %s and %s differ\n", subOriPath, subCmpPath);
				return;
			}		
			// i 옵션일경우 다른 내용 출력
			if(options[2]){
				cmp_file(oriPath, cmpPath, true);
				return;
			}
			// s 옵션인경우 다른 내용 출력
			else if(options[1]){
				cmp_file(oriPath, cmpPath, false);
				return;
			}
			isSame = false;
		}
	}
	if(isSame && options[1]){  // s옵션인경우 출력
		if(fromDir){
			printf("Files %s and %s are identical\n", printPath, subCmpPath);
			return;
		}
		printf("Files %s and %s are identical\n", subOriPath, subCmpPath);
	}

	fclose(fp);
	fclose(cmpFp);
}

// strcmp or strcasecmp 진행, sameAlpha 일경우 대소문자 비교 x
int cmp_str(char *str1, char *str2, bool sameAlpha){
	if(sameAlpha) return strcasecmp(str1, str2);
	return strcmp(str1, str2);
}

// scandir 필터(. 과 .. 제거)
int scanFilter(const struct dirent *info){
	if(strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0){
		return 0; // .이나 .. 이면 filter
	}
	else return 1;
}

// 디렉토리 비교
void cmp_dir(char *inputDir, char *oriPath, char *cmpPath, bool options[OPTION_SIZE]){	
    struct dirent **oriList, **cmpList;
    int oriCnt, cmpCnt; // 원본 리스트 개수, 비교본 리스트 개수

	//todo : 입력하위파일로 바꿔야함(다른곳도)
	char subOriPath[BUF_SIZE]; // 원본파일 path (하위파일 잘라주기 용)
	strcpy(subOriPath, oriPath);

	char subCmpPath[BUF_SIZE]; // 비교파일 path (하위파일 잘라주기 용)
	strcpy(subCmpPath, cmpPath);

	// 다른 인덱스 찾기
	int diffIdx = 0; // 다른 인덱스
	for(int i = 0; i < strlen(subOriPath); i++){
		if(subOriPath[i] != subCmpPath[i]){
			diffIdx = i;
			break;
		}
	}
	memmove(subOriPath, subOriPath + diffIdx, strlen(subOriPath)); // 하위파일로 잘라주기
	memmove(subCmpPath, subCmpPath + diffIdx, strlen(subCmpPath)); // 하위파일로 잘라주기

	// 에러 있을 경우 에러 처리
	if((oriCnt = scandir(oriPath, &oriList, scanFilter, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", oriPath, strerror(errno)); // todo : errno 설정
		return;
	}
	if((cmpCnt = scandir(cmpPath, &cmpList, scanFilter, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", cmpPath, strerror(errno)); // todo : errno 설정
		return;
	}

	// 하위파일 개수만큼 체크 배열 생성 -> 체크 안될경우 only in
	bool* cmpCheck = (bool *)malloc(sizeof(bool) * cmpCnt);
	for(int i = 0; i < cmpCnt; i++)
		cmpCheck[i] = false;

	// 원본 디렉토리 한번씩 순회
	for(int i = 0; i < oriCnt; i++){
		// 원본 경로 + '/하위파일'
		strcat(oriPath, "/");
		strcat(oriPath, oriList[i]->d_name);		
		// 하위파일 자른 경로 + '/하위파일' : printf용
		strcat(subOriPath, "/");
		strcat(subOriPath, oriList[i]->d_name);
		// 원본dir이름 + '/하위파일'
		strcat(inputDir, "/");
		strcat(inputDir, oriList[i]->d_name);

		bool isCheck = false; // 원본 출력 됐는지 확인

		// 비교 디렉토리 한번씩 순회
		for(int j = 0; j < cmpCnt; j++){
			// 비교 경로 + '/하위파일'
			strcat(cmpPath, "/");
			strcat(cmpPath, cmpList[j]->d_name);
			// 하위파일 자른 경로 + '/하위파일' : printf용
			strcat(subCmpPath, "/");
			strcat(subCmpPath, cmpList[j]->d_name);

			// 이름 동일한경우
			if(strcmp(oriList[i]->d_name, cmpList[j]->d_name) == 0){
				// printf("ori : %s\n", oriPath);
				// printf("cmp : %s\n", cmpPath);
				int result_ori = get_fileOrDir(oriPath); // 원본 파일 or 디렉토리인지
				int result_cmp = get_fileOrDir(cmpPath); // 비교본 파일 or 디렉토리인지
				// 파일 종류 다른 경우
				if(result_ori != result_cmp){
					printf("File %s is a %s while file %s is a %s\n", inputDir, getfileStr(result_ori), subCmpPath, getfileStr(result_cmp));
				}
				// 파일이 디렉토리인경우
				else if((result_ori == 2) && (result_cmp == 2)){
					// r 옵션인경우 재귀 비교 실행
					if(options[3]){
						cmp_dir(inputDir, oriPath, cmpPath, options);
					}
					else printf("Common subdirectories: %s and %s\n", inputDir, subCmpPath);
				}
				// 파일 비교 후 내용 다르면 diff 출력
				else{
					cmp_fileOption(oriPath, cmpPath, options, true, inputDir); // 파일 단순 비교, 다를 경우 diff 출력
					if(!options[0]){ // q 옵션 아닐 경우만 내용 비교
						if(options[2]) // i 옵션인 경우
							cmp_file(oriPath, cmpPath, true); // 파일 비교					
						else
							cmp_file(oriPath, cmpPath, false); // 파일 비교
					}
				}
				cmpCheck[j] = true;
				isCheck = true; // 출력 됐으므로 true
			}
			else{ // 이름 다른 경우
				// 원본보다 비교본 파일이름이 아스키 앞 && 비교본 체크 x -> only in
				if(isCmpFirst(oriList[i]->d_name, cmpList[j]->d_name) && (cmpCheck[j] == false)){
					printf("Only in %s: %s\n", subCmpPath, cmpList[j]->d_name);
					cmpCheck[j] = true;
				}
			}

			// 합쳤던 하위파일명 문자열 제거 : 비교본
			char* ptr = strrchr(cmpPath, '/'); // 합쳤던 /하위파일명 포인터 연결
			if(ptr){
				strncpy(ptr, "", 1); // 합쳤던 문자열 제거
			}
			// 합쳤던 sub 문자열 제거 : 비교본
			char* ptr2 = strrchr(subCmpPath, '/'); // 합쳤던 /하위파일명 포인터 연결
			if(ptr2){
				strncpy(ptr2, "", 1); // 합쳤던 문자열 제거
			}					
		}
		// 합쳤던 하위파일명 문자열 제거 : 원본
		char* ptr = strrchr(oriPath, '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr){
			strncpy(ptr, "", 1); // 합쳤던 문자열 제거
		}			
		// 합쳤던 sub 문자열 제거 : 원본
		char* ptr2 = strrchr(subOriPath, '/'); // 합쳤던 /하위파일명 포인터 연결				
		if(ptr2){
			strncpy(ptr2, "", 1); // 합쳤던 문자열 제거
		}
		// 합쳤던 하위파일명 문자열 제거 : 입력원본
		char* ptr3 = strrchr(inputDir, '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr3){
			strncpy(ptr3, "", 1); // 합쳤던 문자열 제거
		}		
		// 출력 안된경우 원본에만 존재
		if(!isCheck){
			printf("Only in %s: %s\n", inputDir, oriList[i]->d_name);
		}		
	}
	
	for(int i = 0; i < cmpCnt; i++){
		// 체크 안된 비교본 있을경우 only in 출력
		if(!cmpCheck[i]){
			printf("Only in %s: %s\n", subCmpPath, cmpList[i]->d_name);
			cmpCheck[i] = true;
		}	
	}

	for(int i = 0; i < oriCnt; i++){
		free(oriList[i]);
	}
	free(oriList);

	for(int i = 0; i < cmpCnt; i++){
		free(cmpList[i]);
	}
	free(cmpList);	

	free(cmpCheck);
}

// 아스키코드 순서 비교
bool isCmpFirst(char *oriName, char *cmpName){
	bool oriMoreLong = (strlen(cmpName) < strlen(oriName)) ? true : false;
	int num = oriMoreLong ? strlen(cmpName) : strlen(oriName);

	for(int i = 0; i < num; i++){
		// 원본이 아스키코드 뒤일경우 true
		if(cmpName[i] < oriName[i]) return true;
		else if(oriName[i] < cmpName[i]) return false;
	}
	// 원본 이름이 비교본 포함한경우
	if(oriMoreLong) return true;
	return false;
}

int get_fileOrDir(char *path){
	struct stat st;
	int fileOrDir = 0;
	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("get_ : stat error");
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