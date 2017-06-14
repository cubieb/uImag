
#include <string.h>

#include "bluetooth.h"
#include "bt_gatt.h"
#include "bt_gatt_server.h"

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gap_if.h"
#include "btut_gatt_server_if.h"

static int g_server_if;
btgatt_server_interface_t *g_bt_gatt_server_interface = NULL;
char demo_char_value[BTGATT_MAX_ATTR_LEN] = "Hi!";
uint16_t demo_cccd_value = 0;
int demo_char_len = 3;
int demo_attr_handle, demo_cccd_handle;
int g_conn_id;

extern void genRandomNumber(uint8_t *prand, uint8_t len);
extern int parseByteArray(const char *pSrc, uint8_t *pDest, uint8_t destlen);

/* register_server_callback */
static void btut_gatt_server_register_server_cb(int status, int server_if, bt_uuid_t *app_uuid)
{
    btgatt_srvc_id_t srvc_id;

    BTUT_Logd("btut_gatt_server_register_client_cb(status=%d, server_if=%d)", status, server_if);
    g_server_if = server_if;
    srvc_id.is_primary = 1;
    genRandomNumber(srvc_id.id.uuid.uu, sizeof(srvc_id.id.uuid.uu));
    memcpy(srvc_id.id.uuid.uu, "\x00\x00\xab\x12\x00\x00\x10\x00\x80\x00\x00\x80\x5F\x9B\x34\xFB", 16);
    g_bt_gatt_server_interface->add_service(g_server_if, &srvc_id, 5);
}
/* connect_callback */
static void btut_gatt_server_connection_cb(int conn_id, int server_if, int connected, bt_bdaddr_t *bda)
{
    g_conn_id = conn_id;
    BTUT_Logd("btut_gatt_server_connection_cb");
}
/* service_added_callback */
static void btut_gatt_server_service_added_cb(int status, int server_if, btgatt_srvc_id_t *srvc_id, int srvc_handle)
{
    // permission, property
    // https://developer.android.com/reference/android/bluetooth/BluetoothGattCharacteristic.html
    bt_uuid_t char_uuid;
    BTUT_Logd("btut_gatt_server_service_added_cb status = %d, srvc_handle = %d", status, srvc_handle);
    memcpy(char_uuid.uu, "\x00\x00\xaa\x13\x00\x00\x10\x00\x80\x00\x00\x80\x5F\x9B\x34\xFB", 16);
    g_bt_gatt_server_interface->add_characteristic(g_server_if, srvc_handle, &char_uuid,
            2 + 8 + 16 + 32, 1 + 16);
}
/* included_service_added_callback */
static void btut_gatt_server_included_service_added_cb(int status, int server_if, int srvc_handle, int incl_srvc_handle)
{
    BTUT_Logd("btut_gatt_server_included_service_added_cb");
}
/* characteristic_added_callback */
static void btut_gatt_server_characteristic_added_cb(int status, int server_if, bt_uuid_t *uuid, int srvc_handle, int char_handle)
{
    bt_uuid_t cccd;
    memcpy(cccd.uu, "\xFB\x34\x9B\x5F\x80\x00\x00\x80\x00\x10\x00\x00\x02\x29\x00\x00", 16);
    BTUT_Logd("btut_gatt_server_characteristic_added_cb status: %d", status);
    demo_attr_handle = char_handle;
    g_bt_gatt_server_interface->add_descriptor(server_if, srvc_handle,
                                  &cccd, 1 + 16);
    g_bt_gatt_server_interface->start_service(server_if, srvc_handle, 3);
}
/* descriptor_added_callback */
static void btut_gatt_server_descriptor_added_cb(int status, int server_if,
        bt_uuid_t *uuid, int srvc_handle,
        int descr_handle)
{
    demo_cccd_handle = descr_handle;
    BTUT_Logd("btut_gatt_server_descriptor_added_cb");
}
/* service_started_callback */
static void btut_gatt_server_service_started_cb(int status, int server_if, int srvc_handle)
{
    BTUT_Logd("btut_gatt_server_service_started_cb");
}
/* service_stopped_callback */
static void btut_gatt_server_service_stopped_cb(int status, int server_if, int srvc_handle)
{
    BTUT_Logd("btut_gatt_server_service_stopped_cb");
}
/* service_deleted_callback */
static void btut_gatt_server_service_deleted_cb(int status, int server_if, int srvc_handle)
{
    BTUT_Logd("btut_gatt_server_service_deleted_cb");
}
/* service_deleted_callback */
static void btut_gatt_server_request_read_cb(int conn_id, int trans_id,
        bt_bdaddr_t *bda, int attr_handle,
        int offset, bool is_long)
{
    btgatt_response_t response;
    response.handle = attr_handle;
    response.attr_value.handle = attr_handle;
    response.attr_value.offset = 0;
    response.attr_value.auth_req = 0;

    if (attr_handle == demo_attr_handle) {
        response.attr_value.len = demo_char_len;
        strncpy(response.attr_value.value, demo_char_value, demo_char_len);
    } else if (attr_handle == demo_cccd_handle) {
        response.attr_value.len = 2;
        strncpy(response.attr_value.value, &demo_cccd_value, 2);
    }
    
    BTUT_Logd("btut_gatt_server_request_read_cb");
    BTUT_Logd("attr_handle %d\n", attr_handle);
    g_bt_gatt_server_interface->send_response(conn_id, trans_id,
            BT_STATUS_SUCCESS, &response);
}
/* request_write_callback */
static void btut_gatt_server_request_write_cb(int conn_id, int trans_id,
        bt_bdaddr_t *bda, int attr_handle,
        int offset, int length,
        bool need_rsp, bool is_prep,
        uint8_t* value)
{
    if (attr_handle == demo_attr_handle) {
        demo_char_len = length;
        strncpy(demo_char_value, value, demo_char_len);
        BTUT_Logd("btut_gatt_server_request_write_cb");
        if (need_rsp) {
            btgatt_response_t response;
            response.handle = attr_handle;
            response.attr_value.handle = attr_handle;
            response.attr_value.offset = 0;
            response.attr_value.len = demo_char_len;
            response.attr_value.auth_req = 0;
            strncpy(response.attr_value.value, demo_char_value, demo_char_len);
            g_bt_gatt_server_interface->send_response(conn_id, trans_id,
                    BT_STATUS_SUCCESS, &response);
        }
    } else if (attr_handle == demo_cccd_handle) {
        strncpy(&demo_cccd_value, value, 2);
        BTUT_Logd("btut_gatt_server_request_write_cb");
        if (need_rsp) {
            btgatt_response_t response;
            response.handle = attr_handle;
            response.attr_value.handle = attr_handle;
            response.attr_value.offset = 0;
            response.attr_value.len = 2;
            response.attr_value.auth_req = 0;
            strncpy(response.attr_value.value, &demo_cccd_value, 2);
            g_bt_gatt_server_interface->send_response(conn_id, trans_id,
                    BT_STATUS_SUCCESS, &response);
        }
    }
}
/* request_exec_write_callback */
static void btut_gatt_server_request_exec_write_cb(int conn_id, int trans_id,
        bt_bdaddr_t *bda, int exec_write)
{
    BTUT_Logd("btut_gatt_server_request_exec_write_cb");
}
/* response_confirmation_callback */
static void btut_gatt_server_response_confirmation_cb(int status, int handle)
{
    BTUT_Logd("btut_gatt_server_response_confirmation_cb");
}

/**
 * Callback confirming that a notification or indication has been sent
 * to a remote device.
 */
static void btut_gatt_server_indication_sent_cb(int conn_id, int status)
{
    BTUT_Logd("btut_gatt_server_indication_sent_cb");
}

/**
 * Callback notifying an application that a remote device connection is currently congested
 * and cannot receive any more data. An application should avoid sending more data until
 * a further callback is received indicating the congestion status has been cleared.
 */
static void btut_gatt_server_congestion_cb(int conn_id, bool congested)
{
}

btgatt_server_callbacks_t g_bt_gatt_server_callbacks =
{
    btut_gatt_server_register_server_cb,
    btut_gatt_server_connection_cb,
    btut_gatt_server_service_added_cb,
    btut_gatt_server_included_service_added_cb,
    btut_gatt_server_characteristic_added_cb,
    btut_gatt_server_descriptor_added_cb,
    btut_gatt_server_service_started_cb,
    btut_gatt_server_service_stopped_cb,
    btut_gatt_server_service_deleted_cb,
    btut_gatt_server_request_read_cb,
    btut_gatt_server_request_write_cb,
    btut_gatt_server_request_exec_write_cb,
    btut_gatt_server_response_confirmation_cb,
    btut_gatt_server_indication_sent_cb,
    btut_gatt_server_congestion_cb,
};

/***************************************************
 *   Cli command interface
 ***************************************************/
static int btut_gatt_server_register_handler(int argc, char *argv[])
{
    bt_status_t ret;
    bt_uuid_t app_uuid;

    BTUT_Logd("btut_gatt_server_register_handler : argc=%d", argc);

    if(!g_bt_gatt_server_interface)
    {
        BTUT_Loge("[GATT cserver] is not initialized\n");
        goto error;
    }
    genRandomNumber(app_uuid.uu, sizeof(app_uuid.uu));
    if((ret = g_bt_gatt_server_interface->register_server(&app_uuid)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT cserver] register_client failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT cserver]: register\n");
error:
    return -1;

}

static int btut_gatt_server_deregister_handler(int argc, char *argv[])
{
    bt_status_t ret;
    int server_if = 0;

    BTUT_Logd("btut_gatt_server_deregister_handler : argc=%d", argc);

    if (argc < 1){
        BTUT_Logw("[GATT cserver] invalid parameter number\n");
        goto usage;
    }
    if(sscanf(argv[0], "%d", &server_if) < 1)
    {
        BTUT_Logw("[GATT cserver] invalid parameter format\n");
        goto usage;
    }
    if(!g_bt_gatt_server_interface)
    {
        BTUT_Loge("[GATT cserver] is not initialized\n");
        goto error;
    }
    if((ret = g_bt_gatt_server_interface->unregister_server(server_if)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT cserver] unregister_server failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT cserver]: deregister [server_if(\%d)]\n");
error:
    return -1;
}

static int btut_gatt_server_notify_handler(int argc, char *argv[])
{
    bt_status_t ret;
    bt_uuid_t app_uuid;
    char notify_value[1];

    BTUT_Logd("btut_gatt_server_notify_handler : argc=%d", argc);

    if(!g_bt_gatt_server_interface)
    {
        BTUT_Loge("[GATT server] is not initialized\n");
        goto error;
    }

    notify_value[0] = atoi(argv[0]);
    BTUT_Logd("notify %d", notify_value[0]);
    if((ret = g_bt_gatt_server_interface->send_indication(g_server_if, demo_attr_handle, g_conn_id,
                    1, 1, notify_value)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT server] notify failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT server]: notify dec_num\n");
error:
    return -1;

}

static BTUT_CLI bt_gatt_server_cli_commands[] =
{
    {"register", btut_gatt_server_register_handler,
        " = register"},
    {"deregister", btut_gatt_server_deregister_handler,
        " = deregister"},
    {"notify", btut_gatt_server_notify_handler,
        " = send notify"},
    {NULL, NULL, NULL},
};

// For handling incoming commands from CLI.
int btut_gatt_server_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_gatt_server_cli_commands;

    BTUT_Logd("[GATT server] argc: %d, argv[0]: %s\n", argc, argv[0]);

    while (cmd->cmd)
    {
        if (!strcmp(cmd->cmd, argv[0]))
        {
            match = cmd;
            count = 1;
            break;
        }
        cmd++;
    }

    if (count == 0)
    {
        BTUT_Logd("[GATT server] Unknown command '%s'\n", argv[0]);

        btut_print_cmd_help(CMD_KEY_GATT_SERVER, bt_gatt_server_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_gatt_server_init(btgatt_server_interface_t *p_interface)
{
    int ret = 0;
    BTUT_MOD gatt_server_mod = {0};

    // init GATT cserver interface
    g_bt_gatt_server_interface = p_interface;
    if (g_bt_gatt_server_interface == NULL)
    {
        BTUT_Loge("[GATT server] Failed to get GATT cserver interface\n");
        return -1;
    }

    // Register command to CLI
    gatt_server_mod.mod_id = BTUT_MOD_GATT_SERVER;
    strncpy(gatt_server_mod.cmd_key, CMD_KEY_GATT_SERVER, sizeof(gatt_server_mod.cmd_key));
    gatt_server_mod.cmd_handler = btut_gatt_server_cmd_handler;
    gatt_server_mod.cmd_tbl = bt_gatt_server_cli_commands;

    ret = btut_register_mod(&gatt_server_mod);
    BTUT_Logd("[GATT cserver] btut_register_mod() returns: %d\n", ret);

    return ret;
}

int btut_gatt_server_deinit()
{
    g_bt_gatt_server_interface = NULL;
}


