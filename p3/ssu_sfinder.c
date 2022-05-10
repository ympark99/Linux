#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h> // scandir 사용
#include <ctype.h>
#include <stdbool.h>
#include <openssl/md5.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pwd.h>
#include "ssu_sfinder.h"
#include "ssu_help.h"

int main(){
	while (1){
		char *oper = malloc(sizeof(char) * BUF_SIZE);
		fprintf(stdout, "20182615> "); // 프롬프트 출력
		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}
		
		char *splitOper[OPER_LEN] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		while (ptr != NULL){
			if(idx < OPER_LEN) splitOper[idx] = ptr;
			idx++;
			ptr = strtok(NULL, " ");
		}

		// fmd5 or fsha1 명령 시
		if(splitOper[0] != NULL && (!strcmp(splitOper[0], "fmd5") || !strcmp(splitOper[0], "fsha1"))){		
			// 명령어 인자 틀리면 에러 처리
			if(idx != OPER_LEN)
				fprintf(stderr, "명령어를 맞게 입력해주세요\n");
			// 확장자 에러 검사 (*또는 *.(확장자)만 ok)
			else if(strcmp(splitOper[1], "*") != 0 && (strlen(splitOper[1]) > 1 && splitOper[1][1] != '.'))
				fprintf(stderr, "올바른 확장자 입력이 아님\n");
			// 최소, 최대 검사는 함수 내에서 진행
			else{
				// 파일경로에 ~입력시 홈디렉토리로 바꿈
				if(splitOper[4][0] == '~'){
					struct passwd *pwd;
					if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
						fprintf(stderr, "user id error");
					}
					memmove(splitOper[4], splitOper[4] + 1, strlen(splitOper[4])); // 맨 처음 ~ 제거
					char *str = malloc(sizeof(char) * BUF_SIZE); // 임시로 저장해둘 문자열
					strcpy(str, splitOper[4]);
					sprintf(splitOper[4], "%s%s", pwd->pw_dir, str); // 홈디렉토리/하위경로로 합쳐줌
					free(str);
				}

				int fileOrDir = check_fileOrDir(splitOper[4]); // 파일 or 디렉토리인지 체크
				// TARGET_DIRECTORY 검사 (디렉토리 아닌경우)
				if(fileOrDir != 2){
					fprintf(stderr, "디렉토리가 아님\n");
				}
				else{
					// 프로세스 생성 및 실행
					int pid, status;
					pid = fork();
					if(pid < 0){
						fprintf(stderr, "fork error :");
						exit(1);
					}
					else if(pid == 0){ // 0인 경우 자식 프로세스
					// fmd5 또는 sha1 실행
					!strcmp(splitOper[0], "fmd5") ?
						execl("./ssu_find-md5", splitOper[0], splitOper[1], splitOper[2], splitOper[3], splitOper[4], NULL) 
						:
						execl("./ssu_find-sha1", splitOper[0], splitOper[1], splitOper[2], splitOper[3], splitOper[4], NULL);
						exit(0);
					}
					else{ // 부모 프로세스
						wait(&status); // child 종료때까지 대기
					}
				}
			}
		}
		// exit 입력 시 종료
		else if(splitOper[0] != NULL && !strcmp(oper, "exit")){
			// exit 뒤 인자 붙으면 help와 동일한 결과 출력
			if(splitOper[1] != NULL){
				ssu_help();
			}
			else break;
		}	
		else if(splitOper[0] != NULL){ // 엔터키 입력 아닌 경우 명령어 사용법 출력
				ssu_help();
		}
		free(oper);
	}

	fprintf(stdout, "Prompt End\n");
	exit(0);
}

// 파일, 디렉토리 판별(파일 : 1, 디렉토리 : 2 리턴)
int check_fileOrDir(char *path){
	struct stat st;
	int fileOrDir = 0;
	// 파일 정보 얻기
	if(lstat(path, &st) == -1){
		fprintf(stderr, "stat error -> checkfile\n");
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