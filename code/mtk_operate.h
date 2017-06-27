#ifndef MTK_OPERATE_H_
#define MTK_OPERATE_H_ 


#include <assert.h>
#include "utils.h"


//从客户端能接受到的message 大小
#define MAX_MSG_SIZ  4096
//ifname 文件名或接口名的大小
#define IFNAMSIZ 32

/*
//用来接受客户端返回某个主路由ssid所对应的所有的信息
typedef struct ssid_message
{
	char Channel[4];
	char ssid[32];
	char bssid[20];
	char security[23];
	char signal[9];
	char mode[9];
	char ext_ch[7];
	char net_type[3];
	char wps[4];
}SSID_MESSAGE;
*/


// 主路由的信息
typedef struct ap_message_s
{
	char APChannel[4];
	char APAuthMode[24];
	char APEncrypType[10];
	char APPasswd[32];
	char APSsid[32];

}ap_message_t;


//扩展路由的信息和管理员密码
typedef struct extend_message_s
{
	char Extend_wifiName[32];
	char Extend_wifiPasswd[32];
	char ManagePasswd[32];
}extend_message_t;

/*
 * 扫描wifiF_repeater周围的wifi热点
 */
void apcli_scan(void);


/*
 * 从网页客户端上获取信息
 * 信息填充到input buffer上。
 */
int get_message_from_web(char *input);


/*
 * 可以有选择的接收数据：主路由的信息 or 扩展路由的设置，两者都接收。
 * ap_msg为NULL， extend_msg不允为NULL，或两者都不为NULL。
 * 从网页客户端上获取的数据，存放到input缓存中。
 */
void get_value_from_web(ap_message_t *ap_msg, extend_message_t *extend_msg, char *input);


//把数据写到nvram
int set_nvram_buf(int nvram, ap_message_t *ap_msg, extend_message_t *ex_msg);


//判断是否连接成功主路由
int is_connect_success(int nvram, ap_message_t *ap_msg);

#endif 
