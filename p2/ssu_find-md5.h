#ifndef SSU_FIND_MD5_H
#define SSU_FIND_MD5_H

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef OPER_LEN
    #define OPER_LEN 5
#endif

void ssu_find_md5(char *splitOper[OPER_LEN]);
long double get_fileSize(char *path);
int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);

#endif