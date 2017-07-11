#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "nvram.h"

#define DHCPFILE  "udhcpd.conf"

int get_start_end_ip(char *ip, char *netmask, char *startip, char *endip);

int main(int argc, char *argv[])
{
#if 0
	char *ip = "192.168.221.101";
	char *mask = "255.255.255.0";
	struct in_addr myip;
	struct in_addr mymak;
	//struct in_add addr1;
	//int n = netmask_aton(ip);
	unsigned long l, l1, l2;
	l = inet_addr("0.0.0.1");
	l1 = inet_addr(ip);
	l2 = inet_addr(mask);

	l1 = ((l1 & l2) | l);
	memcpy(&myip, &l1, 4);

	printf("net = %s\n", inet_ntoa(myip));
#endif 
	char *ip;
	char *netmask;
	ip = get_ipaddr("br0");
	netmask = get_netmask("br0");
	printf("ip = %s\n mask = %s\n", ip, netmask);

	config_duchpd(ip, netmask);
	return 0;

}

int config_duchpd(char *ip, char *netmask)
{
	FILE *fp;
	fp = fopen(DHCPFILE, "w+");
	if(fp == NULL)
	{
		DBG_MSG("open file fialed.");
		return -1;
	}

	char databuf[1024] = {0};
	char startip[16] = {0};
	char endip[16] = {0};
	get_start_end_ip(ip, netmask, startip, endip);
	sprintf(databuf, "start %s\nend %s\ninterface br0\noption subnet %s\noption dns 168.95.1.1 8.8.8.8\noption router %s\noption lease 86400\nlease_file /var/udhcpd.leases",startip, endip, netmask, ip);
	printf("%s\n", databuf);
	fputs(databuf, fp);
	fclose(fp);
	return 0;
}

static int my_pow(int x, int y)
{
	int i;
	int res = 1;
	for(i = 0; i < y; i++)
	{
		res *= x;
	}

	return res;
}

int get_start_end_ip(char *ip , char *netmask, char *start, char *end)
{
	if(start == NULL || end == NULL)
		return -1;

	struct in_addr startip, endip;
	unsigned long mystart, myend, mask, ulong_ip;
	int n;
	int host;

	host = netmask_aton(netmask);
	n = my_pow(2, 32 - host);
	char end_host_ip[16];
	sprintf(end_host_ip,"0.0.0.%d", n - 2);

	mystart = inet_addr("0.0.0.1");
	myend = inet_addr(end_host_ip);

	ulong_ip = inet_addr(ip);
	mask = inet_addr(netmask);

	mystart = ((ulong_ip & mask) | mystart);
	memcpy(&startip, &mystart, 4);
	sprintf(start,"%s", inet_ntoa(startip));
	printf("start %s\n", start);
	myend = ((ulong_ip & mask) | myend);
	memcpy(&endip, &myend, 4);
	sprintf(end,"%s", inet_ntoa(endip));
	printf("end %s\n", end);
}
