/* Imported from the dvbstream project
 *
 * Modified for use with MPlayer, for details see the changelog at
 * http://svn.mplayerhq.hu/mplayer/trunk/
 * $Id: //WIFI_SOC/MP/SDK_5_0_0_0/RT288x_SDK/source/user/rt2880_app/mplayer/stream/rtp.h#1 $
 */

#ifndef RTP_H
#define RTP_H

int read_rtp_from_server(int fd, char *buffer, int length);

#endif
