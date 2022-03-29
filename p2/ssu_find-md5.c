#include <stdio.h>
#include <sys/types.h> // stat 사용
#include <sys/stat.h> // stat 사용
#include <unistd.h> // stat 사용
#include <string.h> // string 관련 함수 사용
#include <stdlib.h>
#include <dirent.h> // scandir 사용
#include <math.h> // modf 사용
// #include <openssl/md5.h>
#include "ssu_find-md5.h"

// 동일 파일 링크드리스트
typedef struct Node{
	struct Node *next; // 다음 주소
	char path[BUF_SIZE]; // 파일 경로
	char mtime[BUF_SIZE]; // mtime
	char atime[BUF_SIZE]; // atime
	char hash[BUF_SIZE]; // hash value
} Node;

// md5 관련 함수 실행
// 입력인자 : 명령어 split
// 인자배열 : fmd5, 파일 확장자, 최소크기, 최대크기, 디렉토리
void ssu_find_md5(char *splitOper[OPER_LEN]){
	struct dirent **filelist; // scandir 파일목록 저장 구조체
	int cnt; // return 값

    // scandir로 파일목록 가져오기 (디렉토리가 아닐 경우 에러)
	if((cnt = scandir(splitOper[4], &filelist, scandirFilter, alphasort)) == -1){
        fprintf(stderr, "target is not directory\n");
		return;
	}
	
	char pathname[BUF_SIZE]; // 합성할 path이름

    // 조건에 맞으면, 파일 : 리스트 조회 | 디렉토리 : 큐에 삽입
	for(int i = 0; i < cnt; i++){
        // 파일일경우
        if(check_fileOrDir(filelist[i]->d_name) == 1){
            // *.(확장자)인 경우
			if(strlen(splitOper[1]) > 1 ){
				char* ori_fname = strrchr(splitOper[1], '.'); // 지정 파일 확장자
				char* cmp_fname = strrchr(filelist[i]->d_name, '.'); // 가져온 파일 확장자

				// 확장자가 없거나 확장자가 다르면 패스
				if((cmp_fname == NULL) || strcmp(ori_fname, cmp_fname) != 0)
					continue;
			}

			sprintf(pathname, "%s/%s", splitOper[4], filelist[i]->d_name); // pathname 만들어줌
			long double filesize = get_fileSize(pathname); // 파일크기 구하기

			// 최소 크기 이상인지 확인
			if(strcmp(splitOper[2], "~")){
				long double min_byte = 0; // 비교할 최소 크기(byte)
				char *pos = NULL;
				min_byte = strtod(splitOper[2], &pos); // 실수, 문자열 분리
				// 수만 입력한 경우
				if(!strcmp(pos, "")){
					double integer, fraction;
					fraction = modf(min_byte, &integer);
					// 실수 입력한 경우 에러
					if(fraction != 0){
						fprintf(stderr, "min size error\n");
						return;
					}
				}
				// KB = 1024byte
				else if(!strcmp(pos, "kb") || !strcmp(pos, "KB")){
					min_byte *= 1024;
				}
				// MB = 1024KB
				else if(!strcmp(pos, "mb") || !strcmp(pos, "MB")){
					min_byte *= (1024 * 1024);
				}				
				// GB = 1024MB
				else if(!strcmp(pos, "gb") || !strcmp(pos, "GB")){
					min_byte *= (1024 * 1024 * 1024);
				}
				else{
					fprintf(stderr, "min size error\n");
					return;					
				}
				// 최소 크기보다 작은 경우 패스
				if(filesize < min_byte) continue;
			}
			// 최대 크기 이하인지 확인
			if(strcmp(splitOper[3], "~")){
				long double max_byte = 0; // 비교할 최소 크기(byte)
				char *pos = NULL;
				max_byte = strtod(splitOper[3], &pos); // 실수, 문자열 분리

				// 수만 입력한 경우
				if(!strcmp(pos, "")){
					double integer, fraction;
					fraction = modf(max_byte, &integer);
					// 실수 입력한 경우 에러
					if(fraction != 0){
						fprintf(stderr, "min size error\n");
						return;
					}
				}
				// KB = 1024byte
				else if(!strcmp(pos, "kb") || !strcmp(pos, "KB")){
					max_byte *= 1024;
				}
				// MB = 1024KB
				else if(!strcmp(pos, "mb") || !strcmp(pos, "MB")){
					max_byte *= (1024 * 1024);
				}				
				// GB = 1024MB
				else if(!strcmp(pos, "gb") || !strcmp(pos, "GB")){
					max_byte *= (1024 * 1024 * 1024);
				}
				else{
					fprintf(stderr, "max size error\n");
					return;					
				}
				// 최대 크기보다 큰 경우 패스
				if(filesize > max_byte) continue;
			}
			// todo : 리스트에 있는지 확인(md5)

        }
        // 디렉토리일 경우
        else if(check_fileOrDir(filelist[i]->d_name) == 2){
            // todo : 루트에서부터 탐색시, proc, run, sys 제외

			// todo : 큐 추가
        }
    }

	for(int i = 0; i < cnt; i++){
		free(filelist[i]);
	}
	free(filelist);    

	// todo : 큐 빌때까지 bfs탐색

}

// 파일 크기 리턴
long double get_fileSize(char *path){
	struct stat st;

	// 파일 정보 얻기
	if(stat(path, &st) == -1){
		perror("stat error");
		return -1;
	}

	return (long double) st.st_size;
}

// scandir 필터(. 과 .. 제거)
int scandirFilter(const struct dirent *info){
	if(strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0){
		return 0; // .이나 .. 이면 filter
	}
	else return 1;
}

// 파일, 디렉토리 판별(파일 : 1, 디렉토리 : 2 리턴)
int check_fileOrDir(char *path){
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