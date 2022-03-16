#ifndef OPTION_H_
#define OPTION_H_

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef FINDOPER_SIZE
    #define FINDOPER_SIZE 3
#endif

#ifndef IDXOPTION_SIZE
    #define IDXOPTION_SIZE 5
#endif

#ifndef OPTION_SIZE
    #define OPTION_SIZE 4
#endif

#ifndef DATEFORMAT_SIZE
    #define DATEFORMAT_SIZE 36
#endif

#ifndef MODE_SIZE
    #define MODE_SIZE 11
#endif

#include <stdbool.h>
#include <dirent.h>

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

void option(int fileOrDir, struct fileLists *fileList, int listSize, char *inputOper[FINDOPER_SIZE]);
void cmp_file(char *oriPath, char *cmpPath, bool sameAlpha);
void cmp_fileOption(char *oriPath, char *cmpPath, bool options[OPTION_SIZE], bool isDiff, char *inputOper[FINDOPER_SIZE]);
int cmp_str(char *str1, char *str2, bool sameAlpha);
int scanFilter(const struct dirent *info);
void cmp_dir(char *inputOper[FINDOPER_SIZE], char *oriPath, char *cmpPath, bool options[OPTION_SIZE]);
bool isCmpFirst(char *oriName, char *cmpName);
int get_fileOrDir(char *path);
char *getfileStr(int fileOrDir);

#endif