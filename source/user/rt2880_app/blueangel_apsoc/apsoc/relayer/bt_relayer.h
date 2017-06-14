#ifndef __BT_HW_TEST_H__
#define __BT_HW_TEST_H__

#include <linux/autoconf.h>
#include <stdio.h>

typedef unsigned long DWORD;
typedef unsigned long* PDWORD;
typedef unsigned long* LPDWORD;
typedef unsigned short USHORT;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned char BOOL;
typedef unsigned long HANDLE;
typedef void VOID;
typedef void* LPCVOID;
typedef void* LPVOID;
typedef void* LPOVERLAPPED;
typedef unsigned char* PUCHAR;
typedef unsigned char* PBYTE;
typedef unsigned char* LPBYTE;

#define TRUE           1
#define FALSE          0

#define BT_RELAYER_DEBUG  1
#define ERR(f, ...)       printf("%s: " f, __func__, ##__VA_ARGS__)
#define WAN(f, ...)       printf("%s: " f, __func__, ##__VA_ARGS__)
#if BT_RELAYER_DEBUG
#define DBG(f, ...)       printf("%s: " f, __func__, ##__VA_ARGS__)
#define TRC(f)            printf("%s #%d", __func__, __LINE__)
#else
#define DBG(...)          ((void)0)
#define TRC(f)            ((void)0)
#endif

#if defined(CONFIG_DEFAULTS_RALINK_MT7628) || defined(CONFIG_DEFAULTS_RALINK_MT7621)
#define SERIAL_DEVICE_NAME	"/dev/ttyS"
#elif defined(CONFIG_ARCH_MT7623)
#define SERIAL_DEVICE_NAME	"/dev/ttyMT"
#else
#define SERIAL_DEVICE_NAME	"/dev/ttyS"
#endif
#define SERIAL_PORT_NUM		0
#define SERIAL_BAUDRATE		115200


int init_serial(int port, int speed);
BOOL RELAYER_start(int serial_port, int serial_speed);
void RELAYER_exit();

#endif
