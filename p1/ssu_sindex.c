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
#include "option.h"

struct fileLists fileList[LMAX]; // 출력 리스트 구조체 선언
int listIdx = 0; // 출력 리스트 index
long long sumDirSize; // 하위 파일들 합친 디렉토리 크기

void ssu_sindex(){
	while (1){
		sumDirSize = 0; // 디렉토리 크기 변수 초기화
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
		// fileList, listIdx 초기화
		memset(&fileList, 0, sizeof(struct fileLists));
		listIdx = 0;
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
	printf("   l : print line length and compare result\n");
}

// find 함수 : findOper[1] : 기준 경로, findOper[2] : 비교 경로
void find_first(char *findOper[FINDOPER_SIZE]){
	char oriFileName[BUF_SIZE]; // 최초 입력 경로 저장
	strcpy(oriFileName, findOper[1]);

	// 상대경로인 경우 절대경로로 변환(기준 FILENAME)
	if(findOper[1][0] != '/'){
		char buf[BUF_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[1], buf) == NULL){
			perror("realpath error -> filename");
			return;
		}
		// strcpy(findOper[1], buf);
		findOper[1] = buf; // 변환한 절대경로 저장
	}
	// 상대경로인 경우 절대경로로 변환(PATH)
	if(findOper[2][0] != '/'){
		char buf2[BUF_SIZE];
		// 절대 경로가 NULL인경우 오류 발생
		if(realpath(findOper[2], buf2) == NULL){
			perror("realpath error -> path");
			return;
		}
		findOper[2] = buf2; // 변환한 절대경로 저장
	}
	
	int fileOrDir = 0; // 1 : 파일, 2 : 디렉토리
	fileOrDir = check_fileOrDir(findOper[1]); // 파일형식 저장

	long long oriSize = 0;
	// 파일인 경우 파일사이즈, 디렉토리인 경우 디렉토리 사이즈 구하기
	if(fileOrDir == 1){
		oriSize = get_fileSize(findOper[1]);
	}
	else{
		char buf[BUF_SIZE];
		strcpy(buf, findOper[1]);
		get_dirSize(buf);
		oriSize = sumDirSize;
		sumDirSize = 0; // 초기화
	}

	// 리스트 출력
	printf("Index Size Mode       Blocks Links UID  GID  Access          Change          Modify          Path\n");
	save_fileInfo(findOper[1], oriSize); // 기준 파일(디렉토리) 리스트에 저장
	print_fileInfo(); // 리스트 출력
	listIdx++; // 리스트 크기 1 증가

	char *fileName = strrchr(findOper[1], '/'); // / 들어간 마지막 위치 반환
	
	// 최초입력 PATH 에러 검사
	struct dirent **namelist; // scandir 관련 선언
	int cnt; // return 값

	// 에러 있을 경우 에러 처리
	if((cnt = scandir(findOper[2], &namelist, scandirFilter, alphasort)) == -1){
		fprintf(stderr, "%s Directory Scan Error : %s \n", findOper[2], strerror(errno));
		return;
	}
	free(namelist);

	dfs_findMatchFiles(findOper[2], fileName, oriSize, fileOrDir); // PATH부터 디렉토리 탐색 & 리스트 저장

	if(listIdx == 1)
		printf("(None)\n"); // 탐색결과 없으면 (None) 출력
	else if(listIdx > 1) 
		option(fileOrDir, fileList, listIdx, findOper, oriFileName); // listIdx > 1이면 옵션 프로세스 실행
}

// scandir 통한 디렉토리 전체 목록 조회 후 파일 정보 탐색(dfs)
// 비교 파일 절대경로, / + 기준 파일 이름, 기준 파일크기
void dfs_findMatchFiles(char *cmpPath, char *fileName, long long oriSize, int fileOrDir){
	// scandir 관련 선언
	struct dirent **namelist;
	int cnt; // return 값

	// dfs -> 디렉토리 아닐 경우 리턴
	if((cnt = scandir(cmpPath, &namelist, scandirFilter, alphasort)) == -1){
		return;
	}
	// 전체 목록 search & dfs
	for(int i = 0; i < cnt; i++){
		// cmpPath : 비교파일절대경로/하위파일명 으로 합치기
		strcat(cmpPath, "/");
		strcat(cmpPath, namelist[i]->d_name);

		// cmpFileName : d_name 앞에 / 붙여준 문자열
		char *cmpFileName = malloc(sizeof(char) * BUF_SIZE); // strcat 위한 충분한 사이즈 할당
		strcpy(cmpFileName, "/"); // fileName 앞에 / 붙어있으므로

		// 이름 같은 파일/dir 발견할 경우
		if(strcmp(fileName, strcat(cmpFileName, namelist[i]->d_name)) == 0){
			long long cmpSize = 0;
			if(fileOrDir == 1) cmpSize = get_fileSize(cmpPath);
			else{
				get_dirSize(cmpPath);
				cmpSize = sumDirSize;
			}

			// 파일/dir 크기 같고 && 기준과 같은 파일 아니면 리스트 등록
			if(oriSize == cmpSize && strcmp(fileList[0].path, cmpPath) != 0 ){
				save_fileInfo(cmpPath, oriSize); // 리스트에 등록
				print_fileInfo();
				listIdx++;
			}
			sumDirSize = 0; // dir 크기 초기화
		}
		free(cmpFileName);

		if(strcmp(fileList[0].path, cmpPath) != 0) // 기준과 같은파일 아닐경우
			dfs_findMatchFiles(cmpPath, fileName, oriSize, fileOrDir); // dfs

		// 합쳤던 하위파일명 문자열 제거
		char* ptr = strrchr(cmpPath, '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr){
			strncpy(ptr, "", 1); // 합쳤던 문자열 제거
		}
	}

	for(int i = 0; i < cnt; i++){
		free(namelist[i]);
	}
	free(namelist);

	return;
}

// scandir 필터(. 과 .. 제거)
int scandirFilter(const struct dirent *info){
	if(strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0){
		return 0; // .이나 .. 이면 filter
	}
	else return 1;
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

// 디렉토리 크기 리턴
void get_dirSize(char *path){
    struct dirent **namelist;
    int cnt;
    struct stat st;

    // scandir로 하위파일 가져오기
	if((cnt = scandir(path, &namelist, scandirFilter, alphasort)) == -1){
		perror("scandir error\n");
		return;
	}

    // 하위리스트 반복 -> 디렉토리 일경우 재귀
    for (int i = 0; i < cnt; i++){
        struct stat st;

		// path : 비교파일절대경로/하위파일명 으로 합치기
		strcat(path, "/");
		strcat(path, namelist[i]->d_name);

		if(stat(path, &st) == -1){
			perror("get dir stat error");
			return;
		}

        sumDirSize += st.st_size; // 각각 파일 크기 더해줌

        // 디렉토리일 경우
        if (((st.st_mode & S_IFMT) == S_IFDIR) || ((st.st_mode & S_IFMT) == S_IFLNK)){
			get_dirSize(path); // 재귀적으로 더해줌
		}
		// 합쳤던 하위파일명 문자열 제거
		char* ptr = strrchr(path, '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr){
			strncpy(ptr, "", 1); // 합쳤던 문자열 제거
		}	
    }
}

// 파일, 디렉토리 판별(파일 : 1, 디렉토리 : 2 리턴)
int check_fileOrDir(char*path){
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
		case S_IFIFO:
			fileOrDir = 0;
			break;
		case S_IFLNK:
			fileOrDir = 0;
			break;
	}
	return fileOrDir;
}

// 파일 정보 리스트에 저장
void save_fileInfo(char *path, long long oriSize){
	struct stat st;

	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("stat error");
		return;
	}

	// st_mode 데이터 형식 rwx형태로 변경
	char mode2str[MODE_SIZE]; // 저장될 문자열
	int idx = 0;

	// 파일 형식
	switch (st.st_mode & S_IFMT){
		case S_IFREG:
			mode2str[idx++] = '-';
			break;
		case S_IFDIR:
			mode2str[idx++] = 'd';
			break;	
		case S_IFIFO:
			mode2str[idx++] = 'p';
			break;
		case S_IFLNK:
			mode2str[idx++] = 'l';
			break;
	}
	for(int i = 0; i < 3; i++){ // 파일 접근 권한
		if(st.st_mode & (S_IREAD >> i * 3))
			mode2str[idx++] = 'r';
		else 
			mode2str[idx++] = '-';
		if(st.st_mode & (S_IWRITE >> i * 3))
			mode2str[idx++] = 'w';
		else 
			mode2str[idx++] = '-';
		if(st.st_mode & (S_IEXEC >> i * 3))
			mode2str[idx++] = 'x';
		else 
			mode2str[idx++] = '-';
	}
	mode2str[idx] = '\0';

	char date[DATEFORMAT_SIZE]; // 저장될 시간 정보

	fileList[listIdx].idx = listIdx;
	fileList[listIdx].size = oriSize;
	strcpy(fileList[listIdx].mode, mode2str);
	fileList[listIdx].blocks = st.st_blocks;
	fileList[listIdx].links = st.st_nlink;
	fileList[listIdx].uid = st.st_uid;
	fileList[listIdx].gid = st.st_gid;
	strcpy(fileList[listIdx].access, dateFormat(date, st.st_atimespec));
	strcpy(fileList[listIdx].change, dateFormat(date, st.st_ctimespec));
	strcpy(fileList[listIdx].modify, dateFormat(date, st.st_mtimespec));
	strcpy(fileList[listIdx].path, path);
}

// 리스트 출력
void print_fileInfo(){
	printf("%-6d", fileList[listIdx].idx);
	printf("%-5lld", fileList[listIdx].size); // 파일 크기
	printf("%-11s", fileList[listIdx].mode); // 모드
	printf("%-7lld", fileList[listIdx].blocks); // 할당된 블록 수
	printf("%-6d", fileList[listIdx].links); // 하드링크
	printf("%-5d", fileList[listIdx].uid); // 사용자id
	printf("%-5d", fileList[listIdx].gid); // 그룹id
	printf("%-16s", fileList[listIdx].access); // 최종 접근 시간
	printf("%-16s", fileList[listIdx].change); // 최종 상태 변경 시간
	printf("%-16s", fileList[listIdx].modify); // 최종 수정 시간
	printf("%-s\n", fileList[listIdx].path);
}

// 시간 정보 포맷에 맞게 변환
char *dateFormat(char * str, struct timespec st){
	const struct tm *stTime = localtime((const time_t *) &st);
	strftime(str, DATEFORMAT_SIZE, "%y-%m-%d %H:%M", stTime);
	return str;
}