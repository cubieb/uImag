#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "utils.h"
#include "nvram.h"


int getuptime(int nvram);

int main(int argc, char *argv[])
{
  //GetUptime();
  getuptime(RT2860_NVRAM);
  return 0;
}

int getuptime(int nvram)
{
  FILE *fp;
  fp = fopen("/proc/uptime", "r");
  if(fp == NULL)
    return -1;

  char sec[32];
  if(fgets(sec, sizeof(sec), fp) != NULL)
  {
	  printf("%s\n", sec);

	  char begin_time[16];
	  get_nth_value(0, sec, ' ', begin_time, sizeof(begin_time));
	  printf("%s\n", begin_time);
	  nvram_bufset(nvram, "connTime", begin_time);
	  nvram_commit(nvram);
	  fclose(fp);
	  return 0;
  }
}
