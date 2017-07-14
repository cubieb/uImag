#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>

#include "utils.h"
#include "nvram.h"

int get_signal(char *mac);
int get_conntime(int nvram);

int main(int argc, char *argv[])
{
#if 1
  char input[MESSAGESIZE];
	int length;

	length = get_message_for_web(input);
	DBG_MSG("get the message form web is %s", input);
	if(length <= 0)
    {
      DBG_MSG("get the message form web is empty!");
      return -1;
    }
#endif

  //get the ap mac
  char mac[18];
  strcpy(mac, nvram_bufget(RT2860_NVRAM, "CMCC_SelectApMac"));
  //get the ap ssid
  char apssid[65];
  strcpy(apssid, nvram_bufget(RT2860_NVRAM, "CMCC_ApCliSsid"));
  //get connecttime
  int apconntime = get_conntime(RT2860_NVRAM);
  //get signal
  int apsignal = get_signal(mac);

#if 0
  printf("{\n");
  printf("\"ssid\":\"%s\",", apssid);
  printf("\"connecttime\":\"%d\",", apconntime);
  printf("\"signal\":\"%d\"\n", apsignal);
  printf("}\n");
#endif

#if 1
	//for cgi, define the message type.
	web_debug_header();
  if(strcmp("manage", web_get("get_manage", input, 2)) == 0) {
    printf("{\n");
    printf("\"ssid\":\"%s\",", apssid);
    printf("\"connecttime\":\"%d\",", apconntime);
    printf("\"signal\":\"%d\"\n", apsignal);
    printf("}\n");
  }
#endif 

  return 0;
}

int get_signal(char *mac)
{
  if(mac == NULL)
    return -1;

  FILE *pp;
	char cmd[CMDLEN], *ptr, wif[IFNAMSIZ];
	char channel[4], ssid[186], bssid[20], security[23];
	char signal[9], mode[9], ext_ch[7], net_type[3];
	char wps[4];

	int i, space_start, mysignal;
	int total_ssid_num, get_ssid_times, round;

	strncpy(wif, "ra0", sizeof(wif));

	do_system("iwpriv %s set SiteSurvey=1", wif);

	sleep(1); // for get the SCAN result. (2G + 5G may enough for 5 seconds)

	sprintf(cmd, "iwpriv %s get_site_survey", wif);
	if(!(pp = popen(cmd, "r"))) {
		DBG_MSG("execute get_site_survey fail!");
		return;
	}

	memset(cmd, 0, sizeof(cmd));

	fgets(cmd, sizeof(cmd), pp);
	fgets(cmd, sizeof(cmd), pp);

	while (fgets(cmd, sizeof(cmd), pp)) {
		if (strlen(cmd) < 4)
			break;
		ptr = cmd;
		sscanf(ptr, "%s ", channel);
		ptr += 37;
		sscanf(ptr, "%s %s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type, wps);

    if(strcmp(mac, bssid) == 0) {
      mysignal = atoi(signal);
      break;
    }

		ptr = cmd+4;
		i = 0;
		while (i < 33) {
			if ((ptr[i] == 0x20) && (i == 0 || ptr[i-1] != 0x20))
				space_start = i;
			i++;
		}
		ptr[space_start] = '\0';
		strcpy(ssid, cmd+4);
		convert_string_display(ssid);
	}
	pclose(pp);
  return mysignal;
}

int get_conntime(int nvram)
{
  FILE *fp;
  char starttime[16];
  char endtime[16];
  int myconntime;
	fp = fopen("/proc/uptime", "r");
	if(fp == NULL)
		return -1;

	char sec[32];
	if(fgets(sec, sizeof(sec), fp) != NULL)
    {
      get_nth_value(0, sec, ' ', endtime, sizeof(endtime));
    }
  strcpy(starttime, nvram_bufget(nvram, "CMCC_ConnectTime"));
  myconntime = atoi(endtime) - atoi(starttime);
	fclose(fp);

	return myconntime;
}
