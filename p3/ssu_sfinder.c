#include "ssu_find.h"
// todo : 옵션 입력 에러처리 ex : list list list, root에서 테스트
int main(){
	// 링크드리스트 head 선언
	Set *head = malloc(sizeof(Set));
	head->next = NULL;

	// 하나만 남은 세트 저장해둘곳, 복구 시 활용
	Set *only = malloc(sizeof(Set));
	only->next = NULL;

	// 링크드리스트 trhead 선언
	Trash *trhead = malloc(sizeof(Trash));
	trhead->next = NULL;
	while (1){
		char *oper = malloc(sizeof(char) * BUF_SIZE);
		fprintf(stdout, "20182615> "); // 프롬프트 출력
		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}
		
		char *splitOper[FIRST_OPER_LEN] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		while (ptr != NULL){
			if(idx < FIRST_OPER_LEN) splitOper[idx] = ptr;
			idx++;
			ptr = strtok(NULL, " ");
		}

		// 큐 선언
		queue q;
		init_queue(&q);

		// fmd5 or fsha1 명령 시
		if(splitOper[0] != NULL && (!strcmp(splitOper[0], "fmd5") || !strcmp(splitOper[0], "fsha1"))){
			// 명령어 인자 틀리면 에러 처리
			if(idx != FIRST_OPER_LEN && idx != FIRST_OPER_LEN - 2)
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

				for(int i = 0; i < FIRST_OPER_LEN; i++){
					if(splitOper[i] == NULL) break;
					split_cnt++;
				}

				bool go_next = true; // 에러 있는지 확인
				int input_opt[5] = {0, }; // 필수 옵션 모두 입력했는지 확인 (-e, -l, -h, -d, -t)

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
							input_opt[0]++;
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
							input_opt[1]++;
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
							input_opt[2]++;
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
							input_opt[3]++;
							break;
						case 't' : 
							// todo : 쓰레드 시간 측정
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
							input_opt[4]++;
							break;												
						default :
							fprintf(stderr, "잘못된 입력\n");
							go_next = false;
							break;
					}
				}
				optind = 0; // optind 초기화

				// 필수 입력 옵션 확인
				for(int i = 0; i < 5; i++){
					if(i < 4 && input_opt[i] == 0){
						fprintf(stderr, "필수 옵션 입력 x\n");
						go_next = false;
						break;
					}
					else if(input_opt[i] > 1){
						fprintf(stderr, "옵션 중복 입력\n");
						go_next = false;
						break;						
					}
				}

				if(go_next){
					// 찾은 파일 저장해둘 fp선언
					FILE *dt = fopen(".writeReadData.txt", "w+");
					if(dt == NULL){
						fprintf(stderr, "data file create error\n");
						exit(1);
					}

					if(get_setLen(head) != 0){ // 세트 있을경우 제거하고 재생성
						delete_set(head); // 기존 세트 제거
						// 링크드리스트 head 선언
						Set *head = malloc(sizeof(Set));
						head->next = NULL;
					}

					if(get_setLen(only) != 0){ // only 있을경우 제거하고 재생성
						delete_set(only); // 기존 세트 제거
						// 링크드리스트 only 선언
						Set *only = malloc(sizeof(Set));
						only->next = NULL;
					}
					struct timeval start;
					gettimeofday(&start, NULL);
					!strcmp(splitOper[0], "fmd5") ?
						ssu_find(true, extension, min_byte, max_byte, dir_path, thread_num, start, head, only, &q, dt)
						:
						ssu_find(false, extension, min_byte, max_byte, dir_path, thread_num, start, head, only, &q, dt);
				}
			}
		}
		// list 명령 시
		else if(splitOper[0] != NULL && !strcmp(splitOper[0], "list")){
			// 명령어 인자 틀리면 에러 처리 (idx개수 1,3,5,7 만 가능)
			if((idx > LIST_LEN) || ((idx % 2) != 1))
				fprintf(stderr, "명령어를 맞게 입력해주세요\n");
			else{
				int option_opt;
				int split_cnt = 0; // 실제 입력한 카운트 계산

				for(int i = 0; i < LIST_LEN; i++){
					if(splitOper[i] == NULL) break;
					split_cnt++;
				}
				bool go_next = true; // 에러 있는지 확인
				bool sort_set = false; // 파일 세트를 정렬하는지 결정
				bool c_opt[5] = {false, true, false, false, false }; // c 옵션 카테고리(filename, size, uid, gid, mode)
				bool sort_up = true; // 오름차순 정렬할지 결정
				int input_opt[3] = {0, }; // 옵션 중복 입력 (-l, -c, -o)

				// getopt로 옵션 분리 및 검사
				while((option_opt = getopt(split_cnt, splitOper, "l:c:o:")) != -1){
					if(!go_next) break;
					switch(option_opt){
						case 'l' :
							if(optarg == NULL) sort_set = true;
							else if(!strcmp(optarg, "fileset"))
								sort_set = true;
							else if(!strcmp(optarg, "filelist"))
								sort_set = false;
							else{
								fprintf(stderr, "l 옵션 입력 에러\n");
								go_next = false;
								break;
							}
							input_opt[0]++;				
							break;
						case 'c' :					
							if(optarg == NULL) c_opt[1] = true;
							else{
								if(!strcmp(optarg, "size")) c_opt[1] = true;								
								else if(!strcmp(optarg, "filename")){
									c_opt[0] = true;
									c_opt[1] = false;
								}
								else if(!strcmp(optarg, "uid")){
									c_opt[2] = true;
									c_opt[1] = false;									
								}
								else if(!strcmp(optarg, "gid")){
									c_opt[3] = true;
									c_opt[1] = false;									
								}
								else if(!strcmp(optarg, "mode")){
									c_opt[4] = true;
									c_opt[1] = false;									
								}																	
								else{
									fprintf(stderr, "c 옵션 입력 에러\n");
									go_next = false;
									break;
								}				
							}
							input_opt[1]++;
							break;
						case 'o' :						
							if(optarg == NULL) sort_up = true;
							else if(!strcmp(optarg, "1")) sort_up = true;
							else if(!strcmp(optarg, "-1")) sort_up = false;
							else{
								fprintf(stderr, "o 옵션 입력 에러\n");
								go_next = false;
								break;
							}
							input_opt[2]++;
							break;
						default :
							fprintf(stderr, "잘못된 입력\n");
							go_next = false;
							break;	
					}
				}
				optind = 0; // optind 초기화

				// 필수 입력 옵션 확인
				for(int i = 0; i < 3; i++){
					if(input_opt[i] > 1){
						fprintf(stderr, "옵션 중복 입력\n");
						go_next = false;
						break;						
					}
				}

				if(go_next){
					// list 함수 수행
					list(head, sort_set, c_opt, sort_up);
				}
			}
		}
		// trash 명령 시
		else if(splitOper[0] != NULL && !strcmp(splitOper[0], "trash")){
			// 명령어 인자 틀리면 에러 처리 (idx개수 1,3,5,7 만 가능)
			if((idx > TRASH_LEN) || ((idx % 2) != 1))
				fprintf(stderr, "명령어를 맞게 입력해주세요\n");
			else{
				int option_opt;
				int split_cnt = 0; // 실제 입력한 카운트 계산

				for(int i = 0; i < TRASH_LEN; i++){
					if(splitOper[i] == NULL) break;
					split_cnt++;
				}
				bool go_next = true; // 에러 있는지 확인
				bool c_opt[5] = {true, false, false, false, false}; // c 옵션 카테고리(파일이름, 절대경로, size, date, time)
				bool sort_up = true; // 오름차순 정렬할지 결정
				int input_opt[2] = {0, }; // 옵션 중복 입력 (-c, -o)

				// getopt로 옵션 분리 및 검사
				while((option_opt = getopt(split_cnt, splitOper, "c:o:")) != -1){
					if(!go_next) break;
					switch(option_opt){
						case 'c' :					
							if(optarg == NULL) c_opt[0] = true;
							else{				
								if(!strcmp(optarg, "filename")){
									c_opt[1] = true;
									c_opt[0] = false;
								}
								else if(!strcmp(optarg, "size")){
									c_opt[2] = true;
									c_opt[0] = false;								
								}
								else if(!strcmp(optarg, "date")){
									c_opt[3] = true;
									c_opt[0] = false;										
								}
								else if(!strcmp(optarg, "time")){
									c_opt[4] = true;
									c_opt[0] = false;										
								}																
								else{
									fprintf(stderr, "c 옵션 입력 에러\n");
									go_next = false;
									break;
								}
							}
							input_opt[0]++;
							break;
						case 'o' :						
							if(optarg == NULL) sort_up = true;
							else if(!strcmp(optarg, "1")) sort_up = true;
							else if(!strcmp(optarg, "-1")) sort_up = false;
							else{
								fprintf(stderr, "o 옵션 입력 에러\n");
								go_next = false;
								break;
							}
							input_opt[1]++;
							break;
						default :
							fprintf(stderr, "잘못된 입력\n");
							go_next = false;
							break;	
					}
				}
				optind = 0; // optind 초기화

				// 필수 입력 옵션 확인
				for(int i = 0; i < 2; i++){
					if(input_opt[i] > 1){
						fprintf(stderr, "옵션 중복 입력\n");
						go_next = false;
						break;						
					}
				}

				if(go_next){
					// 기존 trash 제거
					delete_trash(trhead);
					// 링크드리스트 trhead 선언
					Trash *trhead = malloc(sizeof(Trash));
					trhead->next = NULL;

					// trash 함수 수행
					trash(trhead, c_opt, sort_up);
				}
			}
		}
		// restore 명령 시
		else if(splitOper[0] != NULL && !strcmp(oper, "restore")){
			int trash_size = get_trashLen(trhead);
			// 명령어 인자 틀리면 에러 처리
			if(idx != RESTORE_LEN)
				fprintf(stderr, "명령어를 맞게 입력해주세요\n");
			// RESTORE_INDEX가 0이거나 리스트 크기보다 큰경우
			else if(atoi(splitOper[1]) == 0 || atoi(splitOper[1]) > trash_size){
				fprintf(stderr, "RESTORE_INDEX error\n");
			}
			else{
				int restore_idx = atoi(splitOper[1]);
				restore(head, only, trhead, restore_idx); // restore 함수 수행
				print_trash(trhead); // trash list 출력
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
	// delete_set(head); // 세트 제거
	// delete_trash(trhead); // 쓰레기통 제거
	fprintf(stdout, "Prompt End\n");
	exit(0);
}

void ssu_help(){
	printf("Usage:\n");
	printf("  > fmd5/fsha1  [FILE_EXTENSION] -l [MINSIZE] -h [MAXSIZE] -d [TARGET_DIRECTORY] -t [THREAD_NUM]\n");
	printf("     >>  delete -l [SET_INDEX] -d [OPTARG] -i -f -t\n");
	printf("  > trash -c [CATEGORY] -o [ORDER]\n");
	printf("  > restore [RESTORE_INDEX]\n");
	printf("  > help\n");
	printf("  > exit\n\n");
}

void delete_trash(Trash *tr){
	Trash *cur = tr;
	Trash *next;
	while (cur != NULL){
		next = cur->next;
		free(cur);
		cur = next;
	}
}