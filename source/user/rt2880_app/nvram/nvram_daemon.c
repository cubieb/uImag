#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/autoconf.h>
#include "config/autoconf.h"
#if defined CONFIG_USER_LIGHTY
#include <sys/time.h>
#include "oid.h"
#include "utils.h"
#include <arpa/inet.h>
#include <linux/wireless.h>
#endif
#include "nvram.h"

#if defined (CONFIG_RALINK_GPIO) || defined (CONFIG_RALINK_GPIO_MODULE)
#include "ralink_gpio.h"
#define GPIO_DEV "/dev/gpio"
#endif


#ifdef CONFIG_RT2880_L2_MANAGE
int ramad_start(void);
#endif
static char *saved_pidfile;

void loadDefault(int chip_id)
{
    switch(chip_id)
    {
    case 2860:
	system("ralink_init clear 2860");
#if defined CONFIG_LAN_WAN_SUPPORT || defined CONFIG_MAC_TO_MAC_MODE 
	system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_vlan");
#elif defined(CONFIG_ICPLUS_PHY)
	system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_oneport");
#else
	system("ralink_init renew 2860 /etc_ro/Wireless/RT2860AP/RT2860_default_novlan");
#endif
#if defined CONFIG_MTK_VOIP
	system("ralink_init clear voip");
	system("ralink_init renew voip /etc_ro/voip_default_settings");
#endif
	break;
    case 2880:
	system("ralink_init clear rtdev");
#if defined (CONFIG_RTDEV_PLC)
	system("ralink_init renew rtdev /etc_ro/PLC/plc_default.dat");
#else
	system("ralink_init renew rtdev /etc_ro/Wireless/iNIC/RT2860AP.dat");
#endif
	break;
    case 2561: 
	system("ralink_init clear rtdev");
	system("ralink_init renew rtdev /etc_ro/Wireless/RT61AP/RT2561_default");
	break;
    case 7790:
	system("ralink_init clear wifi3");
	system("ralink_init renew wifi3 /etc_ro/Wireless/WIFI3/RT2870AP.dat");
	break;
    default:
	printf("%s:Wrong chip id\n",__FUNCTION__);
	break;
    }
}

#if defined CONFIG_USER_LIGHTY
#if defined WSC_SUPPORT
#if defined (CONFIG_RALINK_GPIO) || defined (CONFIG_RALINK_GPIO_MODULE)
#if defined (CONFIG_RALINK_RT2880)
#define WPS_AP_PBC_LED_GPIO     13   // 0 ~ 24( or disable this feature by undefine it)

#elif defined (CONFIG_RALINK_RT3883)
#define WPS_AP_PBC_LED_GPIO     0   // 0 ~ 24( or disable this feature by undefine it)

#elif defined (CONFIG_RALINK_RT6855A)
#if defined (CONFIG_RT6855A_PCIE_PORT0_ENABLE)
#define WPS_AP_PBC_LED_GPIO     32      // rt6855 WPS LED
#else
#define WPS_AP_PBC_LED_GPIO     21      // rt6856 WPS LED
#endif

#elif defined (CONFIG_RALINK_MT7620)
#define WPS_AP_PBC_LED_GPIO     39      // MT7620 WPS LED

#elif defined (CONFIG_RALINK_MT7628)
#define WPS_AP_PBC_LED_GPIO     37       // MT7628 WPS LED

#else
#define WPS_AP_PBC_LED_GPIO     14   // 0 ~ 24( or disable this feature by undefine it)
#endif
#define LED_ON                          1
#define LED_OFF                         0
#define WPS_LED_RESET                   1
#define WPS_LED_PROGRESS                2
#define WPS_LED_ERROR                   3
#define WPS_LED_SESSION_OVERLAP         4
#define WPS_LED_SUCCESS                 5
#define LedReset()                  {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_RESET);}
#define LedInProgress()             {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_PROGRESS);}
#define LedError()                  {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_ERROR);}
#define LedSessionOverlapDetected() {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_SESSION_OVERLAP);}
#define LedSuccess()                {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_SUCCESS);}
#else
#define LedReset()
#define LedInProgress()
#define LedError()
#define LedSessionOverlapDetected()
#define LedSuccess()
#endif

#if defined(CONFIG_DBDC_MODE)
#define WPS_AP_CATCH_CONFIGURED_TIMER	500
#else
#define WPS_AP_CATCH_CONFIGURED_TIMER	100 
#endif
#define WPS_AP_TIMEOUT_SECS 		120000
static int wsc_timeout_counter;
static int g_wps_timer_state;
static int triggered_signal;
static int wsc_timer_number;
static int wsc_bssid_num;

int setTimer(int microsec, void ((*sigroutine)(int)))
{
	struct itimerval value, ovalue;

	signal(SIGALRM, sigroutine);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = microsec;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = microsec;
	return setitimer(ITIMER_REAL, &value, &ovalue);
}

void stopTimer(void)
{
	struct itimerval value, ovalue;

	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &value, &ovalue);
}

static void resetTimerAll(void)
{
	stopTimer();
	g_wps_timer_state = 0;
}

#if defined (CONFIG_RALINK_GPIO) || defined (CONFIG_RALINK_GPIO_MODULE)
static int gpioLedSet(int gpio, unsigned int on, unsigned int off,
		unsigned int blinks, unsigned int rests, unsigned int times)
{
	int fd;
	ralink_gpio_led_info led;

	if (gpio < 0 || gpio >= RALINK_GPIO_NUMBER ||
			on > RALINK_GPIO_LED_INFINITY ||
			off > RALINK_GPIO_LED_INFINITY ||
			blinks > RALINK_GPIO_LED_INFINITY ||
			rests > RALINK_GPIO_LED_INFINITY ||
			times > RALINK_GPIO_LED_INFINITY) {
		return -1;
	}
	led.gpio = gpio;
	led.on = on;
	led.off = off;
	led.blinks = blinks;
	led.rests = rests;
	led.times = times;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	if (ioctl(fd, RALINK_GPIO_LED_SET, &led) < 0) {
		perror("ioctl");
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int ledWps(int gpio, int mode)
{
	switch (mode) {
	case WPS_LED_RESET:
		return gpioLedSet(gpio, 0, RALINK_GPIO_LED_INFINITY, 1, 1, RALINK_GPIO_LED_INFINITY);
		break;
	case WPS_LED_PROGRESS:
		return gpioLedSet(gpio, 2, 1, RALINK_GPIO_LED_INFINITY, 1, RALINK_GPIO_LED_INFINITY);
		break;
	case WPS_LED_ERROR:
		return gpioLedSet(gpio, 1, 1, RALINK_GPIO_LED_INFINITY, 1, RALINK_GPIO_LED_INFINITY);
		break;
	case WPS_LED_SESSION_OVERLAP:
		return gpioLedSet(gpio, 1, 1, 10, 5, RALINK_GPIO_LED_INFINITY);
		break;
	case WPS_LED_SUCCESS:
		gpioLedSet(gpio, 3000, 1, 1, 1, 1);
		break;
	}
	return 0;
}
#endif

static void configAP(char *ifname, int ssid_index, int nvram_idx)
{
	WSC_PROFILE *wsc_profile;
	char temp[65];

	if ((wsc_profile = malloc(sizeof(WSC_PROFILE))) == NULL) {
		perror("malloc wsc_profile");
		return;
	}
	getWscProfile(ifname, wsc_profile);

	nvram_init(nvram_idx);

	set_nth_value_flash(nvram_idx, ssid_index, "WscConfStatus", "2");

	nvram_bufset(nvram_idx, racat("SSID", ssid_index + 1), wsc_profile->Profile[0].SSID.Ssid);
	nvram_bufset(nvram_idx, racat("WscSSID", ssid_index + 1), wsc_profile->Profile[0].SSID.Ssid);

	switch (wsc_profile->Profile[0].AuthType) {
		case 0x0002:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "WPAPSK");
			break;
		case 0x0004:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "SHARED");
			break;
		case 0x0008:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "WPA");
			break;
		case 0x0010:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "WPA2");
			break;
		case 0x0020:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "WPA2PSK");
			break;
		case 0x0022:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "WPAPSKWPA2PSK");
			break;
		case 0x0001:
		default:
			set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "OPEN");
	}

	switch (wsc_profile->Profile[0].EncrType) {
		case 0x0002:
			set_nth_value_flash(nvram_idx, ssid_index, "EncrypType", "WEP");
			if ((wsc_profile->Profile[0].KeyLength == 10) || 
					(wsc_profile->Profile[0].KeyLength == 26)) {
				/* Key Entry Method == HEX */
				set_nth_value_flash(nvram_idx, ssid_index, "Key1Type", "0");
				set_nth_value_flash(nvram_idx, ssid_index, "Key2Type", "0");
				set_nth_value_flash(nvram_idx, ssid_index, "Key3Type", "0");
				set_nth_value_flash(nvram_idx, ssid_index, "Key4Type", "0");
			} else {
				/* Key Entry Method == ASCII */
				set_nth_value_flash(nvram_idx, ssid_index, "Key1Type", "1");
				set_nth_value_flash(nvram_idx, ssid_index, "Key2Type", "1");
				set_nth_value_flash(nvram_idx, ssid_index, "Key3Type", "1");
				set_nth_value_flash(nvram_idx, ssid_index, "Key4Type", "1");
			}
			if (wsc_profile->Profile[0].KeyIndex == 1) {
				nvram_bufset(nvram_idx, racat("Key1Str", ssid_index + 1), wsc_profile->Profile[0].Key);
				set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "1");
			} else if (wsc_profile->Profile[0].KeyIndex == 2) {
				nvram_bufset(nvram_idx, racat("Key2Str", ssid_index + 1), wsc_profile->Profile[0].Key);
				set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "2");
			} else if (wsc_profile->Profile[0].KeyIndex == 3) {
				nvram_bufset(nvram_idx, racat("Key3Str", ssid_index + 1), wsc_profile->Profile[0].Key);
				set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "3");
			} else if (wsc_profile->Profile[0].KeyIndex == 4) {
				nvram_bufset(nvram_idx, racat("Key4Str", ssid_index + 1), wsc_profile->Profile[0].Key);
				set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "4");
			}
			break;
		case 0x0004:
			set_nth_value_flash(nvram_idx, ssid_index, "EncrypType", "TKIP");
			set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "2");
			memset(temp, 0, 65);
			memcpy(temp, wsc_profile->Profile[0].Key, wsc_profile->Profile[0].KeyLength);
			nvram_bufset(nvram_idx, racat("WPAPSK", ssid_index + 1), temp);
			break;
		case 0x0008:
			set_nth_value_flash(nvram_idx, ssid_index, "EncrypType", "AES");
			set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "2");
			memset(temp, 0, 65);
			memcpy(temp, wsc_profile->Profile[0].Key, wsc_profile->Profile[0].KeyLength);
			nvram_bufset(nvram_idx, racat("WPAPSK", ssid_index + 1), temp);
			break;
		case 0x000C:
			set_nth_value_flash(nvram_idx, ssid_index, "EncrypType", "TKIPAES");
			set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "2");
			memset(temp, 0, 65);
			memcpy(temp, wsc_profile->Profile[0].Key, wsc_profile->Profile[0].KeyLength);
			nvram_bufset(nvram_idx, racat("WPAPSK", ssid_index + 1), temp);
			break;
		case 0x0001:
		default:
			set_nth_value_flash(nvram_idx, ssid_index, "EncrypType", "NONE");
			set_nth_value_flash(nvram_idx, ssid_index, "DefaultKeyID", "1");
	}

	if (wsc_profile->Profile[0].AuthType == 0x0002 &&
			wsc_profile->Profile[0].EncrType == 0x0004) {
		set_nth_value_flash(nvram_idx, ssid_index, "AuthMode", "WPAPSKWPA2PSK");
		set_nth_value_flash(nvram_idx, ssid_index, "EncrypType", "TKIPAES");
	}

	set_nth_value_flash(nvram_idx, ssid_index, "IEEE8021X", "0");

	nvram_commit(nvram_idx);
//	nvram_close(nvram_idx);

	free(wsc_profile);

	return;
}

int ap_oid_query_info(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t) ptr;
	wrq.u.data.flags = OidQueryCode;

	return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}

static void config_acl(char *ifname, int ssid_index, int nvram_idx)
{
	int i, socket_id = socket(AF_INET, SOCK_DGRAM, 0);
	char temp[18];
	PRT_802_11_ACL alc_list = malloc(sizeof(RT_802_11_ACL));

	nvram_init(nvram_idx);

	ap_oid_query_info(OID_802_11_ACL_LIST, socket_id, ifname, alc_list, sizeof(RT_802_11_ACL));
	for (i = 0; i < alc_list->Num; i++)
	{
		sprintf(temp, "%02X:%02X:%02X:%02X:%02X:%02X",
				alc_list->Entry[i].Addr[0], alc_list->Entry[i].Addr[1], alc_list->Entry[i].Addr[2],
				alc_list->Entry[i].Addr[3], alc_list->Entry[i].Addr[4], alc_list->Entry[i].Addr[5]);
		set_nth_value_flash(nvram_idx, i, racat("AccessControlList", ssid_index), temp);
	}

	nvram_commit(nvram_idx);
//	nvram_close(nvram_idx);
	close(socket_id);
	free(alc_list);
}

static void queryWPSStatus(int signo)
{
	int WscStatus = 0, nvram_idx;
	struct _WSC_CONFIGURED_VALUE *wsc_value;
	char ifname[IFNAMSIZ];
	char ifprefix[4];
	char *multi_wsc_conf_mode = NULL;
	int bssid_num;
	int ssid_idx;
	int wsc_en = 0;

	//fprintf(stderr, "[nvram_daemon][nvram_daemon][%s] <---------\n", __FUNCTION__);
	if (g_wps_timer_state == 0) {
		//resetTimerAll();
		return;
	}

	nvram_init(nvram_idx);
	if (triggered_signal == SIGWINCH) {
		snprintf(ifprefix, sizeof(ifprefix), "rai");
		nvram_idx = RTDEV_NVRAM;
	} else {
		snprintf(ifprefix, sizeof(ifprefix), "ra");
		nvram_idx = RT2860_NVRAM;
	}

	wsc_timeout_counter += WPS_AP_CATCH_CONFIGURED_TIMER;
	//fprintf(stderr, "[nvram_daemon][%s]wsc_timeout_counter=%d\n", __FUNCTION__, wsc_timeout_counter);
	if (wsc_timeout_counter > WPS_AP_TIMEOUT_SECS) {
		resetTimerAll();
		LedError();
		return;
		//Useful if there are multiple wsc_timeout_counter.
		//wsc_timer_number &= ~(1 << ssid_idx);
		//LedError();
		//continue;
	}

	//fprintf(stderr, "[nvram_daemon][%s]wsc_timer_number=%d\n", __FUNCTION__, wsc_timer_number);
	multi_wsc_conf_mode = nvram_bufget(nvram_idx, "WscConfMode");
	for(ssid_idx = 0; ssid_idx < wsc_bssid_num; ssid_idx++){
		wsc_en = get_nth_number(ssid_idx, multi_wsc_conf_mode, ';', 10);
		//fprintf(stderr, "[nvram_daemon][%s]wsc_en=%d\n", __FUNCTION__, wsc_en);
		if (wsc_en == 0){
			//Prevent empty loop if disable WPS between trigger and timeout.
			if ((wsc_timer_number & (1 << ssid_idx)) > 0)
				wsc_timer_number &= ~(1 << ssid_idx);
			continue;
		}

		//fprintf(stderr, "[nvram_daemon][%s]wsc_timer_en=%d\n", __FUNCTION__, wsc_timer_number & (1 << ssid_idx));
		if ((wsc_timer_number & (1 << ssid_idx)) == 0)
			continue;

		snprintf(ifname, sizeof(ifname), "%s%d", ifprefix, ssid_idx);

		//fprintf(stderr, "[nvram_daemon][%s]ifname=%s\n", __FUNCTION__, ifname);
		WscStatus = getWscStatus(ifname);

		//fprintf(stderr, "[nvram_daemon][%s]WscStatus=%d\n", __FUNCTION__, WscStatus);
		switch (WscStatus) {
			case 0x109:	/* PBC_SESSION_OVERLAP */
				//resetTimerAll();
				wsc_timer_number &= ~(1 << ssid_idx);
				LedSessionOverlapDetected();
				break;
			case 34:	/* CONFIGURED */
				//resetTimerAll();
				wsc_timer_number &= ~(1 << ssid_idx);
				LedSuccess();
				if ((wsc_value = malloc(sizeof(struct _WSC_CONFIGURED_VALUE))) == NULL) {
					perror("malloc wsc_value");
					return;
				}
				/* Hard to modify for DBDC */
				getCurrentWscProfile(ifname, wsc_value, sizeof(WSC_CONFIGURED_VALUE));
				if ((strcmp(nvram_bufget(nvram_idx, "WCNTest"), "1") == 0) ||
						(wsc_value->WscConfigured == 2)) {
					configAP(ifname, ssid_idx, nvram_idx);
				}
				free(wsc_value);
				config_acl(ifname, ssid_idx, nvram_idx);
				break;
			case 2:		/* FAIL */
			case 1:		/* IDLE */
				//resetTimerAll();
				wsc_timer_number &= ~(1 << ssid_idx);
				LedReset();
		}
	}

	//fprintf(stderr, "[nvram_daemon][%s]wsc_timer_number=%d\n", __FUNCTION__, wsc_timer_number);
	if (wsc_timer_number == 0)
		resetTimerAll();

	nvram_close(nvram_idx);
}

static void triggerWPS(int signo)
{
//fprintf(stderr, "[nvram_daemon][nvram_daemon][%s] <---------\n", __FUNCTION__);
#if defined(CONFIG_DBDC_MODE)
	char *multi_wsc_conf_mode;
	int nvram_idx;
	int dbdc_en;
	int ssid_idx;
	int wsc_conf_mode;
#endif

	resetTimerAll();

#if defined(CONFIG_DBDC_MODE)
	if (signo == SIGWINCH)
		nvram_idx = RTDEV_NVRAM;
	else 
		nvram_idx = RT2860_NVRAM;

	dbdc_en = 0;
	wsc_bssid_num = 1;
	if (nvram_idx == RT2860_NVRAM)
		dbdc_en = (int)strtol(nvram_bufget(nvram_idx, "DBDC_MODE"), NULL, 10);

	if (dbdc_en > 0)
		wsc_bssid_num = (int)strtol(nvram_bufget(RT2860_NVRAM, "BssidNum"), NULL, 10);

	nvram_init(nvram_idx);
	multi_wsc_conf_mode = nvram_bufget(nvram_idx, "WscConfMode");
	for (ssid_idx = 0; ssid_idx < wsc_bssid_num; ssid_idx++) {
		wsc_conf_mode = get_nth_number(ssid_idx, multi_wsc_conf_mode, ';', 10);
		if(wsc_conf_mode == 0)
			continue;

		wsc_timer_number |= 1 << ssid_idx;
	}
	nvram_close(nvram_idx);
#else
	wsc_bssid_num = 1;
	wsc_timer_number = 1;
#endif
	triggered_signal = signo;
//	resetTimerAll(); //move to top 
	wsc_timeout_counter = 0;
	g_wps_timer_state = 1;
	setTimer(WPS_AP_CATCH_CONFIGURED_TIMER * 1000, queryWPSStatus);
	LedInProgress();
}

#endif

static void dhcpcHandler(int signo)
{
#ifdef CONFIG_IP_NF_FILTER
	system("/etc_ro/lighttpd/www/cgi-bin/firewall.cgi init");
#endif
	system("/etc_ro/lighttpd/www/cgi-bin/internet.cgi ripd");
	system("/sbin/config-igmpproxy.sh");
#ifdef CONFIG_RALINKAPP_SWQOS
	system("/etc_ro/lighttpd/www/cgi-bin/qos.cgi init");
#endif
#ifdef CONFIG_RALINKAPP_HWQOS
	system("/etc_ro/lighttpd/www/cgi-bin/qos.cgi init");
#endif
#if defined (CONFIG_IPV6)
	system("/etc_ro/lighttpd/www/cgi-bin/internet.cgi ipv6");
#endif

#if defined (CONFIG_RA_NAT_HW)
	/* remove all binding entries after getting new IP */
	const char *hwnat_ebl = nvram_bufget(RT2860_NVRAM, "hwnatEnabled");

	if(0 == strcmp(hwnat_ebl, "1")) {
		system("rmmod hw_nat");
		system("insmod hw_nat");
#if defined (CONFIG_RA_HW_NAT_WIFI_NEW_ARCH)
		system("iwpriv ra0 set hw_nat_register=1");
#if defined (CONFIG_SECOND_IF_MT7615E)
		system("iwpriv rai0 set hw_nat_register=1");
#endif
#endif
	}
#endif
}

static void hotPluglerHandler(int signo)
{
	system("/etc_ro/lighttpd/www/cgi-bin/nas.cgi init");
	system("/etc_ro/lighttpd/www/cgi-bin/usb.cgi init");
}

#endif

#if 1
static void doWPSHandler(int signo)
{
#if 0
#if defined CONFIG_RALINK_RT2880
	int gopid;
	FILE *fp = fopen("/var/run/goahead.pid", "r");

	if (NULL == fp) {
		printf("nvram: goAhead is not running\n");
		return;
	}
	fscanf(fp, "%d", &gopid);
	if (gopid < 2) {
		printf("nvram: goAhead pid(%d) <= 1\n", gopid);
		return;
	}

	//send SIGUSR1 signal to goAhead for WPSPBCStart();
	printf("notify goahead to start WPS PBC..\n");
	kill(gopid, SIGUSR1);
	fclose(fp);
#elif defined CONFIG_USER_LIGHTY
	char cmd[128];

	sprintf(cmd, "%s %d", 
		"/etc_ro/lighttpd/www/cgi-bin/wireless.cgi wps_pbc",
		RT2860_NVRAM);
	system(cmd);
	triggerWPS(SIGXFSZ);
	//triggerWPS(SIGWINCH);
#endif
#endif
	system("nvram_set 2860 WpsConfiguring 1");

	/* wps配置 */  
        do_system("iwpriv apcli0 set ApCliEnable=0");
	do_system("iwpriv apcli0 set WscConfMode=1");             
	do_system("iwpriv apcli0 set WscMode=2");                
	do_system("iwpriv apcli0 set ApCliEnable=1");
	do_system("iwpriv apcli0 set WscGetConf=1");
}
#endif

static void loadDefaultHandler(int signo)
{
	printf("load default and reboot..\n");
	loadDefault(2860);
#if defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE)
	loadDefault(2561);
#else
	loadDefault(2880);
#endif
	loadDefault(7790);

	system("reboot");
}

static void signal_handler(void)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR2, loadDefaultHandler);
	signal(SIGUSR1, doWPSHandler);
#if defined CONFIG_USER_LIGHTY
	signal(SIGTSTP, dhcpcHandler);
#if defined WSC_SUPPORT
	signal(SIGXFSZ, triggerWPS);
	signal(SIGWINCH, triggerWPS);
#endif
#if (defined CONFIG_USB) || (defined CONFIG_MMC)
	signal(SIGTTIN, hotPluglerHandler);
#endif
	//signal(SIGXFSZ, doWPSSingleTriggerHandler);
#endif
}

#if defined (CONFIG_RALINK_GPIO) || defined (CONFIG_RALINK_GPIO_MODULE) 
static int initGpio(unsigned int gpio_num)
{
	int fd;
	ralink_gpio_reg_info info;

	fd = open(GPIO_DEV, O_RDONLY);
	if (fd < 0) {
		perror(GPIO_DEV);
		return -1;
	}
	info.pid = getpid();
	info.irq = gpio_num;
	//set gpio direction to input
#if !defined (CONFIG_RALINK_RT6855A)
	if (info.irq < 24) {
		if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, (1<<info.irq)) < 0)
			goto ioctl_err;
	}
#if defined (RALINK_GPIO_HAS_2722)
	else if (22 <= info.irq) {
		if (ioctl(fd, RALINK_GPIO2722_SET_DIR_IN, (1<<(info.irq-22))) < 0)
			goto ioctl_err;
	}
#elif defined (RALINK_GPIO_HAS_4524)
	else if (24 <= info.irq && info.irq < 40) {
		if (ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, (1<<(info.irq-24))) < 0)
			goto ioctl_err;
	}
	else {
		if (ioctl(fd, RALINK_GPIO4540_SET_DIR_IN, (1<<(info.irq-40))) < 0)
			goto ioctl_err;
	}
#elif defined (RALINK_GPIO_HAS_5124)
	else if (24 <= info.irq && info.irq < 40) {
		if (ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, (1<<(info.irq-24))) < 0)
			goto ioctl_err;
	}
	else {
		if (ioctl(fd, RALINK_GPIO5140_SET_DIR_IN, (1<<(info.irq-40))) < 0)
			goto ioctl_err;
	}
#elif defined (RALINK_GPIO_HAS_9524) || defined (RALINK_GPIO_HAS_7224)
	else if (24 <= info.irq && info.irq < 40) {
		if (ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, (1<<(info.irq-24))) < 0)
			goto ioctl_err;
	}
	else if (40 <= info.irq && info.irq < 72) {
		if (ioctl(fd, RALINK_GPIO7140_SET_DIR_IN, (1<<(info.irq-40))) < 0)
			goto ioctl_err;
	}
	else {
#if defined (RALINK_GPIO_HAS_7224)
		if (ioctl(fd, RALINK_GPIO72_SET_DIR_IN, (1<<(info.irq-72))) < 0)
#else
		if (ioctl(fd, RALINK_GPIO9572_SET_DIR_IN, (1<<(info.irq-72))) < 0)
#endif
			goto ioctl_err;
	}
#endif
#else
	if (info.irq < 16) {
		if (ioctl(fd, RALINK_GPIO_SET_DIR_IN, info.irq) < 0)
			goto ioctl_err;
	} else {
		goto ioctl_err;
	}
#endif
	//enable gpio interrupt
	if (ioctl(fd, RALINK_GPIO_ENABLE_INTP) < 0)
		goto ioctl_err;

	//register my information
	if (ioctl(fd, RALINK_GPIO_REG_IRQ, &info) < 0)
		goto ioctl_err;
	close(fd);
	return 0;

ioctl_err:
	perror("ioctl");
	close(fd);
	return -1;
}
#endif

static void pidfile_delete(void)
{
	if (saved_pidfile) unlink(saved_pidfile);
}

int pidfile_acquire(const char *pidfile)
{
	int pid_fd;
	if (!pidfile) return -1;

	pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
	if (pid_fd < 0) {
		printf("Unable to open pidfile %s\n", pidfile);
	} else {
		lockf(pid_fd, F_LOCK, 0);
		if (!saved_pidfile)
			atexit(pidfile_delete);
		saved_pidfile = (char *) pidfile;
	}
	return pid_fd;
}

void pidfile_write_release(int pid_fd)
{
	FILE *out;

	if (pid_fd < 0) return;

	if ((out = fdopen(pid_fd, "w")) != NULL) {
		fprintf(out, "%d\n", getpid());
		fclose(out);
	}
	lockf(pid_fd, F_UNLCK, 0);
	close(pid_fd);
}

static int running = 1;

int main(int argc,char **argv)
{
	pid_t pid;
	int fd;

	if (strcmp(nvram_bufget(RT2860_NVRAM, "WebInit"),"1")) {
		loadDefault(2860);
	}
	
	if (strcmp(nvram_bufget(RTDEV_NVRAM, "WebInit"),"1")) {
#if defined (CONFIG_RT2561_AP) || defined (CONFIG_RT2561_AP_MODULE)
		loadDefault(2561);
#else
		loadDefault(2880);
#endif
	}
	if (strcmp(nvram_bufget(WIFI3_NVRAM, "WebInit"),"1")) {
		loadDefault(7790);
	}
	nvram_close(RT2860_NVRAM);
	nvram_close(RTDEV_NVRAM);
	nvram_close(WIFI3_NVRAM);

#if defined (CONFIG_RALINK_GPIO) || defined (CONFIG_RALINK_GPIO_MODULE) 
#if defined (CONFIG_RALINK_RT2880)
	/* RT2880 reset default & WPS PBC */
	if ((err = initGpio(0) != 0))
		exit(EXIT_FAILURE);
#elif defined (CONFIG_RALINK_RT3052) && ((CONFIG_RALINK_I2S) || defined (CONFIG_RALINK_I2S_MODULE))
	if ((err = initGpio(43) != 0))
		exit(EXIT_FAILURE);
#elif defined (CONFIG_RALINK_RT3883)
	/* RT3883 reset default */
	if ((err = initGpio(27) != 0))
		exit(EXIT_FAILURE);
#if defined CONFIG_USER_LIGHTY && defined WSC_SUPPORT
	/* RT3883 WPS PBC */
	if ((err = initGpio(26) != 0))
		exit(EXIT_FAILURE);
#endif
#elif defined (CONFIG_RALINK_RT6855A)
#if defined (CONFIG_RT6855A_PCIE_PORT0_ENABLE)
	/* RT6855 reset default */
	if (initGpio(0) != 0)
		exit(EXIT_FAILURE);
#if defined CONFIG_USER_LIGHTY && defined WSC_SUPPORT
	/* RT6855 WPS PBC */
	if (initGpio(1) != 0)
		exit(EXIT_FAILURE);
#endif
#else
	/* RT6856 reset default */
	if (initGpio(2) != 0)
		exit(EXIT_FAILURE);
#if defined CONFIG_USER_LIGHTY && defined WSC_SUPPORT
	/* RT6856 WPS PBC */
	if (initGpio(6) != 0)
		exit(EXIT_FAILURE);
#endif
#endif
#elif defined (CONFIG_RALINK_MT7620)
	/* MT7620 reset default */
	if (initGpio(1) != 0)
		exit(EXIT_FAILURE);
#if defined CONFIG_USER_LIGHTY && defined WSC_SUPPORT
	/* MT7620 WPS PBC */
	if (initGpio(2) != 0)
		exit(EXIT_FAILURE);
#endif
#elif defined (CONFIG_RALINK_MT7621)
	/* MT7621 reset default & WPS PBC */
	if (initGpio(18) != 0)
		exit(EXIT_FAILURE);
#elif defined (CONFIG_RALINK_MT7628)
	/* MT7628 reset default */
	if (initGpio(38) != 0)	// RFB-V10/V11: GPIO #45, RFB-V12: GPIO #38 
		exit(EXIT_FAILURE);
#if defined CONFIG_USER_LIGHTY && defined WSC_SUPPORT
	/* MT7628 WPS PBC */
	if (initGpio(38) != 0)	// RFB-V10/V11: GPIO #46, RFB-V12: GPIO #38
		exit(EXIT_FAILURE);
#endif
#else
	/* RT2883, RT3052, RT3352, RT5350 reset default */
	if (initGpio(10) != 0)
		exit(EXIT_FAILURE);
#if defined CONFIG_USER_LIGHTY && defined WSC_SUPPORT
	/* RT2883, RT3052, RT3352, RT5350 WPS PBC */
	if (initGpio(0) != 0)
		exit(EXIT_FAILURE);
#endif
#endif	
#endif
	signal_handler();

	fd = pidfile_acquire("/var/run/nvramd.pid");
	pidfile_write_release(fd);

#ifdef CONFIG_RT2880_L2_MANAGE
	//start the management daemon (blocking)
	ramad_start();
#else
	while (running == 1) {
		pause();
	}
#endif

	exit(EXIT_SUCCESS);
}
