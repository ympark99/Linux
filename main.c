#include <stdio.h>
#include <sys/time.h> // gettimeofday 사용
#include <unistd.h>
#include <string.h> // string 관련 함수 사용
#include "ssu_sindex.h"

// 프로그램 종료
void getRuntime(struct timeval startTime, struct timeval endTime){
	gettimeofday(&endTime, NULL);
	endTime.tv_sec -= startTime.tv_sec; // 초 부분 계산

	if(endTime.tv_usec < startTime.tv_usec){ // ms 연산 결과가 마이너스인 경우 고려
		endTime.tv_sec--;
		endTime.tv_usec += 1000000;
	}

	endTime.tv_usec -= startTime.tv_usec;
	printf("Runtime: %ld:%06d(sec:usec)\n", endTime.tv_sec, endTime.tv_usec);
}

int main(){
	// 프로그램 시간 계산
	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);
	
    ssu_sindex();

	getRuntime(startTime, endTime);

    return 0;
}
