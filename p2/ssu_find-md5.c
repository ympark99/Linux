#include <stdio.h>
#include <sys/types.h> // stat 사용
#include <sys/stat.h> // stat 사용
#include <sys/time.h>
#include <unistd.h> // stat 사용
#include <string.h> // string 관련 함수 사용
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h> // scandir 사용
#include <math.h> // modf 사용
#include <openssl/md5.h>
#include <sys/time.h> // gettimeofday 사용
#include "ssu_find-md5.h"

// md5 관련 함수 실행
// 입력인자 : 명령어 split, 찾을 디렉토리 경로, 현재 링크드리스트, 현재 큐
// 인자배열 : fmd5, 파일 확장자, 최소크기, 최대크기, 디렉토리
void ssu_find_md5(char *splitOper[OPER_LEN], char *find_path, struct timeval start, Node *list, queue *q, bool from_main){
	struct dirent **filelist; // scandir 파일목록 저장 구조체
	int cnt; // return 값

    // scandir로 파일목록 가져오기 (디렉토리가 아닐 경우 에러)
	if((cnt = scandir(find_path, &filelist, scandirFilter, alphasort)) == -1){
        fprintf(stderr, "target is not directory\n");
		return;
	}

	// 절대경로 변환
	char dir_path[BUF_SIZE];
	if(realpath(find_path, dir_path) == NULL){
		fprintf(stderr, "realpath error\n");
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
			if(strlen(splitOper[1]) > 1 ){
				char* ori_fname = strrchr(splitOper[1], '.'); // 지정 파일 확장자
				char* cmp_fname = strrchr(filelist[i]->d_name, '.'); // 가져온 파일 확장자

				// 확장자가 없거나 확장자가 다르면 패스
				if((cmp_fname == NULL) || strcmp(ori_fname, cmp_fname) != 0)
					continue;
			}

			// 파일 정보 조회
			struct stat st;
			// 파일 정보 얻기
			if(lstat(pathname, &st) == -1){
				fprintf(stderr, "stat error\n");
				continue;
			}
			long double filesize = (long double) st.st_size; // 파일크기 구하기

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

			// md5값 구하기
			FILE *fp = fopen(pathname, "r");
			if (fp == NULL){ // fopen 에러시 패스
				fprintf(stderr, "fopen error\n");
				continue;
			}
			unsigned char filehash[MD5_DIGEST_LENGTH]; // 해쉬값 저장할 문자열
			strcpy(filehash, get_md5(fp)); // 해쉬값 구해서 저장
			fclose(fp);

			// 동적할당 후 시간 포맷 구하기
			char* mstr = (char*)malloc(sizeof(char) * BUF_SIZE);
			char* astr = (char*)malloc(sizeof(char) * BUF_SIZE);
			strcpy(mstr, get_time(st.st_mtime, mstr));
			strcpy(astr, get_time(st.st_atime, mstr));

			append_list(list, (long long)filesize, pathname, mstr, astr, filehash); // 리스트에 추가	

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

	// 큐 빌때까지 bfs탐색(bfs이므로 절대경로, 아스키 순서로 정렬되어있음)
	while (!isEmpty_queue(q)){
		// printf("q pop : %s\n", pop_queue(q));
		ssu_find_md5(splitOper, pop_queue(q), start, list, q, false);
	}
	if(!from_main) return; // 처음 메인에서 실행한게 아니라면 리턴 (재귀 종료)	

	struct timeval end; 
	gettimeofday(&end, NULL); // 종료 시간 측정

	del_onlyList(list); // 중복파일 없는 인덱스 삭제

	int list_size = get_listLen(list);
	if(list_size == 0){
		if(realpath(find_path, dir_path) == NULL){
			fprintf(stderr, "realpath error\n");
			return;
		}
		fprintf(stdout, "No duplicates in %s\n", dir_path);
		return;
	}

	// 파일크기대로 정렬 (bfs이므로 파일크기 같을 경우 절대경로 짧은 순 -> 임의(아스키 코드 순))
	sort_list(list, list_size);

	print_list(list); // 리스트 출력
	get_searchtime(start, end); // 탐색 시간 출력
	option(list); // 옵션 실행
}

void option(Node *list){
	while(1){
		fprintf(stdout, ">> ");

		char oper[BUF_SIZE];
		fgets(oper, BUF_SIZE, stdin); // 명령어 입력
		oper[strlen(oper)-1] = '\0'; // 공백 제거

		// 시작 공백 제거
		while(oper[0] == ' '){
			memmove(oper, oper + 1, strlen(oper));
		}

		char *splitOper[OPTION_LEN] = {NULL, }; // 명령어 split
		char *ptr = strtok(oper, " "); // 공백 기준으로 문자열 자르기

		int idx = 0;
		bool goNext = true;
		while (ptr != NULL){
			if(idx < OPTION_LEN) splitOper[idx] = ptr;
			else{
				fprintf(stderr, "option 입력 초과\n");
				goNext = false; // 다시 프롬프트 출력
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
		else if(goNext && !strcmp(splitOper[1], "d")){ // d옵션
			if(splitOper[2] == NULL)
				fprintf(stderr, "LIST_IDX 입력 x\n");
			else option_d(splitOper, list);
		}
		else if(goNext && !strcmp(splitOper[1], "i")){ // i옵션

		}
		else if(goNext && !strcmp(splitOper[1], "f")){ // f옵션

		}
		else if(goNext && !strcmp(splitOper[1], "t")){ // t옵션

		}
		else{
			fprintf(stderr, "올바른 옵션을 입력해주세요\n");
		}
	}	
}

// d옵션
void option_d(char *splitOper[OPTION_LEN], Node *list){
	Node *cur = list->next;

	char *set_idx = malloc(sizeof(char) * BUF_SIZE);
	strcpy(set_idx, splitOper[0]);

	// 세트 같을때까지 탐색
	while (cur->set_num != atoi(set_idx)){
		if(cur->next == NULL){
			fprintf(stderr, "세트 범위 벗어남\n");
			free(set_idx);
			return;
		}
		cur = cur->next;
	}

	char *list_idx = malloc(sizeof(char) * BUF_SIZE);
	strcpy(list_idx, splitOper[2]);

	// 인덱스 같을때까지 탐색
	while (cur->idx_num != atoi(list_idx)){
		// 만약 같은 세트가 아닐경우 인덱스 범위 벗어남
		if(cur->next == NULL || cur->set_num != atoi(set_idx)){
			fprintf(stderr, "인덱스 범위 벗어남\n");
			free(set_idx);
			free(list_idx);
			return;
		}		
		cur = cur->next;
	}

	free(set_idx);
	free(list_idx);
	
	int set_num = cur->set_num; // 삭제할 세트 번호
	int idx_num = cur->idx_num; // 삭제할 인덱스 번호

	//파일 삭제
	if(unlink(cur->path) == -1)
		fprintf(stderr, "%s delete error", cur->path);
	else{
		fprintf(stdout, "\"%s\" has been deleted in #%d\n\n", cur->path, set_num);
		del_node(list, set_num, idx_num); // 해당 노드 연결 리스트에서 삭제
		del_onlyList(list); // 하나만 남은 경우 제거
		print_list(list); // 프린트, 넘버링(인덱스 재배치)
		fprintf(stdout, "\n");
	}
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
	
	MD5_Final(&(md[0]),&c);

	return md;
}

// 시간 정보 포맷에 맞게 변환
char* get_time(time_t stime, char * str){
	struct tm *tm;
	tm = localtime(&stime);

	strftime(str, BUF_SIZE, "%Y-%m-%d %H:%M:%S", tm);
	return str;
}

// 프로그램 종료
void get_searchtime(struct timeval start, struct timeval end){
	end.tv_sec -= start.tv_sec; // 초 부분 계산

	if(end.tv_usec < start.tv_usec){ // ms 연산 결과가 마이너스인 경우 고려
		end.tv_sec--;
		end.tv_usec += 1000000;
	}

	end.tv_usec -= start.tv_usec;
	printf("\nSearching time: %ld:%06ld(sec:usec)\n\n", end.tv_sec, end.tv_usec);
}

// 리스트 끝에 추가
void append_list(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[MD5_DIGEST_LENGTH]){
	// 리스트 빌 경우(처음인경우 포함)
	if(list -> next == NULL){
		Node *newNode = malloc(sizeof(Node));
		newNode->next = list->next;

		newNode->filesize = filesize;
		strcpy(newNode->path, path);
		strcpy(newNode->mtime, mtime);
		strcpy(newNode->atime, atime);
		strcpy(newNode->hash, hash);

		list->next = newNode;
	}
	else{
		Node *cur = list;
		while (cur->next != NULL)
			cur = cur->next;
		
		Node *newNode = malloc(sizeof(Node));
		newNode->next = cur->next;

		newNode->filesize = filesize;
		strcpy(newNode->path, path);
		strcpy(newNode->mtime, mtime);
		strcpy(newNode->atime, atime);
		strcpy(newNode->hash, hash);

		cur->next = newNode;
	}
}

// 노드 크기 구하기
int get_listLen(Node *list){
    int cnt = -1; // head 제외
    Node *cur = list;
    
    while(cur != NULL){
        cnt++;
        cur = cur->next;
    }

    return cnt;
}

// 리스트 전체 데이터 출력 && 넘버링
void print_list(Node *list){
	Node *cur = list->next;
	int cnt = 0;
	int small_cnt = 1;
	unsigned char pre_hash[BUF_SIZE] = "";
	while (cur != NULL){
		// 해쉬값이 다르면 다른 리스트출력
		if(strcmp(pre_hash, cur->hash)){
			cnt++;
			small_cnt = 1;
			if(cnt != 1) fprintf(stdout, "\n"); // 2번째 파일부터는 한칸 씩 더 띄워줌
			//todo : 파일 크기 ,로 끊어서
			fprintf(stdout, "---- Identical files #%d (%lld bytes - ", cnt, cur->filesize);
			for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
				fprintf(stdout, "%02x", cur->hash[i]);
			fprintf(stdout, ") ----\n");

			strcpy(pre_hash, cur->hash);
		}

		cur->set_num = cnt; // 현재 세트 번호 저장
		cur->idx_num = small_cnt; // 세트 내 인덱스 번호 저장

		fprintf(stdout, "[%d] %s (mtime : %-15s) (atime : %-15s)\n", small_cnt, cur->path, cur->mtime, cur->atime);
		small_cnt++;

		cur = cur->next;
	}
}

// 해쉬 일치할경우 인덱스 반환
int search_hash(Node *list, int cmp_idx, unsigned char hash[MD5_DIGEST_LENGTH]){
    Node *cur = list->next; // head 다음
    int idx = 1;	

    while(cur != NULL){
		// 본인이 아닌 같은 해쉬 찾은 경우
        if((idx != cmp_idx) && !strcmp(cur->hash, hash))
			return idx;
        
        cur = cur->next;
        idx++;
    }
	// 못 찾은 경우
    return -1;
}

// 특정 파일 삭제
void del_node(Node *list, int set_num, int idx_num){
    Node *cur = list->next; // head 다음
	if(cur == NULL){
		fprintf(stderr, "node is NULL\n");
		return; // 빈 리스트일 경우 리턴	
	}

	Node *pre = list;

    while(cur != NULL){
		// 삭제할 인덱스 도달하면
        if(cur->set_num == set_num && cur->idx_num == idx_num){
			// 해당 인덱스 삭제
			if(cur->next != NULL){ // 중간 인덱스 삭제할 경우
				pre->next = cur->next; // 이전 노드의 다음 -> 삭제할 노드의 다음
				free(cur);
				cur = pre->next;
			}
			else{ // 마지막 인덱스 삭제할 경우
				pre->next = NULL;
				cur->next = NULL;
				free(cur);
				cur = NULL;
			}
		}
		else{
			pre = cur;
			cur = cur->next;
		}
    }
}

// 같은파일 있는지 찾고 없으면 삭제
void del_onlyList(Node *list){
    Node *cur = list->next; // head 다음
	if(cur == NULL) return; // 빈 리스트일 경우 리턴	

	Node *pre = list;
	int idx = 1; // 현재 cur 인덱스
	int cmp_idx; // 비교할 인덱스

    while(cur != NULL){
		cmp_idx = search_hash(list, idx, cur->hash); // 본인 제외 같은 해쉬 존재하는지 탐색
		// 같은 해시 값 없으면
        if(cmp_idx == -1){
			// 해당 인덱스 삭제
			if(cur->next != NULL){ // 중간 인덱스 삭제할 경우
				pre->next = cur->next; // 이전 노드의 다음 -> 삭제할 노드의 다음
				free(cur);
				cur = pre->next;
			}
			else{ // 마지막 인덱스 삭제할 경우
				pre->next = NULL;
				cur->next = NULL;
				free(cur);
				cur = NULL;
			}
		}
		else{
			pre = cur;
			idx++;
			cur = cur->next;
		}
    }
}

// 리스트 버블정렬
// 파일크기순 정렬 (bfs이므로 파일크기 같을 경우 절대경로 짧은 순 -> 임의(아스키 코드 순))
void sort_list(Node *list, int list_size){
    Node *cur = list->next; // head 다음
    for (int i = 0; i < list_size; i++){
        if(cur->next == NULL) break;
        for (int j = 0; j < list_size - 1 - i; j++){
            if(cur->filesize > cur->next->filesize)
                swap_node(cur, cur->next); //swap    
            cur = cur->next;
        }
        cur = list->next;
    }
}

void swap_node(Node *node1, Node *node2){
    int fileSize;
	char path[BUF_SIZE];
	char mtime[BUF_SIZE];
	char atime[BUF_SIZE];
	unsigned char hash[BUF_SIZE];

	fileSize = node1->filesize;
	strcpy(path, node1->path);
	strcpy(mtime, node1->mtime);
	strcpy(atime, node1->atime);
	strcpy(hash, node1->hash);

    node1->filesize = node2->filesize;
	strcpy(node1->path, node2->path);
	strcpy(node1->mtime, node2->mtime);
	strcpy(node1->atime, node2->atime);
	strcpy(node1->hash, node2->hash);

    node2->filesize = fileSize;
	strcpy(node2->path, path);
	strcpy(node2->mtime, mtime);
	strcpy(node2->atime, atime);
	strcpy(node2->hash, hash);
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