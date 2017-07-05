#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "utils.h"
#include "nvram.h"


void GetUptime(void);
int getuptime(int nvram);

int main(int argc, char *argv[])
{
  //GetUptime();
  getuptime(RT2860_NVRAM);
  return 0;
}

void GetUptime(void)
{
	struct tm *utime;
	time_t usecs;

	time(&usecs);
	utime = localtime(&usecs);
	if(utime == NULL) utime = "";
	if (utime->tm_hour > 0)
		printf("%d hour%s, %d min%s, %d sec%s",
           utime->tm_hour, (utime->tm_hour == 1)? "" : "s",
           utime->tm_min, (utime->tm_min == 1)? "" : "s",
           utime->tm_sec, (utime->tm_sec == 1)? "" : "s");
	else if (utime->tm_min > 0)
		printf("%d min%s, %d sec%s",
           utime->tm_min, (utime->tm_min == 1)? "" : "s",
           utime->tm_sec, (utime->tm_sec == 1)? "" : "s");
	else
		printf("%d sec%s",
           utime->tm_sec, (utime->tm_sec == 1)? "" : "s");
}

int getuptime(int nvram)
{
  FILE *fp;
  fp = fopen("/proc/uptime", "r");
  if(fp == NULL)
    return -1;

  char sec[128];
  if(fgets(sec, sizeof(sec), fp) != NULL)
    {
      char begin_time[32];
      get_nth_value(0, sec, ' ', begin_time, sizeof(begin_time));
      printf("%s\n", sec);
      nvram_bufset(nvram, "connTime", sec);
      nvram_commit(nvram);
      fclose(fp);
      return 0;
    }
}
