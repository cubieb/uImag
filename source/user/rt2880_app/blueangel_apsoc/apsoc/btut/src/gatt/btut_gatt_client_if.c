#include <string.h>

#include "bluetooth.h"
#include "bt_gatt.h"
#include "bt_gatt_client.h"

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gap_if.h"
#include "btut_gatt_client_if.h"


extern void genRandomNumber(uint8_t *prand, uint8_t len);
extern int parseByteArray(const char *pSrc, uint8_t *pDest, uint8_t destlen);

static g_client_if;

/* register_client_callback */
static void btut_gatt_client_register_client_cb(int status, int client_if, bt_uuid_t *app_uuid)
{
    BTUT_Logd("btut_gatt_client_register_client_cb(status=%d, client_if=%d)\n", status, client_if);
    if(status == 0)
    {
        g_client_if = client_if;
    }
    else
    {
    }

}
/* scan_result_callback */
static void btut_gatt_client_scan_result_cb(bt_bdaddr_t* bda, int rssi, uint8_t* adv_data)
{
}
/* connect_callback */
static void btut_gatt_client_open_cb(int conn_id, int status, int client_if, bt_bdaddr_t* bda)
{
}
/* disconnect_callback */
static void btut_gatt_client_close_cb(int conn_id, int status, int client_if, bt_bdaddr_t* bda)
{
}
/* search_complete_callback */
static void btut_gatt_client_search_complete_cb(int conn_id, int status)
{
}
/* search_result_callback */
static void btut_gatt_client_search_result_cb(int conn_id, btgatt_srvc_id_t *srvc_id)
{
}
/* get_characteristic_callback */
static void btut_gatt_client_get_characteristic_cb(int conn_id, int status,
        btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
        int char_prop)
{
}
/* get_descriptor_callback */
static void btut_gatt_client_get_descriptor_cb(int conn_id, int status,
        btgatt_srvc_id_t *srvc_id, btgatt_gatt_id_t *char_id,
        btgatt_gatt_id_t *descr_id)
{
}
/* get_included_service_callback */
static void btut_gatt_client_get_included_service_cb(int conn_id, int status,
        btgatt_srvc_id_t *srvc_id,
        btgatt_srvc_id_t *incl_srvc_id)
{
}
/* register_for_notification_callback */
static void btut_gatt_client_register_for_notification_cb(int conn_id,
        int registered, int status,
        btgatt_srvc_id_t *srvc_id,
        btgatt_gatt_id_t *char_id)
{
}
/* notify_callback */
static void btut_gatt_client_notify_cb(int conn_id, btgatt_notify_params_t *p_data)
{
}
/* read_characteristic_callback */
static void btut_gatt_client_read_characteristic_cb(int conn_id, int status,
        btgatt_read_params_t *p_data)
{
}
/* write_characteristic_callback */
static void btut_gatt_client_write_characteristic_cb(int conn_id, int status,
        btgatt_write_params_t *p_data)
{
}
/* read_descriptor_callback */
static void btut_gatt_client_read_descriptor_cb(int conn_id, int status,
        btgatt_read_params_t *p_data)
{
}
/* write_descriptor_callback */
static void btut_gatt_client_write_descriptor_cb(int conn_id, int status,
        btgatt_write_params_t *p_data)
{
}
/* execute_write_callback */
static void btut_gatt_client_execute_write_cb(int conn_id, int status)
{
}
/* read_remote_rssi_callback */
static void btut_gatt_client_read_remote_rssi_cb(int client_if, bt_bdaddr_t* bda,
        int rssi, int status)
{
}
/* listen_callback */
static void btut_gatt_client_listen_cb(int status, int client_if)
{
}

/** Callback invoked when the MTU for a given connection changes */
static void btut_gatt_client_configure_mtu_cb(int conn_id, int status, int mtu)
{
}

/** Callback invoked when a scan filter configuration command has completed */
static void btut_gatt_client_scan_filter_cfg_cb(int action, int client_if, int status, int filt_type,
        int avbl_space)
{
}

/** Callback invoked when scan param has been added, cleared, or deleted */
static void btut_gatt_client_scan_filter_param_cb(int action, int client_if, int status,
        int avbl_space)
{
}

/** Callback invoked when a scan filter configuration command has completed */
static void btut_gatt_client_scan_filter_status_cb(int enable, int client_if, int status)
{
}

/** Callback invoked when multi-adv enable operation has completed */
static void btut_gatt_client_multi_adv_enable_cb(int client_if, int status)
{
}

/** Callback invoked when multi-adv param update operation has completed */
static void btut_gatt_client_multi_adv_update_cb(int client_if, int status)
{
}

/** Callback invoked when multi-adv instance data set operation has completed */
static void btut_gatt_client_multi_adv_data_cb(int client_if, int status)
{
}

/** Callback invoked when multi-adv disable operation has completed */
static void btut_gatt_client_multi_adv_disable_cb(int client_if, int status)
{
}

/**
 * Callback notifying an application that a remote device connection is currently congested
 * and cannot receive any more data. An application should avoid sending more data until
 * a further callback is received indicating the congestion status has been cleared.
 */
static void btut_gatt_client_congestion_cb(int conn_id, bool congested)
{
}

/** Callback invoked when batchscan storage config operation has completed */
static void btut_gatt_client_batchscan_cfg_storage_cb(int client_if, int status)
{
}

/** Callback invoked when batchscan enable / disable operation has completed */
static void btut_gatt_client_batchscan_enb_disable_cb(int action, int client_if, int status)
{
}

/** Callback invoked when batchscan reports are obtained */
static void btut_gatt_client_batchscan_reports_cb(int client_if, int status, int report_format,
        int num_records, int data_len, uint8_t* rep_data)
{
}

/** Callback invoked when batchscan storage threshold limit is crossed */
static void btut_gatt_client_batchscan_threshold_cb(int client_if)
{
}

/** Track ADV VSE callback invoked when tracked device is found or lost */
static void btut_gatt_client_track_adv_event_cb(int client_if, int filt_index, int addr_type,
        bt_bdaddr_t* bda, int adv_state)
{
}

btgatt_client_interface_t *g_bt_gatt_client_interface = NULL;
btgatt_client_callbacks_t g_bt_gatt_client_callbacks =
{
    btut_gatt_client_register_client_cb,
    btut_gatt_client_scan_result_cb,
    btut_gatt_client_open_cb,
    btut_gatt_client_close_cb,
    btut_gatt_client_search_complete_cb,
    btut_gatt_client_search_result_cb,
    btut_gatt_client_get_characteristic_cb,
    btut_gatt_client_get_descriptor_cb,
    btut_gatt_client_get_included_service_cb,
    btut_gatt_client_register_for_notification_cb,
    btut_gatt_client_notify_cb,
    btut_gatt_client_read_characteristic_cb,
    btut_gatt_client_write_characteristic_cb,
    btut_gatt_client_read_descriptor_cb,
    btut_gatt_client_write_descriptor_cb,
    btut_gatt_client_execute_write_cb,
    btut_gatt_client_read_remote_rssi_cb,
    btut_gatt_client_listen_cb,
    btut_gatt_client_configure_mtu_cb,
    btut_gatt_client_scan_filter_cfg_cb,
    btut_gatt_client_scan_filter_param_cb,
    btut_gatt_client_scan_filter_status_cb,
    btut_gatt_client_multi_adv_enable_cb,
    btut_gatt_client_multi_adv_update_cb,
    btut_gatt_client_multi_adv_data_cb,
    btut_gatt_client_multi_adv_disable_cb,
    btut_gatt_client_congestion_cb,
    btut_gatt_client_batchscan_cfg_storage_cb,
    btut_gatt_client_batchscan_enb_disable_cb,
    btut_gatt_client_batchscan_reports_cb,
    btut_gatt_client_batchscan_threshold_cb,
    btut_gatt_client_track_adv_event_cb,
};

/***************************************************
 *   Cli command interface
 ***************************************************/

static int btut_gatt_client_register_handler(int argc, char *argv[])
{
    bt_status_t ret;
    bt_uuid_t app_uuid;

    BTUT_Logd("btut_gatt_client_register_handler : argc=%d\n", argc);

    if(!g_bt_gatt_client_interface)
    {
        BTUT_Loge("[GATT client] is not initialized\n");
        goto error;
    }
    genRandomNumber(app_uuid.uu, sizeof(app_uuid.uu));
    if((ret = g_bt_gatt_client_interface->register_client(&app_uuid)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT client] register_client failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT client]: register\n");
error:
    return -1;

}

static int btut_gatt_client_deregister_handler(int argc, char *argv[])
{
    bt_status_t ret;
    int client_if = 0;

    BTUT_Logd("btut_gatt_client_deregister_handler : argc=%d\n", argc);

    if (argc < 1){
        BTUT_Logw("[GATT client] invalid parameter number\n");
        goto usage;
    }
    if(sscanf(argv[0], "%d", &client_if) < 1)
    {
        BTUT_Logw("[GATT client] invalid parameter format\n");
        goto usage;
    }
    if(!g_bt_gatt_client_interface)
    {
        BTUT_Loge("[GATT client] is not initialized\n");
        goto error;
    }
    if((ret = g_bt_gatt_client_interface->unregister_client(client_if)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT client] unregister_client failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT client]: deregister [client_if(\%d)]\n");
error:
    return -1;
}

static int btut_gatt_client_advertise_handler(int argc, char *argv[])
{
    bt_status_t ret;
    int client_if = 0;
    int set_scan_rsp, include_name, include_txpower, min_interval, max_interval, appearance;
    int manufacturer_len = 0, service_data_len = 0, service_uuid_len = 0;
    char manufacturer_data[31] = {0};
    char service_data[31] = {0};
    char service_uuid[16] = {0};

    BTUT_Logd("btut_gatt_client_advertise_handler : argc=%d\n", argc);

    if (argc < 10){
        BTUT_Logw("[GATT client] invalid parameter number\n");
        goto usage;
    }
    client_if = strtoul(argv[0], NULL, 16);
    set_scan_rsp = strtoul(argv[1], NULL, 16);
    include_name = strtoul(argv[2], NULL, 16);
    include_txpower = strtoul(argv[3], NULL, 16);
    min_interval = strtoul(argv[4], NULL, 16);
    max_interval = strtoul(argv[5], NULL, 16);
    appearance = strtoul(argv[6], NULL, 16);
    manufacturer_len = parseByteArray(argv[7], manufacturer_data, sizeof(manufacturer_data));
    service_data_len = parseByteArray(argv[8], service_data, sizeof(service_data));
    service_uuid_len = parseByteArray(argv[9], service_uuid, sizeof(service_uuid));
    if(!g_bt_gatt_client_interface)
    {
        BTUT_Loge("[GATT client] is not initialized\n");
        goto error;
    }
    if((ret = g_bt_gatt_client_interface->set_adv_data(client_if,
                    set_scan_rsp ? true : false,
                    include_name ? true : false,
                    include_txpower ? true : false,
                    min_interval,
                    max_interval,
                    appearance,
                    (uint16_t)manufacturer_len, manufacturer_data,
                    (uint16_t)service_data_len, service_data,
                    (uint16_t)service_uuid_len, service_uuid)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT client] advertise failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT client]: advertise"
            "\t["
            "\t\client_if(\%d)\n"
            "\t\tset_scan_rsp(\%d)\n"
            "\t\tinclude_name(\%d)\n"
            "\t\tinclude_txpower(\%d)\n"
            "\t\tmin_interval(\%d)\n"
            "\t\tmax_interval(\%d)\n"
            "\t\tappearance(\%d)\n"
            "\t\tmanufacturer_data(byte array)\n"
            "\t\tservice_data(byte array)\n"
            "\t\tservice_uuid(byte array)\n"
            "\t]\n"
            );
error:
    return -1;

}

static int btut_gatt_client_listen_handler(int argc, char *argv[])
{
    bt_status_t ret;
    int client_if = 0;
    int start;

    BTUT_Logd("btut_gatt_client_listen_handler : argc=%d\n", argc);

    if (argc < 2){
        BTUT_Logw("[GATT client] invalid parameter number\n");
        goto usage;
    }
    client_if = strtoul(argv[0], NULL, 16);
    start = strtoul(argv[1], NULL, 16);
    if(!g_bt_gatt_client_interface)
    {
        BTUT_Loge("[GATT client] is not initialized\n");
        goto error;
    }
    if((ret = g_bt_gatt_client_interface->listen(client_if, start ? true : false)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT client] listen failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT client]: listen [client_if(\%d) start(\%d)]\n");
error:
    return -1;
}

static int btut_gatt_client_advertise_default_handler(int argc, char *argv[])
{
    bt_status_t ret;
    int client_if = 0;
    int set_scan_rsp, include_name, include_txpower, min_interval, max_interval, appearance, adv_type;
    int manufacturer_len = 0, service_data_len = 0, service_uuid_len = 0;
    char manufacturer_data[31] = {0};
    char service_data[31] = {0};
    char service_uuid[16] = {0};
    int chnl_map, tx_power, timeout_s;
    memcpy(service_uuid, "\x00\x00\xaa\x12\x00\x00\x10\x00\x80\x00\x00\x80\x5F\x9B\x34\xFB", 16);
    memcpy(manufacturer_data, "\x00\x01\x02\x03", 4);
    memcpy(service_data, "\x00\x01\x02\x03", 4);

    BTUT_Logd("btut_gatt_client_advertise_default_handler : argc=%d\n", argc);
    client_if = g_client_if;
    min_interval = 0x00a0;
    max_interval = 0x00aa;
    adv_type = 0x0;
    chnl_map = 0x07;
    tx_power = 0x02;
    timeout_s = 0x0;
    if(!g_bt_gatt_client_interface)
    {
        BTUT_Loge("[GATT client] is not initialized\n");
        goto error;
    }
    // enable adv
    if((ret = g_bt_gatt_client_interface->multi_adv_enable(
                    client_if, min_interval, max_interval,
                    adv_type, chnl_map, tx_power
                    )) != BT_STATUS_SUCCESS) {
        BTUT_Loge("[GATT client] multi advertise enable failed %d\n", ret);
        goto error;
    }
    // set adv data
    set_scan_rsp = 0x0;
    include_name = 0x01;
    include_txpower = 0x01;
    appearance = 0x0;
    manufacturer_len = 0x0;
    service_data_len = 0x0;
    service_uuid_len = 0x10;
    if ((ret = g_bt_gatt_client_interface->multi_adv_set_inst_data(
                    client_if, set_scan_rsp, include_name,
                    include_txpower, appearance, manufacturer_len,
                    manufacturer_data, service_data_len,
                    service_data, service_uuid_len, service_uuid))
            != BT_STATUS_SUCCESS) {
        BTUT_Loge("[GATT client] advertise set data fail %d\n", ret);
        goto error;
    }
    // set rsp data
    set_scan_rsp = 0x1;
    include_name = 0x00;
    include_txpower = 0x00;
    appearance = 0x0;
    manufacturer_len = 0x4;
    service_data_len = 0x4;
    service_uuid_len = 0x10;
    if ((ret = g_bt_gatt_client_interface->multi_adv_set_inst_data(
                    client_if, set_scan_rsp, include_name,
                    include_txpower, appearance, manufacturer_len,
                    manufacturer_data, service_data_len,
                    service_data, service_uuid_len, service_uuid))
            != BT_STATUS_SUCCESS) {
        BTUT_Loge("[GATT client] advertise set rsp fail %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT client]: advertise"
            "\t["
            "\t\client_if(\%d)\n"
            "\t\tset_scan_rsp(\%d)\n"
            "\t\tinclude_name(\%d)\n"
            "\t\tinclude_txpower(\%d)\n"
            "\t\tmin_interval(\%d)\n"
            "\t\tmax_interval(\%d)\n"
            "\t\tappearance(\%d)\n"
            "\t\tmanufacturer_data(byte array)\n"
            "\t\tservice_data(byte array)\n"
            "\t\tservice_uuid(byte array)\n"
            "\t]\n"
            );
error:
    return -1;

}

static int btut_gatt_client_listen_defautlf_handler(int argc, char *argv[])
{
    bt_status_t ret;
    int client_if = 0;
    int start;

    BTUT_Logd("btut_gatt_client_listen_defautlf_handler: argc=%d\n", argc);

    if(!g_bt_gatt_client_interface)
    {
        BTUT_Loge("[GATT client] is not initialized\n");
        goto error;
    }
    if((ret = g_bt_gatt_client_interface->listen(g_client_if, true)) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[GATT client] listen failed %d\n", ret);
        goto error;
    }
    return 0;
usage:
    BTUT_Logi("Usage :\n");
    BTUT_Logi("\t[GATT client]: listen [client_if(\%d) start(\%d)]\n");
error:
    return -1;
}

static BTUT_CLI bt_gatt_client_cli_commands[] =
{
    {"register", btut_gatt_client_register_handler,
        " = register"},
    {"deregister", btut_gatt_client_deregister_handler,
        " = deregister"},
    {"advertise", btut_gatt_client_advertise_handler,
        " = advertise"},
    {"listen", btut_gatt_client_listen_handler,
        " = listen"},
    {"advertise_default", btut_gatt_client_advertise_default_handler,
        " = advertise"},
    {"listen_default", btut_gatt_client_listen_defautlf_handler,
        " = listen"},
    {NULL, NULL, NULL},
};

// For handling incoming commands from CLI.
int btut_gatt_client_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;
    int ret = 0;
    int count;

    count = 0;
    cmd = bt_gatt_client_cli_commands;

    BTUT_Logd("[GATT client] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTUT_Logd("Unknown command '%s'\n", argv[0]);

        btut_print_cmd_help(CMD_KEY_GATT_CLIENT, bt_gatt_client_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

int btut_gatt_client_init(btgatt_client_interface_t *p_interface)
{
    int ret = 0;
    BTUT_MOD gatt_client_mod = {0};
    BTUT_Logi("[GATT client] btut_gatt_client_init\n");
    // init GATT client interface
    g_bt_gatt_client_interface = p_interface;
    if (g_bt_gatt_client_interface == NULL)
    {
        BTUT_Loge("[GATT_CLIENT] Failed to get GATT client interface\n");
        return -1;
    }

    // Register command to CLI
    gatt_client_mod.mod_id = BTUT_MOD_GATT_CLIENT;
    strncpy(gatt_client_mod.cmd_key, CMD_KEY_GATT_CLIENT, sizeof(gatt_client_mod.cmd_key));
    gatt_client_mod.cmd_handler = btut_gatt_client_cmd_handler;
    gatt_client_mod.cmd_tbl = bt_gatt_client_cli_commands;

    ret = btut_register_mod(&gatt_client_mod);
    BTUT_Logd("[GATT client] btut_register_mod() returns: %d\n", ret);

    return ret;
}

int btut_gatt_client_deinit()
{
    g_bt_gatt_client_interface = NULL;
}

