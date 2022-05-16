#ifndef SSU_FIND_H
#define SSU_FIND_H

#ifndef BUF_MAX
    #define BUF_MAX 1024 * 16
#endif

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef PATH_SIZE
    #define PATH_SIZE 4096
#endif

#ifndef FILEDATA_SIZE
    #define FILEDATA_SIZE 10 // 마지막은 \n
#endif

#ifndef TRASHDATA_SIZE
    #define TRASHDATA_SIZE 12 // 마지막은 \n
#endif

#ifndef COMMA_SIZE
    #define COMMA_SIZE 64
#endif

#ifndef OPER_LEN
    #define OPER_LEN 5
#endif

#ifndef OPTDEL_LEN
    #define OPTDEL_LEN 5
#endif

#ifndef DELTIME_LEN
    #define DELTIME_LEN 14
#endif

#include <stdio.h>
#include <sys/types.h> // stat 사용
#include <sys/stat.h> // stat 사용
#include <sys/wait.h>
#include <unistd.h> // stat 사용
#include <fcntl.h>
#include <string.h> // string 관련 함수 사용
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <dirent.h> // scandir 사용
#include <math.h> // modf 사용
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <sys/time.h> // gettimeofday 사용
#include <errno.h>
#include <pwd.h>
#include <ctype.h>


// 세트 당 파일리스트
typedef struct Nodes{
	struct Nodes *next; // 다음 주소
	long long filesize; // 파일 크기(byte)
	char path[PATH_SIZE]; // 파일 경로
	char mtime[BUF_SIZE]; // mtime
	char atime[BUF_SIZE]; // atime
	unsigned char hash[BUF_SIZE]; // hash value
	int uid; // 사용자 ID
	int gid; // 그룹 ID
	unsigned long mode; // 모드
}Node;

// 중복 세트
typedef struct Set{
	struct Set *next; // 다음 주소
	Node *nodeList; // 세트 당 파일리스트
	long long filesize; // 파일 크기(byte)
	unsigned char hash[BUF_SIZE]; // hash value
}Set;

typedef struct QNode{
	char path[BUF_SIZE];
	struct QNode *next;
}Qnode;

// BFS 구현 용 큐
typedef struct Queue{
	Qnode *front;
	Qnode *rear;
	int cnt; // 큐 안의 노드 개수
}queue;

// 쓰레기통 관리 링크드리스트
typedef struct Trash{
	struct Trash *next; // 다음 주소
	char delete_date[DELTIME_LEN]; // 삭제 날짜
	char delete_time[DELTIME_LEN]; // 삭제 시간
	long long filesize; // 파일 크기(byte)
	char path[PATH_SIZE]; // 파일 경로
	char mtime[BUF_SIZE]; // mtime
	char atime[BUF_SIZE]; // atime
	unsigned char hash[BUF_SIZE]; // hash value
	int uid; // 사용자 ID
	int gid; // 그룹 ID
	unsigned long mode; // 모드
}Trash;

int digest_len;
void ssu_find(bool is_md5, char extension[BUF_SIZE], long double min_byte, long double max_byte, char find_path[BUF_SIZE], int thread_num, struct timeval start, Set *set, queue *q, FILE *dt, bool from_main);
void file2set(FILE * dt, Set *list);

void option_a(int list_idx, Node *list); // 추가기능

int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);
char *get_md5(FILE *fp);
char *get_sha1(FILE *fp);
char *get_time(time_t stime, char * str);
void get_searchtime(struct timeval start, struct timeval end);
int get_setLen(Set *set);
int get_listLen(Node *list);
const char *size2comma(long long n);

void append_set(Set *set, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[digest_len], int uid, int gid, int mode);
void delete_set(Set *set);
void print_set(Set *set);
int search_set(Set *set, unsigned char hash[digest_len]);
void sort_upSet(Set *set, int set_size);
void swap_set(Set *set1, Set *set2);

void delete(Set *set);
void delete_d(Set *set, Set *set_cur, Set *set_pre, int set_idx, int list_idx);
void delete_i(Set *set, Set *set_cur, Set *set_pre);
void delete_f(Set *set, Set *set_cur, Set *set_pre, int set_idx);
void delete_t(Set *set, Set *set_cur, Set *set_pre, int set_idx);
void del_set(Set *cur, Set *pre);

void append_list(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[digest_len], int uid, int gid, int mode);
void print_list(Node *list);
void del_node(Node *cur, Node *pre, int log_type);
int search_hash(Node *list, int cmp_idx, unsigned char hash[digest_len]);
void sort_list(Node *list, int list_size);
void swap_node(Node *node1, Node *node2);
Node *get_recent(Node *cur);

void init_queue(queue *q);
bool isEmpty_queue(queue *q);
void push_queue(queue *q, char path[BUF_SIZE]);
char *pop_queue(queue *q);

void ssu_help();

void list(Set *set, bool sort_set, bool c_opt[5], bool sort_up);

// ssu_list.c
void sort_downSet(Set *set, int set_size);
void sort_pathReverse(Node *list, int list_size);
void sort_idUpList(Node *list, int list_size, int sortWhat);
void sort_idDownList(Node *list, int list_size, int sortWhat);

// trash
void trash(Trash *tr, bool c_opt[5], bool sort_up);
void file2tr(Trash *tr);
void delete_trash(Trash *tr);
void append_trash(Trash *tr, char *splitFile[TRASHDATA_SIZE]);
void print_trash(Trash *tr);
int get_trashLen(Trash *tr);
void sort_sizeTrash(Trash *tr, int tr_size, bool sort_up);
void swap_trash(Trash *tr1, Trash *tr2);
#endif