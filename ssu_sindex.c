#include <stdio.h>
#include <sys/time.h> // gettimeofday 사용
#include <math.h> // floor 사용

int main(void){
	// 프로그램 시간 계산
	struct timeval startTime, endTime;
	gettimeofday(&startTime, NULL);
	
	char oper[10]; // 초기 명령어
	scanf("%s", oper);

	printf("%s\n", oper);

		gettimeofday(&endTime, NULL);
		int runtimeSec = endTime.tv_sec - startTime.tv_sec;
		double runtime = runtimeSec + (endTime.tv_usec - startTime.tv_usec) / (double)1000000; // 실행 시간 계산

		// todo : 소수점이 0으로 시작하는 경우를 생각해 문자열로 변환 후 출력

		printf("%f\n", runtime);
		printf("Runtime : %.0f:%.0f(sec:usec)\n", floor(runtime), (runtime - floor(runtime)) * 1000000);
	return 0;
}
