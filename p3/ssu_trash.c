#include "ssu_find.h"

void trash(Trash *tr, bool c_opt[5], bool sort_up){
    file2tr(tr); // trash 링크드 리스트 제작

	int trash_size = get_trashLen(tr);
	// 파일 절대 경로 정렬
    if(c_opt[1]){
        if(sort_up) sort_pathTrash(tr, trash_size, true); // 오름차순
        else sort_pathTrash(tr, trash_size, false); // 내림차순
    }	
    // 파일 크기 순 정렬일 경우 그대로 출력 -> 리스트는 크기 같으므로 경로 순 출력
    else if(c_opt[2]){
        if(sort_up) sort_sizeTrash(tr, trash_size, true); // 오름차순
        else sort_sizeTrash(tr, trash_size, false); // 내림차순
    }
	// 날짜 순 정렬일 경우
    else if(c_opt[3]){
        if(sort_up) sort_timesTrash(tr, trash_size, true, true); // 오름차순
        else sort_timesTrash(tr, trash_size, false, true); // 내림차순
    }
	// 시간 순 정렬일 경우
    else if(c_opt[4]){
        if(sort_up) sort_timesTrash(tr, trash_size, true, false); // 오름차순
        else sort_timesTrash(tr, trash_size, false, false); // 내림차순
    }
    print_trash(tr);
}

// 중복파일 쓰레기통 링크드 리스트에 추가 (복구된 파일 : **, 체크 x : *)
void file2tr(Trash *tr){
	struct passwd *pwd; // 사용자 이름
	if((pwd = getpwuid(getuid())) == NULL){ // 사용자 아이디, 홈 디렉토리 경로 얻기
		fprintf(stderr, "user id error");
	}
	char trashinfo_dir[BUF_SIZE]; // 제거된 파일 정보 경로
	strcpy(trashinfo_dir, pwd->pw_dir);
	strcat(trashinfo_dir, "/.Trash/info/info.txt");

    FILE *fp;
	fp = fopen(trashinfo_dir, "r+t"); // r+모드 실행 (체크 표시 남겨야 하므로)
	if(fp == NULL) fprintf(stderr, "fopen read error\n");

	char *line;
	char *cmpline;
	Trash *cur = tr; // 현재 세트 계산

    // 쓰레기통 링크드 리스트 제작
	while (!feof(fp)){	
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
		if(!strcmp(splitFile[0], "**")) continue; // 이미 중복 체크 됐다면, 패스

        append_trash(tr, splitFile); // append trash list
	}
}

// 리스트 끝에 추가
void append_trash(Trash *tr, char *splitFile[TRASHDATA_SIZE]){
	Trash *cur;

    // 리스트에 추가
    long long filesize = atoll(splitFile[3]);
    int uid = atoi(splitFile[8]);
    int gid = atoi(splitFile[9]);
    int mode = atoi(splitFile[10]);

	Trash *newNode = (Trash *)malloc(sizeof(Trash));
	memset(newNode, 0, sizeof(Trash));

    strcpy(newNode->delete_date, splitFile[1]);
    strcpy(newNode->delete_time, splitFile[2]);
	newNode->filesize = filesize;
	strcpy(newNode->path, splitFile[4]);
	strcpy(newNode->mtime, splitFile[5]);
	strcpy(newNode->atime, splitFile[6]);
	strcpy(newNode->hash, splitFile[7]);
	newNode->uid = uid;
	newNode->gid = gid;
	newNode->mode = mode;
	newNode->next = NULL;

	if (tr->next == NULL)
		tr->next = newNode;
	else {
		cur = tr->next;
		while (cur->next != NULL)
			cur = cur->next;

		cur->next = newNode;
	}
}

// 쓰레기통 출력
void print_trash(Trash *tr){
	Trash *cur = tr->next;
	int set_idx = 1;
    fprintf(stdout, "     FILENAME                                                        SIZE      DELETION DATE       DELETION TIME\n");
	while (cur != NULL){
		if(set_idx < 10) fprintf(stdout, "[ %d] ", set_idx++);
		else fprintf(stdout, "[%d] ", set_idx++);
		fprintf(stdout, "%-64s%-10s%-20s%-8s\n", cur->path, size2comma(cur->filesize), cur->delete_date, cur->delete_time);

		cur = cur->next;
	}
}

// 절대 경로 기준 정렬
void sort_pathTrash(Trash *tr, int tr_size, bool sort_up){
    Trash *cur = tr->next; // head 다음
	int get_longPath = -1;
    for (int i = 0; i < tr_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < tr_size - 1 - i; j++){
			get_longPath = long_path(cur->path, cur->next->path);

            if((get_longPath == 1) && sort_up)
                swap_trash(cur, cur->next); // swap
            else if((get_longPath == 0) && !sort_up)
                swap_trash(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = tr->next;
    }
}

// 쓰레기통 파일크기순 정렬 (bfs이므로 파일크기 같을 경우 절대경로 짧은 순 -> 임의(아스키 코드 순))
void sort_sizeTrash(Trash *tr, int tr_size, bool sort_up){
    Trash *cur = tr->next; // head 다음
    for (int i = 0; i < tr_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < tr_size - 1 - i; j++){
            if(cur->filesize > cur->next->filesize && sort_up)
                swap_trash(cur, cur->next); // swap
            else if(cur->filesize < cur->next->filesize && !sort_up)
                swap_trash(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = tr->next;
    }
}

// 날짜 기준 정렬, sort_date : 날짜 소팅하는지 확인
void sort_timesTrash(Trash *tr, int tr_size, bool sort_up, bool sort_date){
    Trash *cur = tr->next; // head 다음
	int get_dateRes = -1;
    for (int i = 0; i < tr_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < tr_size - 1 - i; j++){
			get_dateRes = sort_date ? recent_date(cur->delete_date, cur->next->delete_date) 
			: recent_date(cur->delete_time, cur->next->delete_time);

            if((get_dateRes == 1) && sort_up)
                swap_trash(cur, cur->next); // swap
            else if((get_dateRes == 0) && !sort_up)
                swap_trash(cur, cur->next); // swap
            cur = cur->next;
        }
        cur = tr->next;
    }
}

// path2 절대 경로가 더 길면 0 반환, 더 짧으면 1반환, 같으면 2반환
int long_path(char path1[PATH_SIZE], char path2[PATH_SIZE]){
	int path1_cnt, path2_cnt = 0;
	for(int i = 0; i < PATH_SIZE; i++){
		if(path1[i] == '/') path1_cnt++;
		if(path2[i] == '/') path2_cnt++;
	}
	if(path2_cnt > path1_cnt) return 0;
	else if(path2_cnt < path1_cnt) return 1;
	return 2;
}

// date2가 최근이면(더 크면) 0 반환, 더 작으면 1반환, 같으면 2반환
int recent_date(char date1[DELTIME_LEN], char date2[DELTIME_LEN]){
	for(int i = 0; i < DELTIME_LEN; i++){
		if(date2[i] > date1[i]) return 0;
		else if(date2[i] < date1[i]) return 1;
	}
	return 2;
}

// 리스트 크기 구하기
int get_trashLen(Trash *tr){
    int cnt = 0; // head 제외
    Trash *cur = tr->next;
    while(cur != NULL){
        cnt++;
        cur = cur->next;
    }
    return cnt;
}

void swap_trash(Trash *tr1, Trash *tr2){
    long long fileSize;
	char delete_date[DELTIME_LEN]; // 삭제 날짜
	char delete_time[DELTIME_LEN]; // 삭제 시간
	char path[BUF_SIZE];
	char mtime[BUF_SIZE];
	char atime[BUF_SIZE];
	unsigned char hash[BUF_SIZE];
	int uid;
	int gid;
	unsigned long mode;

	fileSize = tr1->filesize;
	strcpy(delete_date, tr1->delete_date);
	strcpy(delete_time, tr1->delete_time);
	strcpy(path, tr1->path);
	strcpy(mtime, tr1->mtime);
	strcpy(atime, tr1->atime);
	strcpy(hash, tr1->hash);
	uid = tr1->uid;
	gid = tr1->gid;
	mode = tr1->mode;

    tr1->filesize = tr2->filesize;
	strcpy(tr1->delete_date, tr2->delete_date);
	strcpy(tr1->delete_time, tr2->delete_time);
	strcpy(tr1->path, tr2->path);
	strcpy(tr1->mtime, tr2->mtime);
	strcpy(tr1->atime, tr2->atime);
	strcpy(tr1->hash, tr2->hash);
	tr1->uid = tr2->uid;
	tr1->gid = tr2->gid;
	tr1->mode = tr2->mode;

    tr2->filesize = fileSize;
	strcpy(tr2->delete_date, delete_date);
	strcpy(tr2->delete_time, delete_time);
	strcpy(tr2->path, path);
	strcpy(tr2->mtime, mtime);
	strcpy(tr2->atime, atime);
	strcpy(tr2->hash, hash);
	tr2->uid = uid;
	tr2->gid = gid;
	tr2->mode = mode;
}