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

//记录连接过该路由的设备列表的信息
//#define CLIENT_LSIT "/etc_ro/lighttpd/APClient.list"
#define CLIENT_LIST "/var/APClient.list"
#define CLIENT_LIST_NUM 	64  //最多能记录连接扩展路由的设备个数

#define MAX_CLIENT_INFO_LIST  8192 //记录设备信息的最大值 (65 + 18) * 64 < 8192 
#define MAX_ACL_LIST  2048 //记录ACL最大值  18 * 64 < 2048

/*抽出来的小功能模块*/
int find_mac(char *macaddr, FILE *stream);
//get the mac table info
int get_mac_table(rt_mac_table *mac_table);
//获取所有设备(包括在线和不在线)的信息保存到打开的文件流
int get_device_list(FILE *stream);


//从没有记录过设备信息
int no_clientinfo_in_nvram(int nvram);
//是否在记录信息的列表里
int ifmac_in_nvram(int count, char *mac, char *client_info_list);
//add_clientinfo_to_nvram中抽象功能比较单一的函数。
void add_info_to_nvram(int nvram, char *hostname, char *mac);
//找出mac地址在第几个设备的记录
int find_index_mac_in_nvram(int nvram,int count, char *client_info_list, char *mac);

int get_os_host_from_file(char *mac, char *hostname,  char *msg_os);
int get_hostname_in_nvram(int nvram, char *mac, char *hostname);
//添加曾经在线的设备信息到nvram
void add_clientinfo_to_nvram(int nvram);


//讲mac追加到ACLlist里，然后写到nvram.
int add_AccessControlList(int nvram, char *ACLList, char *mac);
//从黑名单中找到需要从黑名单删除的mac地址的索引
//为了能更好的使用delete_nth_value删除列表值
int find_index(int count, char *mac, char *maclist);
/*
   主要的功能函数
   */
/*谁在上网*/
void get_client_list(int nvram);
/*修改主机名，信息保存到nvram的client_info_list变量里　
  格式: client_info_list=hostname1&macaddr1;hostname2&macaddr2 */
int change_hostname(int nvram, char *hostname, char *macaddr);
/*添加黑名单*/
int  add_blacklist(int nvram, char *mac);
/*显示黑名单*/
void show_blacklist(int nvram);
/*删除黑名单*/
int  del_blacklist(int nvram, char *mac);

#endif 
