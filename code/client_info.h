#ifndef CLIENT_INFO_H_
#define CLIENT_INFO_H_

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

//get the RT2860_NVRAM
#include "nvram.h"

//#define RT_PRIV_IOCTL (SIOCIWFIRSTPRIV + 0x01)
//显示所有mac地址列表
//#define RTPRIV_IOCTL_GET_MAC_TABLE (SIOCIWFIRSTPRIV + 0x0F)


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define MAC_ADDR_LEN 6
#define ETH_LENGTH_OF_ADDRESS 6
#define MAX_LEN_OF_MAC_TABLE 64

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

#if 0
//black list 
typedef struct _black_list_entry
{
	char Mac[18];
}_BLACK_LIST_ENTRY;

typedef struct black_list_table
{
	int Num;
	_BLACK_LIST_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
}BLACK_LIST_TABLE;

#endif 

//记录连接过该路由的设备列表的信息
//#define CLIENT_LSIT "/etc_ro/lighttpd/APClient.list"
#define CLIENT_LSIT "/var/APClient.list"

//获取设备列表的信息保存到指定的打开的文件流
int getdevicelist(FILE *stream);


//在指定打开的文件流中，修改设备的主机名
int change_hostname(char *hostname, char *macaddr, FILE *stream);


//get the mac table info
int getmactable(rt_mac_table *mac_info);

//get the client list info
void getclientlist(void);

//add black to the acl
void addblacklist(char *mac);

//从黑名单中找到需要从黑名单删除的mac地址的索引
//为了能更好的使用delete_nth_value删除列表值
int find_index(char *mac, char *maclist);

//delete black list
void delblacklist(char *mac);


void showblacklist(void);

#endif 
