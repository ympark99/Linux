#ifndef MAIN_H
#define MAIN_H

#ifndef OPER_SIZE
    #define OPER_SIZE 1024
#endif

#include <stdbool.h> // bool, true, false 사용

void ssu_sindex();
void print_inst();

bool isFind(char oper[]);
void find(char oper[]);

#endif