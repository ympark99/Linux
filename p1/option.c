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

// 입력인자 :  파일/디렉토리 , 파일리스트, 리스트 크기, 최초 입력 경로(절대경로 변환), 입력한 파일 이름
void option(int fileOrDir, struct fileLists *fileList, int listSize, char *inputOper[FINDOPER_SIZE], char *oriFileName){
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
				bool inputOptions[OPTION_SIZE] = {false, }; // q s i r l 옵션 여부 확인 배열
				// 0 : q, 1 : s, 2 : i, 3 : r, 4 : l 옵션
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
					else if(strcmp(index_option[i], "l") == 0) // 추가기능 : l 옵션
						inputOptions[4] = true;						
					else{
						perror("OPTION 존재 x");// 이상한 값 입력시
						nextStep = false; // 다시 INDEX OPTION 입력
						break;
					}
				}

				if(nextStep){
					// 출력할 path입력한 FILENAME으로 바꿔주기
					strcpy(inputOper[1], oriFileName);

					// 비교경로 : 비교할 path - 최초 입력 경로
					int cutIdx = strlen(inputOper[2]); // 비교할 path에서 자를 인덱스
					strcpy(inputOper[2], fileList[cmpIdx].path);
					memmove(inputOper[2], inputOper[2] + cutIdx + 1, strlen(inputOper[2])); // '/'까지 제거하므로 +1

					if(fileOrDir == 1) // 파일인 경우 파일 비교 실행
						index_option[1] == NULL? // 옵션 입력 여부에따라 cmp_file(), cmp_fileOption 실행
							cmp_file(fileList[0].path, fileList[cmpIdx].path, false) : 
							cmp_fileOption(fileList[0].path, fileList[cmpIdx].path, inputOptions, false, inputOper);
					else if(fileOrDir == 2){	
						cmp_dir(inputOper, fileList[0].path, fileList[cmpIdx].path, inputOptions); // 디렉토리 비교 실행(입력 INDEX, ixd 0 path, 비교 path, 입력 옵션)
					}
					break;
				}
			}
		}
	}
}

// 파일 비교 (입력인자 : 기준 파일 경로, 비교 파일 경로, 대소문자 구분 없이 비교할지)
void cmp_file(char *oriPath, char *cmpPath, bool sameAlpha){
	char line[BUF_SIZE], cmpLine[BUF_SIZE];
	char *readLine, *readCmpLine;
	FILE *fp = fopen(oriPath, "r"); // 기준 파일
	FILE *cmpFp = fopen(cmpPath, "r"); // 비교할 파일

	int lineIdx = 0; // 기준 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인
	int startFtell, cmpStartFtell = 0;

	while (!feof(fp)){
		startFtell = ftell(fp); // 기준파일 다시 탐색시 시작할 라인(읽기 전)
		readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
		lineIdx++;

		// 기준 마지막 줄 끝난경우 -> 비교본이 남았으면 추가 처리
		if(readLine == NULL){
			int cmpStartLine = cmpLineIdx + 1; // 비교본은 아직 안읽었으므로 +1
			cmpStartFtell = ftell(cmpFp); // 현재 위치 저장

			// 마지막 인덱스 알기위해 null까지 읽음
			while (readCmpLine != NULL){
				readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
				cmpLineIdx++;
			}

			if(cmpStartLine > cmpLineIdx - 1) break;

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
			bool isLastNull = false; // 마지막줄 개행 없을경우 No newline 출력 x

			if(readCmpLine[strlen(readCmpLine) - 1] == '\n') isLastNull = true; // 개행 없으면 마지막으로 판정
			readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 처리했으므로 한 줄 추가
			if(feof(cmpFp) && !isLastNull) printf("\n\\ No newline at end of file\n"); // 마지막 줄인경우 출력
			break;
		}

		cmpStartFtell = ftell(cmpFp); // 비교파일 다시 탐색시 시작할 라인(읽기 전)
		readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;

		// 비교 파일 마지막 줄 끝난 경우 -> 기준이 남았으면 삭제 처리
		if(readCmpLine == NULL){
			//상황별 삭제 포맷 출력
			int startLine = lineIdx;

			// 마지막 인덱스 알기위해 null까지 읽음
			while (readLine != NULL){
				readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
				lineIdx++;
			}

			if(startLine > lineIdx - 1) break;
			
			// 상황별 삭제 포맷 출력
			if(startLine == lineIdx - 1)
				printf("%dd%d\n", startLine, cmpLineIdx - 1);
			else
				printf("%d,%dd%d\n", startLine, lineIdx - 1, cmpLineIdx - 1);
			// 삭제된 내용 출력
			fseek(fp, startFtell, SEEK_SET); // 기준 비교시작 라인으로 이동
			for(int i = startLine; i < lineIdx; i++){
				printf("< ");
				readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
				printf("%s", readLine);
			}
			bool isLastNull = false; // 마지막줄 개행 없을경우 No newline 출력 x
			if(readLine[strlen(readLine) - 1] == '\n') isLastNull = true; // 개행 있는지 체크
			readLine = fgets(line, BUF_SIZE, fp); // 처리했으므로 한 줄 추가
			if(feof(fp) && !isLastNull) printf("\n\\ No newline at end of file\n"); // 마지막 줄인경우 출력			
			break;
		}

		int lcsOriIdx = 0; // lcs로 구한 기준 라인 인덱스
		int lcsCmpIdx = 0; // lcs로 구한 비교 파일 라인 인덱스
		int lcsLength = 0; // lcs최대 인덱스

		bool isEdit = false; // 수정하는지 확인하는 함수(마지막 제외)
		
		if(cmp_str(readLine, readCmpLine, sameAlpha) != 0){ // 다르면 나올때까지 비교파일 탐색
			int startLine = lineIdx; // 기준파일 시작 라인(읽은 후)
			int cmpStartLine = cmpLineIdx; // 비교파일 시작 라인(읽은 후)
			bool doOriSeek = true;

			// 기준파일 비교 시작줄 나올떄까지 탐색
			while (!feof(cmpFp)){
				readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
				if(readCmpLine == NULL) break;
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

			// 삭제, 수정 : 비교 파일 끝까지 안나오면 기준 줄 끝까지 반복
			while (!feof(fp) && doOriSeek){
				int preReadFtell = ftell(fp); // 추가로 기준파일 읽기 전 위치 저장
				readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
				if(readLine == NULL) break;
				lineIdx++;
				fseek(cmpFp, cmpStartFtell, SEEK_SET); // 비교 시작 위치로 이동
				cmpLineIdx = cmpStartLine - 1; // 비교 시작 라인(읽기전)으로 초기화

				// 비교시작 라인부터 탐색
				while (!feof(cmpFp)){
					readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
					if(readCmpLine == NULL) break;
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
							fseek(fp, startFtell, SEEK_SET); // 기준 비교시작 라인으로 이동
							for(int i = startLine; i < lineIdx; i++){
								printf("< ");
								readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
								printf("%s", readLine);
							}			
							readLine = fgets(line, BUF_SIZE, fp); // 처리했으므로 한 줄 추가
							// 기준 반복 탈출
							doOriSeek = false;
							break;

						}
						// 수정 처리
						else{					
							// lcs 구하기
							int lcsNow = 1; // 내용이 같은 라인 수
							int bufFtell = ftell(fp); // 기준 위치 저장
							int cmpBufFtell = ftell(cmpFp); // 비교 위치 저장

							bool keepLcs = true; // 공백일 경우 계속 계산 진행할지 판별
							// 공백일경우 다음 줄도 일치해야 lcs 구함
							if(strcmp(readCmpLine, "\n") == 0){
								while (!feof(fp) && !feof(cmpFp)){			
									readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
									readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기

									// 마지막 줄 개행없는 경우
									if(readLine == NULL){
										// 비교 파일 마지막줄, 기준 마지막줄이므로 lcs + 1
										if(readCmpLine == NULL) lcsNow++;
										fseek(fp, preReadFtell, SEEK_SET); // 기준파일 되돌리기
										readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
										break;
									}
									if(readCmpLine == NULL){
										break;
									}		
									// 두 파일 다 다음 줄도 공백일 경우 : 한번 더 테스트
									if(cmp_str(readLine, readCmpLine, sameAlpha) == 0 && strcmp(readLine, "\n") == 0){
										lcsNow++;
									}
									// 다음 줄이 일치하고 공백이 아닐경우 진행
									else if(cmp_str(readLine, readCmpLine, sameAlpha) == 0 && strcmp(readLine, "\n") != 0){
										lcsNow++;
										keepLcs = true;
										break;
									}
									else{ // 다음 줄이 일치하지 않을경우 진행 x -> 기준 라인은 그대로, 비교라인만 계속 비교
										keepLcs = false;
										strcpy(readLine, "\n"); // 기준라인 그대로여야 하므로 \n
										fseek(fp, bufFtell, SEEK_SET);
										fseek(cmpFp, cmpBufFtell, SEEK_SET);					
										break;
									}
								}					
							}
							if(keepLcs){						
								// 일치할경우 내용같은 라인 + 1
								while (!feof(fp) && !feof(cmpFp)){						
									readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
									readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
									// 마지막 줄 개행 없는 경우 readLine을 그대로 돌려놔야함
									if(readLine == NULL){
										fseek(fp, preReadFtell, SEEK_SET); // 기준파일 되돌리기
										readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
										break;
									}
									if(readCmpLine == NULL){
										break;										
									}
									// 기준과 비교 파일 내용이 일치할경우
									if(cmp_str(readLine, readCmpLine, sameAlpha) == 0){
										lcsNow++; // 내용같은라인 + 1				
									}
								}
								fseek(fp, bufFtell, SEEK_SET);
								fseek(cmpFp, cmpBufFtell, SEEK_SET);
								// 가장 일치하는 라인이 많은경우 lcs 등록
								if(lcsNow > lcsLength){
									lcsLength = lcsNow;
									lcsOriIdx = lineIdx;
									lcsCmpIdx = cmpLineIdx;
								}
							}
							isEdit = true;
						}
					}
				}
			}
			// 수정 처리			
			if(doOriSeek){
				if(!isEdit && (feof(fp) && feof(cmpFp))){ // 파일 전체 끝난경우, 라인 저장
					lcsCmpIdx = cmpLineIdx + 1;
					lcsOriIdx = lineIdx + 1;					
				}
				cmpLineIdx = lcsCmpIdx;
				lineIdx = lcsOriIdx;				

                // 상황별 수정 포맷 출력
                if(startLine == (lineIdx - 1) && cmpStartLine == (cmpLineIdx - 1))
                    printf("%dc%d\n", startLine, cmpStartLine);
                else if(startLine == (lineIdx - 1) && cmpStartLine != (cmpLineIdx - 1))
                    printf("%dc%d,%d\n", startLine, cmpStartLine, cmpLineIdx - 1);
                else if(startLine != (lineIdx - 1) && cmpStartLine == (cmpLineIdx - 1))
                    printf("%d,%dc%d\n", startLine, lineIdx - 1, cmpStartLine);
                else printf("%d,%dc%d,%d\n", startLine, lineIdx - 1, cmpStartLine, cmpLineIdx - 1);		

                // 수정 내용 출력 : 기준
				fseek(fp, startFtell, SEEK_SET); // 기준 비교시작 라인으로 이동
                for(int i = startLine; i < lineIdx; i++){
                    printf("< ");
                    readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
                    printf("%s", readLine);
                }

				bool isLastNull = false; // 마지막줄 개행 없을경우 No newline 출력 x
				if(readLine[strlen(readLine) - 1] == '\n') isLastNull = true;

                readLine = fgets(line, BUF_SIZE, fp); // 처리했으므로 한 줄 추가

				if(feof(fp) && !isLastNull) printf("\n\\ No newline at end of file\n"); // 마지막 줄인경우 출력
                printf("---\n");
                // 수정 내용 출력 : 비교
                fseek(cmpFp, cmpStartFtell, SEEK_SET); // 비교 시작 위치로 이동
                for(int i = cmpStartLine; i < cmpLineIdx; i++){
                    printf("> ");
                    readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
                    printf("%s", readCmpLine);
                }

				isLastNull = false;
				if(readCmpLine[strlen(readCmpLine) - 1] == '\n') isLastNull = true; // 개행 문자 있는지 체크

                readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 처리했으므로 한 줄 추가

				if(feof(cmpFp) && !isLastNull) printf("\n\\ No newline at end of file\n"); // 마지막 줄인경우 출력
			}
		}
	}
	fclose(fp);
	fclose(cmpFp);
}

// 파일, 디렉토리 옵션 (fromDir : 0 -> ssu_index에서, 1 -> 디렉토리 함수에서)
// 입력 인자(기준 파일 경로, 비교 파일 경로, 옵션 배열, 디렉토리에서 호출됐는지 체크, diff 메시지 출력할 기준/비교 파일 문자열 배열)
void cmp_fileOption(char *oriPath, char *cmpPath, bool options[OPTION_SIZE], bool fromDir, char *inputOper[FINDOPER_SIZE]){
	bool isSame = true; // 파일내용이 같은지 확인하는 변수

	char line[BUF_SIZE], cmpLine[BUF_SIZE];
	char *readLine, *readCmpLine;

	// 추가기능 : l 옵션인 경우, 라인 줄 수 비교 및 출력
	if(options[4]) cmp_length(oriPath, cmpPath, inputOper);
	
	FILE *fp = fopen(oriPath, "r"); // 기준 파일
	FILE *cmpFp = fopen(cmpPath, "r"); // 비교할 파일

	int lineIdx = 0; // 기준 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인

	while (!feof(fp) && !feof(cmpFp)){
		readLine = fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
		lineIdx++;

		readCmpLine = fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;

		if(cmp_str(readLine, readCmpLine, options[2]) != 0){ // 내용 다를경우
			// 디렉토리에서 호출했을 경우 출력 후 리턴
			if(fromDir){
				// r 옵션
				if(options[3]){
					if(options[0]) printf("Files %s and %s differ\n", inputOper[1], inputOper[2]); // r, q 옵션
					else if(options[2]) printf("diff -i -r %s %s\n", inputOper[1], inputOper[2]); // r, i 옵션		
					else printf("diff %s %s\n", inputOper[1], inputOper[2]); // r 옵션			
				}
				else{
					if(options[0]) printf("Files %s and %s differ\n", inputOper[1], inputOper[2]); // q 옵션
					else if(options[2]) printf("diff -i %s %s\n", inputOper[1], inputOper[2]); // i 옵션
					else printf("diff %s %s\n", inputOper[1], inputOper[2]); // 옵션 x
				}
				return;
			}
			// 파일 옵션인경우
			// q 옵션일경우 출력 후 리턴
			if(options[0]){
				printf("Files %s and %s differ\n", inputOper[1], inputOper[2]);
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
			printf("Files %s and %s are identical\n", inputOper[1], inputOper[2]);
			return;
		}
		printf("Files %s and %s are identical\n", inputOper[1], inputOper[2]);
	}
	fclose(fp);
	fclose(cmpFp);
}

// 추가기능 : l옵션 -> 두 파일의 라인 수(set nu)를 출력하고, 비교 메시지 출력
// 입력인자 : 기준 파일 경로, 비교 파일 경로, diff 메시지에 출력할 기준/비교 경로
void cmp_length(char *oriPath, char *cmpPath, char *inputOper[FINDOPER_SIZE]){
	FILE *fp = fopen(oriPath, "r"); // 기준 파일
	FILE *cmpFp = fopen(cmpPath, "r"); // 비교할 파일

	char line[BUF_SIZE], cmpLine[BUF_SIZE];

	int lineIdx = 0; // 기준 현재 라인
	int cmpLineIdx = 0; // 비교파일 현재 라인
	
	while (!feof(fp)){
		fgets(line, BUF_SIZE, fp); // 기준파일 한 줄 읽기
		lineIdx++;
	}

	while (!feof(cmpFp)){
		fgets(cmpLine, BUF_SIZE, cmpFp); // 비교파일 한 줄 읽기
		cmpLineIdx++;
	}

	// 결과 출력
	printf("%s lines: %d, %s lines: %d\n", inputOper[1], lineIdx, inputOper[2], cmpLineIdx);
	// 비교결과 출력
	if(lineIdx > cmpLineIdx)
		printf("File %s is longer than %s\n", inputOper[1], inputOper[2]);
	else if(lineIdx == cmpLineIdx)
		printf("Files %s and %s have same line length!\n", inputOper[1], inputOper[2]);
	else
		printf("File %s is shorter than %s\n", inputOper[1], inputOper[2]);

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

// 디렉토리 비교 (입력인자 : 출력할 기준/비교 파일 경로, 조회할 기준 파일 경로, 조회할 비교 파일 경로, 옵션 배열)
void cmp_dir(char *inputOper[FINDOPER_SIZE], char *oriPath, char *cmpPath, bool options[OPTION_SIZE]){
    struct dirent **oriList, **cmpList;
    int oriCnt, cmpCnt; // 기준 리스트 개수, 비교본 리스트 개수

	// 에러 있을 경우 에러 처리
	if((oriCnt = scandir(oriPath, &oriList, scanFilter, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", oriPath, strerror(errno));
		return;
	}
	if((cmpCnt = scandir(cmpPath, &cmpList, scanFilter, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", cmpPath, strerror(errno));
		return;
	}

	char cpName[BUF_SIZE]; // 디렉터리명 복사본
	strcpy(cpName, inputOper[2]); // 비교본(디렉터리명 복사)

	// 하위파일 개수만큼 체크 배열 생성 -> 체크 안될경우 only in
	bool* cmpCheck = (bool *)malloc(sizeof(bool) * cmpCnt);
	for(int i = 0; i < cmpCnt; i++)
		cmpCheck[i] = false;

	// 기준 디렉토리 한번씩 순회
	for(int i = 0; i < oriCnt; i++){
		// 기준 경로 + '/하위파일'
		strcat(oriPath, "/");
		strcat(oriPath, oriList[i]->d_name);		

		// 기준dir이름 + '/하위파일'
		strcat(inputOper[1], "/");
		strcat(inputOper[1], oriList[i]->d_name);

		bool isCheck = false; // 기준 출력 됐는지 확인

		// 비교 디렉토리 한번씩 순회
		for(int j = 0; j < cmpCnt; j++){
			// 비교 경로 + '/하위파일'
			strcat(cmpPath, "/");
			strcat(cmpPath, cmpList[j]->d_name);

			// 비교dir이름 + '/하위파일'
			strcat(inputOper[2], "/");
			strcat(inputOper[2], cmpList[j]->d_name);

			// 이름 동일한경우
			if(strcmp(oriList[i]->d_name, cmpList[j]->d_name) == 0){
				int result_ori = get_fileOrDir(oriPath); // 기준 파일 or 디렉토리인지
				int result_cmp = get_fileOrDir(cmpPath); // 비교본 파일 or 디렉토리인지
				// 파일 종류 다른 경우
				if(result_ori != result_cmp){
					printf("File %s is a %s while file %s is a %s\n", inputOper[1], getfileStr(result_ori), inputOper[2], getfileStr(result_cmp));
				}
				// 파일이 디렉토리인경우
				else if((result_ori == 2) && (result_cmp == 2)){
					// r 옵션인경우 재귀 비교 실행
					if(options[3]){
						cmp_dir(inputOper, oriPath, cmpPath, options);
					}
					else printf("Common subdirectories: %s and %s\n", inputOper[1], inputOper[2]);
				}
				// 파일 비교 후 내용 다르면 diff 출력
				else{
					cmp_fileOption(oriPath, cmpPath, options, true, inputOper); // 파일 단순 비교, 다를 경우 diff 출력
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
				// 기준보다 비교본 파일이름이 아스키 앞 && 비교본 체크 x -> only in
				if(isCmpFirst(oriList[i]->d_name, cmpList[j]->d_name) && (cmpCheck[j] == false)){
					printf("Only in %s: %s\n", cpName, cmpList[j]->d_name);
					cmpCheck[j] = true;
				}
			}

			// 합쳤던 하위파일명 문자열 제거 : 비교본
			char* ptr = strrchr(cmpPath, '/'); // 합쳤던 /하위파일명 포인터 연결
			if(ptr){
				strncpy(ptr, "", 1); // 합쳤던 문자열 제거
			}
			// 합쳤던 하위파일명 문자열 제거 : 입력비교본
			char* ptr2 = strrchr(inputOper[2], '/'); // 합쳤던 /하위파일명 포인터 연결
			if(ptr2){
				strncpy(ptr2, "", 1); // 합쳤던 문자열 제거
			}			
		}
		// 합쳤던 하위파일명 문자열 제거 : 기준
		char* ptr = strrchr(oriPath, '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr){
			strncpy(ptr, "", 1); // 합쳤던 문자열 제거
		}
		// 합쳤던 하위파일명 문자열 제거 : 입력기준
		char* ptr2 = strrchr(inputOper[1], '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr2){
			strncpy(ptr2, "", 1); // 합쳤던 문자열 제거
		}		
		// 출력 안된경우 기준에만 존재
		if(!isCheck){
			printf("Only in %s: %s\n", inputOper[1], oriList[i]->d_name);
		}		
	}
	
	for(int i = 0; i < cmpCnt; i++){
		// 체크 안된 비교본 있을경우 only in 출력
		if(!cmpCheck[i]){
			printf("Only in %s: %s\n", inputOper[2], cmpList[i]->d_name);
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
		// 기준이 아스키코드 뒤일경우 true
		if(cmpName[i] < oriName[i]) return true;
		else if(oriName[i] < cmpName[i]) return false;
	}
	// 기준 이름이 비교본 포함한경우
	if(oriMoreLong) return true;
	return false;
}

// 파일, 디렉토리 판별 함수(파일 : 1, 디렉토리 : 2 리턴)
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

// 파일 : regular file, 디렉토리 : directory 리턴
char *getfileStr(int fileOrDir){
	if(fileOrDir == 1) return "regular file";
	return "directory";
}