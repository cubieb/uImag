#include "mtk_operate.h" 
#include "utils.h"
//#include "nvram.h"

#include <string.h>

/*
 * 主要是实现系统的进本设置
 * 重启，恢复出厂设置，固件升级三个功能
 */
void recover_factory_setting(void);
void update_firmeware(void);

int main(int argc, char *argv[])
{
	char input[MAX_MSG_SIZ];
	int length;

	length = get_message_from_web(input);
	DBG_MSG("get the message from web: %s", input);
	if(length <= 0)
		DBG_MSG("get the message is empty or failed!");

	web_debug_header();

	char *temp;
	temp = web_get("reboot_sys", input, 0);
	DBG_MSG("reboot_sys: %s", temp);


	if(strcmp("reboot", temp) == 0)
	{
		printf("reboot");
		do_system("reboot");
	}

	temp = web_get("recover_sys", input, 0);
	DBG_MSG("recover_sys: %s", temp);
	if(strcmp("recover", temp) == 0)
	{
		printf("recover");
		recover_factory_setting();
	}

	temp = web_get("update_sys", input, 0);
	DBG_MSG("update_sys: %s", temp);
	if(strcmp("update", temp) == 0)
	{
		printf("update");
		//还没有做,在考虑如何升级固件
	}

}

void recover_factory_setting(void)
{
	do_system("ralink_init clear 2860");
	//sleep(1);
	do_system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
	sleep(1);
	do_system("reboot");
}

void update_firmeware(void)
{
}
