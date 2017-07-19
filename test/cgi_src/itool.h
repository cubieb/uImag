#ifndef I_TOOL_H_
#define I_TOOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <errno.h>
#include <linux/wireless.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//#include <asm/types.h>
#include "stapriv.h"
#include "utils.h"
#include "media.h"
#include "oid.h"


unsigned int get_uptime(void);
//unsigned int get_conntime(int nvram);
int getStaLinkStatus(char *ifname);
unsigned int ConvertRssiToSignalQuality(long RSSI);
int get_signal(char *ifname);


#endif
