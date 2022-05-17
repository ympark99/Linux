#include "ssu_find.h"

// todo : 복구시 **표시

void restore(Set *set, Trash *tr, int restore_idx){
    Trash *cur = tr;
    Trash *pre;

    // 복구할 노드 찾기
	for(int i = 0; i < restore_idx; i++){
		pre = cur;
		cur = cur->next;
	}

	struct passwd *pwd; // 사용자 이름
	if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
		fprintf(stderr, "user id error");
	}
	char trashinfo_dir[BUF_SIZE]; // 제거된 파일 정보 경로
	strcpy(trashinfo_dir, pwd->pw_dir);
	strcat(trashinfo_dir, "/.Trash/info/info.txt");

	char now_path[BUF_SIZE];
	getcwd(now_path, BUF_SIZE); // 현재 디렉토리 경로 얻기

	if(chdir(pwd->pw_dir) < 0){
		fprintf(stderr, "chdir error\n");
		exit(1);
	}

    // info.txt open
    FILE *fp;
	fp = fopen(trashinfo_dir, "r+t"); // r+모드 실행 (체크 표시 남겨야 하므로)
	if(fp == NULL) fprintf(stderr, "fopen read error\n");

	char *line;
	char *cmpline;

    // 복구할 파일 적혀있는 곳 찾아서 복구 표시 남기기(**)
	while (!feof(fp)){	
        int rest_ftell = ftell(fp); // 복구 표시 위해 위치 저장
		char buf[BUF_SIZE * TRASHDATA_SIZE]; // 한 라인 읽기
		line = fgets(buf, BUF_SIZE * TRASHDATA_SIZE, fp);
		if(line == NULL) break; // 파일 끝인경우 종료
		char *splitFile[TRASHDATA_SIZE] = {NULL, }; // 파일 크기, 파일 경로, hash 분리
		char *ptr = strtok(buf, "|"); // | 기준으로 문자열 자르기
		int idx = 0;
		while (ptr != NULL){
			if(idx < TRASHDATA_SIZE) splitFile[idx] = ptr;
			idx++;
			ptr = strtok(NULL, "|");
		}
		if(!strcmp(splitFile[0], "**")) continue; // 이미 복구 체크 됐다면, 패스

        // 절대경로 같으면 복구표시 남기기
        if(!strcmp(splitFile[4], cur->path)){
            fseek(fp, rest_ftell, SEEK_SET); // 체크 위치로 이동
            fputs("**|", fp); // **으로 체크 표시
            break; // 종료  
        }
	}
    fclose(fp);

    // 파일 이동을 위해 먼저 복사하기
    char *str = malloc(sizeof(char) * PATH_SIZE);
    sprintf(str, ".Trash/files%s", strrchr(cur->path, '/')); // trash 파일 경로 만들어주기
    // path에 link
    if(link(str, cur->path) == -1){
        if(errno != EEXIST){
            fprintf(stderr, "link error\n");
            exit(1);
        }
        // 같은 이름의 파일이 존재할 경우 -> cp1, cp2, ... 이름붙여 휴지통으로 이동
        else{
        }
    }
    // 그 후 쓰레기통 파일 삭제
    if(unlink(str) == -1){
        fprintf(stderr, "%s delete error", str);
        return;
    }
    free(str);
    // // todo : 리스트가 애초에 없을텐데 어떻게?
    // restore_set(set, cur); // 해당 파일과 해시가 같은 set가 존재하면 추가

    fprintf(stdout, "[RESTORE] success for %s\n", cur->path); // 성공 메시지 출력
    del_trnode(cur, pre); // trash list에서 제거

	if(chdir(now_path) < 0){ // 현재 디렉토리로 복귀
		fprintf(stderr, "chdir error\n");
		exit(1);
	}
}

// 세트에 링크드리스트 존재할 경우 복구
void restore_set(Set *set, Trash *cur_tr){
    Set *cur_set = set->next;
    while(cur_set != NULL){
        // 해시값 같은 set 있으면 존재한다는 것 -> 추가
        if(!strcmp(cur_set->hash, cur_tr->hash)){
            append_list(cur_set->nodeList, cur_tr->filesize, cur_tr->path, cur_tr->mtime, cur_tr->atime,
            cur_tr->hash, cur_tr->uid, cur_tr->gid, cur_tr->mode);
            break;
        }
    }
}

// 쓰레기통 노드 삭제(복구 표시)
void del_trnode(Trash *cur, Trash *pre){
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

    fputs("[RESTORE]", fp);
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