/*
 * Modified for use with MPlayer, detailed changelog at
 * http://svn.mplayerhq.hu/mplayer/trunk/
 * $Id: //WIFI_SOC/MP/SDK_5_0_0_0/RT288x_SDK/source/user/rt2880_app/mplayer/loader/driver.h#1 $
 */

#ifndef LOADER_DRIVER_H
#define LOADER_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "wine/windef.h"
#include "wine/driver.h"

void SetCodecPath(const char* path);
void CodecAlloc(void);
void CodecRelease(void);

HDRVR DrvOpen(LPARAM lParam2);
void DrvClose(HDRVR hdrvr);

#ifdef __cplusplus
}
#endif

#endif /* LOADER_DRIVER_H */
