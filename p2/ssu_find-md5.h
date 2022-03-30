#ifndef SSU_FIND_MD5_H
#define SSU_FIND_MD5_H

#ifndef BUF_MAX
    #define BUF_MAX 1024 * 16
#endif

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef DATEFORMAT_SIZE
    #define DATEFORMAT_SIZE 36
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
} Node;

void ssu_find_md5(char *splitOper[OPER_LEN], Node *list);
int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);
char *get_md5(FILE *fp);
char* get_time(time_t stime);

int get_listLen(Node *list);
void append(Node *list, long long filesize, char *path, char *mtime, char *atime, unsigned char hash[MD5_DIGEST_LENGTH]);
void print_list(Node *list);
void delete_list(Node *list);
int search_hash(Node *list, int cmp_idx, unsigned char hash[MD5_DIGEST_LENGTH]);
void filter_node(Node *list);

#endif