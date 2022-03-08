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

struct fileLists fileList[LMAX]; // 출력 리스트 구조체 선언
int listIdx = 0; // 출력 리스트 index

void ssu_sindex(){
	while (1){
		char *oper = malloc(sizeof(char) * BUF_SIZE);
		printf("20182615> "); // 프롬프트 출력
		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
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

		// find 명령 시 find_first 함수 실행
		if(findOper[0] != NULL && strcmp(findOper[0], "find") == 0){
			// 명령어 인자 틀리면 에러 처리
			if(idx != FINDOPER_SIZE){
				printf("error\n");
			}
			else{
				find_first(findOper);
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
void find_first(char *findOper[FINDOPER_SIZE]){
	// 상대경로인 경우 절대경로로 변환(원본 FILENAME)
	if(findOper[1][0] != '/'){
		char buf[BUF_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[1], buf) == NULL){
			perror("realpath error -> filename"); // todo : 전역변수 errno에 설정
			return;
		}
		findOper[1] = buf; // 변환한 절대경로 저장

	}
	// 상대경로인 경우 절대경로로 변환(PATH)
	if(findOper[2][0] != '/'){
		char buf[BUF_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[2], buf) == NULL){
			perror("realpath error -> path"); // todo : 전역변수 errno에 설정
			return;
		}
		printf("%s\n", buf);
		findOper[2] = buf; // 변환한 절대경로 저장
	}

	printf("Index Size Mode       Blocks Links UID  GID  Access         Change         Modify         Path\n");
	save_fileInfo(findOper[1]); // 원본 파일(디렉토리) 리스트에 저장	
	print_fileInfo(); // 리스트 출력
	listIdx++;

	char *fileName = strrchr(findOper[1], '/'); // / 들어간 마지막 위치 반환
	long long fileSize = get_fileSize(findOper[1]); // 비교할 파일 크기 저장

	// 최초입력 PATH 에러 검사
	struct dirent **namelist; // scandir 관련 선언
	int cnt; // return 값

	// 에러 있을 경우 에러 처리
	if((cnt = scandir(findOper[2], &namelist, NULL, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", findOper[2], strerror(errno)); // todo : errno 설정
		return;
	}
	// printf("%s\n", namelist[0]->d_name);
	free(namelist);
	dfs_findMatchFiles(findOper[2], fileName, fileSize); // PATH부터 디렉토리 탐색 & 리스트 저장
}

// scandir 통한 디렉토리 전체 목록 조회 후 파일 정보 탐색(dfs)
// 비교 파일 절대경로, / + 원본 파일 이름, 파일크기
void dfs_findMatchFiles(char *comparePath, char *fileName, long long fileSize){
	// scandir 관련 선언
	struct dirent **namelist;
	int cnt; // return 값

	// dfs -> 디렉토리 아닐 경우 리턴
	if((cnt = scandir(comparePath, &namelist, NULL, alphasort)) == -1){
		return;
	}
	// 전체 목록 search
	for(int i = 0; i < cnt; i++){
		// path : 비교파일절대경로/하위파일명 으로 합친 문자열
		char *path = malloc(sizeof(char) * BUF_SIZE);
		// char *path = strcat(comparePath, "/");
		strcpy(path, comparePath);
		path = strcat(path, "/");
		path = strcat(path, namelist[i]->d_name);	

		// cmpFileName : d_name 앞에 / 붙여준 문자열
		char *cmpFileName = malloc(sizeof(char) * BUF_SIZE); // strcat 위한 충분한 사이즈 할당
		strcpy(cmpFileName, "/"); // fileName 앞에 / 붙어있으므로
		
		// 이름 같은 파일/dir 발견할 경우
		if(strcmp(fileName, strcat(cmpFileName, namelist[i]->d_name)) == 0){
			// 파일/dir 크기 같으면 리스트 등록
			if(fileSize == get_fileSize(path)){
				save_fileInfo(path); // 리스트에 등록
				print_fileInfo();
				listIdx++;
			};
		}
		// dfs해주기
		// dfs_findMatchFiles(path, fileName, fileSize);
		free(path);
		free(cmpFileName);
	}

	for(int i = 0; i < cnt; i++){
		free(namelist[i]);
	}
	free(namelist);

	// 배열에 저장한 파일들 하나씩 실행
	return;
}

// 파일 크기 리턴
long long get_fileSize(char *path){
	struct stat st;

	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("stat error");
		return -1;
	}

	return (long long) st.st_size;
}

// 파일 정보 리스트에 저장
void save_fileInfo(char *path){
	struct stat st;

	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("stat error");
		return;
	}

	char date[DATEFORMAT_SIZE];

	fileList[listIdx].idx = listIdx;
	fileList[listIdx].size = (long long) st.st_size;
	fileList[listIdx].mode = st.st_mode;
	fileList[listIdx].blocks = st.st_blocks;
	fileList[listIdx].links = st.st_nlink;
	fileList[listIdx].uid = st.st_uid;
	fileList[listIdx].gid = st.st_gid;
	strcpy(fileList[listIdx].access, dateFormat(date, st.st_atimespec));
	strcpy(fileList[listIdx].change, dateFormat(date, st.st_ctimespec));
	strcpy(fileList[listIdx].modify, dateFormat(date, st.st_mtimespec));
	fileList[listIdx].path = path;

	// printf("size : %lld bytes \n", (long long) st.st_size); // 파일 크기
	// printf("mode : %hu\n", st.st_mode); // 모드
	// printf("block : %lld\n", st.st_blocks); // 할당된 블록 수
	// printf("hardlink : %hu\n", st.st_nlink); // 하드링크
	// printf("UID : %u\n", st.st_uid); // 사용자id
	// printf("GID : %u\n", st.st_gid); // 그룹id
	// printf("access : %s\n", dateFormat(date, st.st_atimespec)); // 최종 접근 시간
	// printf("change : %s\n", dateFormat(date, st.st_ctimespec)); // 최종 상태 변경 시간
	// printf("modify : %s\n", dateFormat(date, st.st_mtimespec)); // 최종 수정 시간
}

void print_fileInfo(){
	printf("%d     ", fileList[listIdx].idx);
	printf("%lld   ", fileList[listIdx].size); // 파일 크기
	printf("%d      ", fileList[listIdx].mode); // 모드
	printf("%lld      ", fileList[listIdx].blocks); // 할당된 블록 수
	printf("%d     ", fileList[listIdx].links); // 하드링크
	printf("%d  ", fileList[listIdx].uid); // 사용자id
	printf("%d   ", fileList[listIdx].gid); // 그룹id
	printf("%s ", fileList[listIdx].access); // 최종 접근 시간
	printf("%s ", fileList[listIdx].change); // 최종 상태 변경 시간
	printf("%s ", fileList[listIdx].modify); // 최종 수정 시간
	printf("%s\n", fileList[listIdx].path);
}

// 시간 정보 포맷에 맞게 변환
char *dateFormat(char * str, struct timespec st){
	const struct tm *stTime = localtime((const time_t *) &st);
	strftime(str, DATEFORMAT_SIZE, "%y-%m-%d %H:%M", stTime);
	return str;
}