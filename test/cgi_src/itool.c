#include "itool.h"

/**************可能會調用到的函數**********************/
/*
 * 在開發過程用可能會公用到的函數
 * 放到統一的模塊中
 */

int G_ConnectStatus = 0;
unsigned int m_nSigQua[3] = {0, 0, 0};
unsigned long m_lChannelQuality = 0;

//連接主路由的時長
unsigned int get_conntime(int nvram)
{
	FILE *fp;
	char starttime[32];
	char endtime[32];
	unsigned int myconntime;
	fp = fopen("/proc/uptime", "r");
	if(fp == NULL)
		return -1;

	char sec[64];
	if(fgets(sec, sizeof(sec), fp) != NULL)
	{
		get_nth_value(0, sec, ' ', endtime, sizeof(endtime));
	}
	strcpy(starttime, nvram_bufget(nvram, "CMCC_ConnectTime"));
	myconntime = atoi(endtime) - atoi(starttime);
	fclose(fp);

	return myconntime;
}

//distconnect return 0  connect return 1
int getStaLinkStatus(char *ifname)
{
	int s, ret;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s < 0)
	{
		DBG_MSG("socket error!");
		return -1;
	}

	ret = OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, ifname, &G_ConnectStatus, sizeof(G_ConnectStatus));
	if (ret < 0 ) {
		close(s);
		//printf("Disconnected");
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

int get_signal(char *ifname)
{
	RT_802_11_LINK_STATUS LinkStatus;
	int s;
	unsigned int nSigQua;
	long RSSI;
	int oid[3] = {RT_OID_802_11_RSSI, RT_OID_802_11_RSSI_1, RT_OID_802_11_RSSI_2};
	const char *G_bdBm_ischeck;

	G_ConnectStatus = getStaLinkStatus(ifname);
	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		printf("0%%");
		return 0;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Get Link Status Info from driver
	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, ifname, &LinkStatus, sizeof(RT_802_11_LINK_STATUS));

	// Signal Strength

	// Get Rssi Value from driver
	OidQueryInformation(oid[0], s, ifname, &RSSI, sizeof(RSSI));

	if (RSSI > 20 || RSSI < -200) {
		close(s);
		DBG_MSG("none");
		return -1;
	}

	// Use convert formula to getSignal Quality
	nSigQua = ConvertRssiToSignalQuality(RSSI);
	if (m_nSigQua[0] != 0)
		nSigQua = (unsigned int)((m_nSigQua[0] + nSigQua) / 2.0 + 0.5);

	close(s);

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
}
