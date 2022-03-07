//todo : filename ./붙여주기 -> 절대경로 변환

#include <stdio.h>
#include <sys/types.h> // stat 사용
#include <sys/stat.h> // stat 사용
#include <unistd.h> // stat 사용
#include <string.h> // string 관련 함수 사용
#include <stdlib.h> // realpath 사용
#include <dirent.h> // scandir 사용
#include <errno.h> // errno 설정
#include <time.h> // strftime 사용
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
void find(char *findOper[FINDOPER_SIZE]){
	// 상대경로인 경우 절대경로로 변환(FILENAME)
	if(findOper[1][0] != '/'){
		char buf[OPER_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[1], buf) == NULL){
			perror("realpath error -> filename"); // todo : 전역변수 errno에 설정
			return;
		}
		printf("변환 결과 : %s\n", buf);
		findOper[0] = buf; // 변환한 절대경로 저장

		long long fileSize = find_info(findOper, findOper[0]); // 파일 크기 저장
		printf("file size : %lld\n", fileSize);
	}
	// 상대경로인 경우 절대경로로 변환(PATH)
	if(findOper[2][0] != '/'){
		char buf[OPER_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[2], buf) == NULL){
			perror("realpath error -> path"); // todo : 전역변수 errno에 설정
			return;
		}
		printf("%s\n", buf);
		findOper[2] = buf; // 변환한 절대경로 저장
	}

	// find_info(findOper); // 파일 정보 탐색

	// find_files(findOper);

}

// scandir 통한 디렉토리 전체 목록 조회 후 파일 정보 탐색
void find_files(char *findOper[FINDOPER_SIZE]){
	// scandir 관련 선언
	struct dirent **namelist;
	int cnt; // return 값

	if((cnt = scandir(findOper[2], &namelist, NULL, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", findOper[2], strerror(errno)); // todo : errno 설정
		return;
	}

	for(int i = 0; i < cnt; i++){
		// 이름 같은 파일/dir 발견할 경우(todo : FILENAME 경로이므로 수정해야함)
		if(strcmp(findOper[1], namelist[i]->d_name) == 0){
			// 파일 경로 합치기
			char *path = strcat(findOper[2], "/");
			path = strcat(path, namelist[i]->d_name);

			//todo: find_info
		}
	}

	for(int i = 0; i < cnt; i++){
		free(namelist[i]);
	}

	free(namelist);	
}

// 파일 정보 탐색
long long find_info(char *findOper[FINDOPER_SIZE], char *path){
	struct stat st;

	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("stat");
		return -1;
	}
	printf("file size : %lld bytes \n", (long long) st.st_size);
	printf("mode : %hu\n", st.st_mode); // 모드
	printf("block : %lld\n", st.st_blocks); // 할당된 블록 수
	printf("hardlink : %hu\n", st.st_nlink); // 하드링크
	printf("UID : %u\n", st.st_uid); // 사용자id
	printf("GID : %u\n", st.st_gid); // 그룹id
	printf("UID : %u\n", st.st_uid); // 사용자id

	char date[DATEFORMAT_SIZE];
	printf("access : %s\n", dateFormat(date, st.st_atimespec)); // 최종 접근 시간
	printf("change : %s\n", dateFormat(date, st.st_ctimespec)); // 최종 상태 변경 시간
	printf("modify : %s\n", dateFormat(date, st.st_mtimespec)); // 최종 수정 시간
	printf("path : %s\n", path); // 절대경로

	return (long long) st.st_size;
}

/*
void find_info(char *findOper[FINDOPER_SIZE]){
	// scandir 관련 선언
	struct dirent **namelist;
	int cnt; // return 값

	struct stat st;

	if((cnt = scandir(findOper[2], &namelist, NULL, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", findOper[2], strerror(errno)); // todo : errno 설정
		return;
	}

	for(int i = 0; i < cnt; i++){
		// 이름 같은 파일/dir 발견할 경우
		if(strcmp(findOper[1], namelist[i]->d_name) == 0){
			// 파일 경로 합치기
			char *path = strcat(findOper[2], "/");
			path = strcat(path, namelist[i]->d_name);

			// 파일 정보 얻기
			if(stat(path, &st) == -1){
				perror("stat");
				return;
			}
			printf("file size : %lld bytes \n", (long long) st.st_size);
			printf("mode : %hu\n", st.st_mode); // 모드
			printf("block : %lld\n", st.st_blocks); // 할당된 블록 수
			printf("hardlink : %hu\n", st.st_nlink); // 하드링크
			printf("UID : %u\n", st.st_uid); // 사용자id
			printf("GID : %u\n", st.st_gid); // 그룹id
			printf("UID : %u\n", st.st_uid); // 사용자id

			char date[DATEFORMAT_SIZE];
			printf("access : %s\n", dateFormat(date, st.st_atimespec)); // 최종 접근 시간
			printf("change : %s\n", dateFormat(date, st.st_ctimespec)); // 최종 상태 변경 시간
			printf("modify : %s\n", dateFormat(date, st.st_mtimespec)); // 최종 수정 시간
			printf("path : %s\n", path); // 절대경로
		}
	}

	for(int i = 0; i < cnt; i++){
		free(namelist[i]);
	}

	free(namelist);
}
*/

// 시간 정보 포맷에 맞게 변환
char *dateFormat(char * str, struct timespec st){
	const struct tm *stTime = localtime((const time_t *) &st);
	strftime(str, DATEFORMAT_SIZE, "%y-%m-%d %H:%M", stTime);
	return str;
}