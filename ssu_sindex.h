#ifndef MAIN_H
#define MAIN_H

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef FINDOPER_SIZE
    #define FINDOPER_SIZE 3
#endif

#ifndef DATEFORMAT_SIZE
    #define DATEFORMAT_SIZE 36
#endif

#define LMAX 100

struct fileLists{
	int idx;
	long long size;
	int mode;
	long long blocks;
    int links;
	int uid;
	int gid;
	char access[DATEFORMAT_SIZE];
	char change[DATEFORMAT_SIZE];
	char modify[DATEFORMAT_SIZE];
	char *path;
};

void ssu_sindex();
void print_inst();

void find_first();
void dfs_findMatchFiles(char *findOper, char *fileName, long long fileSize);
long long get_fileSize(char *path);
void save_fileInfo(char *path);
void print_fileInfo();
char *dateFormat(char *str, struct timespec st);

#endif