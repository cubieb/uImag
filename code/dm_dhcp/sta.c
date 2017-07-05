#include "utils.h"
#include "client_info.h"
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


#define RT_QUERY_SIGNAL_CONTEXT		0x0402

//user spaces buffer data;
char data[4096];  //在get_mac_table中要用到，将内核的数据保存到用户空间中。

typedef struct _RT_SIGNAL_STRUC
{
	unsigned short Sequence;
	unsigned short MacAddr[6];
	unsigned short CurrAPAddr[6];
	unsigned char  Sig;
} RT_SIGNAL_STRUC;

int main(int argc, char *argv[])
{

	char name[25];
	int socket_id;
	struct iwreq wrq;
	int ret;
	// open socket based on address family: AF_NET ----------------------------
	socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_id < 0)
	{
		printf("\nrtuser::error::Open socket error!\n\n");
		return -1;
	}
	// set interface name as "ra0" --------------------------------------------
	sprintf(name, "ra0");

	//get AP's mac table, remove "get_mac_table" string -----------------------
	memset(data, 0, sizeof(RT_SIGNAL_STRUC));
	strcpy(wrq.ifr_name, name);
	wrq.u.data.length = sizeof(RT_SIGNAL_STRUC);
	wrq.u.data.pointer = data;
	wrq.u.data.flags = RT_QUERY_SIGNAL_CONTEXT;

	//RTPRIV_IOCTL_GET_MAC_TABLE 在oid.h中声明的。
	ret = ioctl(socket_id, RT_PRIV_IOCTL, &wrq);
	if(ret != 0)
	{
		printf("\nrtuser::error::get mac table\n\n");
		goto rtuser_exit;
	}

	RT_SIGNAL_STRUC *sp;
	sp = (RT_SIGNAL_STRUC *)wrq.u.data.pointer;
	printf("Sequence = 0x%04x\n", sp->Sequence);
	printf("MacAddr = %02x:%02x:%02x:%02x:%02x:%02x\n", 
			sp->MacAddr[0], sp->MacAddr[1],
			sp->MacAddr[2], sp->MacAddr[3],
			sp->MacAddr[4], sp->MacAddr[5]);

	printf("CurrAPAddr = %02x:%02x:%02x:%02x:%02x:%02x\n", 
			sp->CurrAPAddr[0], sp->CurrAPAddr[1],
			sp->CurrAPAddr[2], sp->CurrAPAddr[3],
			sp->CurrAPAddr[4], sp->CurrAPAddr[5]);
	printf("Signal = %d\n", sp->Sig);

rtuser_exit:
	if (socket_id >= 0)
		close(socket_id);
	if(ret)
		return ret;
	else
		return 0;
}

