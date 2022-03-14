#ifndef OPTION_H_
#define OPTION_H_

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef IDXOPTION_SIZE
    #define IDXOPTION_SIZE 5
#endif

#ifndef DATEFORMAT_SIZE
    #define DATEFORMAT_SIZE 36
#endif

#ifndef MODE_SIZE
    #define MODE_SIZE 11
#endif

#include <stdbool.h>

struct fileLists{
	int idx;
	long long size;
	char mode[MODE_SIZE];
	long long blocks;
    int links;
	int uid;
	int gid;
	char access[DATEFORMAT_SIZE];
	char change[DATEFORMAT_SIZE];
	char modify[DATEFORMAT_SIZE];
	char path[BUF_SIZE];
};

void option(int fileOrDir, struct fileLists *fileList, int listSize);
void cmp_file(char *oriPath, char *cmpPath);
void cmp_fileOption(char *oriPath, char *cmpPath, char *options, bool isDiff);
void cmp_dir(int cmpIdx, struct fileLists *filelist, char *options);
int get_fileOrDir(char *path);
char *getfileStr(int fileOrDir);

#endif