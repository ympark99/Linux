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

void ssu_sindex();
void print_inst();

void find();
void find_Info(char *findOper[FINDOPER_SIZE]);
char *dateFormat(char *str, struct timespec st);

#endif