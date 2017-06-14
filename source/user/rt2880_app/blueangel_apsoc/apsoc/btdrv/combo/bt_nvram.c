/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <fcntl.h>

/* use nvram */
//#include "CFG_BT_File.h"
#if 0
#include "CFG_file_lid.h"
#include "libnvram.h"
#endif


//#include "bt_kal.h"
//#include "cust_bt.h"


#include <stdlib.h>
#include "os_dep.h"
#include "bt_nvram.h"

#if BT_NVRAM_SUPPORT
#include "nvram.h"

#define BT_NVRAM_NAME  "BT_NVRAM"
#define BT_NVRAM_INDEX RT2860_NVRAM
#endif

#if BT_FACTORY_SUPPORT
#include <linux/autoconf.h>
#include <fcntl.h>

#define MTD_FACTORY	"Factory"
#define EMMC_PARTITION	"/dev/mmcblk0"
#define DUMCHAR_INFO	"/proc/emmc_partition"
#define BT_FACTORY_NAME	"bt"

#define COMBO_BT_LEN		0x40
#define COMBO_BT_BDADDR_LEN	0x6
#define COMBO_BT_VOICE_LEN	0x2
#define COMBO_BT_CODEC_LEN	0x4
#define COMBO_BT_RADIO_LEN	0x6
#define COMBO_BT_SLEEP_LEN	0x7
#define COMBO_BT_OTHER_LEN	0x2
#define COMBO_BT_TX_PWR_LEN	0x3
#define COMBO_BT_COEX_LEN	0x6

#define COMBO_BT_OFFSET 	0xE100
#define COMBO_BT_BDADDR_OFFSET	(0x0 + COMBO_BT_OFFSET)
#define COMBO_BT_VOICE_OFFSET	(0x6 + COMBO_BT_OFFSET)
#define COMBO_BT_CODEC_OFFSET	(0x8 + COMBO_BT_OFFSET)
#define COMBO_BT_RADIO_OFFSET	(0xC + COMBO_BT_OFFSET)
#define COMBO_BT_SLEEP_OFFSET	(0x12 + COMBO_BT_OFFSET)
#define COMBO_BT_OTHER_OFFSET	(0x19 + COMBO_BT_OFFSET)
#define COMBO_BT_TX_PWR_OFFSET	(0x1B + COMBO_BT_OFFSET)
#define COMBO_BT_COEX_OFFSET	(0x1E + COMBO_BT_OFFSET)

#define COMBO_WIFIRF_LEN 	512
#define COMBO_WIFI_OFFSET 	0xE200
#endif

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/
#if BT_NVRAM_SUPPORT
static unsigned char _get_str_byte_value(char *str);
#endif
#if BT_FACTORY_SUPPORT
static int get_offset(char *side);
static int get_start_address(long long *size, long long *start_addr);
static int emmc_read_offset(char *side, const unsigned int offset, const unsigned int len, char *buf);
static int emmc_write_offset(char *side, const unsigned int offset, char *value, const unsigned int len);
#endif

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

int bt_read_nvram(ap_nvram_btradio_struct *ucNvRamData)
{
	ap_nvram_btradio_struct bt_nvram;
	int len = 0;
#if BT_NVRAM_SUPPORT
	unsigned char i = 0;
	unsigned char offset = 0;
	const char *buf = NULL;

	LOG_TRC();

	/* get from NVRAM */
	nvram_init(BT_NVRAM_INDEX);
	buf = nvram_bufget(BT_NVRAM_INDEX, BT_NVRAM_NAME);
	nvram_close(BT_NVRAM_INDEX);

	len = strlen(buf);

	memset(&bt_nvram, '0', sizeof(ap_nvram_btradio_struct));


	LOG_DBG("len: %d, sizeof(ap_nvram_btradio_struct): %d\n", len, sizeof(ap_nvram_btradio_struct));

	if (len != sizeof(ap_nvram_btradio_struct)*2)
	{
		LOG_ERR("len != sizeof(ap_nvram_btradio_struct)*2\n");
		return -1;
	}
	/* Extract address */
	for (i = 0; i < 6; i++)
	{ 
		bt_nvram.addr[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract voice */
	for (i = 0; i < 2; i++)
	{
		bt_nvram.Voice[i]= _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract codec */
	for (i = 0; i < 4; i++)
	{
		bt_nvram.Codec[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract radio */
	for (i = 0; i < 6; i++)
	{
		bt_nvram.Radio[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract sleep */
	for (i = 0; i < 7; i++)
	{
		bt_nvram.Sleep[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract other feature */
	for (i = 0; i < 2; i++)
	{
		bt_nvram.BtFTR[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract tx power channel offset */
	for (i = 0; i < 3; i++)
	{
		bt_nvram.TxPWOffset[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}

	/* Extract BT/WIFI coexistence adjustment */
	for (i = 0; i < 6; i++)
	{
		bt_nvram.CoexAdjust[i] = _get_str_byte_value(buf + offset);
		offset += 2;
	}
#endif

#if BT_FACTORY_SUPPORT
	LOG_TRC();

	memset(&bt_nvram, '0', sizeof(ap_nvram_btradio_struct));

	len = emmc_read_offset(BT_FACTORY_NAME, 0, 64, (char*)&bt_nvram);
	if (len < 0)
	{
		LOG_ERR("Get BT Factory Data failed\n");
		return -1;
	}

	LOG_DBG("len: %d, sizeof(ap_nvram_btradio_struct): %d\n", len, sizeof(ap_nvram_btradio_struct));
#endif	

	memcpy(ucNvRamData, &bt_nvram, sizeof(ap_nvram_btradio_struct));

	return 0;
}

int bt_write_nvram(ap_nvram_btradio_struct *ucNvRamData)
{
	int ret = -1;

#if BT_NVRAM_SUPPORT
	unsigned char buf[sizeof(ap_nvram_btradio_struct)*2 + 1];
	unsigned char *dst = buf;
	unsigned char *src = NULL;

	LOG_TRC();

	/* Place address */
	src = ucNvRamData->addr;
	snprintf(dst, 13, "%02x%02x%02x%02x%02x%02x", src[0], src[1], src[2], src[3], src[4], src[5]);
	dst = dst + 12;

	/* Place voice setting */
	src = ucNvRamData->Voice;
	snprintf(dst, 5, "%02x%02x", src[0], src[1]);
	dst = dst + 4;

	/* Place PCM codec setting */
	src = ucNvRamData->Codec;
	snprintf(dst, 9, "%02x%02x%02x%02x", src[0], src[1], src[2], src[3]);
	dst = dst + 8;

	/* Place RF */
	src = ucNvRamData->Radio;
	snprintf(dst, 13, "%02x%02x%02x%02x%02x%02x", src[0], src[1], src[2], src[3], src[4], src[5]);
	dst = dst + 12;

	/* Place sleep mode config */
	src = ucNvRamData->Sleep;
	snprintf(dst, 15, "%02x%02x%02x%02x%02x%02x%02x", src[0], src[1], src[2], src[3], src[4], src[5], src[6]);
	dst = dst + 14;

	/* Place other feature setting */
	src = ucNvRamData->BtFTR;
	snprintf(dst, 5, "%02x%02x", src[0], src[1]);
	dst = dst + 4;

	/* Place Tx power channel offset compensation */
	src = ucNvRamData->TxPWOffset;
	snprintf(dst, 7, "%02x%02x%02x", src[0], src[1], src[2]);
	dst = dst + 6;

	/* Place BT/WIFI coexistence performance adjustment */
	src = ucNvRamData->CoexAdjust;
	snprintf(dst, 13, "%02x%02x%02x%02x%02x%02x", src[0], src[1], src[2], src[3], src[4], src[5]);
	dst = dst + 12;

	/* Place reserved */
	src = ucNvRamData->Reserved1;
	snprintf(dst, 5, "%02x%02x", src[0], src[1]);
	dst = dst + 4;

	/* Place reserved */
	src = ucNvRamData->Reserved2;
	snprintf(dst, 5, "%02x%02x", src[0], src[1]);
	dst = dst + 4;

	/* Place reserved */
	src = ucNvRamData->Reserved3;
	snprintf(dst, 9, "%02x%02x%02x%02x", src[0], src[1], src[2], src[3]);
	dst = dst + 8;

	/* Place reserved */
	src = ucNvRamData->Reserved4;
	snprintf(dst, 9, "%02x%02x%02x%02x", src[0], src[1], src[2], src[3]);
	dst = dst + 8;

	/* Place reserved */
	src = ucNvRamData->Reserved5;
	snprintf(dst, 17, "%02x%02x%02x%02x%02x%02x%02x%02x", src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7]);
	dst = dst + 16;

	/* Place reserved */
	src = ucNvRamData->Reserved6;
	snprintf(dst, 17, "%02x%02x%02x%02x%02x%02x%02x%02x", src[0], src[1], src[2], src[3], src[4], src[5], src[6], src[7]);
	dst = dst + 16;

	/* Set to NVRAM */
	nvram_init(BT_NVRAM_INDEX);
	ret = nvram_set(BT_NVRAM_INDEX, BT_NVRAM_NAME, buf);
	nvram_close(BT_NVRAM_INDEX);

	LOG_DBG("return: %d, bt nvram name: %s, value: %s", ret, BT_NVRAM_NAME, buf);
#endif

#if BT_FACTORY_SUPPORT
	LOG_TRC();

	ret = emmc_write_offset(BT_FACTORY_NAME, 0, (char *)ucNvRamData, sizeof(ap_nvram_btradio_struct));

	LOG_DBG("return: %d, bt factory name: %s, first 6 chars: %02x%02x%02x%02x%02x%02x\n", 
		ret, BT_FACTORY_NAME, 
		((char *)ucNvRamData)[0], ((char *)ucNvRamData)[1], ((char *)ucNvRamData)[2], 
		((char *)ucNvRamData)[3], ((char *)ucNvRamData)[4], ((char *)ucNvRamData)[5]);
#endif

	return ret;
}


// Utility functions
#if BT_NVRAM_SUPPORT
static unsigned char _get_str_byte_value(char *str)
{
    char buf[3] = {0};

	LOG_TRC();

    buf[0] = str[0];
    buf[1] = str[1];

    return (unsigned char) strtol(buf, NULL, 16);
}
#endif

#if BT_FACTORY_SUPPORT
static int get_offset(char *side)
{
	int offset = 0;
#if defined(ETH_MAC)	
	if (!strcmp(side, "wan"))
		offset = WAN_OFFSET;
	else if (!strcmp(side, "lan"))
		offset = LAN_OFFSET;
	else
#endif
#if defined(CONFIG_ARCH_MT7623)
	if (!strcmp(side, "bt"))
		offset = COMBO_BT_OFFSET;
	else if (!strcmp(side, "wifi"))
		offset = COMBO_WIFI_OFFSET;
	else if (!strcmp(side, "bdaddr"))
		offset = COMBO_BT_BDADDR_OFFSET;
	else if (!strcmp(side, "bdvoice"))
		offset = COMBO_BT_VOICE_OFFSET;
	else if (!strcmp(side, "bdcodec"))
		offset = COMBO_BT_CODEC_OFFSET;
	else if (!strcmp(side, "bdradio"))
		offset = COMBO_BT_RADIO_OFFSET;
	else if (!strcmp(side, "bdsleep"))
		offset = COMBO_BT_SLEEP_OFFSET;
	else if (!strcmp(side, "bdother"))
		offset = COMBO_BT_OTHER_OFFSET;
	else if (!strcmp(side, "bdtxpwr"))
		offset = COMBO_BT_TX_PWR_OFFSET;
	else if (!strcmp(side, "bdcoex"))
		offset = COMBO_BT_COEX_OFFSET;
	else 
#endif /* CONFIG_ARCH_MT7623 */
	{
		LOG_DBG("Get offset \"%s\" failed.\n", side);
		offset = -1;
	}

	return offset;
}

static int get_start_address(long long *size, long long *start_addr)
{
	FILE *fp = fopen(DUMCHAR_INFO, "r");
	char p_name[20];
	char p_size[20];
	char p_start_addr[20];

	if (fp == NULL) {
		LOG_DBG("Can't open %s\n", DUMCHAR_INFO);
		return -1;
	}

	if (!size){
		fprintf(stderr, "Invalid size\n");
		fclose(fp);
		return -1;
	}

	if (!start_addr){
		fprintf(stderr, "Invalid start_addr\n");
		fclose(fp);
		return -1;
	}

	memset(p_name, 0, 20);
	while (EOF != fscanf(fp, "\n%20s\t%20s\t%20s\t%*s\t%*s", 
			     p_name, p_size, p_start_addr)) {
		if (!strcmp(p_name, "factory"))
			break;
		memset(p_name, 0, 20);
		memset(p_size, 0, 20);
		memset(p_start_addr, 0, 20);
	}
	*size = strtoll(p_size, NULL, 16);
	*start_addr = strtoull(p_start_addr, NULL, 16);
	if (size == 0) {
		LOG_DBG("not found \"factory\" partition\n");
		fclose(fp);
		return -1;
	}
	fclose(fp);

	return 0;
}

static int emmc_read_offset(char *side, const unsigned int offset, const unsigned int len, char *buf)
{
	int fd, offset_total, offset_base;
	long long size, start_addr;

	fd = open(EMMC_PARTITION, O_RDWR | O_SYNC);
	if(fd < 0) {
		LOG_DBG("Could not open emmc device: %s\n", EMMC_PARTITION);
		return -1;
	}

	if (get_start_address(&size, &start_addr) < 0) {
		close(fd);
		return -1;
	}

	offset_base = get_offset(side);
	if(offset_base < 0){
		LOG_DBG("wrong partition name: %s\n", side);
		close(fd);
		return -1;
	}

	offset_total = offset_base + offset;
	if ((offset_total + len) >= size) {
		LOG_DBG("exceed than the maximum size: %llx\n", size);
		close(fd);
		return -1;
	}
/*
	if (!strcmp(side, "wan"))
		lseek(fd, start_addr+WAN_OFFSET, SEEK_SET);
	else (!strcmp(side, "lan"))
		lseek(fd, start_addr+LAN_OFFSET, SEEK_SET);
*/
	lseek(fd, start_addr + offset_total, SEEK_SET);

	//*buf = malloc(len * sizeof(char));
	if(!buf){
		LOG_DBG("buf is null\n");
		close(fd);
		return -1;
	}

	if(read(fd, buf, len) != len){ 
		LOG_DBG("read() failed\n");
		close(fd);
		return -1;
	}

	LOG_DBG("read %s[%08X]=", side, offset_total);
/*
	for (i = 0; i < len; i++)
	{ 
		LOG_DBG("%02X", buf[i]);
		if (i < len-1)
			LOG_DBG(":");
		else
			LOG_DBG("\n");
	}
*/
	close(fd);

	return len;
}

static int emmc_write_offset(char *side, const unsigned int offset, char *value, const unsigned int len)
{
	int fd, offset_total, offset_base;
	//char buf[MACADDR_LEN];
	long long size, start_addr;

 	fd = open(EMMC_PARTITION, O_RDWR | O_SYNC);
 	if(fd < 0) {
		LOG_DBG("Could not open emmc device: %s\n", EMMC_PARTITION);
		return -1;
	}

	if (get_start_address(&size, &start_addr) < 0) {
		close(fd);
		return -1;
	} 

	offset_base = get_offset(side);
	if(offset_base < 0){
		LOG_DBG("wrong partition name: %s\n", side);
		close(fd);
		return -1;
	} 

	offset_total = offset_base + offset;
	if ((offset_total + len) >= size) {
		LOG_DBG("exceed than the maximum size: %llx\n", size);
		close(fd);
		return -1;
	}

LOG_DBG("seek offset=%08x\n", offset_total);
	lseek(fd, start_addr + offset_total, SEEK_SET);
	if (write(fd, value, len) != len) {
		LOG_DBG("write() %s failed\n", EMMC_PARTITION);
		close(fd);
		return -1;
	}
	close(fd);

	return len;
}
#endif
