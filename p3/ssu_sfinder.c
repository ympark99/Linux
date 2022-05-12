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
#include <math.h>
#include "ssu_sfinder.h"
#include "ssu_help.h"
#include "ssu_find.h"

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

				char extension[BUF_SIZE]; // 저장할 확장자
				long double min_byte = -1; // 비교할 최소 크기(byte)
				long double max_byte = -1; // 비교할 최대 크기(byte)
				char dir_path[BUF_SIZE]; // 저장할 디렉토리 경로
				char *pos = NULL; // 실수 계산시 저장할 포인터
				int thread_num = 1; // 쓰레드 개수

				for(int i = 0; i < OPER_LEN; i++){
					if(splitOper[i] == NULL) break;
					split_cnt++;
				}
				// todo : 같은 옵션 중복 입력, -옵션 아닌 다른 문자열 입력

				bool go_next = true; // 에러 있는지 확인
				bool input_opt[4] = {false, }; // 필수 옵션 모두 입력했는지 확인 (-e, -l, -h, -d, -t)

				// getopt로 옵션 분리 및 검사
				while((option_opt = getopt(split_cnt, splitOper, "e:l:h:d:t:")) != -1){
					if(!go_next) break;
					switch(option_opt){
						case 'e' :
							// 확장자 에러 검사 (*또는 *.(확장자)만 ok)
							if(strcmp(optarg, "*") != 0 && (strlen(optarg) > 1 && optarg[1] != '.')){
								fprintf(stderr, "올바른 확장자 입력이 아님\n");
								go_next = false;
								break;
							}
							strcpy(extension, optarg);
							input_opt[0] = true;
							break;
						case 'l' :
							if(strcmp(optarg, "~")){
								pos = NULL;
								min_byte = strtold(optarg, &pos); // 문자열 변환
								// 수만 입력한 경우
								if(!strcmp(pos, "")){
									double integer, fraction;
									fraction = modf(min_byte, &integer); // 정수부 소수부 분리
									// 실수 입력한 경우 에러
									if(fraction != 0){
										fprintf(stderr, "min size error\n");
										go_next = false;
										break;
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
									go_next = false;
									break;
								}		
							}
							input_opt[1] = true;
							break;
						case 'h' : 
							if(strcmp(optarg, "~")){
								pos = NULL;
								max_byte = strtold(optarg, &pos); // 문자열 변환
								// 수만 입력한 경우
								if(!strcmp(pos, "")){
									double integer, fraction;
									fraction = modf(max_byte, &integer); // 정수부 소수부 분리
									// 실수 입력한 경우 에러
									if(fraction != 0){
										fprintf(stderr, "max size error\n");
										go_next = false;
										break;
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
									go_next = false;
									break;
								}
							}						
							input_opt[2] = true;
							break;
						case 'd' : 
							// 파일경로에 ~입력시 홈디렉토리로 바꿈
							if(optarg[0] == '~'){
								struct passwd *pwd;
								if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
									fprintf(stderr, "user id error");
									go_next = false;
									break;
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
								break;
							}								
							strcpy(dir_path, optarg);
							input_opt[3] = true;
							break;
						case 't' : 
							// 두자리 이상 입력한 경우 에러
							if(strlen(optarg) != 1){
								fprintf(stderr, "잘못된 쓰레드 개수 입력\n");
								go_next = false;
								break;
							}
							thread_num = atoi(optarg);
							if(thread_num == 0 || thread_num > 5){
								fprintf(stderr, "잘못된 쓰레드 개수 입력\n");
								go_next = false;
								break;
							}
							break;												
						default :
							fprintf(stderr, "잘못된 입력\n");
							go_next = false;
							break;
					}
				}
				optind = 0; // optind 초기화

				// 필수 입력 옵션 확인
				for(int i = 0; i < 4; i++){
					if(!input_opt[i]){
						fprintf(stderr, "필수 옵션 입력 x\n");
						go_next = false;
						break;
					}
				}

				if(go_next){
					// 큐 선언
					queue q;
					init_queue(&q);
					// 링크드리스트 head 선언
					Set *head = malloc(sizeof(Set));
					head->next = NULL;

					// 찾은 파일 저장해둘 fp선언
					FILE *dt = fopen(".writeReadData.txt", "w+");
					if(dt == NULL){
						fprintf(stderr, "data file create error\n");
						exit(1);
					}

					struct timeval start;
					gettimeofday(&start, NULL);
					// todo : md5, sha1 구분
					!strcmp(splitOper[0], "fmd5") ?
					ssu_find(true, extension, min_byte, max_byte, dir_path, thread_num, start, head, &q, dt, true)
					:
					ssu_find(false, extension, min_byte, max_byte, dir_path, thread_num, start, head, &q, dt, true);
					delete_set(head); // 세트 제거
				}
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