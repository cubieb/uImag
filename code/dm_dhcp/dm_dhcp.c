#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <time.h>
#include "nvram.h"

//#define BUFF_COUNTER 2

//LinkStatus
#define NCONNECTED   0
#define CONNECTED    1

//ApCliEnable
#define APCLIDISABLE 0
#define APCLIENABLE  1

#define SLEEP_TIME   1


int getuptime(int nvram);
void clear_time(int nvram);

int main()
{

//	char long_buf[BUFF_COUNTER];
//	FILE *fp;
//	char *str;

	//0:need start dhcpc server; 1:dhcpc server have been started,need being kill;
	int dhcpc_status = 0;
	//0:need start dhcpd server; 1:dhcpd server have been started,need being kill;
	int dhcpd_status = 1;

	int ApCliEnable;
	int ApcliEnable_status = 1;

	while(1)
	{
#if 0 
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
#endif 
		//if(atoi(long_buf) == CONNECTED)
		if(atoi(nvram_bufget(RT2860_NVRAM, "ApCliEnable")) == CONNECTED 
				&& atoi(nvram_bufget(RT2860_NVRAM, "LinkStatus")) == CONNECTED)
		{
			//connneted
			if(dhcpc_status == 0)
			{
				getuptime(RT2860_NVRAM);
				system("killall udhcpc");
				usleep(25000);
				//system("killall udhcpd");
				//usleep(25000);
				system("udhcpc -i br0 -s /sbin/udhcpc.sh -p /var/run/udhcpc.p");
				dhcpc_status = 1;
				dhcpd_status = 0;
			}
		} else if(dhcpd_status == 0){
			//nconneted
			clear_time(RT2860_NVRAM);
			system("killall udhcpd");
			usleep(25000);
			system("start_dhcpd.sh");
			system("udhcpd /etc/udhcpd.conf");
			dhcpd_status = 1;
			dhcpc_status = 0;
		}

#if 0
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
#endif 
		ApCliEnable = atoi(nvram_bufget(RT2860_NVRAM, "ApCliEnable"));
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

static int get_nth_value(int index, char *value, char delimit, char *result, int len)
{
	int i=0, result_len=0;
	char *begin, *end;

	if(!value || !result || !len)
		return -1;

	begin = value;
	end = strchr(begin, delimit);

	while(i<index && end){
		begin = end+1;
		end = strchr(begin, delimit);
		i++;
	}

	//no delimit
	if(!end){
		if(i == index){
			end = begin + strlen(begin);
			result_len = (len-1) < (end-begin) ? (len-1) : (end-begin);
		}else
			return -1;
	} else {
		result_len = (len-1) < (end-begin)? (len-1) : (end-begin);
	}

	memcpy(result, begin, result_len );
	*(result+ result_len ) = '\0';

	return 0;
}

int getuptime(int nvram)
{
	FILE *fp;
	fp = fopen("/proc/uptime", "r");
	if(fp == NULL)
		return -1;

	char sec[32];
	if(fgets(sec, sizeof(sec), fp) != NULL)
	{
		char begin_time[16];
		get_nth_value(0, sec, ' ', begin_time, sizeof(begin_time));
		nvram_bufset(nvram, "connTime", begin_time);
		nvram_commit(nvram);
		fclose(fp);
		return 0;
	}

}

void clear_time(int nvram)
{
	nvram_bufset(nvram, "connTime", "");
	nvram_commit(nvram);
}
