#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h> /* for close */
#include <string.h>


#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <linux/wireless.h>

#include "oid.h"


#include "nvram.h"


#define CLIENT_LIST "/var/APClient.list"

#define MAX_LEN_OF_MAC_TABLE  64

//get mac table
typedef struct _rt_mac_entry
{
	char Mac[18]; //设备的mac地址
	int  Signal; //设备的信号强度，数值越大，信号越强
	unsigned int ConnectedTime; //设备链接的时间单位秒
}rt_mac_entry;

typedef struct _rt_mac_table
{
	int Num;
	rt_mac_entry entry[MAX_LEN_OF_MAC_TABLE];
}rt_mac_table;



#define MAX_CLIENT_INFO  8192
#define MAX_CLIENT       60  //最多记录设备个数

//user spaces buffer data;
char data[4096];


//从打开指定的文件流里找mac地址，找到返回0，找不到返回-1
static int find_macaddr(char *macaddr, FILE *stream)
{
	if( macaddr == NULL || stream == NULL)
		return -1;

	char buff[2048];
	char *tmp = macaddr;
	char *file_mac;

	while(fgets(buff, 2048, stream) > 0)
	{
		file_mac = web_get("mac", buff, 0);
		if(strcmp(file_mac, tmp) == 0)
			return 0;
	}

	return -1;
}


//获取设备列表的信息到指定的打开的文件流
static int getdevicelist(FILE *stream)
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
//	unsigned long expires;
//	unsigned d, h, m;
	//char tmpValue[256];

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

    /*
		int j;
		sprintf(&mac[0], "%02X", lease.mac[0]);
		for(i = 1, j= 2; i < 6; i++, j += 3)
			sprintf(&mac[j], ":%02X", lease.mac[i]);

		mac[j] = '\0';
    */
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
				lease.mac[0], lease.mac[1], lease.mac[2],
				lease.mac[3], lease.mac[4], lease.mac[5]);
		mac[17] = '\0';

		//这里还感觉逻辑有点理不通。优化的时候注意这里一下
		//如果mac地址在列表中，不再重复添加，跳过
		if(find_macaddr(mac, stream) == 0 && (*(&hostname) != ""))
			continue;

		addr.s_addr = lease.ip;
		//expires = ntohl(lease.expires);

		strcpy(ip, inet_ntoa(addr));


#if 0
		d = expires / (24*60*60); expires %= (24*60*60);
		h = expires / (60*60); expires %= (60*60);
		m = expires / 60; expires %= 60;
		if (d) printf("%u days ", d);
		printf("%02u:%02u:%02u\n", h, m, (unsigned)expires);
#endif
		FILE *pp;
		char cmd[128], msg_os[64];
		sprintf(cmd, "fingerprint.sh query %s", inet_ntoa(addr));
		pp = popen(cmd, "r");
		if(pp == NULL)
			pp = "";
		memset(msg_os, 0, 64);
		fread(msg_os, 64, 1, pp);
		pclose(pp);

	//	char my_os[64];
	//	strncpy(my_os, msg_os, strlen(msg_os));

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
int getmactable(rt_mac_table *mac_table)
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

#if 1
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
	{
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
	}

#endif 

rtuser_exit:
	if (socket_id >= 0)
		close(socket_id);
	if(ret)
		return ret;
	else
		return 0;
}

static int is_no_recored_cliinfo(int nvram)
{
	char buf[MAX_CLIENT_INFO];
	if(strcmp(nvram_bufget(nvram, "client_info_list"), "") == 0)
		return 0;
	else
		return -1;
}

//是否已记录在册0是， 1否
static int if_in_flash(char *mac, char *client_info_list)
{	
	int count = get_nums(client_info_list, ';');

	char flash_mac[18];
	int i;
	//从已存在的设备信息中的flash_mac与要添加的mac对比
	//不相等（说明没有添加该信息，则添加。否则，不添加）
	for(i = 0; i < count; i ++)
	{
		char client_info[128];
		get_nth_value(i, client_info_list, ';', client_info, sizeof(client_info_list));

		DBG_MSG("client_info is %s", client_info);

		get_nth_value(1, client_info, '&', flash_mac, sizeof(client_info));

		DBG_MSG("flash_mac is %s", flash_mac);
		DBG_MSG("mac is %s", mac);

		if(strcmp(mac, flash_mac) == 0)
			return 0;
	}

	return 1;
}

static void add_info_to_nvram(int nvram, char *hostname, char *mac)
{
	if(hostname == NULL || mac == NULL)
		return ;

	char buf[MAX_CLIENT_INFO];
	strcpy(buf, nvram_bufget(nvram, "client_info_list"));
	DBG_MSG("client_info_list is %s", buf);
	int count = get_nums(buf, ';');

	DBG_MSG("count of flash info: %d", count);

	char flash_mac[18];
	if(count < MAX_CLIENT)
	{
		//未记录
		if(if_in_flash(mac, buf) == 1)
		{
			sprintf(buf, "%s;%s&%s", buf, hostname, mac);
			nvram_bufset(nvram, "client_info_list", buf);
		}
	}
	//MAX_CLIENT 是存储信息最大设备个数，如果大于会删除第一个设备信息，然后在末尾添加新的信息。
	else if(count > MAX_CLIENT)
	{

		if(if_in_flash(mac, buf) == 1)
		{	
			//删除第一个设备信息
			int index = 0; 
			delete_nth_value(&index, count, buf, ';');
			sprintf(buf, "%s;%s&%s", buf, hostname, mac);
			nvram_bufset(nvram, "client_info_list", buf);

		}
	}
	nvram_commit(nvram);
}
//调用此函数，必须先执行getdevicelist
void add_cliinfo_to_flash(int nvram)
{
	rt_mac_table mac_table;
	getmactable(&mac_table);

	DBG_MSG("mac_table.Num is %d", mac_table.Num);

	if(mac_table.Num > 0)
	{
		FILE *fp;
		//打开自己保存的设备记录的文件
		fp = fopen(CLIENT_LIST, "r");
		if(fp == NULL)
			return ;

		int i; 
		for(i = 0; i < mac_table.Num; i++)
		{
			char buff[1024];
			//获取文件里的设备信息
			while(fgets(buff, 1024, fp))
			{

				char HostName[32];
				char Mac[18];

				strcpy(HostName,web_get("hostname", buff, 0));
				strcpy(Mac,web_get("mac", buff, 0));

				//跟之前记录的mac作比较,相同则显示
				if(strcmp(Mac, mac_table.entry[i].Mac) == 0)
				{
					//nvram里面没有记录设备信息
					if(is_no_recored_cliinfo(nvram) == 0)
					{
						char info[83];
						sprintf(info, "%s&%s", HostName, Mac);
						nvram_bufset(nvram, "client_info_list", info);

						DBG_MSG("");
						nvram_commit(nvram);
					}
					else
					{
						add_info_to_nvram(nvram, HostName, Mac);
					}
					rewind(fp);
					break;
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{

#if 1  //获取设备的信息存储到本地
	FILE *fp; 
	fp = fopen(CLIENT_LIST, "a+");
	if(fp == NULL)
		return -1;
	getdevicelist(fp);
	fclose(fp);
#endif 
	add_cliinfo_to_flash(RT2860_NVRAM);

	return 0;
}

