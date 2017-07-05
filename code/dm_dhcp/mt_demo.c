#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h> /* for close */
#include <linux/wireless.h>

#define RT_PRIV_IOCTL (SIOCIWFIRSTPRIV + 0x01)
#define RTPRIV_IOCTL_SET (SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_BBP (SIOCIWFIRSTPRIV + 0x03)
#define RTPRIV_IOCTL_MAC (SIOCIWFIRSTPRIV + 0x05)
#define RTPRIV_IOCTL_E2P (SIOCIWFIRSTPRIV + 0x07)
#define RTPRIV_IOCTL_STATISTICS (SIOCIWFIRSTPRIV + 0x09)
#define RTPRIV_IOCTL_ADD_PMKID_CACHE (SIOCIWFIRSTPRIV + 0x0A)
#define RTPRIV_IOCTL_RADIUS_DATA (SIOCIWFIRSTPRIV + 0x0C)
#define RTPRIV_IOCTL_GSITESURVEY (SIOCIWFIRSTPRIV + 0x0D)
#define RTPRIV_IOCTL_ADD_WPA_KEY (SIOCIWFIRSTPRIV + 0x0E)
#define RTPRIV_IOCTL_GET_MAC_TABLE (SIOCIWFIRSTPRIV + 0x0F)
#define OID_GET_SET_TOGGLE 0x8000
#define RT_QUERY_ATE_TXDONE_COUNT 0x0401
#define RT_QUERY_SIGNAL_CONTEXT 0x0402
#define RT_SET_APD_PID (OID_GET_SET_TOGGLE + 0x0405)
#define RT_SET_DEL_MAC_ENTRY (OID_GET_SET_TOGGLE + 0x0406)


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define MAC_ADDR_LEN 6
#define ETH_LENGTH_OF_ADDRESS 6
#define MAX_LEN_OF_MAC_TABLE 64


typedef struct _RT_SIGNAL_STRUC {
unsigned short Sequence;
unsigned char MacAddr[MAC_ADDR_LEN];
unsigned char CurrAPAddr[MAC_ADDR_LEN];
unsigned char Sig;
} RT_SIGNAL_STRUC, *PRT_SIGNAL_STRUC;

char data[4096];



int main( int argc, char ** argv )
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
	//sprintf(name, "apcli0");
	memset(data, 0x00, 255);


	//
	//example of iwconfig ioctl function ==========================================
	//
	// get wireless name ------------------------------------------------------
	strcpy(wrq.ifr_name, name);
	wrq.u.data.length = 255;
	wrq.u.data.pointer = data;
	wrq.u.data.flags = 0;
	ret = ioctl(socket_id, SIOCGIWNAME, &wrq);
	if(ret != 0)
	{
		printf("\nrtuser::error::get wireless name\n\n");
		goto rtuser_exit;
	}
	printf("\nrtuser[%s]:%s\n", name, wrq.u.name);
	//get SIGNAL_CONTEXT ------------------------------------------------------
	printf("\nrtuser::get SIGNAL_CONTEXT\n\n");
	{
		RT_SIGNAL_STRUC *sp;
		//memset(data, 0, sizeof(RT_SIGNAL_STRUC));
		memset(data, 0, 2048);
		strcpy(wrq.ifr_name, name);
		//wrq.u.data.length = sizeof(RT_SIGNAL_STRUC);
		wrq.u.data.length = 2048;
		wrq.u.data.pointer = data;
		wrq.u.data.flags = RT_QUERY_SIGNAL_CONTEXT;
		ret = ioctl(socket_id, RT_PRIV_IOCTL, &wrq);
		if(ret != 0)
		{
			printf("\nrtuser::error::get SIGNAL_CONTEXT\n\n");
			goto rtuser_exit;
		}
		printf("pointer = %s", wrq.u.data.pointer);
		sp = (RT_SIGNAL_STRUC *)wrq.u.data.pointer;
		printf("sp = %s", sp);
		printf("\n===== SIGNAL_CONTEXT =====\n\n");
		printf("Sequence = 0x%04x\n", sp->Sequence);
		printf("Mac.Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
				sp->MacAddr[0], sp->MacAddr[1],
				sp->MacAddr[2], sp->MacAddr[3],
				sp->MacAddr[4], sp->MacAddr[5]);
		printf("CurrAP.Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
				sp->CurrAPAddr[0], sp->CurrAPAddr[1],
				sp->CurrAPAddr[2], sp->CurrAPAddr[3],
				sp->CurrAPAddr[4], sp->CurrAPAddr[5]);
		printf("Sig = %d\n\n", sp->Sig);
	}

rtuser_exit:
	if (socket_id >= 0)
		close(socket_id);
	if(ret)
		return ret;
	else
		return 0;
}
