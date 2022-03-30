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

#include "ssu_find-md5.h"

int node_size = 0; // 노드의 데이터 개수

// md5 관련 함수 실행
// 입력인자 : 명령어 split, 현재 링크드리스트
// 인자배열 : fmd5, 파일 확장자, 최소크기, 최대크기, 디렉토리
void ssu_find_md5(char *splitOper[OPER_LEN], Node *list){
	struct dirent **filelist; // scandir 파일목록 저장 구조체
	int cnt; // return 값

    // scandir로 파일목록 가져오기 (디렉토리가 아닐 경우 에러)
	if((cnt = scandir(splitOper[4], &filelist, scandirFilter, alphasort)) == -1){
        fprintf(stderr, "target is not directory\n");
		return;
	}

	// 절대경로 변환
	char dir_path[BUF_SIZE];
	if(realpath(splitOper[4], dir_path) == NULL){
		fprintf(stderr, "realpath error\n");
		return;
	}

	char pathname[BUF_SIZE]; // 합성할 path이름

    // 조건에 맞으면, 파일 : 리스트 조회 | 디렉토리 : 큐에 삽입
	for(int i = 0; i < cnt; i++){
        // 파일일경우
        if(check_fileOrDir(filelist[i]->d_name) == 1){
            // *.(확장자)인 경우
			if(strlen(splitOper[1]) > 1 ){
				char* ori_fname = strrchr(splitOper[1], '.'); // 지정 파일 확장자
				char* cmp_fname = strrchr(filelist[i]->d_name, '.'); // 가져온 파일 확장자

				// 확장자가 없거나 확장자가 다르면 패스
				if((cmp_fname == NULL) || strcmp(ori_fname, cmp_fname) != 0)
					continue;
			}

			sprintf(pathname, "%s/%s", splitOper[4], filelist[i]->d_name); // pathname 만들어줌

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

			// 절대경로 변환
			char real_path[BUF_SIZE];
			if(realpath(pathname, real_path) == NULL){
				fprintf(stderr, "realpath error\n");
				continue;
			}

			// md5값 구하기
			FILE *fp = fopen(filelist[i]->d_name, "r");
			if (fp == NULL) // fopen 에러시 패스
				continue;

			unsigned char filehash[MD5_DIGEST_LENGTH]; // 해쉬값 저장할 문자열
			strcpy(filehash, get_md5(fp)); // 해쉬값 구해서 저장(todo : 이 다음 pathname이 초기화됨, real_path로 복사해둠)
			fclose(fp);

			// 리스트에 추가
			append(list, (long long)filesize, real_path, get_time(st.st_mtime), get_time(st.st_atime), filehash);
        }
        // 디렉토리일 경우
        else if(check_fileOrDir(filelist[i]->d_name) == 2){
            // todo : 루트에서부터 탐색시, proc, run, sys 제외

			// todo : 찾은 디렉토리들 큐 추가
        }
    }

	for(int i = 0; i < cnt; i++){
		free(filelist[i]);
	}
	free(filelist);    

	// todo : 큐 빌때까지 bfs탐색

	// bfs이므로 절대경로, 아스키 순서로 정렬되어있음
	if(get_listLen(list) == 0){
		printf("No duplicates in %s\n", dir_path);
		return;
	}

	filter_node(list); // 리스트 필터링
	// todo : 중복파일 없는경우 출력

	print_list(list); // 리스트 출력
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
char* get_time(time_t stime){
	char* time = (char*)malloc(sizeof(char) * BUF_SIZE);
	struct tm *tm;

	tm = localtime(&stime);
	sprintf(time, "%02d-%02d-%02d %02d:%02d", tm->tm_year-100, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min);

	return time;
}

// 리스트 끝에 추가
void append(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[MD5_DIGEST_LENGTH]){
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

// 리스트 전체 데이터 출력
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
			if(cnt != 1) printf("\n"); // 2번째 파일부터는 한칸 씩 더 띄워줌
			//todo : 파일 크기 ,로 끊어서
			printf("---- Identical files #%d (%lld bytes - ", cnt, cur->filesize);
			for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
				printf("%02x", cur->hash[i]);
			printf(") ----\n");

			strcpy(pre_hash, cur->hash);
		}

		printf("[%d] %s (mtime : %-15s) (atime : %-15s)\n", small_cnt, cur->path, cur->mtime, cur->atime);
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

// 같은파일 있는지 찾고 없으면 삭제, 있으면 가장 앞에있는거의 뒤에 붙임
void filter_node(Node *list){
    Node *cur = list->next; // head 다음
	Node *pre = NULL;
	int idx = 1; // 현재 cur 인덱스

    while(cur != NULL){
		printf("%s\n", cur->path);
		// 같은 해시 값 없으면
        if(search_hash(list, idx, cur->hash) == -1){
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
			// todo : 가장 상위에 있는 인덱스 찾은 후 이동
			pre = cur;
			idx++;
			cur = cur->next;
		}	
    }
}