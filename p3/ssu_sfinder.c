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

// todo : fork 제거 필요

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
			if(idx != OPER_LEN && idx != OPER_LEN - 2)
				fprintf(stderr, "명령어를 맞게 입력해주세요\n");
			else{
				int option_opt;
				int split_cnt = 0; // 실제 입력한 카운트 계산
				for(int i = 0; i < OPER_LEN; i++){
					if(splitOper[i] == NULL) break;
					split_cnt++;
				}
				// todo : 같은 옵션 중복 입력, -옵션 아닌 다른 문자열 입력

				bool go_next = true; // 에러 있는지 확인
				bool input_opt[4] = {false, }; // 필수 옵션 모두 입력했는지 확인 (-e, -l, -h, -d)

				// getopt로 옵션 분리 및 검사
				while((option_opt = getopt(split_cnt, splitOper, "e:l:h:d:t:")) != -1){
					if(!go_next) break;
					switch(option_opt){
						case 'e' :
							// 확장자 에러 검사 (*또는 *.(확장자)만 ok)
							if(strcmp(optarg, "*") != 0 && (strlen(optarg) > 1 && optarg[1] != '.')){
								fprintf(stderr, "올바른 확장자 입력이 아님\n");
								go_next = false;
							}
							printf("e : %s\n", optarg);
							input_opt[0] = true;
							break;
						case 'l' : 
							printf("l : %s\n", optarg);
							input_opt[1] = true;
							break;
						case 'h' : 
							printf("h : %s\n", optarg);
							input_opt[2] = true;
							break;
						case 'd' : 
							// 파일경로에 ~입력시 홈디렉토리로 바꿈
							if(optarg[0] == '~'){
								struct passwd *pwd;
								if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
									fprintf(stderr, "user id error");
									go_next = false;
								}
								memmove(optarg, optarg + 1, strlen(optarg)); // 맨 처음 ~ 제거
								char *str = malloc(sizeof(char) * BUF_SIZE); // 임시로 저장해둘 문자열
								strcpy(str, optarg);
								sprintf(optarg, "%s%s", pwd->pw_dir, str); // 홈디렉토리/하위경로로 합쳐줌
								free(str);
							}

							int fileOrDir = check_fileOrDir(optarg); // 파일 or 디렉토리인지 체크
							// TARGET_DIRECTORY 검사 (디렉토리 아닌경우)
							if(fileOrDir != 2){
								fprintf(stderr, "디렉토리가 아님\n");
								go_next = false;
							}								
							printf("d : %s\n", optarg);
							input_opt[3] = true;
							break;
						case 't' : 
							// todo : t 에러처리
							printf("t : %s\n", optarg);
							break;												
						default :
							fprintf(stderr, "잘못된 입력\n");
							go_next = false;
							break;
					}
				}
				optind = 0; // optind 초기화

				for(int i = 0; i < 4; i++){
					if(!input_opt[i]){
						fprintf(stderr, "필수 옵션 입력 x\n");
						go_next = false;
						break;
					}
				}

				// todo : go_next true일경우 진행
				// 최소, 최대 검사는 함수 내에서 진행
				// fmd5 or fsha1 실행
				// !strcmp(splitOper[0], "fmd5") ?
					// ssu_find_md5
					// :
					// ssu_find_sha1
			}
		}
		// exit 입력 시 종료
		else if(splitOper[0] != NULL && !strcmp(oper, "exit")){
			// exit 뒤 인자 붙으면 help와 동일한 결과 출력
			if(splitOper[1] != NULL) 
				ssu_help();
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