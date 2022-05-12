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

#ifndef COMMA_SIZE
    #define COMMA_SIZE 64
#endif

#ifndef OPER_LEN
    #define OPER_LEN 5
#endif

#ifndef OPTDEL_LEN
    #define OPTDEL_LEN 5
#endif

#include <openssl/md5.h>
#include <openssl/sha.h>

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
	int set_num; // 현재 세트 번호
	int idx_num; // 세트 내 인덱스 번호
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

int digest_len;
void ssu_find(bool is_md5, char extension[BUF_SIZE], long double min_byte, long double max_byte, char find_path[BUF_SIZE], int thread_num, struct timeval start, Set *set, queue *q, FILE *dt, bool from_main);
void file2set(FILE * dt, Set *list);

void option_a(int list_idx, Node *list); // 추가기능

int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);
char *get_md5(FILE *fp);
char *get_sha1(FILE *fp);
char* get_time(time_t stime, char * str);
void get_searchtime(struct timeval start, struct timeval end);
int get_setLen(Set *set);
int get_listLen(Node *list);
const char *size2comma(long long n);

void append_set(Set *set, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[digest_len], int uid, int gid, int mode);
void delete_set(Set *set);
void print_set(Set *set);
int search_set(Set *set, unsigned char hash[digest_len]);
void sort_set(Set *set, int set_size);
void swap_set(Set *set1, Set *set2);

void delete(Set *set);
void delete_d(Set *set, Set *set_cur, Set *set_pre, int set_idx, int list_idx);
void delete_i(Set *set, Set *set_cur, Set *set_pre);
void delete_f(Set *set, Set *set_cur, Set *set_pre, int set_idx);
void delete_t(Set *set, Set *set_cur, Set *set_pre, int set_idx);
void del_set(Set *cur, Set *pre);

void append_list(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[digest_len], int uid, int gid, int mode);
void print_list(Node *list);
void delete_list(Node *list);
void del_node(Node *cur, Node *pre, int log_type);
int search_hash(Node *list, int cmp_idx, unsigned char hash[digest_len]);
void sort_list(Node *list, int list_size);
void swap_node(Node *node1, Node *node2);
Node *get_recent(Node *cur);

void init_queue(queue *q);
bool isEmpty_queue(queue *q);
void push_queue(queue *q, char path[BUF_SIZE]);
char *pop_queue(queue *q);

#endif