#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#define BUFF_COUNTER 2

//LinkStatus
#define NCONNECTED   0
#define CONNECTED    1

#define SLEEP_TIME   1

int main()
{

	char long_buf[BUFF_COUNTER];
	FILE *fp;
	char *str;

	int dhcp_status = 0;

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
			if(dhcp_status == 0)
			{
				system("killall udhcpc");
				usleep(50000);
				system("udhcpc -i br0 -s /sbin/udhcpc.sh -p /var/run/udhcpc.p");
				dhcp_status = 1;
			}
		} else {
			//nconneted
			dhcp_status = 0;
		}

		sleep(SLEEP_TIME);
	}

	return 0;
}
