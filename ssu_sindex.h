#ifndef MAIN_H
#define MAIN_H

#ifndef OPER_SIZE
    #define OPER_SIZE 1024
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
	char *access;
	char *change;
	char *modify;
	char *path;
};

void ssu_sindex();
void print_inst();

void find();
void find_files(char *findOper[FINDOPER_SIZE], char *fileName, long long fileSize, int idx);
long long get_fileSize(char *path);
void get_fileInfo(char *path, int idx);
char *dateFormat(char *str, struct timespec st);

#endif