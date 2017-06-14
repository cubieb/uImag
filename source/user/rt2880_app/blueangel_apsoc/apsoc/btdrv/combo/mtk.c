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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#if 0
#include <cutils/properties.h>
#endif

/* use nvram */
#include "bt_nvram.h"
#if 0
#include "CFG_file_lid.h"
#include "libnvram.h"
#endif


#include "bt_kal.h"
#include "cust_bt.h"


/* This file name must be referred to nvram_bt.c */
#define BT_NVRAM_DATA_CLONE_FILE_NAME    "/data/BT_Addr"

unsigned long g_chipId = 0;

/**************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                 *
***************************************************************************/

extern
BOOL BT_InitDevice(
    HANDLE  hComPortFile,
    PBYTE   ucNvRamData,
    DWORD   dwBaud,
    DWORD   dwHostBaud,
    DWORD   dwFlowControl,
    SETUP_UART_PARAM_T setup_uart_param
);

extern
BOOL BT_DeinitDevice(
    HANDLE  hComPortFile
);

/**************************************************************************
 *                          F U N C T I O N S                             *
***************************************************************************/

/* Initialize UART driver */
static int init_uart(char *dev)
{
    int fd, i;
    
    LOG_TRC();
    
    fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        LOG_ERR("Can't open serial port\n");
        return -1;
    }
    
    return fd;
}

#if 0
int bt_get_combo_id(unsigned long *pChipId)
{
    int  chipId_ready_retry = 0;
    char chipId_val[PROPERTY_VALUE_MAX];
    
    do {
        if(property_get("persist.mtk.wcn.combo.chipid", chipId_val, NULL) &&
            0 != strcmp(chipId_val, "-1")){
            *pChipId = strtoul(chipId_val, NULL, 16);
            break;
        }
        else {
            chipId_ready_retry ++;
            usleep(500000);
        }
    } while(chipId_ready_retry < 10);
    
    LOG_DBG("Get combo chip id retry %d\n", chipId_ready_retry);
    if (chipId_ready_retry >= 10){
        LOG_DBG("Invalid combo chip id!\n");
        return -1;
    }
    else{
        LOG_DBG("Combo chip id %x\n", *pChipId);
        return 0;
    }
}
#else
int bt_get_combo_id(unsigned long *pChipId)
{
	*pChipId = 0x6625;

	LOG_DBG("Combo chip id %lx\n", *pChipId);
	
	return 0;
}
#endif


int mtk(void)
{
    int fd = -1;
    
    unsigned char ucNvRamData[sizeof(ap_nvram_btradio_struct)] = {0};
    unsigned char zero[sizeof(ap_nvram_btradio_struct)] = {0};
    int speed = 0, iUseFlowControl =0;
    SETUP_UART_PARAM_T uart_setup_callback = NULL;
    
    
    LOG_DBG("%s %d\n", __FILE__, __LINE__);
    
    fd = init_uart(CUST_BT_SERIAL_PORT);
    if (fd < 0){
        LOG_ERR("Can't initialize" CUST_BT_SERIAL_PORT);
        return -1;
    }
	
#if 1
    /* Read Nvram data */
    if(bt_read_nvram((ap_nvram_btradio_struct*) ucNvRamData) < 0){
        LOG_ERR("Read Nvram data fails\n");
        //goto error;
    }
#else

	/* read default value */
	ap_nvram_btradio_struct NvramDefaultData =
	{
	    {0x00, 0x00, 0x46, 0x81, 0x27, 0x01},
	    {0x60, 0x00}, //not used
	    {0x23, 0x10, 0x00, 0x00}, //not used
	    {0x07, 0x80, 0x00, 0x06, 0x05, 0x07},
	    {0x03, 0x40, 0x1F, 0x40, 0x1F, 0x00, 0x04},
	    {0x80, 0x00}, //not used
	    {0xFF, 0xFF, 0xFF},
	    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //not used
	    ///////////// Reserved /////////////
	    {0x00, 0x00},
	    {0x00, 0x00},
	    {0x00, 0x00, 0x00, 0x00},
	    {0x00, 0x00, 0x00, 0x00},
	    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	};

	memcpy(ucNvRamData, &NvramDefaultData, sizeof(ap_nvram_btradio_struct));

#endif

    if(0 == memcmp(ucNvRamData, zero, sizeof(ap_nvram_btradio_struct)))
	{
		/* use default value */
		ap_nvram_btradio_struct NvramDefaultData =
		{
		    {0x00, 0x00, 0x46, 0x76, 0x23, 0x01},
		    {0x60, 0x00}, //not used
		    {0x23, 0x10, 0x00, 0x00}, //not used
		    {0x07, 0x80, 0x00, 0x06, 0x05, 0x07},
		    {0x03, 0x40, 0x1F, 0x40, 0x1F, 0x00, 0x04},
		    {0x80, 0x00}, //not used
		    {0xFF, 0xFF, 0xFF},
		    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //not used
		    ///////////// Reserved /////////////
		    {0x00, 0x00},
		    {0x00, 0x00},
		    {0x00, 0x00, 0x00, 0x00},
		    {0x00, 0x00, 0x00, 0x00},
		    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
		};
		
		LOG_WAN("Nvram data all zero! use default value.\n");

		memcpy(ucNvRamData, &NvramDefaultData, sizeof(ap_nvram_btradio_struct));
		
		if(bt_write_nvram((ap_nvram_btradio_struct*)ucNvRamData) < 0)
		{
			LOG_ERR("write Nvram data fails\n");
		}
    }
    
    LOG_WAN("[BDAddr %02x-%02x-%02x-%02x-%02x-%02x][Voice %02x %02x][Codec %02x %02x %02x %02x] \
		[Radio %02x %02x %02x %02x %02x %02x][Sleep %02x %02x %02x %02x %02x %02x %02x][BtFTR %02x %02x] \
		[TxPWOffset %02x %02x %02x][CoexAdjust %02x %02x %02x %02x %02x %02x]\n",
		ucNvRamData[0], ucNvRamData[1], ucNvRamData[2], ucNvRamData[3], ucNvRamData[4], ucNvRamData[5],
		ucNvRamData[6], ucNvRamData[7], ucNvRamData[8], ucNvRamData[9], ucNvRamData[10], ucNvRamData[11],
		ucNvRamData[12], ucNvRamData[13], ucNvRamData[14], ucNvRamData[15], ucNvRamData[16], ucNvRamData[17],
		ucNvRamData[18], ucNvRamData[19], ucNvRamData[20], ucNvRamData[21], ucNvRamData[22], ucNvRamData[23], 
 		ucNvRamData[24], ucNvRamData[25], ucNvRamData[26], ucNvRamData[27], ucNvRamData[28], ucNvRamData[29],
		ucNvRamData[30], ucNvRamData[31], ucNvRamData[32], ucNvRamData[33], ucNvRamData[34], ucNvRamData[35]);

    /* Get combo chip id */
    if(bt_get_combo_id(&g_chipId) < 0)
	{
        LOG_ERR("Get combo chip id fails\n");
        goto error;
    }
	
    if(BT_InitDevice(fd, ucNvRamData, speed, speed, iUseFlowControl, uart_setup_callback) == FALSE)
    {
        LOG_ERR("Initialize BT device fails\n");        
        goto error;
    }

    LOG_WAN("mtk() success\n");
    return fd;

error:
    if(fd >= 0)
        close(fd);
    return -1;
}

int bt_restore(int fd)
{
    LOG_DBG("%s %d\n", __FILE__, __LINE__);
    
    BT_DeinitDevice(fd);
    close(fd);
    
    return 0; 
}

int write_com_port(int fd, unsigned char *buffer, unsigned long len)
{
    int nWritten = 0;
    int bytesToWrite = len;

    if (fd < 0){
        LOG_ERR("No available com port\n");
        return -EIO;
    }
    
    while (bytesToWrite > 0){
        nWritten = write(fd, buffer, bytesToWrite);
        if (nWritten < 0){
            if(errno == EINTR || errno == EAGAIN)
                break;
            else
                return -errno; // errno used for whole chip reset
        }
        bytesToWrite -= nWritten;
        buffer += nWritten;
    }

    return (len - bytesToWrite);
}

int read_com_port(int fd, unsigned char *buffer, unsigned long len)
{
    int nRead = 0;
    int bytesToRead = len;

    if (fd < 0){
        LOG_ERR("No available com port\n");
        return -EIO;
    }
    
    nRead = read(fd, buffer, bytesToRead);
    if (nRead < 0){
        if(errno == EINTR || errno == EAGAIN)
            return 0;
        else
            return -errno; // errno used for whole chip reset
    }

    return nRead;
}

int bt_send_data(int fd, unsigned char *buffer, unsigned long len)
{
    int bytesWritten = 0;
    int bytesToWrite = len;

    // Send len bytes data in buffer
    while (bytesToWrite > 0){
        bytesWritten = write_com_port(fd, buffer, bytesToWrite);
        if (bytesWritten < 0){
            return -1;
        }
        bytesToWrite -= bytesWritten;
        buffer += bytesWritten;
    }

    return 0;
}

int bt_receive_data(int fd, unsigned char *buffer, unsigned long len)
{
    int bytesRead = 0;
    int bytesToRead = len;
    
    int ret = 0;
    struct timeval tv;
    fd_set readfd;
    
    tv.tv_sec = 5;  //SECOND
    tv.tv_usec = 0;   //USECOND
    FD_ZERO(&readfd);
    
    // Hope to receive len bytes
    while (bytesToRead > 0){
    
        FD_SET(fd, &readfd);
        ret = select(fd + 1, &readfd, NULL, NULL, &tv);
        
        if (ret > 0){
            bytesRead = read_com_port(fd, buffer, bytesToRead);
            if (bytesRead < 0){
                return -1;
            }
            else{
                bytesToRead -= bytesRead;
                buffer += bytesRead;
            }
        }
        else if (ret == 0){
            LOG_ERR("Read com port timeout 5000ms!\n");
            return -1;
        }
        else if ((ret == -1) && (errno == EINTR)){
            LOG_ERR("select error EINTR\n");
        }
        else{
            LOG_ERR("select error %s(%d)!\n", strerror(errno), errno);
            return -1;
        }
    }
    
    return 0;
}
