#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "nvram.h"

#define BUFF_COUNTER 2

//LinkStatus
#define NCONNECTED   0
#define CONNECTED    1

//ApCliEnable
#define APCLIDISABLE 0
#define APCLIENABLE  1

#define SLEEP_TIME   1

int main()
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;
	char *str;

	struct timeval tv;

	//0:need start dhcpc server; 1:dhcpc server have been started,need being kill;
	int dhcpc_status = 0;
	//0:need start dhcpd server; 1:dhcpd server have been started,need being kill;
	int dhcpd_status = 1;

	int ApCliEnable;
	int ApcliEnable_status = 1;

	while(1)
	{
		memset(long_buf, 0, BUFF_COUNTER);

        //get LinkStatus
		if(!(fp = popen("nvram_get 2860 LinkStatus", "r")))
		{
			printf("nvram get LinkStatus error.\n");
			return 1;
		}

		if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
		{
			printf("fread read none.\n");
			pclose(fp);
            return 1;
		}
		pclose(fp);

		if(atoi(long_buf) == CONNECTED)
		{
			//connneted
			if(dhcpc_status == 0)
			{
#if 1
				//获取系统的当前时间
				gettimeofday(&tv,NULL);
				//连接主路由时计时

				long int conn_time;
				sprintf(&conn_time,"%d", (long int)tv.tv_sec);
				nvram_bufset(RT2860_NVRAM, "LinkTime",(char *)(&conn_time));
				nvram_commit(RT2860_NVRAM);
#endif 


				system("killall udhcpc");
				usleep(25000);
				system("killall udhcpd");
				usleep(25000);
				system("udhcpc -i br0 -s /sbin/udhcpc.sh -p /var/run/udhcpc.p");
				dhcpc_status = 1;
				dhcpd_status = 0;
			}
		} else if(dhcpd_status == 0){
			//nconneted
#if 1
			nvram_bufset(RT2860_NVRAM, "LinkTime", "");
			nvram_commit(RT2860_NVRAM);
#endif 

			system("start_dhcpd.sh");
			system("udhcpd /etc/udhcpd.conf");
			dhcpd_status = 1;
			dhcpc_status = 0;
		}

		memset(long_buf, 0, BUFF_COUNTER);
        //get ApCliEnable
		if(!(fp = popen("nvram_get 2860 ApCliEnable", "r")))
		{
			printf("nvram get ApCliEnable error.\n");
			return 1;
		}

		if(0 == fread(long_buf, 1, BUFF_COUNTER, fp))
		{
			printf("ApCliEnable fread read none.\n");
			pclose(fp);
            return 1;
		}
		pclose(fp);
		ApCliEnable = atoi(long_buf);

		if(ApCliEnable == APCLIENABLE)
		{
			//ApcliEnable
			if(ApcliEnable_status == 1)
			{
				system("intercept.sh");
				ApcliEnable_status = 0;
			}
		}

		sleep(SLEEP_TIME);
	}

	return 0;
}
