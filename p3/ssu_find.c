#include "ssu_find.h"
// todo : node에서 filesize 제거 , 옵션, gettimeofday thread

// md5, sha1 관련 함수 실행
void ssu_find(bool is_md5, char extension[BUF_SIZE], long double min_byte, long double max_byte, char find_path[BUF_SIZE], int thread_num, struct timeval start, Set *set, queue *q, FILE *dt, bool from_main){
    int digest_len = is_md5? MD5_DIGEST_LENGTH : SHA_DIGEST_LENGTH; // md5, sha1 구분

	struct dirent **filelist; // scandir 파일목록 저장 구조체
	int cnt; // return 값
    // scandir로 파일목록 가져오기 (디렉토리가 아닐 경우 에러)
	if((cnt = scandir(find_path, &filelist, scandirFilter, alphasort)) == -1){
		fprintf(stderr, "%s error, ERROR : %s\n", find_path, strerror(errno));
		return;
	}

	// 절대경로 변환
	char dir_path[BUF_SIZE];
	if(realpath(find_path, dir_path) == NULL){
		fprintf(stderr, "realpath error : %s\n", strerror(errno));
		return;
	}

	char pathname[BUF_SIZE]; // 합성할 path이름
	strcpy(pathname, dir_path);
	strcat(pathname, "/"); // 처음에 / 제거하고 시작하므로 붙여줌

    // 조건에 맞으면, 파일 : 리스트 조회 | 디렉토리 : 큐에 삽입
	for(int i = 0; i < cnt; i++){
		// 합쳤던 이전 하위파일명 문자열 제거
		char* ptr = strrchr(pathname, '/'); // 합쳤던 /하위파일명 포인터 연결
		if(ptr)	strncpy(ptr, "", 1); // 합쳤던 문자열 제거

		// 현재 하위파일명 문자열 붙이기
		strcat(pathname, "/");
		strcat(pathname, filelist[i]->d_name); // 경로 + '/파일이름'

		int fileOrDir = check_fileOrDir(pathname); // 파일 or 디렉토리인지 체크

        // 파일일경우
        if(fileOrDir == 1){
            // *.(확장자)인 경우
			if(strlen(extension) > 1 ){
				char* ori_fname = strrchr(extension, '.'); // 지정 파일 확장자
				char* cmp_fname = strrchr(filelist[i]->d_name, '.'); // 가져온 파일 확장자

				// 확장자가 없거나 확장자가 다르면 패스
				if((cmp_fname == NULL) || strcmp(ori_fname, cmp_fname) != 0)
					continue;
			}

			// 파일 정보 조회
			struct stat st;
			// 파일 정보 얻기
			if(lstat(pathname, &st) == -1){			
				// 파일 읽기 권한 없으면 패스
				if(!st.st_mode & S_IRWXU) continue;
				fprintf(stderr, "stat error : %s\n", strerror(errno));
				continue;	
			}
			long double filesize = (long double) st.st_size; // 파일크기 구하기
			if(filesize == 0) continue; // 0바이트인경우 패스

            // ~ 입력한경우 검사 x, 최소 크기보다 작은 경우
            if(min_byte != -1 && filesize < min_byte) continue;

            // ~ 입력한경우 검사 x, 최대 크기보다 큰 경우
            if(max_byte != -1 && filesize > max_byte) continue;	

			// 파일 읽기 권한 없으면 패스
			if(!st.st_mode & S_IRWXU) continue;
			// md5값 구하기
			FILE *fp = fopen(pathname, "r");
			if (fp == NULL) continue; // fopen 에러시 패스

			unsigned char filehash[digest_len * 2 + 1]; // 해시값 저장할 문자열
            is_md5 ? strcpy(filehash, get_md5(fp)) : strcpy(filehash, get_sha1(fp)); // 해시값 구해서 저장

			fclose(fp);

			// 동적할당 후 시간 포맷 구하기
			char* mstr = (char*)malloc(sizeof(char) * BUF_SIZE);
			char* astr = (char*)malloc(sizeof(char) * BUF_SIZE);
			strcpy(mstr, get_time(st.st_mtime, mstr));
			strcpy(astr, get_time(st.st_atime, mstr));
			uid_t uid = st.st_uid;
			gid_t gid = st.st_gid;
			unsigned long mode = st.st_mode;

			// 파일에 저장
			if(dt != NULL){
				char to_str[BUF_SIZE];
				sprintf(to_str, "%lld", (long long)filesize);
				fputs("*", dt); // 체크 여부
				fputs("|", dt);
				fputs(to_str, dt); // 파일 크기
				fputs("|", dt);
				fputs(pathname, dt); // 파일 절대경로
				fputs("|", dt);
				fputs(mstr, dt); // mtime
				fputs("|", dt);
				fputs(astr, dt); // atime
				fputs("|", dt);
				fputs(filehash, dt); // hash
				fputs("|", dt);
				sprintf(to_str, "%d", (int)uid); // uid
				fputs(to_str, dt);
				fputs("|", dt);
				sprintf(to_str, "%d", (int)gid); // gid
				fputs(to_str, dt);
				fputs("|", dt);
				sprintf(to_str, "%lu", mode); // mode
				fputs(to_str, dt);
				fputs("|", dt);		
				fputs("\n", dt); // enter
			}
			free(mstr);
			free(astr);
        }
        // 디렉토리일 경우
        else if(fileOrDir == 2){
            // 루트에서부터 탐색시, proc, run, sys 제외
			if(!strcmp(find_path, "/")){
				if((!strcmp(filelist[i]->d_name, "proc") || !strcmp(filelist[i]->d_name, "run")) || !strcmp(filelist[i]->d_name, "sys"))
					continue;
			}
			push_queue(q, pathname); // 찾은 디렉토리경로 큐 추가
        }
    }

	for(int i = 0; i < cnt; i++){
		free(filelist[i]);
	}
	free(filelist);

	if(!from_main) return; // 처음 메인에서 실행한게 아니라면 리턴 (재귀 종료)	
	// 큐 빌때까지 bfs탐색(bfs이므로 절대경로, 아스키 순서로 정렬되어있음)
	while (!isEmpty_queue(q)){
        ssu_find(is_md5, extension, min_byte, max_byte, pop_queue(q), thread_num, start, set, q, dt, false);
	}
	fclose(dt); // w모드 종료
	dt = fopen(".writeReadData.txt", "r+t"); // r+모드 실행 (체크 표시 남겨야 하므로)
	if(dt == NULL) fprintf(stderr, "fopen read error\n");
	// 중복파일 리스트 추가
	file2set(dt, set);
	fclose(dt);

	// 데이터 파일 삭제
	if(unlink(".writeReadData.txt") == -1)
		fprintf(stderr, "writeReadData delete error\n");

	struct timeval end; 
	gettimeofday(&end, NULL); // 종료 시간 측정

	int set_size = get_setLen(set);

	if(set_size == 0){
		if(realpath(find_path, dir_path) == NULL){
			fprintf(stderr, "realpath error : %s\n", strerror(errno));
			return;
		}
		fprintf(stdout, "No duplicates in %s\n", dir_path);
		return;
	}

	// 파일크기 오름차순 정렬 (bfs이므로 파일크기 같을 경우 절대경로 짧은 순 -> 임의(아스키 코드 순))
	sort_upSet(set, set_size);
	print_set(set); // 세트 출력
	get_searchtime(start, end); // 탐색 시간 출력
	delete(set);
	// option(list); // 옵션 실행
}

// 중복파일 리스트에 추가 (체크한 파일 : **, 체크 x : *)
void file2set(FILE * dt, Set *set){
	char *line;
	char *cmpline;
	Set *set_cur = set; // 현재 세트 계산
	while (!feof(dt)){	
		char buf[BUF_SIZE * FILEDATA_SIZE]; // 한 라인 읽기
		line = fgets(buf, BUF_SIZE * FILEDATA_SIZE, dt);
		if(line == NULL) break; // 파일 끝인경우 종료
		char *splitFile[FILEDATA_SIZE] = {NULL, }; // 파일 크기, 파일 경로, hash 분리
		char *ptr = strtok(buf, "|"); // | 기준으로 문자열 자르기
		int idx = 0;
		while (ptr != NULL){
			if(idx < FILEDATA_SIZE) splitFile[idx] = ptr;
			idx++;
			ptr = strtok(NULL, "|");
		}
		if(!strcmp(splitFile[0], "**")) continue; // 이미 중복 체크 됐다면, 패스
		int now_ftell = ftell(dt); // 돌아갈 위치 저장
		bool is_first = true; // 기준 파일 추가해줘야 하는지

		// 중복 파일 있는지 체크
		while (!feof(dt)){
			int cmp_ftell = ftell(dt); // 체크 표시 위해 위치 저장
			char cmp_buf[BUF_SIZE * FILEDATA_SIZE]; // 한 라인 읽기
			cmpline = fgets(cmp_buf, BUF_SIZE * FILEDATA_SIZE, dt);
			if(cmpline == NULL) break; // 파일 끝인경우 종료
			
			char *cmp_split[FILEDATA_SIZE] = {NULL, }; // 파일 크기, 파일 경로, hash 분리
			char *cmp_ptr = strtok(cmp_buf, "|"); // | 기준으로 문자열 자르기
			int cmp_idx = 0;
			while (cmp_ptr != NULL){
				if(cmp_idx < FILEDATA_SIZE) cmp_split[cmp_idx] = cmp_ptr;
				cmp_idx++;
				cmp_ptr = strtok(NULL, "|");
			}
			if(!strcmp(cmp_split[0], "**")) continue; // 이미 중복 체크 됐다면, 패스
			// 파일 크기 다르면 패스
			if(strcmp(splitFile[1], cmp_split[1]))
				continue;
			// 해시값 같으면 리스트에 추가(처음 찾은 경우 기준 라인부터 추가)
			if(!strcmp(splitFile[5], cmp_split[5])){
				if(is_first){
					// 처음 찾은 경우 세트 리스트 생성
					long long filesize = atoll(splitFile[1]);
					int uid = atoi(splitFile[6]);
					int gid = atoi(splitFile[7]);
					int mode = atoi(splitFile[8]);
					append_set(set, filesize, splitFile[2], splitFile[3], splitFile[4], splitFile[5], uid, gid, mode); // 세트에 추가
					set_cur = set_cur->next;
					is_first = false;
				}
				// 리스트에 추가
				long long filesize = atoll(cmp_split[1]);
				int uid = atoi(splitFile[6]);
				int gid = atoi(splitFile[7]);
				int mode = atoi(splitFile[8]);
				append_list(set_cur->nodeList, filesize, cmp_split[2], cmp_split[3], cmp_split[4], cmp_split[5], uid, gid, mode); // 리스트에 추가
				fseek(dt, cmp_ftell, SEEK_SET); // 체크 위치로 이동
				fputs("**|", dt); // **으로 체크 표시
				fseek(dt, cmp_ftell, SEEK_SET); // 체크 위치로 이동
			}
		}
		fseek(dt, now_ftell, SEEK_SET); // 다시 위치로 이동
	}
}

void delete(Set *set){
	while(1){
		int set_size = get_setLen(set); // 세트 크기
		if(set_size == 0) break; // 리스트 없으면 탈출
		
		fprintf(stdout, ">> ");

		char oper[BUF_SIZE];
		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}

		char *splitOper[OPTDEL_LEN] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		bool go_next = true;
		while (ptr != NULL){
			if(idx < OPTDEL_LEN) splitOper[idx] = ptr;
			else{
				fprintf(stderr, "option 입력 초과\n");
				go_next = false; // 다시 프롬프트 출력
				break;
			}			
			idx++;
			ptr = strtok(NULL, " ");
		}

		// INDEX 입력 없을 경우
		if(splitOper[0] == NULL){
			fprintf(stderr, "index 입력이 없음\n");
		}
		else if(!strcmp(splitOper[0], "exit")){
			fprintf(stdout, ">> Back to Prompt\n");
			break;
		}
		else if(splitOper[1] == NULL){
			fprintf(stderr, "옵션 입력 x\n");
		}
		else{
			int option_opt;
			int split_cnt = 0; // 실제 입력한 카운트 계산
			int set_idx = 0;  // 세트 인덱스
			int list_idx = 0; // 리스트 인덱스

			for(int i = 0; i < OPTDEL_LEN; i++){
				if(splitOper[i] == NULL) break;
				split_cnt++;
			}

			int input_opt[5] = {0, }; // 옵션 확인 (-l, -d, -i, -f, -t)

			// getopt로 옵션 분리 및 검사
			while((option_opt = getopt(split_cnt, splitOper, "l:d:ift")) != -1){
				if(!go_next) break; 
				switch(option_opt){
							case 'l' :
								if((optarg == NULL || atoi(optarg) > set_size) || atoi(optarg) == 0){
									fprintf(stderr, "SET_IDX 입력 에러\n");
									go_next = false;
									break;
								}
								set_idx = atoi(optarg);
								input_opt[0]++;
								break;
							case 'd' :
								if(optarg == NULL || atoi(optarg) == 0){
									fprintf(stderr, "LIST_IDX 입력 에러\n");
									go_next = false;
									break;
								}
								list_idx = atoi(optarg);
								input_opt[1]++;
								break;
							case 'i' : 
								input_opt[2]++;
								break;
							case 'f' : 
								input_opt[3]++;
								break;
							case 't' : 
								input_opt[4]++;
								break;												
							default :
								fprintf(stderr, "잘못된 입력\n");
								go_next = false;
								break;
				}
			}
			optind = 0; // optind 초기화

			int opt_idx = 0; // 어느 옵션 실행할지 결정

			if(input_opt[0] != 1){
				fprintf(stderr, "SET_IDX 입력 x\n");
				go_next = false;
			}
			else{
				int input_cnt = 0; // 2개 조합으로 이루어졌는지 확인
				// 입력 옵션 확인
				for(int i = 0; i < 5; i++){
					if(input_opt[i] > 1){
						fprintf(stderr, "옵션 중복 입력\n");
						go_next = false;
						break;						
					}
					else if((idx == OPTDEL_LEN && input_opt[i] == 1) && i > 1){ // i, f, t뒤에 숫자 붙을때
						fprintf(stderr, "올바른 입력 x\n");
						go_next = false;
						break;					
					}
					else if(input_opt[i] == 1){
						opt_idx = i;
						input_cnt++;
					}
				}
				if(input_cnt != 2){
					fprintf(stderr, "옵션 조합 에러\n");
					go_next = false;
				}
			}

			if(go_next){
				Set *set_cur = set;
				Set *set_pre;
				// set_cur 계산
				for(int i = 0; i < set_idx; i++){
					set_pre = set_cur;
					set_cur = set_cur->next;
				}				
				switch (opt_idx){
					case 1: // d옵션
						// 리스트 인덱스 에러 처리
						if(list_idx > get_listLen(set_cur->nodeList)){
							fprintf(stderr, "LIST_IDX 입력 에러\n");
							break;
						}
						delete_d(set, set_cur, set_pre, set_idx, list_idx); // 현재 세트 인덱스 삭제
						break;
					case 2: // i옵션
						delete_i(set, set_cur, set_pre);
						break;
					case 3: // f옵션
						delete_f(set, set_cur, set_pre, set_idx);
						break;
					case 4: // t옵션
						delete_t(set, set_cur, set_pre, set_idx);
						break;														
					default:
						break;
				}
			}
		}
	}
}

// 삭제 d옵션
void delete_d(Set *set, Set *set_cur, Set *set_pre, int set_idx, int list_idx){
	Node *list_cur = set_cur->nodeList; // 현재 리스트
	Node *list_pre; // 이전 리스트

	// 인덱스 같을때까지 탐색
	for(int i = 0; i < list_idx; i++){
		list_pre = list_cur;
		list_cur = list_cur->next;
	}

	// 파일 삭제
	if(unlink(list_cur->path) == -1)
		fprintf(stderr, "%s delete error", list_cur->path);
	else{
		fprintf(stdout, "\"%s\" has been deleted in #%d\n\n", list_cur->path, set_idx);

		del_node(list_cur, list_pre, 1); // 해당 노드 연결 리스트에서 삭제

		// 하나만 남은 경우 제거
		if(get_listLen(set_cur->nodeList) <= 1)
			del_set(set_cur, set_pre);
		print_set(set); // 세트 출력
		if(get_setLen(set)) fprintf(stdout, "\n"); // 학번 프롬프트 출력 시 \n x
	}
}

// 삭제 i옵션
void delete_i(Set *set, Set *set_cur, Set *set_pre){
	Node *cur = set_cur->nodeList->next;
	Node *pre = set_cur->nodeList; // 삭제 시 cur 위치 복구해줄 포인터

	// 세트 내에서 탐색
	while (cur != NULL){
		char *oper = malloc(sizeof(char) * BUF_SIZE);
		fprintf(stdout, "Delete \"%s\"? [y/n] ", cur->path);

		fgets(oper, BUF_SIZE, stdin); // yes or no
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}		
		
		if(!strcasecmp(oper, "Y") || !strcasecmp(oper, "y")){
			//파일 삭제
			if(unlink(cur->path) == -1){
				fprintf(stderr, "%s delete error", cur->path);
				return;
			}
			else{
				del_node(cur, pre, 1); // 해당 노드 삭제
				cur = pre->next; // 삭제했으므로 cur 위치 복구
			}
		}
		else if(!strcasecmp(oper, "N") && !strcasecmp(oper, "n")){
			pre = cur;
			cur = cur->next;
		}
		else{
			fprintf(stderr, "y, Y, n, N 중 하나 입력\n");
			return;
		}
		free(oper);
	
		if(cur == NULL) break; // 마지막인 경우 종료
	}
	fprintf(stdout, "\n");

	// 하나만 남은 경우 제거
	if(get_listLen(set_cur->nodeList) <= 1)
		del_set(set_cur, set_pre);
	print_set(set); // 세트 출력
	if(get_setLen(set)) fprintf(stdout, "\n"); // 학번 프롬프트 출력 시 \n x
}

// 삭제 f옵션
void delete_f(Set *set, Set *set_cur, Set *set_pre, int set_idx){
	Node *cur = set_cur->nodeList->next;
	Node *pre = set_cur->nodeList; // 삭제 시 cur 위치 복구해줄 포인터

	Node *recent = get_recent(cur); // 가장 최근 시간 노드 구하기

	// 세트 내에서 탐색
	while (cur != NULL){
		// 가장 최근 수정 노드 아니라면
		if(cur != recent){
			//파일 삭제
			if(unlink(cur->path) == -1){
				fprintf(stderr, "%s delete error", cur->path);
				return;
			}
			else{
				del_node(cur, pre, 1);
				cur = pre->next; // 삭제했으므로 cur 위치 복구	
			}		
		}
		else{ // 최근 수정 노드인 경우
			pre = cur;
			cur = cur->next;
		}
		if(cur == NULL) break; // 마지막인 경우 종료
	}
	fprintf(stdout, "Left file in #%d : %s (%-15s)\n\n", set_idx, recent->path, recent->mtime);

	// 하나만 남은 경우 제거
	if(get_listLen(set_cur->nodeList) <= 1)
		del_set(set_cur, set_pre);
	print_set(set); // 세트 출력
	if(get_setLen(set)) fprintf(stdout, "\n"); // 학번 프롬프트 출력 시 \n x
}

// t옵션
void delete_t(Set *set, Set *set_cur, Set *set_pre, int set_idx){
	struct passwd *pwd; // 사용자 이름
	if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
		fprintf(stderr, "user id error");
	}
	char trash_dir[BUF_SIZE]; // 제거된 파일 경로
	char trashinfo_dir[BUF_SIZE]; // 제거된 파일 정보 경로
	strcpy(trash_dir, pwd->pw_dir);
	strcpy(trashinfo_dir, pwd->pw_dir);

	char now_path[BUF_SIZE];
	getcwd(now_path, BUF_SIZE); // 현재 디렉토리 경로 얻기

	if(chdir(pwd->pw_dir) < 0){
		fprintf(stderr, "chdir error\n");
		exit(1);
	}

	strcat(trash_dir, "/.Trash");
	strcat(trashinfo_dir, "/.Trash");
	// 휴지통 경로 생성 (이미 존재한 경우는 에러x)
	if(mkdir(trash_dir, 0776) == -1 && errno != EEXIST){ 
		fprintf(stderr, "directory create error: %s\n", strerror(errno)); 
		exit(1);
	}
	// files 폴더 생성
	strcat(trash_dir, "/files");
	if(mkdir(trash_dir, 0776) == -1 && errno != EEXIST){ 
		fprintf(stderr, "directory create error: %s\n", strerror(errno)); 
		exit(1);
	}
	// info 폴더 생성
	strcat(trashinfo_dir, "/info");
	if(mkdir(trashinfo_dir, 0776) == -1 && errno != EEXIST){ 
		fprintf(stderr, "directory create error: %s\n", strerror(errno)); 
		exit(1);
	}

	Node *cur = set_cur->nodeList->next;
	Node *pre = set_cur->nodeList; // 삭제 시 cur 위치 복구해줄 포인터

	Node *recent = get_recent(cur); // 가장 최근 시간 노드 구하기

	// 세트 내에서 탐색
	while (cur != NULL){
		// 가장 최근 수정 노드 아니라면
		if(cur != recent){
			// 파일 이동을 위해 먼저 복사하기
			char *str = malloc(sizeof(char) * PATH_SIZE);
			sprintf(str, ".Trash/files%s", strrchr(cur->path, '/')); // trash 파일 경로 만들어주기
			if(link(cur->path, str) == -1){
				if(errno != EEXIST){
					fprintf(stderr, "link error\n");
					exit(1);
				}
				// 같은 이름의 파일이 존재할 경우 -> cp1, cp2, ... 이름붙여 휴지통으로 이동
				else{
					// int cpnum = 1;
					// while(1){
					// 	char str2[PATH_SIZE];
					// 	if(strrchr(cur->path, '.') == NULL) // 확장자 없는 파일인경우
					// 		sprintf(str2, ".Trash/files/cp%d", cpnum);
					// 	else 
					// 		sprintf(str2, ".Trash/files/cp%d%s", cpnum, strrchr(cur->path, '.'));
					// 	cpnum++;
					// 	if(!(link(cur->path, str2) == -1 && errno == EEXIST)) break; // 중복파일 있으면 다음 숫자 이름붙임
					// }
				}
			}
			free(str);

			// 그 후 기존 파일 삭제
			if(unlink(cur->path) == -1){
				fprintf(stderr, "%s delete error", cur->path);
				return;
			}
			else{
				FILE *fp;
				if((fp = fopen(".Trash/info/info.txt", "a+")) == NULL){ // a+모드 실행 (계속 쓰기 위해)
					fprintf(stderr, "fopen error\n");
				}	
				// info.txt에 삭제 파일 정보 저장
				if(fp != NULL){
					fputs("*", fp); // 체크 여부, **이면 복구된 파일
					fputs("|", fp);
	
					time_t t = time(NULL);
					struct tm *tm = localtime(&t);
					char *time_str = malloc(sizeof(char) * BUF_SIZE);
					strftime(time_str, BUF_SIZE, "%Y-%m-%d", tm);
					fputs(time_str, fp); // 날짜
					fputs("|", fp);
					free(time_str);

					char *time_str2 = malloc(sizeof(char) * BUF_SIZE);
					strftime(time_str, BUF_SIZE, "%H:%M:%S", tm);
					fputs(time_str2, fp); // 시간
					fputs("|", fp);
					free(time_str2);

					char to_str[BUF_SIZE];
					sprintf(to_str, "%lld", cur->filesize);
					fputs(to_str, fp); // 파일 크기
					fputs("|", fp);
					fputs(cur->path, fp); // 파일 절대경로
					fputs("|", fp);
					fputs(cur->mtime, fp); // mtime
					fputs("|", fp);
					fputs(cur->atime, fp); // atime
					fputs("|", fp);
					fputs(cur->hash, fp); // hash
					fputs("|", fp);
					sprintf(to_str, "%d", cur->uid); // uid
					fputs(to_str, fp);
					fputs("|", fp);
					sprintf(to_str, "%d", cur->gid); // gid
					fputs(to_str, fp);
					fputs("|", fp);
					sprintf(to_str, "%lu", cur->mode); // mode
					fputs(to_str, fp);
					fputs("|", fp);		
					fputs("\n", fp); // enter
				}
				fclose(fp);
				del_node(cur, pre, 0);
				cur = pre->next; // 삭제했으므로 cur 위치 복구	
			}
		}
		else{ // 최근 수정 노드인 경우
			pre = cur;
			cur = cur->next;
		}
		if(cur == NULL) break; // 마지막인 경우 종료
	}
	fprintf(stdout, "All files in #%d have moved to Trash except \"%s\" (%-15s)\n\n", set_idx, recent->path, recent->mtime);

	if(chdir(now_path) < 0){ // 현재 디렉토리로 복귀
		fprintf(stderr, "chdir error\n");
		exit(1);
	}

	// 하나만 남은 경우 제거
	if(get_listLen(set_cur->nodeList) <= 1)
		del_set(set_cur, set_pre);
	print_set(set); // 세트 출력
	if(get_setLen(set)) fprintf(stdout, "\n"); // 학번 프롬프트 출력 시 \n x
}

// 추가기능 : a옵션
// [LIST_IDX] a : 세트의 해당번째 인덱스 모두 삭제
// 해당 인덱스 없을 경우 삭제 x
void option_a(int list_idx, Node *list){
	// Node *cur = list->next;
	// Node *pre = list; // 삭제 시 cur 위치 복구해줄 포인터

	// // 세트 같을때까지 탐색
	// while (cur != NULL){
	// 	if(cur->idx_num == list_idx){ // 인덱스 같으면 삭제
	// 		//파일 삭제
	// 		if(unlink(cur->path) == -1){
	// 			fprintf(stderr, "%s delete error", cur->path);
	// 			return;
	// 		}
	// 		else{
	// 			del_node(list, cur->set_num, cur->idx_num); // 노드 삭제
	// 			cur = pre->next;
	// 		}
	// 	}
	// 	else{
	// 		pre = cur;
	// 		cur = cur->next;
	// 	}
	// }
	// del_onlyList(list); // 하나 남은 경우 삭제
	// print_list(list); // 프린트
	// if(get_listLen(list)) fprintf(stdout, "\n"); // 학번 프롬프트 출력 시 \n x
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

// md5 값 조회
char *get_md5(FILE *fp){
	MD5_CTX c;
	static unsigned char md[MD5_DIGEST_LENGTH];
	static unsigned char buf[BUF_MAX];
	int fd = fileno(fp);

	MD5_Init(&c);
	int i = read(fd, buf, BUF_MAX);
	MD5_Update(&c, buf, (unsigned long)i);

	MD5_Final(md,&c);

	static char md5string[MD5_DIGEST_LENGTH * 2 + 1];
	for(int i = 0; i < MD5_DIGEST_LENGTH; ++i)
		sprintf(&md5string[i*2], "%02x", (unsigned int)md[i]);
	return md5string;
}

// sha1 값 조회
char *get_sha1(FILE *fp){
	SHA_CTX c;
	static unsigned char md[SHA_DIGEST_LENGTH];
	static unsigned char buf[BUF_MAX];
	int fd = fileno(fp);

	SHA1_Init(&c);
	int i = read(fd, buf, BUF_MAX);
	SHA1_Update(&c, buf, (unsigned long)i);

	SHA1_Final(md,&c);

	static char sha1string[SHA_DIGEST_LENGTH * 2 + 1];
	for(int i = 0; i < SHA_DIGEST_LENGTH; ++i)
		sprintf(&sha1string[i*2], "%02x", (unsigned int)md[i]);
	return sha1string;
}

// 시간 정보 포맷에 맞게 변환
char* get_time(time_t stime, char * str){
	struct tm *tm;
	tm = localtime(&stime);

	strftime(str, BUF_SIZE, "%Y-%m-%d %H:%M:%S", tm);
	return str;
}

// 탐색 소요 시간 계산
void get_searchtime(struct timeval start, struct timeval end){
	end.tv_sec -= start.tv_sec; // 초 부분 계산

	if(end.tv_usec < start.tv_usec){ // ms 연산 결과가 마이너스인 경우 고려
		end.tv_sec--;
		end.tv_usec += 1000000;
	}

	end.tv_usec -= start.tv_usec;
	fprintf(stdout, "\nSearching time: %ld:%06ld(sec:usec)\n\n", end.tv_sec, end.tv_usec);
}

const char *size2comma(long long n){ 
	static char comma_str[COMMA_SIZE];
	char str[COMMA_SIZE];
	int idx, len, cidx = 0, mod; 
	
	sprintf(str, "%lld", n); 
	len = strlen(str); 
	mod = len % 3; 
	for(idx = 0; idx < len; idx++){
		if(idx != 0 && (idx) % 3 == mod){ 
			comma_str[cidx++] = ',';
		} 
		comma_str[cidx++] = str[idx]; 
	} 
	comma_str[cidx] = 0x00; 
	return comma_str; 
}

// 세트 추가
void append_set(Set *set, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[digest_len], int uid, int gid, int mode){
	Set *cur_set;
    Set *newSet = (Set *)malloc(sizeof(Set));
    memset(newSet, 0, sizeof(Set));

    newSet->filesize = filesize;
    strcpy(newSet->hash, hash);

    newSet->nodeList = (Node *)malloc(sizeof(Node));
    memset(newSet->nodeList, 0, sizeof(Node));

    append_list(newSet->nodeList, filesize, path, mtime, atime, hash, uid, gid, mode);
    newSet->next = NULL;

    if (set->next == NULL) {
        set->next = newSet;
    }    
	else{
		cur_set = set->next;
		while (cur_set->next != NULL)
			cur_set = cur_set->next;

		cur_set->next = newSet;
	}
}

// 리스트 끝에 추가
void append_list(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[digest_len], int uid, int gid, int mode){
	Node *cur_list;

	Node *newNode = (Node *)malloc(sizeof(Node));
	memset(newNode, 0, sizeof(Node));
	newNode->filesize = filesize;
	strcpy(newNode->path, path);
	strcpy(newNode->mtime, mtime);
	strcpy(newNode->atime, atime);
	strcpy(newNode->hash, hash);
	newNode->uid = uid;
	newNode->gid = gid;
	newNode->mode = mode;
	newNode->next = NULL;

	if (list->next == NULL)
		list->next = newNode;
	else {
		cur_list = list->next;
		while (cur_list->next != NULL)
			cur_list = cur_list->next;

		cur_list->next = newNode;
	}
}

// 세트 크기 구하기
int get_setLen(Set *set){
    int cnt = 0; // head 제외
    Set *cur = set->next;
    
    while(cur != NULL){
        cnt++;
        cur = cur->next;
    }

    return cnt;
}

// 리스트 크기 구하기
int get_listLen(Node *list){
    int cnt = 0; // head 제외
    Node *cur = list->next;
    
    while(cur != NULL){
        cnt++;
        cur = cur->next;
    }

    return cnt;
}

// 세트 출력
void print_set(Set *set){
	Set *set_cur = set->next;
	int set_idx = 1;

	// 세트 출력
	while (set_cur != NULL){
		Node *node_cur = set_cur->nodeList->next;
		int i = 1;

		fprintf(stdout, "---- Identical files #%d (%s bytes - ", set_idx++, size2comma(node_cur->filesize));
		fprintf(stdout, "%s", node_cur->hash);
		fprintf(stdout, ") ----\n");

		while (node_cur != NULL){
			fprintf(stdout, "[%d] %s (mtime : %-15s) (atime : %-15s) (uid : %d) (gid : %d) (mode : %ld)\n", i++, node_cur->path, node_cur->mtime, node_cur->atime, node_cur->uid, node_cur->gid, node_cur->mode);
			node_cur = node_cur->next;
		}
		printf("\n");
		set_cur = set_cur->next;
	}
}

// 메모리 해제
void delete_set(Set *set){
	Set *cur = set;
	Set *next;
	while (cur != NULL){
		next = cur->next;
		free(cur);
		cur = next;
	}
}

// 큐 초기화
void init_queue(queue *q){
    q->front = q->rear = NULL; 
    q->cnt = 0;
}

// 특정 세트 삭제
void del_set(Set *cur, Set *pre){
	// 해당 인덱스 삭제
	if(cur->next != NULL){ // 중간 인덱스 삭제할 경우
		pre->next = cur->next; // 이전 노드의 다음 -> 삭제할 노드의 다음
		free(cur);
		return;
	}
	else{ // 마지막 인덱스 삭제할 경우
		pre->next = NULL;
		cur->next = NULL;
		free(cur);
		return;
	}
}

// todo : RESTORE 제거
// 특정 노드 삭제
// log_type : 0 -> REMOVE, 1 -> DELETE, 2 -> RESTORE
void del_node(Node *cur, Node *pre, int log_type){
	struct passwd *pwd; // 사용자 이름
	// 로그 기록
	if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
		fprintf(stderr, "user id error");
	}
	char log_dir[BUF_SIZE]; // 제거된 파일 경로
	strcpy(log_dir, pwd->pw_dir);
	strcat(log_dir, "/.duplicate_20182615.log");
	FILE *fp;
	if((fp = fopen(log_dir, "a+")) == NULL){ // a+모드 실행 (계속 쓰기 위해)
		fprintf(stderr, "fopen error\n");
	}

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char *time_str = malloc(sizeof(char) * BUF_SIZE);
	strcpy(time_str, get_time(t, time_str)); // 현재 시간 포맷화

	// 타입에 따른 로그 기록
	switch (log_type){
		case 0 :
			fputs("[REMOVE]", fp);
			break;
		case 1 : 
			fputs("[DELETE]", fp);
			break;	
		case 2 : 
			fputs("[RESTORE]", fp);
			break;						
		default:
			break;
	}

	fputs(" ", fp);
	fputs(cur->path, fp);
	fputs(" ", fp);
	fputs(time_str, fp);
	fputs(" ", fp);
	fputs(pwd->pw_name, fp);
	fputs("\n", fp); // enter
	
	free(time_str);

	// 해당 인덱스 삭제
	if(cur->next != NULL){ // 중간 인덱스 삭제할 경우
		pre->next = cur->next; // 이전 노드의 다음 -> 삭제할 노드의 다음
		free(cur);
	}
	else{ // 마지막 인덱스 삭제할 경우
		pre->next = NULL;
		cur->next = NULL;
		free(cur);
	}
	fclose(fp);
}

// 해시 일치할경우 인덱스 반환
int search_set(Set *set, unsigned char hash[digest_len]){
	Set *cur = set;
	int idx = 0;

	while (cur != NULL){
		if (!strcmp(cur->hash, hash))
			return idx;
		cur = cur->next;
		idx++;
	}

	return 0;
}

// 세트 버블정렬(오름차순)
// 파일크기순 정렬 (bfs이므로 파일크기 같을 경우 절대경로 짧은 순 -> 임의(아스키 코드 순))
void sort_upSet(Set *set, int set_size){
    Set *cur = set->next; // head 다음
    for (int i = 0; i < set_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < set_size - 1 - i; j++){
            if(cur->filesize > cur->next->filesize)
                swap_set(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = set->next;
    }
}

void swap_set(Set *set1, Set *set2){
    int fileSize;
	Node *nodeList;
	unsigned char hash[BUF_SIZE];

	fileSize = set1->filesize;
	nodeList = set1->nodeList;
	strcpy(hash, set1->hash);

    set1->filesize = set2->filesize;
	set1->nodeList = set2->nodeList;
	strcpy(set1->hash, set2->hash);

    set2->filesize = fileSize;
	set2->nodeList = nodeList;
	strcpy(set2->hash, hash);
}

void swap_node(Node *node1, Node *node2){
    long long fileSize;
	char path[BUF_SIZE];
	char mtime[BUF_SIZE];
	char atime[BUF_SIZE];
	unsigned char hash[BUF_SIZE];
	int uid;
	int gid;
	unsigned long mode;

	fileSize = node1->filesize;
	strcpy(path, node1->path);
	strcpy(mtime, node1->mtime);
	strcpy(atime, node1->atime);
	strcpy(hash, node1->hash);
	uid = node1->uid;
	gid = node1->gid;
	mode = node1->mode;

    node1->filesize = node2->filesize;
	strcpy(node1->path, node2->path);
	strcpy(node1->mtime, node2->mtime);
	strcpy(node1->atime, node2->atime);
	strcpy(node1->hash, node2->hash);
	node1->uid = node2->uid;
	node1->gid = node2->gid;
	node1->mode = node2->mode;

    node2->filesize = fileSize;
	strcpy(node2->path, path);
	strcpy(node2->mtime, mtime);
	strcpy(node2->atime, atime);
	strcpy(node2->hash, hash);
	node2->uid = uid;
	node2->gid = gid;
	node2->mode = mode;
}

// 가장 최근 시간 노드 구하기
Node *get_recent(Node *cur){
	Node *recent;

	// 파일 정보 조회
	struct stat st;
	time_t recent_time = -9999999;

	// 세트 내에서 탐색
	while (cur != NULL){
		// 파일 정보 얻기
		if(lstat(cur->path, &st) == -1){
			fprintf(stderr, "stat error\n");
			exit(1);
		}
		// 가장 최근 시간 노드 구하기
		if(recent_time < st.st_mtime){
			recent_time = st.st_mtime;
			recent = cur;
		}
		cur = cur->next;
		if(cur == NULL) break; // 마지막인 경우 종료
	}
	return recent;
}

bool isEmpty_queue(queue *q){
	return q->cnt == 0;
}

// 큐에 데이터 삽입
void push_queue(queue *q, char path[BUF_SIZE]){
    Qnode *newNode = malloc(sizeof(Qnode)); // newNode 생성
	strcpy(newNode->path, path);
    newNode->next = NULL;

    if (isEmpty_queue(q)) // 큐가 비어있을 때
        q->front = newNode;
    else
        q->rear->next = newNode; //맨 뒤의 다음을 newNode로 설정
    
    q->rear = newNode; //맨 뒤를 newNode로 설정   
    q->cnt++; //큐 노드 개수 1 증가
}

// 큐 pop
char *pop_queue(queue *q){
    static char data[BUF_SIZE];
    Qnode *ptr;
    if (isEmpty_queue(q)){
        fprintf(stderr, "Error : Queue is empty!\n");
        return data;
    }
    ptr = q->front;
	strcpy(data, ptr->path);
    q->front = ptr->next;  // ptr의 다음 노드를 front로 설정
    free(ptr);
    q->cnt--;
    return data;
}