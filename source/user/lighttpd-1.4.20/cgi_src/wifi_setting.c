#include "wifi_setting.h"
#include "nvram.h"

#include <string.h>
#include <unistd.h>

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
		if(strcmp(ssid, "") == 0)
			continue;
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
	fflush(NULL);

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
		security = web_get("security", input, 0);
		strcpy(ap_msg->security, security);
#if 1
		if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/TKIPAES") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
			strcpy(ap_msg->APEncrypType, "AES");
		}
		else if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/AES") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
			strcpy(ap_msg->APEncrypType, "AES");
		}
		else if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/TKIP") == 0)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
			strcpy(ap_msg->APEncrypType, "TKIP");
		}
		else if((strcmp(ap_msg->security, "WPA1PSK/TKIPAES") == 0) || (strcmp(ap_msg->security, "WPA2PSK/TKIPAES") == 0))
		{
			//strcpy(ap_msg->APAuthMode, "WPA2PSK");
			get_nth_value(0, security, '/', ap_msg->APAuthMode, strlen(security));
			strcpy(ap_msg->APEncrypType, "TKIP");
		}
		else
		{
			get_nth_value(0, security, '/', ap_msg->APAuthMode, strlen(security));
			get_nth_value(1, security, '/', ap_msg->APEncrypType, strlen(security));
		}

#endif 
#if 0
		get_nth_value(0, security, '/', ap_msg->APAuthMode, strlen(security));
		if(strstr(ap_msg->APAuthMode, "WPA2PSK") != NULL)
		{
			strcpy(ap_msg->APAuthMode, "WPA2PSK");
		}
		get_nth_value(1, security, '/', ap_msg->APEncrypType, strlen(security));
		if(strstr(ap_msg->APEncrypType, "AES") != NULL)
		{
			strcpy(ap_msg->APEncrypType, "AES");
		}
#endif 

		strcpy(ap_msg->APMac, web_get("bssid", input, 0));
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
		//使能主路由
		//nvram_bufset(nvram, "CMCC_ApCliEnable", "1");

		//主路由频道
		nvram_bufset(nvram, "Channel", ap_msg->APChannel);

		//主路由的加密方式，类型
		nvram_bufset(nvram, "CMCC_ApCliAuthMode", ap_msg->APAuthMode);
		nvram_bufset(nvram, "CMCC_ApCliEncrypType", ap_msg->APEncrypType);

		//主路由的名字，频道和密码
		nvram_bufset(nvram, "CMCC_ApCliSsid", ap_msg->APSsid);
		nvram_bufset(nvram, "CMCC_ApCliWPAPSK", ap_msg->APPasswd);

		//扩展路由的加密方式，频道和类型跟主路由一致
		nvram_bufset(nvram, "AuthMode", ap_msg->APAuthMode);
		nvram_bufset(nvram, "EncrypType", ap_msg->APEncrypType);
		nvram_bufset(nvram, "CMCC_SelectApMac", ap_msg->APMac);
		//nvram_bufset(nvram, "CMCC_SelectApSecurity", ap_msg->security);
	}
	if(ex_msg != NULL)
	{
		//如果用户没有输入管理员密码，表示跟扩展路由密码一致
		if(strcmp(ex_msg->ManagePasswd, "") == 0)
			strcpy(ex_msg->ManagePasswd, ex_msg->Extend_wifiPasswd);

		//扩展路由的名字，密码和管理密码
		nvram_bufset(nvram, "SSID1", ex_msg->Extend_wifiName);
		nvram_bufset(nvram, "WPAPSK1", ex_msg->Extend_wifiPasswd);
		nvram_bufset(nvram, "CMCC_ManagePasswd", ex_msg->ManagePasswd);
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
	do_system("isconnect.sh %d %s %s %s %s", atoi(ap_msg->APChannel), 
			ap_msg->APAuthMode, ap_msg->APEncrypType, ap_msg->APSsid, ap_msg->APPasswd);
	sleep(6);
	do_system("iwpriv apcli0 show connStatus");
	sleep(4);
	DBG_MSG("CMCC_LinkStatus=%s", nvram_bufget(nvram, "CMCC_LinkStatus"));

	return atoi(nvram_bufget(nvram, "CMCC_LinkStatus"));
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
			printf("wifiSettingSuccess");
			fflush(NULL);
			DBG_MSG("I send a string wifiSettingSuccess\n");

			ap_message_t ap_msg;
			extend_message_t ex_msg;

			DBG_MSG("wificommit message is %s\n", input);
			//从web端获取数据，回填到结构体参数地址空间中
			get_value_from_web(&ap_msg, &ex_msg, input);

			//设置相应的参数到开发板的ram
			set_nvram_buf(RT2860_NVRAM, &ap_msg, &ex_msg);
	}

#if 0
	//选中的wifi：选中连接的主路由信息,web端会需要用到
	if(strcmp("data", web_get("data_commit", input, 2)) == 0)
	{
		DBG_MSG("get message from web %s", input);
		char buffer[1024];
		char mac[65];
		char wifiname[65];
		char wifipwd[65];
		char newwifiname[65];
		char newwifipwd[65];
		char managepwd[65];

		strcpy(mac, web_get("mac", input, 2));
		strcpy(wifiname, web_get("wifiName", input, 2));
		strcpy(wifipwd, web_get("wifiPwd", input, 2));
		strcpy(newwifiname, web_get("newWifiName", input, 2));
		strcpy(newwifipwd, web_get("newPassword", input, 2));
		strcpy(managepwd, web_get("managePassword", input, 2));
		if(strcmp(managepwd, "") == 0)
			strcpy(managepwd, newwifipwd);
		sprintf(buffer, "%s;%s;%s;%s;%s;%s", mac, wifiname, wifipwd, newwifiname, newwifipwd, managepwd);
		nvram_bufset(RT2860_NVRAM, "select_info", buffer);
		nvram_commit(RT2860_NVRAM);
	}
#endif 

#if 1

#if 0
	//这个验证还有点问题，目前不做验证过程
	//进入配置界面,web端请求，后台进行主路由密码检验。不成功返回false，成功，立即重启
	if(strcmp("success", web_get("if_success", input, 2)) == 0)
	{
		ap_message_t ap_msg;
		strcpy(ap_msg.APChannel, nvram_bufget(RT2860_NVRAM, "Channel"));
		strcpy(ap_msg.APAuthMode, nvram_bufget(RT2860_NVRAM, "CMCC_ApCliAuthMode"));
		strcpy(ap_msg.APEncrypType, nvram_bufget(RT2860_NVRAM, "CMCC_ApCliEncrypType"));
		strcpy(ap_msg.APSsid, nvram_bufget(RT2860_NVRAM, "CMCC_ApCliSsid"));
		strcpy(ap_msg.APPasswd, nvram_bufget(RT2860_NVRAM, "CMCC_ApCliWPAPSK"));
		if(is_connect_success(RT2860_NVRAM, &ap_msg) == 1)
		{
			printf("conntrue");
			fflush(NULL);
			web_redirect(getenv("HTTP_REFERER"));
			DBG_MSG("I send a conntrue string to web client.");
		}
		else 
		{
			printf("connfalse");
			fflush(NULL);
			//web_redirect(getenv("HTTP_REFERER"));
			DBG_MSG("I send a connfalse string to web client.");
		}
	}
#endif 
	//web端进入配置成功界面，请求启动配置参数
	if(strcmp("restart", web_get("init_restart", input, 2)) == 0)
	{
		DBG_MSG("reboot.");
		nvram_bufset(RT2860_NVRAM, "CMCC_ApCliEnable", "1");
		//nvram_bufset(RT2860_NVRAM, "CMCC_LinkStatus", "0");
		nvram_commit(RT2860_NVRAM);
		do_system("init_system restart");
		do_system("killall dm_dhcp");
		do_system("dm_dhcp");
	}
#endif

	/*
	   管理界面模块
	 */
	//web端进入管理界面，请求数据
	if(strcmp("data", web_get("get_data", input, 2)) == 0)
	{
		printf("{\n");
		printf("\"mac\":\"%s\",", nvram_bufget(RT2860_NVRAM, "CMCC_SelectApMac"));
		printf("\"wifiName\":\"%s\",", nvram_bufget(RT2860_NVRAM, "CMCC_ApCliSsid"));
		printf("\"wifiPwd\":\"%s\",", nvram_bufget(RT2860_NVRAM, "CMCC_ApCliWPAPSK"));
		printf("\"newWifiName\":\"%s\",", nvram_bufget(RT2860_NVRAM, "SSID1"));
		printf("\"newPassword\":\"%s\",", nvram_bufget(RT2860_NVRAM, "WPAPSK1"));
		printf("\"managePassword\":\"%s\"", nvram_bufget(RT2860_NVRAM, "CMCC_ManagePasswd"));
		printf("}\n");
	}

#if 0
	//web端进入管理界面时请求后台：主路由是否链接成功，以便显示
	if(strcmp("manage_request", web_get("connect", input, 2)) == 0)
	{
		int status = atoi(nvram_bufget(RT2860_NVRAM, "CMCC_LinkStatus"));
		if( status == 1)
		{
			printf("success");
			fflush(NULL);

		}
		else
		{
			printf("false");
			fflush(NULL);
		}
	}
#endif 

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

		if(strcmp(ap_msg.security, "WPA1PSKWPA2PSK/TKIPAES") == 0)
		{
			strcpy(ap_msg.APAuthMode, "WPA2PSK");
			strcpy(ap_msg.APEncrypType, "AES");
		}
		else if(strcmp(ap_msg.security, "WPA1PSKWPA2PSK/AES") == 0)
		{
			strcpy(ap_msg.APAuthMode, "WPA2PSK");
			strcpy(ap_msg.APEncrypType, "AES");
		}
		else if(strcmp(ap_msg.security, "WPA1PSKWPA2PSK/TKIP") == 0)
		{
			strcpy(ap_msg.APAuthMode, "WPA2PSK");
			strcpy(ap_msg.APEncrypType, "TKIP");
		}
		else if((strcmp(ap_msg.security, "WPA1PSK/TKIPAES") == 0) || (strcmp(ap_msg.security, "WPA2PSK/TKIPAES") == 0))
		{
			get_nth_value(0, ap_msg.security, '/', ap_msg.APAuthMode, strlen(ap_msg.security));
			strcpy(ap_msg.APEncrypType, "AES");
		}
		else
		{
			get_nth_value(0, ap_msg.security, '/', ap_msg.APAuthMode, strlen(ap_msg.security));
			get_nth_value(1, ap_msg.security, '/', ap_msg.APEncrypType, strlen(ap_msg.security));

			ap_msg.APAuthMode[strlen(ap_msg.APAuthMode) + 1] = '\0';
			ap_msg.APEncrypType[strlen(ap_msg.APEncrypType) + 1] = '\0';
		}

		set_nvram_buf(RT2860_NVRAM, &ap_msg, NULL);

		//返回给web客户端，这是客户端要求要返回的数据
		printf("routeSuccess");
		fflush(NULL);
	}
	//管理界面的扩展路由设置模块的提交
	if(strcmp("station", web_get("ex_station", input, 2)) == 0)
	{
		DBG_MSG("ex_station commit is %s", input);
#if 1
		extend_message_t ex_msg;
		
		//wifi_name wifi_passwd managePasswd mode_elect 
		strcpy(ex_msg.Extend_wifiName, web_get("newWifiName", input, 2));
		strcpy(ex_msg.Extend_wifiPasswd, web_get("newPassword", input, 2));

		char manage_pwd[65];
		strcpy(manage_pwd, web_get("managePassword", input, 2));
		if(strcmp(manage_pwd, "") == 0)
			strcpy(ex_msg.ManagePasswd, ex_msg.Extend_wifiPasswd);
		set_nvram_buf(RT2860_NVRAM, NULL, &ex_msg);

		/*
		  模式选择:穿墙模式(throughWall) --> 100
		  		   标准模式(standard) --> 95
				   孕妇模式(pregnant) --> 85
		 */
		char *select_mode;
		select_mode = web_get("throughWall", input, 2);
		if(strcmp(select_mode, "true") == 0)
		{
			nvram_bufset(RT2860_NVRAM, "TxPower", "100");
		}	
		select_mode = web_get("satandard", input, 2);
		if(strcmp(select_mode, "true") == 0)
		{
			nvram_bufset(RT2860_NVRAM, "TxPower", "95");
		}
		select_mode = web_get("pregnant", input, 2);
		if(strcmp(select_mode, "true") == 0)
		{
			nvram_bufset(RT2860_NVRAM, "TxPower", "85");
		}
		nvram_commit(RT2860_NVRAM);
#endif 
		//返回给web客户端，这是客户端要求要返回的数据
		printf("wifiSuccess");
		fflush(NULL);
	}
	return 0;
}
