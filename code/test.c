#include "utils.h"
#include "client_info.h"

//user spaces buffer data;
char data[4096];  //在get_mac_table中要用到，将内核的数据保存到用户空间中。

//从打开指定的文件流里找mac地址，找到返回0，找不到返回-1
//int find_mac(char *macaddr, FILE *stream)
int find_mac(char *macaddr, FILE *stream)
{
	if( macaddr == NULL || stream == NULL)
		return -1;

	char buff[2048];
	char file_mac[18];

	while(fgets(buff, 2048, stream) > 0)
	{
		strcpy(file_mac, web_get("mac", buff, 0));
		if(strcmp(file_mac, macaddr) == 0)
			return 0;
	}

	return -1;
}

//获取设备列表的信息到指定的打开的文件流
int get_device_list(FILE *stream)
{
	if(stream == NULL)
		return -1;

	FILE *fp; 
	struct dhcpOfferedAddr {
		unsigned char hostname[16];
		unsigned char mac[16];
		unsigned long ip;
		unsigned long expires;
	} lease;

	int i;
	struct in_addr addr;

	char hostname[32];
	char mac[18];
	char ip[16];

	do_system("killall -q -USR1 udhcpd");
	fp = fopen("/var/udhcpd.leases", "r");
	if (NULL == fp)
		return;

	while (fread(&lease, 1, sizeof(lease), fp) == sizeof(lease)) {
		if (strlen(lease.hostname) > 0) {
			sprintf(hostname, "%s", lease.hostname);
			convert_string_display(hostname);
		} else {
			strcpy(hostname, "");
		}
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				lease.mac[0], lease.mac[1], lease.mac[2],
				lease.mac[3], lease.mac[4], lease.mac[5]);
		mac[17] = '\0';

		//如果mac地址在列表中，不再重复添加，跳过
		if(find_mac(mac, stream) == 0)
			continue;

		addr.s_addr = lease.ip;

		strcpy(ip, inet_ntoa(addr));

		FILE *pp;
		char cmd[128], msg_os[64];
		sprintf(cmd, "fingerprint.sh query %s", inet_ntoa(addr));
		pp = popen(cmd, "r");
		if(pp == NULL)
			pp = "";
		memset(msg_os, 0, 64);
		fread(msg_os, 64, 1, pp);
		pclose(pp);

		char msg_info[2048];
		snprintf(msg_info, 2048, "hostname=%s&&mac=%s&ip=%s&msg_os=%s", hostname, mac, ip, msg_os);

		fputs(msg_info, stream);

	}
	fclose(fp);

}
/*
 * 获取连接该路由的设备信息
 * mac地址　signal信号的强度　connectime连接的时间(秒为单位)
 *
 */
//经过测试  ok
int get_mac_table(rt_mac_table *mac_table)
{
	if(mac_table == NULL)
		return -1;

	char name[25];
	int socket_id;
	struct iwreq wrq;
	int ret;
	// open socket based on address family: AF_NET ----------------------------
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
		printf("\nrtuser::error::Open socket error!\n\n");
		return -1;
	}
	// set interface name as "ra0" --------------------------------------------
	sprintf(name, "ra0");
	memset(data, 0x00, 255);

	//get AP's mac table, remove "get_mac_table" string -----------------------
	memset(data, 0x00, 2048);
	strcpy(data, "");
	strcpy(wrq.ifr_name, name);
	wrq.u.data.length = 2048;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;

	//RTPRIV_IOCTL_GET_MAC_TABLE 在oid.h中声明的。
	ret = ioctl(socket_id, RTPRIV_IOCTL_GET_MAC_TABLE, &wrq);
	if(ret != 0)
	{
		printf("\nrtuser::error::get mac table\n\n");
		goto rtuser_exit;
	}

	//printf("\n========== Get Associated MAC Table ==========\n");
	//RT_802_11_MAC_TABLE 在oid.h中声明
	RT_802_11_MAC_TABLE *mp;
	int i;
	mp = (RT_802_11_MAC_TABLE *)wrq.u.data.pointer;

	mac_table->Num = mp->Num;
	for(i = 0 ; i < mp->Num ; i++)
	{
		sprintf(mac_table->entry[i].Mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				mp->Entry[i].Addr[0], 
				mp->Entry[i].Addr[1],
				mp->Entry[i].Addr[2],
				mp->Entry[i].Addr[3],
				mp->Entry[i].Addr[4],
				mp->Entry[i].Addr[5]);

		mac_table->entry[i].Signal = mp->Entry[i].AvgRssi0;
		mac_table->entry[i].ConnectedTime = mp->Entry[i].ConnectedTime;
	}

rtuser_exit:
	if (socket_id >= 0)
		close(socket_id);
	if(ret)
		return ret;
	else
		return 0;
}
//检查是否在内存中有记录设备信息
int no_clientinfo_in_nvram(int nvram)
{
	char buf[MAX_CLIENT_INFO_LIST];
	if(strcmp(nvram_bufget(nvram, "client_info_list"), "") == 0)
		return 0;
	else
		return -1;
}
void add_info_to_nvram(int nvram, char *hostname, char *mac)
{
	if(hostname == NULL || mac == NULL)
		return ;

	char buf[MAX_CLIENT_INFO_LIST];
	strcpy(buf, nvram_bufget(nvram, "client_info_list"));
	int count = get_nums(buf, ';');
	if(count < CLIENT_LIST_NUM)
	{
		sprintf(buf, "%s;%s&%s", buf, hostname, mac);
		nvram_bufset(nvram, "client_info_list", buf);
	}
	//MAX_CLIENT 是存储信息最大设备个数，如果大于会删除第一个设备信息，然后在末尾添加新的信息。
	else if(count > CLIENT_LIST_NUM)
	{
			//删除第一个设备信息
			int index = 0; 
			delete_nth_value(&index, count, buf, ';');
			sprintf(buf, "%s;%s&%s", buf, hostname, mac);
			nvram_bufset(nvram, "client_info_list", buf);
	}
	nvram_commit(nvram);
}

//找出mac地址在第几个设备的记录
int find_index_mac_in_nvram(int nvram,int count, char *client_info_list, char *mac)
{
	int i;
	for(i = 0; i < count; i++)
	{
		char flash_mac[18];
		char client_info[128];
		get_nth_value(i, client_info_list, ';', client_info, strlen(client_info_list) + 1);
		get_nth_value(1, client_info, '&', flash_mac, strlen(client_info) + 1);
		if(strcmp(mac, flash_mac) == 0)
			return i;
	}
	return -1;
}

//从给出的mac地址在/var/APCLient.list中找出，对应的macaddr
//获取数据到参数给出的地址中
int get_os_host_from_file(char *mac, char *hostname,  char *msg_os)
{
	if(mac == NULL)
		return -1;

	FILE *fp;
	//打开自己保存的设备记录的文件
	fp = fopen(CLIENT_LIST, "r");
	if(fp == NULL)
		return -2;

	char buff[1024];
	while(fgets(buff, 1024, fp))
	{

		char Mac[18];
		char Msg_os[65];
		char HostName[65];

		strcpy(HostName,web_get("hostname", buff, 0));
		strcpy(Mac,web_get("mac", buff, 0));
		strcpy(Msg_os,web_get("msg_os", buff, 0));
		Msg_os[strlen(Msg_os) - 1] = '\0';
		//跟之前记录的mac作比较,相同则显示
		if(strcmp(Mac, mac) == 0)
		{
			if(msg_os != NULL)
				strcpy(msg_os, Msg_os);
			if(hostname != NULL)
				strcpy(hostname, HostName);
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
//mac是给出的地址，用来跟nvarm的clieng_info_list中的macaddr比较
//如果相等，回填hostname到所给的参数中
//１找到，０找不到
int get_hostname_in_nvram(int nvram, char *mac, char *hostname)
{
	char client_list[MAX_CLIENT_INFO_LIST];
	strcpy(client_list, nvram_bufget(nvram, "client_info_list"));
	int client_num = get_nums(client_list, ';');
	int i;
	if(client_num > 0)
	{
		for(i = 0; i < client_num; i++)
		{
			char flash_hostname[65];
			char flash_mac[18];
			char info[128];
			get_nth_value(i, client_list, ';', info, strlen(client_list) + 1);
			get_nth_value(0, info, '&', flash_hostname, strlen(info) + 1);
			get_nth_value(1, info, '&', flash_mac, strlen(info) + 1);
			//跟之前记录的mac作比较,相同则显示
			if(strcmp(mac, flash_mac) == 0)
			{
				if(hostname != NULL)
					strcpy(hostname, flash_hostname);
				return 1;
			}
		}
	}
	return 0;
}

//调用此函数，必须先执行get_device_list
void add_clientinfo_to_nvram(int nvram)
{
	rt_mac_table mac_table;
	get_mac_table(&mac_table);
	if(mac_table.Num > 0)
	{
		int i; 
		for(i = 0; i < mac_table.Num; i++)
		{
			char hostname[65];
			//不在nvram中，在file中
			if(get_hostname_in_nvram(nvram, mac_table.entry[i].Mac, NULL) != 1 && 
					get_os_host_from_file(mac_table.entry[i].Mac, hostname, NULL) == 1)
			{
				//nvram里面没有记录设备信息
				if(no_clientinfo_in_nvram(nvram) == 0)
				{
					char info[83];
					sprintf(info, "%s&%s", hostname, mac_table.entry[i].Mac);
					nvram_bufset(nvram, "client_info_list", info);
					nvram_commit(nvram);
				}
				else
				{
					add_info_to_nvram(nvram, hostname, mac_table.entry[i].Mac);
				}
			}
		}
	}
}

/*谁在上网*/
void get_client_list(int nvram)
{
	rt_mac_table mac_table;
	get_mac_table(&mac_table);
	if(mac_table.Num > 0)
	{
		int i; 
		printf("{\n");
		printf("\t\"Client_Info\":\n");
		printf("\t[\n");

		for(i = 0; i < mac_table.Num; i++)
		{
			//跟之前记录的mac作比较,相同则显示
			char hostname[65] = {0};
			char msg_os[65] = {0};
			if(get_hostname_in_nvram(nvram, mac_table.entry[i].Mac, hostname) == 1 && 
					get_os_host_from_file(mac_table.entry[i].Mac, NULL, msg_os) == 1)
			{
				printf("\t\t{\n");
				printf("\t\t\"HostName\":\"%s\",\n", hostname);
				printf("\t\t\"Signal\":\"%d\",\n", mac_table.entry[i].Signal);
				printf("\t\t\"ConnectedTime\":\"%d\",\n", mac_table.entry[i].ConnectedTime);
				printf("\t\t\"Mac\":\"%s\",\n", mac_table.entry[i].Mac);
				printf("\t\t\"Msg_os\":\"%s\"\n", msg_os);
				printf("\t\t},\n");
			}
		}
		printf("\t\t{\n\t\t}\n");
		printf("\t]\n");
		printf("}\n");
	}
	else
	{
		printf("{}");
	}

}

/*修改主机名，信息保存到nvram的client_info_list变量里　
  格式: client_info_list=hostname1&macaddr1;hostname2&macaddr2 */
int change_hostname(int nvram, char *hostname, char *macaddr)
{
	if(hostname == NULL || macaddr == NULL)
		return -1;

	char client_list[MAX_CLIENT_INFO_LIST];
	strcpy(client_list, nvram_bufget(nvram, "client_info_list"));

	int client_num = get_nums(client_list, ';');
	int index = find_index_mac_in_nvram(nvram, client_num, client_list, macaddr);
	if(index >= 0)
	{
		delete_nth_value(&index, client_num, client_list, ';');
		sprintf(client_list, "%s;%s&%s", client_list, hostname, macaddr);
		nvram_bufset(nvram, "client_info_list", client_list);
		nvram_commit(nvram);
	}
	return 0;
}

int add_AccessControlList(int nvram, char *ACLList, char *mac)
{
	if(ACLList != NULL && mac != NULL)
	{
		int count = get_nums(ACLList, ';');
		if(count < CLIENT_LIST_NUM)
		{
			sprintf(ACLList, "%s;%s",ACLList, mac);
			nvram_bufset(nvram, "AccessControlList0", ACLList);
			nvram_commit(nvram);
			return 0;
		}
		else 
			return -1;
	}
}

/*添加黑名单*/
int add_blacklist(int nvram, char *mac)
{
	if(mac == NULL)
		return -1;

	nvram_bufset(nvram, "AccessPolicy0", "2");
	//当黑名单为空时
	char ACL0[MAX_ACL_LIST];
	strcpy(ACL0, nvram_bufget(RT2860_NVRAM, "AccessControlList0"));
	if(strcmp(ACL0, "") == 0)
	{
		nvram_bufset(nvram, "AccessControlList0", mac);
		nvram_commit(nvram);
	}
	else
	{
		if(add_AccessControlList(nvram, ACL0, mac) != 0)
			return -1;

	}

	do_system("iwpriv ra0 set AccessPolicy=2");
	do_system("iwpriv ra0 set ACLAddEntry=%s", mac);

	//do_system("iwpriv ra0 set ACLShowAll=1");
	//添加完黑名单以后，显示现在在线的设备信息返回给web端
	get_client_list(nvram);
	return 0;
}


/*显示黑名单*/
void show_blacklist(int nvram)
{
	char ACLlist[2048];
	strcpy(ACLlist,nvram_bufget(nvram, "AccessControlList0"));
	int count = get_nums(ACLlist, ';');
	if(count == 0)
	{
		printf("{}");
	}
	else
	{
		int i;
		printf("{\n");
		printf("\t\"Black_List\":\n");
		//printf("\t\"Client_Info\":\n");
		printf("\t[\n");
		for(i = 0; i < count; i++)
		{
			char blackmac[18];
			char hostname[65];
			char msg_os[65];
			//char filemac[18];
			get_nth_value(i, ACLlist, ';', blackmac, sizeof(ACLlist));
			blackmac[17] = '\0';

			if(get_hostname_in_nvram(nvram, blackmac, hostname) == 1 && 
					get_os_host_from_file(blackmac, NULL, msg_os) == 1)
			{
				printf("\t\t{\n");
				printf("\t\t\"HostName\":\"%s\",\n", hostname);
				printf("\t\t\"Mac\":\"%s\",\n", blackmac);
				printf("\t\t\"Msg_os\":\"%s\"\n", msg_os);
				printf("\t\t},\n");
			}
		}
		printf("\t\t{\n\t\t}\n");
		printf("\t]\n");
		printf("}\n");
	}
}

/*count 为maclist 的个数，
  mac要寻找的地址
  返回值为mac在maclist的下标位置
  */
int find_index(int count, char *mac, char *maclist)
{
	int i;
	char substr[18];
	if(mac != NULL && maclist != NULL && count >= 0)
	{
		for(i = 0 ; i < count; i++)
		{
			//这里有点问题
			//get_nth_value(i, maclist, ';', substr, strlen(maclist) + 1 );
			get_nth_value(i, maclist, ';', substr, 2048 );
			if(strcmp(mac, substr) == 0)
				return i;
		}
	}
	return -1;
}
/*删除黑名单*/
int del_blacklist(int nvram, char *mac)
{
	if(mac == NULL)
		return -1;

	char ACLlist[MAX_ACL_LIST];
	strcpy(ACLlist,nvram_bufget(nvram, "AccessControlList0"));
	if(strcmp(ACLlist, "") == 0)
	{
		DBG_MSG("black list is empty!");
		return 0;
	}
	int count = get_nums(ACLlist, ';');
	int index = find_index(count, mac, ACLlist);
	if(index < 0)
	{
		DBG_MSG("can not find the mac in the blacklist!");
		return -1;
	}
	do_system("iwpriv ra0 set ACLDelEntry=%s", mac);
	delete_nth_value(&index, count, ACLlist, ';');

	nvram_bufset(nvram, "AccessControlList0", ACLlist);
	nvram_commit(nvram);

	return 0;
}

int main(int argc, char *argv[])
{

	//获取设备的信息存储到本地
	FILE *fp; 
	fp = fopen(CLIENT_LIST, "a+");
	if(fp == NULL)
		return -1;
	get_device_list(fp);
	sleep(4);
	fclose(fp);

	rt_mac_table mac_table;
	get_mac_table(&mac_table);
	int i;
	if(mac_table.Num > 0)
	{
		for(i = 0; i < mac_table.Num; i++)
		{
				printf("\t\t\"Signal\":\"%d\",\n", mac_table.entry[i].Signal);
				printf("\t\t\"ConnectedTime\":\"%d\",\n", mac_table.entry[i].ConnectedTime);
				printf("\t\t\"Mac\":\"%s\",\n", mac_table.entry[i].Mac);
		}

	}
	
#if 0
	add_clientinfo_to_nvram(RT2860_NVRAM);
#if 1
	char input[MAX_MSG_SIZ];  //buffer for get message from web
	int length;

	length = get_message_for_web(input);

	//return the message type for web client
	web_debug_header();

	//web端请求谁在上网
	if(strcmp("wholine", web_get("Client_Info", input, 0)) == 0)
	{
		get_client_list(RT2860_NVRAM);
	}
	//change the hostname
	if(strcmp("changehost", web_get("changename", input, 0)) == 0)
	{
		char HostName[32];
		char Mac[64];
		strcpy(HostName, web_get("hostname", input, 0));
		strcpy(Mac, web_get("mac", input, 0));
		change_hostname(RT2860_NVRAM, HostName, Mac);
	}
	//add to the black list
	if(strcmp("black", web_get("blacklist", input, 0)) == 0)
	{
		char Mac[18];
		if(strcpy(Mac, web_get("mac", input, 0)) != NULL)
			add_blacklist(RT2860_NVRAM, Mac);
	}
	//delete black list
	if(strcmp("delete", web_get("delblacklist", input, 0)) == 0)
	{
		char Mac[18];
		strcpy(Mac, web_get("mac", input, 0));
		del_blacklist(RT2860_NVRAM, Mac);
		printf("del");
	}
	//show black list
	if(strcmp("show", web_get("showblacklist", input, 0)) == 0)
	{
		show_blacklist(RT2860_NVRAM);
	}
	return 0;
#endif 
#endif 
	return 0;
}

