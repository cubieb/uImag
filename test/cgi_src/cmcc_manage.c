#include "itool.h"

/****************管理界面顯示信息**********************/
/*
 *  主路由信息：
 * ------------------------------------------------
 *  連接源(信號源)
 *  連接時長
 *  信號強度
 */

//連接主路由的時長
unsigned int get_conntime(int nvram)
{
	unsigned int start_time, end_time;
	start_time = atoi(nvram_bufget(nvram, "CMCC_ConnectTime"));
	end_time = get_uptime();

	return start_time - end_time;
}

int main(int argc, char *argv[])
{
	char input[MAX_MSG_SIZ];
	int length;

	length = get_message_for_web(input);
	DBG_MSG("get the message form web is %s", input);
	if(length <= 0)
	{
		DBG_MSG("get the message form web is empty!");
		return -1;
	}

	//for cgi, define the message type.
	web_debug_header();
	if(strcmp("manage", web_get("get_manage", input, 2)) == 0)
	{
		//get the ap mac
		char mac[18];
		strcpy(mac, nvram_bufget(RT2860_NVRAM, "CMCC_SelectApMac"));
		//get the ap ssid
		char apssid[33];
		strcpy(apssid, nvram_bufget(RT2860_NVRAM, "CMCC_ApCliSsid"));
		//get connecttime
		int apconntime = get_conntime(RT2860_NVRAM);
		//get signal
		int apsignal = get_signal("apcli0");

		printf("{\n");
		printf("\"ssid\":\"%s\",", apssid);
		printf("\"connecttime\":\"%d\",", apconntime);
		printf("\"signal\":\"%d\"\n", apsignal);
		printf("}\n");
	}
	return 0;
}

