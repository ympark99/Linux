#ifndef SSU_FIND_MD5_H
#define SSU_FIND_MD5_H

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
    #define FILEDATA_SIZE 7 // 마지막은 \n
#endif

#ifndef COMMA_SIZE
    #define COMMA_SIZE 64
#endif

#ifndef OPER_LEN
    #define OPER_LEN 5
#endif

#ifndef OPTION_LEN
    #define OPTION_LEN 3
#endif

#include <openssl/md5.h>

// 동일 파일 링크드리스트
typedef struct Nodes{
	struct Nodes *next; // 다음 주소
	long long filesize; // 파일 크기(byte)
	char path[PATH_SIZE]; // 파일 경로
	char mtime[BUF_SIZE]; // mtime
	char atime[BUF_SIZE]; // atime
	unsigned char hash[BUF_SIZE]; // hash value
	int set_num; // 현재 세트 번호
	int idx_num; // 세트 내 인덱스 번호
}Node;

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

void ssu_find_md5(char *splitOper[OPER_LEN], char *find_path, struct timeval start, Node *list, queue *q, FILE *dt, bool from_main);
void file2list(FILE * dt, Node *list);
void option(Node *list);
void option_d(char *splitOper[OPTION_LEN], Node *list);
void option_i(int set_idx, Node *list);
void option_f(int set_idx, Node *list);
void option_t(int set_idx, Node *list);
void option_a(int list_idx, Node *list); // 추가기능

int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);
char *get_md5(FILE *fp);
char* get_time(time_t stime, char * str);
void get_searchtime(struct timeval start, struct timeval end);
int get_listLen(Node *list);
const char *size2comma(long long n);

void append_list(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[MD5_DIGEST_LENGTH]);
void print_list(Node *list);
void delete_list(Node *list);
void del_node(Node *list, int set_num, int idx_num);
void del_onlySet(Node *list, int set_num);
void del_onlyList(Node *list);
int search_hash(Node *list, int cmp_idx, unsigned char hash[MD5_DIGEST_LENGTH]);
void sort_list(Node *list, int list_size);
void swap_node(Node *node1, Node *node2);
Node *get_recent(int set_idx, Node *cur);

void init_queue(queue *q);
bool isEmpty_queue(queue *q);
void push_queue(queue *q, char path[BUF_SIZE]);
char *pop_queue(queue *q);

#endif