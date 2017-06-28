#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/wireless.h>

#define SLEEP_TIME            1
#define BUFF_COUNTER       18
#define RTPRIV_IOCTL_GSITESURVEY					(SIOCIWFIRSTPRIV + 0x0D)

/* repeater ��ǰ״̬ */
#define NO_CONFIG                                          0
#define CONFIG_NOLINK                                   1
#define CONFIG_LINK_NOSIGNAL                      2
#define CONFIG_LINK_SIGNAL_NOSTA            3
#define CONFIG_LINK_SIGNAL_STA                  4

#define WPS_CONFIGURING                             5
/*****************************************
*** GPIO44 ���Ӻ�ɫLED
*** GPIO11 ������ɫLED
*******************************************/
//��ɫ��˸
#define RED_BLINKING                                \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");               \
	system("gpio l 44 10 10 4000 0 4000");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}			                                               \

//��ɫ������˸
#define GREEN_BLINKING_FAST                     \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");               \
	system("gpio l 11 1 1 4000 0 4000");             \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}			                                               \

//��ɫ����
#define GREEN_LIGHT                                    \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("gpio l 11 4000 0 0 0 0");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}			                                               \

//��ɫ����
#define RED_LIGHT                                         \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("gpio l 44 4000 0 0 0 0");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}	                                                             \

//��ɫ��˸
#define GREEN_BLINKING                             \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("gpio l 11 10 10 4000 0 4000");        \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}                                                                     \	

int link_status(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	/* 临时提高printk打印级别 */
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk");
	system("iwpriv apcli0 show connStatus");	
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 LinkStatus", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("link_status: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	//没有查找到字符串
	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

int isReapeterConfigued(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 ApCliEnable", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("isReapeterConfigued: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	//没有查找到字符串
	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

int isWpsConfiguring(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 WpsConfiguring", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("isWpsConfiguring: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	//没有查找到字符串
	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

/* ����repeater����·������mac ��ַ*/
char * router_mac_address(void* buff)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;
	char *str;

	if(1 == link_status())
	{
		/* 临时提高printk打印级别 */
		system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk");
		system("iwpriv apcli0 show connStatus");	
		system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");

		memset(long_buf, 0, BUFF_COUNTER);

		if(!(fp = popen("nvram_get 2860 RouterMac", "r")))
		{		
			return NULL;
		}

		if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
		{
			printf("router_mac_address: fread read none.\n");
			pclose(fp);
	             return NULL;
		}
		pclose(fp);

		long_buf[17] = '\0';

		strcpy(buff, long_buf);
		return buff;
	}
	else 
		return NULL;
}

/* repeater�Ƿ���������豸 */
int isHaveSta(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;
	char *str;

	/* 临时提高printk打印级别 */
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk");
	system("iwpriv ra0 show stainfo");	
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 isHaveSta", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("link_status: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	//没有查找到字符串
	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

/*******************************************************
*** ����repeater��ǰ������·�ɵ��ź�ǿ��
**********************************************************/
int router_signal(unsigned int *routerSignal)
{
        FILE *fp;
        char cmd[256], *ptr;
        char bssid[20], security[23], signal[9];
	 char long_buf[BUFF_COUNTER];

	/* ��������·�� */
	if(1 == link_status())
	{
		system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk");
		system("iwpriv ra0 set SiteSurvey=1");
		system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");

		sleep(2); // for get the SCAN result. (2G + 5G may enough for 5 seconds)

		if(!(fp = popen("iwpriv ra0 get_site_survey", "r"))) {
		        printf("execute get_site_survey fail!\n");
		        return 0;
		}

		memset(cmd, 0, sizeof(cmd));

		/* ***************************************************************************************
		**** ��ȡ
		**** ra0       get_site_survey:
		**** Ch  SSID                             BSSID               Security               Signal(%)W-Mode  ExtCH  NT WPS DPID 
		**** ������
		******************************************************************************************/
		fgets(cmd, sizeof(cmd), fp);
		fgets(cmd, sizeof(cmd), fp);

		router_mac_address(long_buf);

		printf("long_buf = %s \n", long_buf);

		while (fgets(cmd, sizeof(cmd), fp)) 
		{
			if (strlen(cmd) < 4)
				break;

			ptr = cmd + 37;
			sscanf(ptr, "%s %s %s", bssid, security, signal);

			if(0 == strcasecmp(bssid, long_buf))
			{
				*routerSignal = (unsigned int)atoi(signal);
				pclose(fp);
				return 1;
			}

		}
	}

	pclose(fp);

	 return 0;
}

int main()
{
	FILE *fp;
	char *str;
	int signal = 0;
	int curStatus= NO_CONFIG;
	int lastStatus = -1;

	/* 配置WLED_N管脚为普通GPIO管脚 */	
	system("reg set 0xb0000064");
	system("reg w 0 0x1");   
	//��rcSִ��

	while(1)
	{
		router_signal(&signal);

		if(1 == isReapeterConfigued())
		{		
			//没有查找到字符串
			if(1 == link_status())
			{
				/* �źŽϺ� */
				if(signal > 60)
				{
					/* repeater��������STA */
					if(1 == isHaveSta())
					{
						curStatus= CONFIG_LINK_SIGNAL_STA;
						//GREEN_BLINKING;
					}
					else
					{
						curStatus= CONFIG_LINK_SIGNAL_NOSTA;
						//GREEN_LIGHT;
					}
				}
				else
				{
					curStatus= CONFIG_LINK_NOSIGNAL;
					//RED_LIGHT;
				}
			
			}
			else
			{
				curStatus= CONFIG_NOLINK;
				//RED_LIGHT;
			}
		}
		else
		{
			//RED_BLINKING;
			curStatus= NO_CONFIG;
		}

		if(1 == isWpsConfiguring())
		{
			//curStatus= WPS_CONFIGURING;
			//ʲôʱ��״̬���
			GREEN_BLINKING_FAST;
			system("nvram_set 2860 WpsConfiguring 0");
			sleep(5);
			continue;
		}

		if(curStatus == lastStatus)
		{
			sleep(SLEEP_TIME);
			continue;
		}
		else
		{
			lastStatus = curStatus;	
		}

		switch(curStatus)
		{
			case NO_CONFIG:
				RED_BLINKING;
				break;
				
			case CONFIG_NOLINK:
			case CONFIG_LINK_NOSIGNAL:
				RED_LIGHT;
				break;

			case CONFIG_LINK_SIGNAL_NOSTA:
				GREEN_LIGHT;
				break;

			case CONFIG_LINK_SIGNAL_STA:
				GREEN_BLINKING;
				break;

			//case WPS_CONFIGURING:
				//GREEN_BLINKING_FAST;
				//break;

			default:
				RED_BLINKING;
		}

		sleep(SLEEP_TIME);
	}
	
	return 0;
}
