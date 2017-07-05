#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

int main()
{
	time_t itime;
	time(&itime);
	printf("%d\n", time);
	struct timeval tv;
	gettimeofday(&tv, NULL);
	printf("curren time is %d\n", (int)tv.tv_sec);
	return 0;
}
