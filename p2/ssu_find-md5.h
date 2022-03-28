#ifndef SSU_FIND_MD5_H
#define SSU_FIND_MD5_H

#ifndef OPER_LEN
    #define OPER_LEN 5
#endif

void ssu_find_md5(char *splitOper[OPER_LEN]);
int scandirFilter(const struct dirent *info);
int check_fileOrDir(char *path);

#endif