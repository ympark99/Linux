#ifndef SSU_SFINDER_H
#define SSU_SFINDER_H

#ifndef BUF_SIZE
    #define BUF_SIZE 1024
#endif

#ifndef OPER_LEN
    #define OPER_LEN 11
#endif

#ifndef LIST_LEN
    #define LIST_LEN 7
#endif

#ifndef TRASH_LEN
    #define TRASH_LEN 5
#endif

int check_fileOrDir(char *path);

#endif