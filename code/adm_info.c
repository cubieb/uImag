#include "adm_info.h"
#include "utils.h"

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
int getdevicelist(FILE *stream)
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


//在指定的打开的文件流中，修改设备的主机名
// open file mode is r+


// is ok
int change_hostname(char *hostname, char *macaddr, FILE *stream)
{
	if(hostname == NULL || macaddr == NULL || stream == NULL)
		return -1;


	char buff[2048]; //获取单个设备的信息到buff中

	char *tmp = macaddr;

	char *file_mac = NULL;

	char *myip = NULL;
	char temp[18]; //record the ip data
	char *msg_os = NULL;

	while(fgets(buff, 2048, stream) > 0)
	{
		int length = strlen(buff);
		file_mac = web_get("mac", buff, 0);

		DBG_MSG("file_mac is %s", file_mac);

		//find the mac the same as stream mac
		if(strcmp(file_mac, tmp) == 0)
		{
			DBG_MSG(" %d %s", __LINE__, __FILE__);
			myip = web_get("ip", buff, 2);
			strcpy(temp, myip);

			msg_os = web_get("msg_os", buff, 0);

			//clear a line
			memset(buff, 0, length);
			//fputs(buff, stream);
			fwrite(buff, 1, length, stream);
			fseek(stream, -length, SEEK_CUR);

			//write a line
			snprintf(buff, length, "hostname=%s&mac=%s&ip=%s&msg_os=%s", hostname, macaddr, temp, msg_os);
			fseek(stream, -length, SEEK_CUR);
			fwrite(buff,1, length, stream);

			return 0;
		}
	}
	fclose(stream);

	return -1;
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


/*
 *
 * 直接获取连接路由的设备信息返回给web端
 * hostname mac signal connectime msg_os 
 *
 */
//经过测试 ok
void getclientlist(void)
{
	rt_mac_table mac_table;
	getmactable(&mac_table);

	DBG_MSG(" %d  %s", __LINE__, __FILE__);
	DBG_MSG(" mac_table.Num = %d", mac_table.Num);

	if(mac_table.Num > 0)
	{
		FILE *fp;
		//打开自己保存的设备记录的文件
		fp = fopen(CLIENT_LSIT, "r");
		if(fp == NULL)
			return ;

		int i; 
		DBG_MSG(" %d  %s", __LINE__, __FILE__);
		printf("{\n");
		printf("\t\"Client_Info\":\n");
		printf("\t[\n");

		for(i = 0; i < mac_table.Num; i++)
		{
			DBG_MSG(" %d  %s", __LINE__, __FILE__);
			char buff[1024];
			//获取设备信息
			while(fgets(buff, 1024, fp))
			{

				char HostName[32];
				char Mac[18];
				char Msg_os[64];

				strcpy(HostName,web_get("hostname", buff, 0));
				strcpy(Mac,web_get("mac", buff, 0));
				strcpy(Msg_os,web_get("msg_os", buff, 0));
				Msg_os[strlen(Msg_os) - 1] = '\0';

				//DBG_MSG("hostname= %s mac= %s  msg_os = %s", HostName, Mac, Msg_os);

				//跟之前记录的mac作比较,相同则显示
				if(strcmp(Mac, mac_table.entry[i].Mac) == 0)
				{

					printf("\t\t{\n");
					printf("\t\t\"HostName\":\"%s\",\n", HostName);
					printf("\t\t\"Signal\":\"%d\",\n", mac_table.entry[i].Signal);
					printf("\t\t\"ConnectedTime\":\"%d\",\n", mac_table.entry[i].ConnectedTime);
					printf("\t\t\"Mac\":\"%s\",\n", Mac);
					printf("\t\t\"Msg_os\":\"%s\"\n", Msg_os);
					printf("\t\t},\n");

					DBG_MSG("\t\t{\n");
					DBG_MSG("\t\t\"HostName\":\"%s\",\n", HostName);
					DBG_MSG("\t\t\"Signal\":\"%d\",\n", mac_table.entry[i].Signal);
					DBG_MSG("\t\t\"ConnectedTime\":\"%d\",\n", mac_table.entry[i].ConnectedTime);
					DBG_MSG("\t\t\"Mac\":\"%s\",\n", Mac);
					DBG_MSG("\t\t\"Msg_os\":\"%s\"\n", Msg_os);
					DBG_MSG("\t\t},\n");


					rewind(fp);
					break;
				}
			}
		}
		printf("\t\t{\n\t\t}\n");

		printf("\t]\n");

		printf("}\n");
	}
	else
	{
		DBG_MSG(" %d  %s  no one in line!", __LINE__, __FILE__);
		printf("{}");
	}
}

static void addAccessControlList(char *ACLList, char *mac)
{
	if(ACLList != NULL && mac != NULL)
	{
		sprintf(ACLList, "%s;%s",ACLList, mac);

		nvram_bufset(RT2860_NVRAM, "AccessControlList0", ACLList);
		nvram_commit(RT2860_NVRAM);
	}
}

void addblacklist(char *mac)
{
	if(mac == NULL)
		return ;

	nvram_bufset(RT2860_NVRAM, "AccessPolicy0", "2");
	//当黑名单为空时
	char ACL0[1024];
	strcpy(ACL0, nvram_bufget(RT2860_NVRAM, "AccessControlList0"));
	if(strcmp(ACL0, "") == 0)
	{
		nvram_bufset(RT2860_NVRAM, "AccessControlList0", mac);
		nvram_commit(RT2860_NVRAM);
	}
	else
	{
		addAccessControlList(ACL0, mac);
	}

	do_system("iwpriv ra0 set AccessPolicy=2");

	do_system("iwpriv ra0 set ACLAddEntry=%s", mac);

	//do_system("iwpriv ra0 set ACLShowAll=1");
	getclientlist();
}

int find_index(char *mac, char *maclist)
{
	int i, count;
	char substr[18];

	if(mac != NULL && maclist != NULL)
	{
		count = get_nums(maclist, ';');
		for(i = 0 ; i < count; i++)
		{
			//这里有点问题
			//get_nth_value(i, maclist, ';', substr, strlen(maclist) + 1 );
			get_nth_value(i, maclist, ';', substr, 2048 );
			DBG_MSG("in find_index function substr mac: %s %d %s", substr, __LINE__, __FILE__);
			DBG_MSG("need to del mac : %s %d %s", mac, __LINE__, __FILE__);
			if(strcmp(mac, substr) == 0)
				return i;
		}
	}
	return -1;
}



void delblacklist(char *mac)
{
	if(mac == NULL)
		return ;

	DBG_MSG("need to del  mac  in blacklist = %s", mac);
	do_system("iwpriv ra0 set ACLDelEntry=%s", mac);

	char ACLlist[2048];
	strcpy(ACLlist,nvram_bufget(RT2860_NVRAM, "AccessControlList0"));

	DBG_MSG("before delete blacklist ACL = %s", ACLlist);
	int index = find_index(mac, ACLlist);
	if(index < 0)
	{
		DBG_MSG("can not find the mac in the blacklist!");
		return ;
	}

	int count = get_nums(ACLlist, ';');
	delete_nth_value(&index, count, ACLlist, ';');
	DBG_MSG("after delete blacklist ACL = %s", ACLlist);

	nvram_bufset(RT2860_NVRAM, "AccessControlList0", ACLlist);
	nvram_commit(RT2860_NVRAM);
}



void showblacklist(void)
{
	char ACLlist[2048];
	strcpy(ACLlist,nvram_bufget(RT2860_NVRAM, "AccessControlList0"));


	int count = get_nums(ACLlist, ';');
	if(count == 0)
	{
		printf("{}");
	}
	else
	{

		char client_info[2048];
		FILE *fp;
		fp = fopen(CLIENT_LSIT, "r");
		if(fp == NULL)
			DBG_MSG("open file failed %d %s", __LINE__, __FILE__);

		int i;

		printf("{\n");
		printf("\t\"Black_List\":\n");
		//printf("\t\"Client_Info\":\n");
		printf("\t[\n");
		for(i = 0; i < count; i++)
		{
			char blackmac[18];
			//char filemac[18];
			char *filemac;
			get_nth_value(i, ACLlist, ';', blackmac, sizeof(ACLlist));
			blackmac[17] = '\0';

			while(fgets(client_info, sizeof(client_info), fp))
			{
				filemac = web_get("mac",client_info, 2);
				if(strcmp(blackmac, filemac) == 0)
				{
					char hostname[32];
					char msg_os[64];
					strcpy(hostname, web_get("hostname", client_info, 0));
					strcpy(msg_os, web_get("msg_os", client_info, 0));
					msg_os[strlen(msg_os) -1] = '\0';

					printf("\t\t{\n");
					printf("\t\t\"HostName\":\"%s\",\n", hostname);
					printf("\t\t\"Mac\":\"%s\",\n", blackmac);
					printf("\t\t\"Msg_os\":\"%s\"\n", msg_os);
					printf("\t\t},\n");



				}
			}
		}
		printf("\t\t{\n\t\t}\n");
		printf("\t]\n");
		printf("}\n");

		fclose(fp);

	}
}
