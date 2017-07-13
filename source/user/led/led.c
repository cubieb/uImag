#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/wireless.h>
#include <time.h>

#define SLEEP_TIME            2
#define BUFF_COUNTER       18
#define RTPRIV_IOCTL_GSITESURVEY					(SIOCIWFIRSTPRIV + 0x0D)

/* repeater current status */
#define NO_CONFIG                                          0
#define CONFIG_NOLINK                                   1
#define CONFIG_LINK_NOSIGNAL                      2
#define CONFIG_LINK_SIGNAL_NOSTA            3
#define CONFIG_LINK_SIGNAL_STA                  4

#define WPS_CONFIGURING                             5
#define LED_OFF                                 		    6

/*****************************************
*** GPIO11 connect RED LED
*** GPIO44 connect GREEN LED
*******************************************/
//red blink
#define RED_BLINKING                                \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");               \
	system("gpio l 11 10 10 4000 0 4000");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}			                                               \

//green blink rapidly
#define GREEN_BLINKING_FAST                     \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");               \
	system("gpio l 44 1 1 4000 0 4000");             \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}			                                               \

//green light
#define GREEN_LIGHT                                    \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("gpio l 44 4000 0 0 0 0");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}			                                               \

//red light
#define RED_LIGHT                                         \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("gpio l 11 4000 0 0 0 0");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}	                                                             \

//green blink
#define GREEN_BLINKING                             \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("gpio l 44 10 10 4000 0 4000");        \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}                                                                     \	

//LED off
#define RED_GREEN_OFF                                         \
{                                                                 \
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk"); \
	system("gpio l 11 0 4000 0 0 0");              \
	system("gpio l 44 0 4000 0 0 0");              \
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");   \
}

int link_status(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	/* 临时提高printk打印级别 */
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk");
	system("iwpriv apcli0 show connStatus");	
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 CMCC_LinkStatus", "r")))
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

int is_set_ledoff(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 CMCC_LedOff", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("is_set_ledoff: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

int is_repeater_configued(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 CMCC_ApCliEnable", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("is_repeater_configued: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	//没有查找到字符串
	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

int is_wps_configuring(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 CMCC_WpsConfiguring", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("is_wps_configuring: fread read none.\n");
		pclose(fp);
             return 0;
	}
	pclose(fp);

	//没有查找到字符串
	if(atoi(long_buf) == 1)
		return 1;	

	return 0;
}

/* return mac_address of router that repeater connected */
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

		if(!(fp = popen("nvram_get 2860 CMCC_RouterMac", "r")))
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


int get_limited_time(int iType, int* iTime)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;

	memset(long_buf, 0, BUFF_COUNTER);

	if(0 == iType)
	{
		if(!(fp = popen("nvram_get 2860 CMCC_LedBegin", "r")))
		{		
			return 0;
		}
	}
	else
	{
		if(!(fp = popen("nvram_get 2860 CMCC_LedEnd", "r")))
		{		
			return 0;
		}
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("get_limited_time: fread read none.\n");
		pclose(fp);
             return 0;
	}

	if(NULL == long_buf)
	{
		return 0;
	}

	*iTime = atoi(long_buf);
	
	pclose(fp);

	return 1;

}

/* whether STA connect repeater */
int is_have_sta(void)
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;
	char *str;

	/* 临时提高printk打印级别 */
	system("echo \"0 4 1 7\" >  /proc/sys/kernel/printk");
	system("iwpriv ra0 show stainfo");	
	system("echo \"7 4 1 7\" >  /proc/sys/kernel/printk");

	memset(long_buf, 0, BUFF_COUNTER);

	if(!(fp = popen("nvram_get 2860 CMCC_IsHaveSta", "r")))
	{		
		return 0;
	}

	if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
	{
		printf("is_have_sta: fread read none.\n");
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
*** return signal of router that repeater connected
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
		**** read lines below:
		**** ra0       get_site_survey:
		**** Ch  SSID                             BSSID               Security               Signal(%)W-Mode  ExtCH  NT WPS DPID 
		**** 
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

int current_time(void)
{
	struct tm *t;
      time_t tt;
      time(&tt);
      t = localtime(&tt);

	return t->tm_hour;
}

int main()
{
	FILE *fp;
	char *str;
	int signal = 0;
	int curStatus= NO_CONFIG;
	int lastStatus = -1;
	int beginTime = 0;
	int endTime = 0;
	int curTime = 0;

	/* 配置WLED_N管脚为普通GPIO管脚 */	
	system("reg set 0xb0000064");
	system("reg w 0 0x1");   
	//execute in rcS

	while(1)
	{
		/* update link status when repeater is configued */
		if(1 == is_repeater_configued())
		{		
			link_status();
		}
		
		/* press wps button */
		if(1 == is_wps_configuring())
		{
			curStatus= WPS_CONFIGURING;
			system("nvram_set 2860 CMCC_WpsConfiguring 0");
			goto state_machine;
		}
				
		/* set LED off */
		if(1 == is_set_ledoff())
		{
			curStatus= LED_OFF;
			goto state_machine;
		}
		/* set LED OFF in some times */
		else if((1 == get_limited_time(0, &beginTime)) && (1 == get_limited_time(1, &endTime)))
		{
			curTime = current_time();

			printf("beginTime = %d\n", beginTime);
			printf("endTime = %d\n", endTime);
			printf("curTime = %d\n", curTime);
			
			/* the time set not in the same day */
			if(beginTime > endTime)
			{
				if((curTime >= beginTime) || (curTime <= endTime))
				{
					curStatus= LED_OFF;
					goto state_machine;
				}
			}
			else
			{
				if((curTime >= beginTime) && (curTime <= endTime))
				{
					curStatus= LED_OFF;
					goto state_machine;
				}
			}
		}

		if(1 == is_repeater_configued())
		{		
			/* repeater is configued */
			if(1 == link_status())
			{
				router_signal(&signal);

				/* good signal */
				if(signal > 60)
				{
					/* some STA connect repeater */
					if(1 == is_have_sta())
					{
						curStatus= CONFIG_LINK_SIGNAL_STA;
					}
					else
					{
						curStatus= CONFIG_LINK_SIGNAL_NOSTA;
					}
				}
				else
				{
					curStatus= CONFIG_LINK_NOSIGNAL;
				}
			
			}
			else
			{
				curStatus= CONFIG_NOLINK;
			}
		}
		else
		{
			curStatus= NO_CONFIG;
		}

state_machine:
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

			case WPS_CONFIGURING:
				GREEN_BLINKING_FAST;
				sleep(4);
				break;

			case LED_OFF:
				RED_GREEN_OFF;
				break;

			default:
				RED_BLINKING;
		}

		sleep(SLEEP_TIME);
	}
	
	return 0;
}
