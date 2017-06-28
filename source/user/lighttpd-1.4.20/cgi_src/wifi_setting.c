#include "wifi_setting.h"
//#include "nvram.h"

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
	//	char special_if[IFNAMSIZ];
	//char filter[4];
	int total_ssid_num, get_ssid_times, round;

	//get_nth_value(2, input, '&', special_if, sizeof(special_if));
	//get_nth_value(3, input, '&', filter, sizeof(filter));

	strncpy(wif, "ra0", sizeof(wif));

	do_system("iwpriv %s set SiteSurvey=1", wif);

	sleep(1); // for get the SCAN result. (2G + 5G may enough for 5 seconds)

	sprintf(cmd, "iwpriv %s get_site_survey", wif);
	if(!(pp = popen(cmd, "r"))) {
		DBG_MSG("execute get_site_survey fail!");
		return;
	}

	memset(cmd, 0, sizeof(cmd));

	// web_debug_header();

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

#if 0
int get_message_for_web(char *input)
{
	assert(input != NULL);

	int length;
	char *method;

	method = getenv("REQUEST_METHOD");
	if(method == NULL)
		return -1;
	//POST方法
	if(!strcmp(method, "POST"))
	{
		length = atoi(getenv("CONTENT_LENGTH"));
		if(length != 0)
		{
			//从标准输入读取一定的数据
			fgets(input, length + 1, stdin);
		}
	}
	else if(!strcmp(method, "GET"))
	{
		input = getenv("QUERY_STRING");
		length = strlen(input);
	}

	if(length == 0)
		return 0;

	return length;
}

#endif 


void get_value_from_web(ap_message_t *ap_msg, extend_message_t *extend_msg, char *input)
{

	if(ap_msg == NULL && extend_msg == NULL)
		DBG_MSG("get_value_from_web is error!");

	char *security;

	if(ap_msg != NULL)
	{
		strcpy(ap_msg->APChannel, web_get("Channel", input, 2));
		strcpy(ap_msg->APSsid, web_get("ssid", input, 2));

		security = web_get("security", input, 2);
		strcpy(ap_msg->security, security);

		get_nth_value(0, security, '/', ap_msg->APAuthMode, strlen(security));

		get_nth_value(1, security, '/', ap_msg->APEncrypType, strlen(security));

		strcpy(ap_msg->APPasswd, web_get("wifiPassword", input, 2));
	}
	if(extend_msg != NULL)
	{
		strcpy(extend_msg->Extend_wifiName, web_get("newWifiName", input, 2));
		strcpy(extend_msg->Extend_wifiPasswd, web_get("newPassword", input, 2));
		strcpy(extend_msg->ManagePasswd, web_get("managePassword", input, 2));
	}

}

void apcli_connect(ap_message_t *ap_msg)
{
	if(ap_msg == NULL)
		DBG_MSG("apcli_connect error! the ap message is empty.");

#if 1
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
//	do_system("udhcpc -i br0 -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid");
#endif 
}

int set_nvram_buf(int nvram, ap_message_t *ap_msg, extend_message_t *ex_msg)
{
	if(ap_msg == NULL  || NULL == ex_msg)
		return -1;
	//如果用户没有输入管理员密码，表示跟扩展路由密码一致
	if(strcmp(ex_msg->ManagePasswd, "") == 0)
		strcpy(ex_msg->ManagePasswd, ex_msg->Extend_wifiPasswd);

	if(strcmp(ap_msg->security, "WPA1PSKWPA2PSK/TKIPAES") == 0)
		nvram_bufset(nvram, "WpaMixPairCipher","WPA_TKIP_WPA2_AES");

	nvram_bufset(nvram, "ApCliEnable", "1");
	nvram_bufset(nvram, "ApCliAuthMode", ap_msg->APAuthMode);
	nvram_bufset(nvram, "ApCliEncrypType", ap_msg->APEncrypType);
	nvram_bufset(nvram, "ApCliSsid", ap_msg->APSsid);
	nvram_bufset(nvram, "ApCliWPAPSK", ap_msg->APPasswd);

	//扩展路由的加密方式，频道和类型跟主路由一致
	nvram_bufset(nvram, "Channel", ap_msg->APChannel);
	nvram_bufset(nvram, "AuthMode", ap_msg->APAuthMode);
	nvram_bufset(nvram, "EncrypType", ap_msg->APEncrypType);

	nvram_bufset(nvram, "SSID1", ex_msg->Extend_wifiName);
	nvram_bufset(nvram, "WPAPSK1", ex_msg->Extend_wifiPasswd);
	nvram_bufset(nvram, "ManagePasswd", ex_msg->ManagePasswd);
	nvram_commit(nvram);

	return 0;
}


int main(int argc, char *argv[])
{
	char input[MESSAGESIZE];
	int length;


	length = get_message_for_web(input);
	if(length <= 0)
		DBG_MSG("get the message form web is empty!");

	//for cgi, define the message type.
	web_debug_header();


	//发送路由周围的ssid给web客户端
	if(strcmp("Scan", web_get("wifiScan", input, 2)) == 0)
	{
		apcli_scan();
	}

	if(strcmp("commit", web_get("wifiCommit", input, 2)) == 0)
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

		//设置相应的参数到开发板的ram
		if(set_nvram_buf(RT2860_NVRAM, ap_msg, ex_msg) != 0)
		{
			DBG_MSG("set the nvram failed!");
		}
		
		//连接到主路由
	//	apcli_connect(ap_msg);
		//sleep(30);

		//释放内存
		free(ap_msg);
		ap_msg = NULL;

		free(ex_msg);
		ex_msg = NULL;

		do_system("init_system restart");
	}

	//重启,该脚本需重新改写
	//do_system("internet.sh");
	return 0;

}
