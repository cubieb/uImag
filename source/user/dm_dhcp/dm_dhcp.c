#include "nvram.h"
#include "dm_dhcp.h"

#define  MYIFNAMESIZ 4

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
		if(atoi(nvram_bufget(RT2860_NVRAM, "CMCC_ApCliEnable")) == CONNECTED 
				&& atoi(nvram_bufget(RT2860_NVRAM, "CMCC_LinkStatus")) == CONNECTED)
		{
			//connneted
			if(dhcpc_status == 0)
			{
				system("ifconfig ra0 down");
				getuptime(RT2860_NVRAM);
				system("killall udhcpc");
				usleep(500000);
				system("killall udhcpd");
				usleep(500000);
				system("udhcpc -i br0 -s /sbin/udhcpc.sh -p /var/run/udhcpc.p");
				usleep(500000);
				system("ifconfig ra0 up");
				usleep(300000);
				config_duchpd("br0");
				system("udhcpd /etc/repeater_udhcpd.conf");
				dhcpc_status = 1;
				dhcpd_status = 0;
			}
		} else if(dhcpd_status == 0){
			//nconneted
			clear_time(RT2860_NVRAM);
			system("killall udhcpd");
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
		ApCliEnable = atoi(nvram_bufget(RT2860_NVRAM, "CMCC_ApCliEnable"));
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
		nvram_bufset(nvram, "CMCC_ConnectTime", begin_time);
		nvram_commit(nvram);
	}
	fclose(fp);
	return 0;
}

void clear_time(int nvram)
{
	nvram_bufset(nvram, "CMCC_ConnectTime", "");
	nvram_commit(nvram);
}


char *get_ipaddr(char *ifname)
{
	struct ifreq ifr;
	int skfd = 0;
	static char if_addr[16];

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}

	strncpy(ifr.ifr_name, ifname, MYIFNAMESIZ);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl SIOCGIFADDR error for %s\n", __func__, ifname);
		return "";
	}
	strcpy(if_addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);

	return if_addr;
}


char *get_netmask(char *ifname)
{
	struct ifreq ifr;
	int skfd = 0;
	static char addr[16];

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "%s: open socket error\n", __func__);
		return "";
	}

	strncpy(ifr.ifr_name, ifname, MYIFNAMESIZ);
	if (ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0) {
		close(skfd);
		fprintf(stderr, "%s: ioctl SIOCGIFNETMASK error for %s\n", __func__, ifname);
		return "";
	}
	strcpy(addr, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	close(skfd);

	return addr;
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

static int netmask_aton(const char *ip)
{
	int i, a[4], result = 0;
	sscanf(ip, "%d.%d.%d.%d", &a[0], &a[1], &a[2], &a[3]);
	for(i=0; i<4; i++){ //this is dirty
		if(a[i] == 255){
			result += 8;
			continue;
		}
		if(a[i] == 254)
			result += 7;
		if(a[i] == 252)
			result += 6;
		if(a[i] == 248)
			result += 5;
		if(a[i] == 240)
			result += 4;
		if(a[i] == 224)
			result += 3;
		if(a[i] == 192)
			result += 2;
		if(a[i] == 128)
			result += 1;
		//if(a[i] == 0)
		//  result += 0;
		break;
	}
	return result;
}

//主机ip 的分配范围
////ip = 网络号+主机号
////网络号 = ip & netmask
int get_start_end_ip(char *ip , char *netmask, char *start, char *end)
{
	if(start == NULL || end == NULL)
		return -1;

	struct in_addr startip, endip;
	unsigned long mystart, myend, mask, ulong_ip;
#if 0
	int n; //主机号个数
	int host;

	host = netmask_aton(netmask);
	n = my_pow(2, 32 - host);
	char end_host_ip[16];
	sprintf(end_host_ip,"0.0.0.%d", n - 2);

	mystart = inet_addr("0.0.0.1");
	myend = inet_addr(end_host_ip);

#endif 
	mystart = inet_addr("0.0.0.100");
	myend = inet_addr("0.0.0.200");

	ulong_ip = inet_addr(ip);
	mask = inet_addr(netmask);

	mystart = ((ulong_ip & mask) | mystart);
	memcpy(&startip, &mystart, 4);
	sprintf(start,"%s", inet_ntoa(startip));
	myend = ((ulong_ip & mask) | myend);
	memcpy(&endip, &myend, 4);
	sprintf(end,"%s", inet_ntoa(endip));
}


void get_dns(int index, char *mydns)
{
	if(mydns == NULL)
		return ;
	if(index > 2)
		index = 0;
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

	strcpy(mydns, dns);
}

void get_gata_way(char *gateway)
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
		strcpy(gateway, sgw);
	}
}

int config_duchpd(char *ifname)
{   
	char databuf[1024] = {0};
	char startip[16] = {0};
	char endip[16] = {0};

	char gateway[16] = {0};
	char mydns[16] = {0};

	char *ip, *netmask;
	ip = get_ipaddr(ifname);
	netmask = get_netmask(ifname);
	get_start_end_ip(ip, netmask, startip, endip);

	get_gata_way(gateway);
	get_dns(0, mydns);

	FILE *fp;
	fp = fopen(DHCPFILE, "w+");
	if(fp == NULL)
	{   
		return -1;
	}
	sprintf(databuf, "start %s\nend %s\ninterface br0\noption subnet %s\noption dns %s 8.8.8.8\noption router %s\noption lease 86400\nlease_file /var/udhcpd.leases\n",startip, endip, netmask, mydns, gateway);  
	fputs(databuf, fp);
	fclose(fp);
	return 0;
}
