#include "utils.h"
#include "nvram.h"

#include <string.h>

/*
 * 主要是实现系统的进本设置
 * 重启，恢复出厂设置，固件升级三个功能
 */

#define VERSION 	"/etc_ro/conf/verson"

#define CANUPDATE 	1
#define NOUPDATE 	0


void recover_factory_setting(void);
int update_firmeware(int nvram);
int get_update_status(int nvram);

int main(int argc, char *argv[])
{
	char input[MAX_MSG_SIZ];
	int length;

	length = get_message_for_web(input);
	DBG_MSG("get the message from web client is %s", input);
	if(length <= 0)
		DBG_MSG("get the message is empty or failed!");

	web_debug_header();

	char *temp;
	temp = web_get("get_version", input, 0);
	if(strcmp("version", temp) == 0)
	{
		FILE *fp;
		char version[16];
		fp = fopen(VERSION, "r");
		if(fp == NULL)
		{
			DBG_MSG("Failed to fopen!");
			return -1;
		}

		size_t length = fread(version, 1, sizeof(version), fp);
		if(length < 0)
		{
			DBG_MSG("read the version file failed!");
			return -1;
		}
		version[length + 1] = '\0';
		fclose(fp);

		int update_status = get_update_status(RT2860_NVRAM);

		printf("{\n");
		printf("\"version\":\"%s\",", version);
		printf("\"update\":\"%d\"", update_status);
		printf("}\n");
	}

	temp = web_get("reboot_sys", input, 0);
	if(strcmp("reboot", temp) == 0)
	{
		printf("reboot\n");
		do_system("reboot");
	}

	temp = web_get("recover_sys", input, 0);
	if(strcmp("recover", temp) == 0)
	{
		printf("recover\n");
		recover_factory_setting();
	}

	temp = web_get("update_sys", input, 0);
	if(strcmp("update", temp) == 0)
	{
		DBG_MSG("updating now.");
		if(update_firmeware(RT2860_NVRAM) == 1)
		{
			printf("updateSuccess");
			DBG_MSG("updataSuccess");
			fflush(NULL);
		}
	}

	//web端是否进入设置界面还是管理界面
	temp = web_get("isconfig", input, 0);
	if(strcmp("config", temp) == 0)
	{
		int ap_enable;
		ap_enable = atoi(nvram_bufget(RT2860_NVRAM, "CMCC_ApCliEnable"));
		printf("%d", ap_enable);
	}

}

//恢复出厂设置
void recover_factory_setting(void)
{
	do_system("ralink_init clear 2860");
	//sleep(1);
	do_system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
	//sleep(1);
	//do_system("reboot");
}

//升级
int  update_firmeware(int nvram)
{
	do_system("update 183.230.102.49 1003 wifi-repeator 1");
	return atoi(nvram_bufget(nvram, "CMCC_UpdateSuccess"));
}

//获取有没有升级的情况
int get_update_status(int nvram)
{
	do_system("update 183.230.102.49 1003 wifi-repeator 0");
	return atoi(nvram_bufget(nvram, "CMCC_HaveNewVersion"));
}
