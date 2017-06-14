
#include <string.h>

#include "bluetooth.h"
#include "bt_gatt.h"
#include "bt_gatt_client.h"
#include "bt_gatt_server.h"

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gap_if.h"
#include <sys/time.h>

extern int btut_gatt_client_init(btgatt_client_interface_t *p_interface);
extern btgatt_client_callbacks_t g_bt_gatt_client_callbacks;
extern btgatt_server_callbacks_t g_bt_gatt_server_callbacks;

static btgatt_interface_t *g_bt_gatt_interface;
static btgatt_callbacks_t g_bt_gatt_callbacks = 
{
    sizeof(btgatt_callbacks_t),
    &g_bt_gatt_client_callbacks,
    &g_bt_gatt_server_callbacks,
};

int btut_gatt_init()
{
    int ret = 0;
    BTUT_MOD gatt_mod = {0};


    BTUT_Logi("[GATT] btut_gatt_init\n");

    // Get GATT interface
    g_bt_gatt_interface = (btgatt_interface_t *) btut_gap_get_profile_interface(BT_PROFILE_GATT_ID);
    if (g_bt_gatt_interface == NULL)
    {
        BTUT_Loge("[GATT] Failed to get GATT interface\n");
        return -1;
    }

    // Init GATT interface
    if (g_bt_gatt_interface->init(&g_bt_gatt_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT] Failed to init GATT interface\n");
        return -1;
    }
    btut_gatt_client_init(g_bt_gatt_interface->client);
    btut_gatt_server_init(g_bt_gatt_interface->server);
    return ret;
}

int btut_gatt_deinit()
{
    // Deinit A2DP interface
    if (g_bt_gatt_interface != NULL)
    {
        btut_gatt_client_deinit();
        btut_gatt_server_deinit();
        g_bt_gatt_interface->cleanup();
        g_bt_gatt_interface = NULL;
    }
    return 0;
}

void genRandomNumber(uint8_t *prand, uint8_t len)
{
    int i, count = (len+3)/4;
    int random;
    while(len >= 4)
    {
        random = rand();
        memcpy(prand, &random, 4);
        len -= 4;
        prand += 4;
    }
    if(len)
    {
        random = rand();
        memcpy(prand, &random, len);
    }
}

/* 0 : success, otherwise */
int parseByteArray(const char *pSrc, uint8_t *pDest, uint8_t destlen)
{
    uint8_t oridestlen = destlen;
    char *pch = pSrc;

    BTUT_Logi("[GATT] parseByteArray(%p, %p, %d)\n", pSrc, pDest, destlen);
    
    if(pSrc && pDest && destlen)
    {
        do
        {
            BTUT_Logi("pch=%p, pDest=%p, destlen=%d\n", pch, pDest, destlen);        
            *(pDest++) = (uint8_t)strtoul(pch, &pch, 16);
            destlen--;
        }while (*(pch++) != '\0' && destlen);
    }
    return (oridestlen - destlen);
}
