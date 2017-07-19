#include "utils.h"
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <errno.h>
#include <linux/wireless.h>
#include "oid.h"

#if 0
#include "wps.h"
#include "user_conf.h"
#include "busybox_conf.h"
#include "openssl/pem.h"
#include "openssl/x509.h"
#include "openssl/evp.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "media.h"

//#include <asm/types.h>
#include <errno.h>
#include "stapriv.h"

int G_ConnectStatus = 0;
unsigned int m_nSigQua[3] = {0, 0, 0};
unsigned long m_lChannelQuality = 0;

#if 1
static int getStaLinkStatus()
{
	int s, ret;
	s = socket(AF_INET, SOCK_DGRAM, 0);

	ret = OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, "apcli0", &G_ConnectStatus, sizeof(G_ConnectStatus));
	if (ret < 0 ) {
		close(s);
		printf("Disconnected");
		return NdisMediaStateDisconnected;
	}

	return G_ConnectStatus;
}

unsigned int ConvertRssiToSignalQuality(long RSSI)
{
	unsigned int signal_quality;
	if (RSSI >= -50)
		signal_quality = 100;
	else if (RSSI >= -80)    // between -50 ~ -80dbm
		signal_quality = (unsigned int)(24 + (RSSI + 80) * 2.6);
	else if (RSSI >= -90)   // between -80 ~ -90dbm
		signal_quality = (unsigned int)((RSSI + 90) * 2.6);
	else    // < -84 dbm
		signal_quality = 0;

	return signal_quality;
}
#endif 

int get_signal(char *argv)
{
	RT_802_11_LINK_STATUS LinkStatus;
	int s;
	unsigned int nSigQua;
	long RSSI;
	int oid[3] = {RT_OID_802_11_RSSI, RT_OID_802_11_RSSI_1, RT_OID_802_11_RSSI_2};
	const char *G_bdBm_ischeck;
#if 1
	G_ConnectStatus = getStaLinkStatus();
	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		printf("0%%");
		//websWrite(wp, "0%%");
		return 0;
	}
#endif 

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Get Link Status Info from driver
	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, argv, &LinkStatus, sizeof(RT_802_11_LINK_STATUS));

	// Signal Strength

	// Get Rssi Value from driver
	OidQueryInformation(oid[0], s, argv, &RSSI, sizeof(RSSI));
	printf("rssi = %ld\n", RSSI);
	if (RSSI > 20 || RSSI < -200) {
		close(s);
		DBG_MSG("none");
		return -1;
	}

	// Use convert formula to getSignal Quality
#if 1
	nSigQua = ConvertRssiToSignalQuality(RSSI);
	if (m_nSigQua[0] != 0)
		nSigQua = (unsigned int)((m_nSigQua[0] + nSigQua) / 2.0 + 0.5);

#endif 
	close(s);
#if 1
	m_nSigQua[0] = nSigQua;
	G_bdBm_ischeck = nvram_bufget(RT2860_NVRAM, "G_bdBm_ischeck");
	if (nSigQua > 70) {
		if (strcmp(G_bdBm_ischeck, "1") == 0) { //checked
			//return printf("Good     %ld dBm", RSSI);
			return RSSI;
		}
		else {
			//return printf("Good     %ld dBm", nSigQua);
			return nSigQua;
		}
	}
	else if (nSigQua > 40) {
		if (strcmp(G_bdBm_ischeck, "1") == 0) { //checked
			//return printf("Normal     %ld dBm", RSSI);
			return RSSI;
		}
		else {
			//return printf("Normal     %d%%", nSigQua);
			return nSigQua;
		}
	}
	else {
		if (strcmp(G_bdBm_ischeck, "1") == 0) { //checked
			//printf("Weak     %ld dBm", RSSI);
			return RSSI;
		}
		else {
			//printf("Weak     %d%%", nSigQua);
			return nSigQua;
		}
	}
#endif 
}

int main(int argc, char *argv[])
{
	int sig;
	sig = get_signal(argv[1]);
	printf("sig = %d\n", sig);
	return 0;
}
