#include <stdio.h>
#include <sys/time.h> // gettimeofday 사용
#include <unistd.h>
#include <string.h> // string 관련 함수 사용
#include "ssu_sindex.h"

// 프로그램 종료
void getRuntime(struct timeval startTime, struct timeval endTime){
	gettimeofday(&endTime, NULL);
	int runtimeSec = endTime.tv_sec - startTime.tv_sec; // 초 부분 계산
	// 실행 시간 계산(ms 연산 결과가 마이너스인 경우 고려)	
	double runtime = runtimeSec + (endTime.tv_usec - startTime.tv_usec) / (double)1000000; 
	

	// 소수점이 0으로 시작하는 경우를 고려해 문자열로 변환 후 출력
	char runtimePrint[9];
	sprintf(runtimePrint, "%f", runtime);
	runtimePrint[1] = ':'; // 형식에 맞게 변경
	printf("Prompt End\n");
	printf("Runtime : %s(sec:usec)\n", runtimePrint);	
}

int main(int argc, char *argv[]){
	// 프로그램 시간 계산
	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);
	
	// char oper[10]; // 초기 명령어
	// scanf("%s", oper);
	// printf("%s\n", oper);
    ssu_sindex(argc, argv);

	getRuntime(startTime, endTime);
	
    // exit(0);
    return 0;
}
