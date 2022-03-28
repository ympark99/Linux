#include <stdio.h>
#include <sys/types.h> // stat 사용
#include <sys/stat.h> // stat 사용
#include <unistd.h> // stat 사용
#include <string.h> // string 관련 함수 사용
#include <stdlib.h> // realpath 사용
#include <dirent.h> // scandir 사용

#include "ssu_find-md5.h"

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

    // 조건에 맞으면, 파일 : 리스트 조회 | 디렉토리 : 큐에 삽입
	for(int i = 0; i < cnt; i++){
        // 파일일경우
        if(check_fileOrDir(filelist[i]->d_name) == 1){
            // * 모두
        }
        // 디렉토리일 경우
        else if(check_fileOrDir(filelist[i]->d_name) == 2){
            // 루트에서부터 탐색시, proc, run, sys 제외
        }



    }

	for(int i = 0; i < cnt; i++){
		free(filelist[i]);
	}
	free(filelist);    

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