#ifndef SSU_FIND_MD5_H
#define SSU_FIND_MD5_H

#ifndef BUF_MAX
    #define BUF_MAX 1024 * 16
#endif

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef OPER_LEN
    #define OPER_LEN 5
#endif

#include <openssl/md5.h>

// 동일 파일 링크드리스트
typedef struct Nodes{
	struct Nodes *next; // 다음 주소
	long long filesize; // 파일 크기(byte)
	char path[BUF_SIZE]; // 파일 경로
	char mtime[BUF_SIZE]; // mtime
	char atime[BUF_SIZE]; // atime
	unsigned char hash[BUF_SIZE]; // hash value
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

void ssu_find_md5(char *splitOper[OPER_LEN], char *find_path, struct timeval start, Node *list, queue *q, bool from_main);
int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);
char *get_md5(FILE *fp);
char* get_time(time_t stime, char * str);
void get_searchtime(struct timeval start, struct timeval end);

int get_listLen(Node *list);
void append_list(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[MD5_DIGEST_LENGTH]);
void print_list(Node *list);
void delete_list(Node *list);
int search_hash(Node *list, int cmp_idx, unsigned char hash[MD5_DIGEST_LENGTH]);
void del_onlyList(Node *list);
void sort_list(Node *list, int list_size);
void swap_node(Node *node1, Node *node2);

void init_queue(queue *q);
bool isEmpty_queue(queue *q);
void push_queue(queue *q, char path[BUF_SIZE]);
char *pop_queue(queue *q);

#endif