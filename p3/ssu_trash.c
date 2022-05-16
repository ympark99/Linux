#include "ssu_find.h"

void trash(Trash *tr, bool c_opt[5], bool sort_up){
    file2tr(tr);

    // todo : 정렬
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
        fprintf(stdout, "[ %d] ", set_idx++);
		fprintf(stdout, "%-64s%-10s%-20s%-8s\n", cur->path, size2comma(cur->filesize), cur->delete_date, cur->delete_time);

		cur = cur->next;
	}
}