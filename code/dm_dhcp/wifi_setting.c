#include "wifi_setting.h"
#include "nvram.h"

#include <string.h>

#define MESSAGESIZE 2048
#define IFNAMSIZ 32

void apcli_scan(void)
{
	FILE *pp;
	char cmd[CMDLEN], *ptr, wif[IFNAMSIZ];
	char channel[4], ssid[186], bssid[20], security[23];
	char signal[9], mode[9], ext_ch[7], net_type[3];
	char wps[4];

	int i, space_start;
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

	printf("{\n");
	printf("\t\"Scan\":\n\t[\n");
	while (fgets(cmd, sizeof(cmd), pp)) {
		if (strlen(cmd) < 4)
			break;
		ptr = cmd;
		sscanf(ptr, "%s ", channel);
		ptr += 37;
		sscanf(ptr, "%s %s %s %s %s %s %s", bssid, security, signal, mode, ext_ch, net_type, wps);
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

		printf("\t\t{\n");
		printf("\t\t\"Channel\":\"%s\",\n", channel);
		printf("\t\t\"ssid\":\"%s\",\n", ssid);
		printf("\t\t\"bssid\":\"%s\",\n", bssid);
		printf("\t\t\"security\":\"%s\",\n", security);
		printf("\t\t\"signal\":\"%s\",\n", signal);
		printf("\t\t\"mode\":\"%s\",\n", mode);
		printf("\t\t\"ext_ch\":\"%s\",\n", ext_ch);
		printf("\t\t\"net_type\":\"%s\",\n", net_type);
		printf("\t\t\"wps\":\"%s\"\n", wps);
		printf("\t\t},");
		printf("\n");
	}
	printf("\t\t{\n");
	printf("\t\t}\n");
	printf("\t]\n");
	printf("}");

	pclose(pp);
}

/*
value: input 是从web端获取到的数据buffer, ap_msg, extend_msg从input中提取有用的数据到该结构体.
*/
void get_value_from_web(ap_message_t *ap_msg, extend_message_t *extend_msg, char *input)
{

	if(ap_msg == NULL && extend_msg == NULL)
		return ;

	char *security;

	if(ap_msg != NULL)
	{
		strcpy(ap_msg->APChannel, web_get("Channel", input, 0));
		strcpy(ap_msg->APSsid, web_get("ssid", input, 0));

		//record the ap mac addr
		strcpy(ap_msg->AP_Mac, web_get("bssid", input, 0));

		security = web_get("security", input, 0);
		strcpy(ap_msg->security, security);


#if 0
		if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/TKIPAES") == 0 || strcmp(ap_msg->security, "WPA1PSKWPA2PSK/AES") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPAPSKWPA2PSK");
			strcpy(ap_msg->APEncrypType, "TKIPAES");
			ap_msg->mix_flag = 1;
		}
#endif 
		if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/TKIPAES") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
			strcpy(ap_msg->APEncrypType, "AES");
			ap_msg->mix_flag = 0;
		}
		else if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/AES") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
			strcpy(ap_msg->APEncrypType, "AES");
			ap_msg->mix_flag = 0;
		}
		else if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/TKIP") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
			strcpy(ap_msg->APEncrypType, "TKIP");
			ap_msg->mix_flag = 0;
		}
		else
		{
			get_nth_value(0, security, '/', ap_msg->APAuthMode, strlen(security));
			get_nth_value(1, security, '/', ap_msg->APEncrypType, strlen(security));
			ap_msg->mix_flag = 0;
		}

		strcpy(ap_msg->APPasswd, web_get("wifiPassword", input, 0));
	}
	if(extend_msg != NULL)
	{
		strcpy(extend_msg->Extend_wifiName, web_get("newWifiName", input, 0));
		strcpy(extend_msg->Extend_wifiPasswd, web_get("newPassword", input, 0));
		strcpy(extend_msg->ManagePasswd, web_get("managePassword", input, 0));
	}

}

int set_nvram_buf(int nvram, ap_message_t *ap_msg, extend_message_t *ex_msg)
{
	if(ap_msg != NULL)
	{
		if(ap_msg->mix_flag == 1)
			nvram_bufset(nvram, "WpaMixPairCipher", "WPA_TKIP_WPA2_AES");

		//使能主路由
		nvram_bufset(nvram, "ApCliEnable", "1");

		//主路由频道
		nvram_bufset(nvram, "Channel", ap_msg->APChannel);

		//主路由的加密方式，类型
		nvram_bufset(nvram, "ApCliAuthMode", ap_msg->APAuthMode);
		nvram_bufset(nvram, "ApCliEncrypType", ap_msg->APEncrypType);

		//主路由的名字，频道和密码
		nvram_bufset(nvram, "ApCliSsid", ap_msg->APSsid);
		nvram_bufset(nvram, "ApCliWPAPSK", ap_msg->APPasswd);

		//扩展路由的加密方式，频道和类型跟主路由一致
		nvram_bufset(nvram, "AuthMode", ap_msg->APAuthMode);
		nvram_bufset(nvram, "EncrypType", ap_msg->APEncrypType);

		//主路由的bssid，即mac地址
		nvram_bufset(nvram, "ApMac", ap_msg->AP_Mac);

	}
	if(ex_msg != NULL)
	{
		//如果用户没有输入管理员密码，表示跟扩展路由密码一致
		if(strcmp(ex_msg->ManagePasswd, "") == 0)
			strcpy(ex_msg->ManagePasswd, ex_msg->Extend_wifiPasswd);

		//扩展路由的名字，密码和管理密码
		nvram_bufset(nvram, "SSID1", ex_msg->Extend_wifiName);
		nvram_bufset(nvram, "WPAPSK1", ex_msg->Extend_wifiPasswd);
		nvram_bufset(nvram, "ManagePasswd", ex_msg->ManagePasswd);

	}

	nvram_commit(nvram);
	return 0;
}

//当前的nvram = RT2860_NVRAM
//把相应的参数配置到驱动参数中
int is_connect_success(int nvram, ap_message_t *ap_msg)
{
	if(ap_msg == NULL)
		return -1;
#if 0
	do_system("ifconfig apcli0 down");
	do_system("ifconfig apcli0 up");
	do_system("brctl addif br0 ra0");
	do_system("brctl addif br0 apcli0");
	do_system("iwpriv ra0 set Channel=%s", ap_msg->APChannel);
	do_system("iwpriv apcli0 set ApCliEnabel=0");
	do_system("iwpriv apcli0 set ApCliAuthMode=%s", ap_msg->APAuthMode);
	do_system("iwpriv apcli0 set ApCliEncrypType=%s", ap_msg->APEncrypType);
	do_system("iwpriv apcli0 set ApCliSsid=%s", ap_msg->APSsid);
	do_system("iwpriv apcli0 set ApCliWPAPSK=%s", ap_msg->APPasswd);
	do_system("iwpriv apcli0 set ApCliSsid=%s", ap_msg->APSsid);
	do_system("iwpriv apcli0 set ApCliEnabel=1");

#endif 

	do_system("isconnect.sh %d %s %s %s %s", atoi(ap_msg->APChannel), 
			ap_msg->APAuthMode, ap_msg->APEncrypType, ap_msg->APSsid, ap_msg->APPasswd);
	sleep(4);
	do_system("iwpriv apcli0 show connStatus");

	return atoi(nvram_bufget(nvram, "LinkStatus"));
}


int main(int argc, char *argv[])
{
	char input[MESSAGESIZE];
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

	/* 
	   设置界面模块
	*/
	//发送路由周围的ssid给web客户端
	if(strcmp("Scan", web_get("wifiScan", input, 2)) == 0)
	{
		apcli_scan();
	}

	//web客户端提交的路由信息。然后后台进行接收
	if(strcmp("commit", web_get("wifiCommit", input, 2)) == 0)
	{

		//web端要求立马返回一个数据
		//printf("wifiSettingSuc");
		//DBG_MSG("I send a string wifiSettingSuc\n");

		ap_message_t ap_msg;
		extend_message_t ex_msg;

		DBG_MSG("wificommit message is %s\n", input);
		//从web端获取数据，回填到结构体参数地址空间中
		get_value_from_web(&ap_msg, &ex_msg, input);
		if(is_connect_success(RT2860_NVRAM, &ap_msg) == 1)
		{
			//设置相应的参数到开发板的ram
			set_nvram_buf(RT2860_NVRAM, &ap_msg, &ex_msg);
			//do_system("init_system restart");
			//do_system("apclient_con.sh");
			//do_system("internet.sh");
		}
		else 
		{
			printf("false");
		}
	}
	//
	if(strcmp("success", web_get("if_success", input, 2)) == 0)
	{
		sleep(5);
		int status = atoi(nvram_bufget(RT2860_NVRAM, "LinkTest"));
		if( status == 1)
			do_system("init_system restart");
		else
		{
			printf("false");
			DBG_MSG("I seed a false string.");
		}
	}


	/*
	   管理界面模块
	 */
	//web端进入管理界面时请求后台：主路由是否链接成功，以便显示
	if(strcmp("manage_request", web_get("connect", input, 2)) == 0)
	{
		int status = atoi(nvram_bufget(RT2860_NVRAM, "LinkTest"));
		if( status == 1)
			printf("success");
		else
		{
			printf("false");
		}
	}
#if 0
	//检查用户输入主路由密码是否正确
	if(strcmp("ckpwd", web_get("mainpasswd", input, 0)) == 0)
	{
		ap_message_t ap_msg;

		DBG_MSG("check ap passwd  message is %s\n", input);
		get_value_from_web(&ap_msg, NULL, input);

		if(is_connect_success(RT2860_NVRAM, &ap_msg) == 1)
		{
			printf("true");
			DBG_MSG("I send a string true\n");
			DBG_MSG("I get the msg from web is %s\n", input);

			DBG_MSG("APChannel is %s\n", ap_msg.APChannel);
			DBG_MSG("APAuthMode is %s\n", ap_msg.APAuthMode);
			DBG_MSG("APEncrypType is %s\n", ap_msg.APEncrypType);
			DBG_MSG("APSsid is %s\n", ap_msg.APSsid);
			DBG_MSG("APPasswd is %s\n", ap_msg.APPasswd);
		}
		else
		{
			printf("false");
			DBG_MSG("I send a string false\n");
			DBG_MSG("I get the msg from web is %s\n", input);

			DBG_MSG("APChannel is %s\n", ap_msg.APChannel);
			DBG_MSG("APAuthMode is %s\n", ap_msg.APAuthMode);
			DBG_MSG("APEncrypType is %s\n", ap_msg.APEncrypType);
			DBG_MSG("APSsid is %s\n", ap_msg.APSsid);
			DBG_MSG("APPasswd is %s\n", ap_msg.APPasswd);
		}

	}
#endif 

	//管理界面的主路由设置模块过来的请求
	//客户端请求主路由信息
	if(strcmp("station", web_get("ap_station", input, 2)) == 0)
	{
		char mac[18];
		char passwd[65];
		strcpy(mac, nvram_bufget(RT2860_NVRAM, "ApMac"));
		strcpy(passwd, nvram_bufget(RT2860_NVRAM, "ApCliWPAPSK"));
		printf("{");
		printf("\"mac\":\"%s\",", mac);
		printf("\"wifiPwd\":\"%s\"", passwd);
		printf("}");

	}
	//路由设置：web客户端提交的信息
	if(strcmp("commit", web_get("station_commit", input, 2)) == 0)
	{

		ap_message_t ap_msg;
		//extend_message_t ex_msg;

		DBG_MSG("station commit  is %s\n", input);
		//从web端获取数据，回填到结构体参数地址空间中
		//get_value_from_web(&ap_msg, NULL, input);
		strcpy(ap_msg.APPasswd, web_get("routePwd", input, 2));
		strcpy(ap_msg.APSsid, web_get("ssid", input, 2));
		strcpy(ap_msg.APChannel, web_get("Channel", input, 2));
		strcpy(ap_msg.security, web_get("security", input, 2));
		strcpy(ap_msg.AP_Mac, web_get("bssid", input, 2));

		set_nvram_buf(RT2860_NVRAM, &ap_msg, NULL);
		printf("routeSuccess");
	}
	//管理界面的扩展路由设置模块的提交
	if(strcmp("station", web_get("ex_station", input, 2)) == 0)
	{
		DBG_MSG("ex_station commit is %s", input);
#if 0
		extend_message_t ex_msg;
		
		//wifi_name wifi_passwd managePasswd mode_elect 
		strcpy(ex_msg.Extend_wifiName, web_get("", input, 2));
		strcpy(ex_msg.Extend_wifiPasswd, web_get("", input, 2));
		char manage_pwd[65];
		strcpy(manage_pwd, web_get("", input, 2));
		if(strcmp(manage_pwd, "") == 0)
			strcpy(ex_msg.managePasswd, ex_msg.Extend_wifiPasswd);
		set_nvram_buf(RT2860_NVRAM, NULL, &ex_msg);

		int txpower;
		txpower = strtol(web_get("", input, 2));
	
		nvram_bufset(RT2860_NVRAM, "TxPower", txpower);
		do_system("init_system restart");
#endif 
		printf("wifiSuccess");
	}
	return 0;
}
