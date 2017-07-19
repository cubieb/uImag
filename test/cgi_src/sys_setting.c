#include "utils.h"
#include "nvram.h"

#include <string.h>

/****************** 設置模塊 ***********************/
/* 功能：
 * -----------------------------------
 *
 *　１重啓
 *　２恢復出廠設置
 *　３固件升級（獲取當前版本號&&獲取有沒有升級版本）
 */

#define VERSION 	"/etc_ro/conf/verson"

#define CANUPDATE 	1
#define NOUPDATE 	0

typedef enum _JOBS
{
	GET_UPDATE_STATUS = 1,
	UPDATE,
	RECOVER,
	REBOOT,
	IFCONFIG
}JOBS;

static int getupdatestatus(int nvram)
{
	do_system("update 183.230.102.49 1003 wifi-repeator 0");
	return atoi(nvram_bufget(nvram, "CMCC_HaveNewVersion"));
}

/***********獲取升級相關信息***************/
/*
 * １獲取當前版本號
 * ２获取有没有可升級版本
 */
void GetUpdateStatus()
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

	int status = getupdatestatus(RT2860_NVRAM);

	printf("{\n");
	printf("\"version\":\"%s\",", version);
	printf("\"update\":\"%d\"", status);
	printf("}\n");
}

//重啓
void Reboot(void)
{
	printf("reboot"); //立即返回給客戶端：這是前端工程師的要求
	fflush(NULL);
	do_system("reboot");
}

//恢復出廠設置
void Recover(void)
{
	printf("recover"); //立即返回給客戶端：這是前端工程師的要求
	fflush(NULL);
	do_system("ralink_init clear 2860");
	do_system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
}

//升級
void Update(void)
{
	do_system("update 183.230.102.49 1003 wifi-repeator 1");
	//升級完成後，CMCC_UpdateSuccess = 1
	if(atoi(nvram_bufget(RT2860_NVRAM, "CMCC_UpdateSuccess")))
	{
		printf("updateSuccess"); //立即返回給客戶端：這是前端工程師的要求
		fflush(NULL);
	}
}

//web端是否进入设置界面还是管理界面
void Ifconfig(void)
{
	int ap_enable;
	ap_enable = atoi(nvram_bufget(RT2860_NVRAM, "CMCC_ApCliEnable"));
	printf("%d", ap_enable);
	fflush(NULL);
}

int main(int argc, char *argv[])
{
	char input[MAX_MSG_SIZ];
	int length;
	JOBS jobs;

	length = get_message_for_web(input);
	DBG_MSG("get the message from web client is %s", input);
	if(length <= 0)
		DBG_MSG("get the message is empty or failed!");

	web_debug_header();

	if (!strncmp("version", web_get("get_version", input, 0), 7))
		jobs = GET_UPDATE_STATUS;
	else if (!strcmp("reboot", web_get("reboot_sys", input, 0), 6))
		jobs = REBOOT;
	else if (!strcmp("recover", web_get("recover_sys", input, 0), 7))
		jobs = RECOVER;
	else if (!strcmp("update", web_get("update_sys", input, 0), 6))
		jobs = UPDATE;
	else (!strcmp("config", web_get("ifconfig", input, 0), 6))
		jobs = IFCONFIG;

	switch(jobs)
	{
		case GET_UPDATE_STATUS:
			GetUpdateStatus();
			break;
		case REBOOT:
			Reboot();
			break;
		case RECOVER:
			Recover();
			break;
		case UPDATE:
			Update();
			break;
		case IFCONFIG:
			Ifconfig();
			break;
		default:
			break;
	}

	return 0;
}
