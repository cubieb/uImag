#include "mtk_operate.h"
#include "nvram.h"


/*
 * WIFI的基本设置
 * 1.扫描周围的SSID，2.检查主路由的密码是否正确
 * 3.把主路由和扩展路由的相关信息写入WIFI相关参数的内存区
 *
 */
int main(int argc, char *argv[])
{
	char input[MAX_MSG_SIZ];
	int length;


	length = get_message_from_web(input);
	if(length <= 0)
		DBG_MSG("get the message form web is empty!");

#if 0
	ap_message_t *ap_msg;
	ap_msg = malloc(sizeof(struct ap_message_s));
	if(NULL == ap_msg)
		DBG_MSG("malloc the AP struct failed!");

	extend_message_t *ex_msg;
	ex_msg =  malloc(sizeof(struct extend_message_s));
	if(NULL == ex_msg)
		DBG_MSG("malloc the extend struct failed!");

#endif 
	DBG_MSG("%s\n", input);


	//for cgi, define the message type.

	web_debug_header();

	//发送路由周围的ssid给web客户端
	if(strcmp("Scan", web_get("wifiScan", input, 0)) == 0)
	{
		apcli_scan();
	}
#if 1
	if(strcmp("ckpwd", web_get("mainpasswd", input, 0)) == 0)
	{
		ap_message_t *ap_msg;
		ap_msg = malloc(sizeof(struct ap_message_s));
		if(NULL == ap_msg)
			DBG_MSG("malloc the AP struct failed!");

		get_value_from_web(ap_msg, NULL, input);

		if(is_connect_success(RT2860_NVRAM, ap_msg) == 1)
		{
			printf("true");
			DBG_MSG("I send a string true\n");
			DBG_MSG("I get the msg from web is %s\n", input);

			DBG_MSG("APChannel is %s\n", ap_msg->APChannel);
			DBG_MSG("APAuthMode is %s\n", ap_msg->APAuthMode);
			DBG_MSG("APEncrypType is %s\n", ap_msg->APEncrypType);
			DBG_MSG("APSsid is %s\n", ap_msg->APSsid);
			DBG_MSG("APPasswd is %s\n", ap_msg->APPasswd);
		}
		else
		{
			printf("false");
			DBG_MSG("I send a string false\n");
			DBG_MSG("I get the msg from web is %s\n", input);

			DBG_MSG("APChannel is %s\n", ap_msg->APChannel);
			DBG_MSG("APAuthMode is %s\n", ap_msg->APAuthMode);
			DBG_MSG("APEncrypType is %s\n", ap_msg->APEncrypType);
			DBG_MSG("APSsid is %s\n", ap_msg->APSsid);
			DBG_MSG("APPasswd is %s\n", ap_msg->APPasswd);
		}

		free(ap_msg);
		ap_msg = NULL;
	}

#endif

	if(strcmp("commit", web_get("wifiCommit", input, 0)) == 0)
	{

		ap_message_t *ap_msg;
		ap_msg = malloc(sizeof(struct ap_message_s));
		if(NULL == ap_msg)
			DBG_MSG("malloc the AP struct failed!");

		extend_message_t *ex_msg;
		ex_msg =  malloc(sizeof(struct extend_message_s));
		if(NULL == ex_msg)
			DBG_MSG("malloc the extend struct failed!");

		//从web端获取数据，回填到结构体参数地址空间中
		get_value_from_web(ap_msg, ex_msg, input);

#if 0
		if(strcmp("checkmainpasswd", web_get("mainpasswd", input, 0)) == 0)
		{
			if(is_connect_success(RT2860_NVRAM, ap_msg) == 1)
			{
				printf("mainpasswd_success");

				//设置相应的参数到开发板的ram
				if(set_nvram_buf(RT2860_NVRAM, ap_msg, ex_msg) != 0)
				{
					DBG_MSG("set the nvram failed!");
				}
			}
			else
				printf("mainpasswd_failure");
		}

#endif 
#if 1
		//设置相应的参数到开发板的ram
		if(set_nvram_buf(RT2860_NVRAM, ap_msg, ex_msg) != 0)
		{
			DBG_MSG("set the nvram failed!");
		}

#endif
		//释放内存
		free(ap_msg);
		ap_msg = NULL;
		free(ex_msg);
		ex_msg = NULL;

		do_system("init_system restart");
	}

	return 0;
}
