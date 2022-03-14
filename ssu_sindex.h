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

#ifndef MODE_SIZE
    #define MODE_SIZE 11
#endif

#define LMAX 1024

#include <dirent.h>

void ssu_sindex();
void print_inst();

void find_first();
void dfs_findMatchFiles(char *cmpPath, char *fileName, long long fileSize, int fileOrDir);
int scandirFilter(const struct dirent *info);
long long get_fileSize(char *path);
void get_dirSize(char *path);
int check_fileOrDir(char*path);
void save_fileInfo(char *path, long long oriSize);
void print_fileInfo();
char *dateFormat(char *str, struct timespec st);

#endif