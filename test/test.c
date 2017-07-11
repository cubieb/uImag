#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include "utils.h"
#include "nvram.h"

void GetGateway(char *gateway);
void GetDns(int index, char *mydns);

int main(int argc, char *argv[])
{
	char gateway[16], dns[16];
	GetGateway(gateway);
	GetDns(0, dns);
	printf("gateway = %s\n dns = %s\n", gateway, dns);
	return 0;
}
void GetDns(int index, char *mydns)
{
	FILE *fp;
	char buf[80] = {0}, ns_str[11], dns[16] = {0};
	int type, i = 0;

	fp = fopen("/etc/resolv.conf", "r");
	if (NULL == fp)
		return;
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (strncmp(buf, "nameserver", 10) != 0)
			continue;
		sscanf(buf, "%11s%16s", ns_str, dns);
		i++;
		if (i == index)
			break;
	}
	fclose(fp);

	//printf("%s", dns);
	strcpy(mydns, dns);
}


void GetGateway(char *gateway)
{
	if(gateway == NULL)
		return ;
	char   buff[256];
	int    nl = 0 ;
	struct in_addr dest;
	struct in_addr gw;
	int    flgs, ref, use, metric;
	unsigned long int d,g,m;
	int    find_default_flag = 0;
	char sgw[16];
	FILE *fp = fopen("/proc/net/route", "r");
	if (fp == NULL) fp = "";
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (nl) {
			int ifl = 0;
			while (buff[ifl]!=' ' && buff[ifl]!='\t' && buff[ifl]!='\0')
				ifl++;
			buff[ifl]=0;    /* interface */
			if (sscanf(buff+ifl+1, "%lx%lx%X%d%d%d%lx",
						&d, &g, &flgs, &ref, &use, &metric, &m)!=7) {
				fclose(fp);
				DBG_MSG("format error");
				return;
			}

			if (flgs&RTF_UP) {
				dest.s_addr = d;
				gw.s_addr   = g;
				strcpy(sgw, (gw.s_addr==0 ? "" : inet_ntoa(gw)));

				if (dest.s_addr == 0) {
					find_default_flag = 1;
					break;
				}
			}
		}
		nl++;
	}
	fclose(fp);

	if (find_default_flag == 1)
	{
		//printf("%s", sgw);
		strcpy(gateway, sgw);
	}
}

