#include <stdio.h>
#include <sys/time.h> // gettimeofday 사용
#include <unistd.h>
#include <math.h> // floor 사용
#include <string.h> // string 관련 함수 사용

void ssu_sindex(int argc, char *argv[]){
	while (1){	
		char oper[10]; // 초기 명령어
		scanf("%s", oper);

		// exit 입력 시 종료
		if(strcmp(oper, "exit") == 0){
			break;
		}

		printf("%s\n", oper);
	}

	return;
}