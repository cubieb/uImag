
#include <stdlib.h>
#include <string.h>

#include "btut_cli.h"
#include "btut_debug.h"

#include "btut_gap_if.h"
#include "btut_a2dp_snk_if.h"
//#include "btut_a2dp_src_if.h"
//#include "btut_hid_if.h"

#include "bluetooth.h"

extern int open_bluetooth_stack (const struct hw_module_t* module, char const* name,
    struct hw_device_t** abstraction);

static int btut_gap_set_scan_mode (int mode);
static int btut_gap_enable_handler(int argc, char *argv[]);
static int btut_gap_disable_handler(int argc, char *argv[]);
static int btut_gap_get_adapter_properties_handler(int argc, char *argv[]);
static int btut_gap_get_adapter_property_handler(int argc, char *argv[]);
static int btut_gap_set_adapter_property_handler(int argc, char *argv[]);
static int btut_gap_set_device_name_handler(int argc, char *argv[]);
static int btut_gap_set_scan_mode_handler(int argc, char *argv[]);
static int btut_gap_set_discovery_timeout_handler(int argc, char *argv[]);
static int btut_gap_get_remote_device_properties_handler(int argc, char *argv[]);
static int btut_gap_get_remote_device_property_handler(int argc, char *argv[]);
static int btut_gap_set_remote_device_property_handler(int argc, char *argv[]);
static int btut_gap_get_remote_service_record_handler(int argc, char *argv[]);
static int btut_gap_get_remote_services_handler(int argc, char *argv[]);
static int btut_gap_start_discovery_handler(int argc, char *argv[]);
static int btut_gap_cancel_discovery_handler(int argc, char *argv[]);
static int btut_gap_create_bond_handler(int argc, char *argv[]);
static int btut_gap_remove_bond_handler(int argc, char *argv[]);
static int btut_gap_cancel_bond_handler(int argc, char *argv[]);
static int btut_gap_dut_mode_configure_handler(int argc, char *argv[]);
static int btut_gap_le_test_mode_handler(int argc, char *argv[]);
static int btut_gap_config_hci_snoop_log_handler(int argc, char *argv[]);
static void btut_gap_show_properties_debug(int num_properties, bt_property_t *properties);
static void btut_gap_show_properties_info(int num_properties, bt_property_t *properties);
static void btut_gap_btaddr_stoh(char *btaddr_s, bt_bdaddr_t *bdaddr_h);

// Callback functions declaration
static void btut_gap_state_changed_cb(bt_state_t state);

static void btut_gap_properties_cb(bt_status_t status,
                                       int num_properties,
                                       bt_property_t *properties);

static void btut_gap_remote_device_properties_cb(bt_status_t status,
                                                        bt_bdaddr_t *bd_addr,
                                                        int num_properties,
                                                        bt_property_t *properties);

static void btut_gap_device_found_cb(int num_properties,
                                           bt_property_t *properties);

static void btut_gap_discovery_state_changed_cb(bt_discovery_state_t state);

static void btut_gap_pin_request_cb(bt_bdaddr_t *remote_bd_addr,
                                         bt_bdname_t *bd_name, uint32_t cod);


static void btut_gap_ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
                                                bt_bdname_t *bd_name, uint32_t cod,
                                                bt_ssp_variant_t pairing_variant,
                                                uint32_t pass_key);

static void btut_gap_bond_state_changed_cb(bt_status_t status,
                                                         bt_bdaddr_t *remote_bd_addr,
                                                         bt_bond_state_t state);

static void btut_gap_acl_state_changed_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr,
                                            bt_acl_state_t state);

static void btut_gap_reset_ind_cb(void);

static void btut_gap_thread_event_cb(bt_cb_thread_evt evt);

static void btut_gap_dut_mode_recv_cb(uint16_t opcode, uint8_t *buf, uint8_t len);

static void btut_gap_le_test_mode_cb(bt_status_t status, uint16_t num_packets);

int default_scan_mode = 0;

static bluetooth_device_t *g_bt_device;
static bt_interface_t *g_bt_interface;
static bt_callbacks_t g_bt_callbacks = 
{
    sizeof(bt_callbacks_t),
    btut_gap_state_changed_cb,
    btut_gap_properties_cb,
    btut_gap_remote_device_properties_cb,
    btut_gap_device_found_cb,
    btut_gap_discovery_state_changed_cb,
    btut_gap_pin_request_cb,
    btut_gap_ssp_request_cb,
    btut_gap_bond_state_changed_cb,
    btut_gap_acl_state_changed_cb,
    btut_gap_thread_event_cb,
    btut_gap_reset_ind_cb,
    btut_gap_dut_mode_recv_cb,
    btut_gap_le_test_mode_cb,
};

static BTUT_CLI bt_gap_cli_commands[] = {
    { "set_device_name", btut_gap_set_device_name_handler,
      "= change bluetooth device name" },
    { "set_scan_mode", btut_gap_set_scan_mode_handler,
      "= change bluetooth scan mode" },
    { "start_discovery", btut_gap_start_discovery_handler,
      "= search device" },
    { "cancel_discovery", btut_gap_cancel_discovery_handler,
      "= stop search device" },
    { "create_bond", btut_gap_create_bond_handler,
      "= start pairing" },
    { "remove_bond", btut_gap_remove_bond_handler,
      "= remove pairing" },
    { "cancel_bond", btut_gap_cancel_bond_handler,
      "= cancel pairing" },
    { "get_adapter_properties", btut_gap_get_adapter_properties_handler,
      "= get bluetooth device properties" },
    { "config_hci_snoop_log", btut_gap_config_hci_snoop_log_handler,
      "= enable virtual sniffer" },      
    { NULL, NULL, NULL }
};

static int btut_gap_enable_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    g_bt_interface->enable();

    return 0;
}

static int btut_gap_disable_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    g_bt_interface->disable();

    return 0;
}

static int btut_gap_get_adapter_properties_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    g_bt_interface->get_adapter_properties();

    return 0;
}

static int btut_gap_get_adapter_property_handler(int argc, char *argv[])
{
    bt_property_type_t num;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc < 1)
    {        
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP get_adapter_property [1 - 12]\n");

        return -1;
    }
    
    num = (bt_property_type_t)atoi(argv[0]);

    g_bt_interface->get_adapter_property(num);

    return 0;
}

static int btut_gap_set_adapter_property_handler(int argc, char *argv[])
{
    int num;
    unsigned int disc_timeout;
    char *ptr;    
    bt_property_t property;
    bt_property_t *property_p;    
    bt_scan_mode_t scan_mode;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc < 2)
    {        
        BTUT_Logi("Usage is :\n");        
        BTUT_Logi("  GAP set_adapter_property 1 [BT_NAME]\n");        
        BTUT_Logi("  GAP set_adapter_property 7 [scan mode, 0:none, 1:connectable, 2:connectable_discoverable]\n");
        BTUT_Logi("  GAP set_adapter_property 9 [discovery timeout (decimal number) (unit:second)]\n");

        return -1;
    }

    num = atoi(argv[0]);
    property_p = &property;
    ptr = argv[1];

    switch (num)
    {
    /* 1 */
    case BT_PROPERTY_BDNAME:        
        property_p->type = BT_PROPERTY_BDNAME;
        property_p->len = strlen(ptr);
        property_p->val = ptr;
        
        break;

    /* 7 */
    case BT_PROPERTY_ADAPTER_SCAN_MODE:        
        scan_mode = (bt_scan_mode_t)atoi(ptr);;

        property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
        property_p->len = sizeof(bt_scan_mode_t);
        property_p->val = (void*)&scan_mode;

        break;
    /* 9 */
    case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
        disc_timeout = atoi(ptr);
        
        property_p->type = BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT;
        property_p->len = sizeof(unsigned int);
        property_p->val = &disc_timeout;

        break;
    default:
        BTUT_Logi("We don't support the format.\n");
        break;
    }

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_set_device_name_handler(int argc, char *argv[])
{
    bt_property_t property;
    bt_property_t *property_p;    
    char *ptr;    

    if (argc < 1)
    {        
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP set_device_name [BT_NAME]\n");

        return -1;
    }

    ptr = argv[0];
    property_p = &property;

    property_p->type = BT_PROPERTY_BDNAME;
    property_p->len = strlen(ptr);
    property_p->val = ptr;

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_set_scan_mode(int mode)
{
    bt_property_t property;
    bt_property_t *property_p;   
    bt_scan_mode_t scan_mode;     

    scan_mode = (bt_scan_mode_t)mode;

    property_p = &property;
    property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
    property_p->len = sizeof(bt_scan_mode_t);
    property_p->val = (void*)&scan_mode;

    g_bt_interface->set_adapter_property(property_p);

    return 0;    
}

static int btut_gap_set_scan_mode_handler(int argc, char *argv[])
{
    bt_property_t property;
    bt_property_t *property_p;   
    bt_scan_mode_t scan_mode;
    char *ptr;    

    if (argc < 1)
    {        
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP set_scan_mode [0:none, 1:connectable, 2:connectable_discoverable]\n");

        return -1;
    }

    ptr = argv[0];
    property_p = &property;
    
    scan_mode = (bt_scan_mode_t)atoi(ptr);

    property_p->type = BT_PROPERTY_ADAPTER_SCAN_MODE;
    property_p->len = sizeof(bt_scan_mode_t);
    property_p->val = (void*)&scan_mode;

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_set_discovery_timeout_handler(int argc, char *argv[])
{
    bt_property_t property;
    bt_property_t *property_p;
    unsigned int disc_timeout;
    char *ptr;    

    if (argc < 1)
    {        
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP set_discovery_timeout [num (unit:second)]\n");

        return -1;
    }

    ptr = argv[0];
    property_p = &property;

    disc_timeout = atoi(ptr);

    property_p->type = BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT;
    property_p->len = sizeof(unsigned int);
    property_p->val = &disc_timeout;

    g_bt_interface->set_adapter_property(property_p);

    return 0;
}

static int btut_gap_get_remote_device_properties_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP get_remote_device_properties [BT_ADDR]\n");

        return -1;
    }
    
    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("BTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->get_remote_device_properties(&bdaddr);

    return 0;
}

static int btut_gap_get_remote_device_property_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;
    bt_property_type_t num;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc < 2)
    {        
        BTUT_Logi("Usage is :\n");        
        BTUT_Logi("  GAP get_remote_device_property [BT_ADDR] [1 - 12]\n");        

        return -1;
    }

    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("BTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    num = (bt_property_type_t)atoi(argv[1]);

    g_bt_interface->get_remote_device_property(&bdaddr, num);

    return 0;
}

static int btut_gap_set_remote_device_property_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    //g_bt_interface->set_remote_device_property();

    return 0;
}
static int btut_gap_get_remote_service_record_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    //g_bt_interface->get_remote_service_record();

    return 0;
}
static int btut_gap_get_remote_services_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  get_remote_device_properties [BT_ADDR]\n");

        return -1;
    }
    
    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("CBTADDR = %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->get_remote_services(&bdaddr);

    return 0;
}

static int btut_gap_start_discovery_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    g_bt_interface->start_discovery();

    return 0;
}

static int btut_gap_cancel_discovery_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    g_bt_interface->cancel_discovery();

    return 0;
}

static int btut_gap_create_bond_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP create_bond [BT_ADDR]\n");

        return -1;
    }
    
    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("Cancel pairing to %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->create_bond(&bdaddr, NULL);

    return 0;
}

static int btut_gap_remove_bond_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP remove_bond [BT_ADDR]\n");

        return -1;
    }
    
    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("Cancel pairing to %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->remove_bond(&bdaddr);

    return 0;
}

static int btut_gap_cancel_bond_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc <= 0)
    {
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP cancel_bond [BT_ADDR]\n");

        return -1;
    }
    
    ptr = argv[0];
    btut_gap_btaddr_stoh(ptr, &bdaddr);
    BTUT_Logd("Cancel pairing to %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_interface->cancel_bond(&bdaddr);

    return 0;
}

static int btut_gap_dut_mode_configure_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    //g_bt_interface->dut_mode_configure();

    return 0;
}

static int btut_gap_le_test_mode_handler(int argc, char *argv[])
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    //g_bt_interface->le_test_mode();

    return 0;
}

static int btut_gap_config_hci_snoop_log_handler(int argc, char *argv[])
{
    unsigned int enable;
    char *ptr;    

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    if (argc < 1)
    {
        BTUT_Logi("Usage :\n");        
        BTUT_Logi("  GAP config_hci_snoop_log [1|0]\n");

        return -1;
    }

    ptr = argv[0];

    enable = atoi(ptr);
    g_bt_interface->config_hci_snoop_log(enable == 0 ? 0 : 1);

    return 0;
}

static void btut_gap_btaddr_stoh(char *btaddr_s, bt_bdaddr_t *bdaddr_h)
{
    int i;

    for (i = 0; i <6; i++)
    {
        bdaddr_h->address[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
}

static void btut_gap_show_properties_debug(int num_properties, bt_property_t *properties)
{
    int i, j;
    int len;
    char *name;        
    bt_scan_mode_t *mode;
    bt_bdaddr_t *btaddr;
    bt_uuid_t *uuid;
    bt_property_t *property;    
    unsigned int *cod;    
	unsigned int *devicetype;    
    unsigned int *disc_timeout;
    short *rssi;

    BTUT_Logd("=======================================Properties==============================================\n");
    BTUT_Logd("[GAP] num_properties = %d\n", num_properties);

    for (i = 0; i < num_properties; i++)
    {
        property = &properties[i];
    
        switch (property->type)
        {
        /* 1 */
        case BT_PROPERTY_BDNAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);            
            
            break;
        /* 2 */
        case BT_PROPERTY_BDADDR:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bdaddr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len, 
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);

            break;
        /* 3 */
        case BT_PROPERTY_UUIDS:
            uuid = (bt_uuid_t*)property->val;
            len = property->len;

            for (j=0; j<len; j+=16)
            {
                BTUT_Logd("[GAP] type = %d, len = %d, uuid = %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n", 
                    property->type, property->len, 
                    uuid->uu[j+0], uuid->uu[j+1], uuid->uu[j+2], uuid->uu[j+3], 
                    uuid->uu[j+4], uuid->uu[j+5], uuid->uu[j+6], uuid->uu[j+7], 
                    uuid->uu[j+8], uuid->uu[j+9], uuid->uu[j+10], uuid->uu[j+11], 
                    uuid->uu[j+12], uuid->uu[j+13], uuid->uu[j+14], uuid->uu[j+15] 
                    );
            }

            break;
        /* 4 */
        case BT_PROPERTY_CLASS_OF_DEVICE:
            cod = (unsigned int *)property->val;

            BTUT_Logd("[GAP] type = %d, len = %d, cod = 0x%lX\n", property->type, property->len, *cod);
            break;
        /* 5 */
        case BT_PROPERTY_TYPE_OF_DEVICE:
            devicetype= (unsigned int *)property->val;

            /* 0 - BLE, 1 - BT, 2 - DUAL MODE */
            BTUT_Logd("[GAP] type = %d, len = %d, devicetype = %d\n", property->type, property->len, *devicetype);
            break;
        /* 6 */            
        case BT_PROPERTY_SERVICE_RECORD:
            BTUT_Logd("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* 7 */
        case BT_PROPERTY_ADAPTER_SCAN_MODE: 
            mode = (bt_scan_mode_t *)property->val;
            
            BTUT_Logd("[GAP] type = %d, len = %d, scan mode = %d\n", property->type, property->len, (uint32_t)*mode);
            break;
        /* 8 */
        case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logd("[GAP] type = %d, len = %d, bonded_addr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len, 
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
            break;
        /* 9 */
        case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
            disc_timeout = (unsigned int *)property->val;
            
            BTUT_Logd("[GAP] type = %d, len = %d, disc_timeout = %d\n", property->type, property->len, *disc_timeout);
            break;
        /* 10 */
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:            
            name = (char *)property->val;
            BTUT_Logd("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);

            break;
        /* 11 */
        case BT_PROPERTY_REMOTE_RSSI:
            rssi = (short *)property->val;
                
            BTUT_Logd("[GAP] type = %d, len = %d, rssi = %d\n", property->type, property->len, *rssi);
            break;
        /* 12 */
        case BT_PROPERTY_REMOTE_VERSION_INFO:            
            BTUT_Logd("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* FF */
        case BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP:            
            BTUT_Logd("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        default:
            BTUT_Logd("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        }
    }

    BTUT_Logd("==========================================End==================================================\n");
}

static void btut_gap_show_properties_info(int num_properties, bt_property_t *properties)
{
    int i, j;
    int len;
    char *name;        
    bt_scan_mode_t *mode;
    bt_bdaddr_t *btaddr;
    bt_uuid_t *uuid;
    bt_property_t *property;    
    unsigned int *cod;    
	unsigned int *devicetype;    
    unsigned int *disc_timeout;
    short *rssi;

    BTUT_Logi("=======================================Properties==============================================\n");
    BTUT_Logi("[GAP] num_properties = %d\n", num_properties);

    for (i = 0; i < num_properties; i++)
    {
        property = &properties[i];
    
        switch (property->type)
        {
        /* 1 */
        case BT_PROPERTY_BDNAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);            
            
            break;
        /* 2 */
        case BT_PROPERTY_BDADDR:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bdaddr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len, 
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);

            break;
        /* 3 */
        case BT_PROPERTY_UUIDS:
            uuid = (bt_uuid_t*)property->val;
            len = property->len;

            for (j=0; j<len; j+=16)
            {
                BTUT_Logi("[GAP] type = %d, len = %d, uuid = %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n", 
                    property->type, property->len, 
                    uuid->uu[j+0], uuid->uu[j+1], uuid->uu[j+2], uuid->uu[j+3], 
                    uuid->uu[j+4], uuid->uu[j+5], uuid->uu[j+6], uuid->uu[j+7], 
                    uuid->uu[j+8], uuid->uu[j+9], uuid->uu[j+10], uuid->uu[j+11], 
                    uuid->uu[j+12], uuid->uu[j+13], uuid->uu[j+14], uuid->uu[j+15] 
                    );
            }

            break;
        /* 4 */
        case BT_PROPERTY_CLASS_OF_DEVICE:
            cod = (unsigned int *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, cod = 0x%lX\n", property->type, property->len, *cod);
            break;
        /* 5 */
        case BT_PROPERTY_TYPE_OF_DEVICE:
            devicetype= (unsigned int *)property->val;

            /* 0 - BLE, 1 - BT, 2 - DUAL MODE */
            BTUT_Logi("[GAP] type = %d, len = %d, devicetype = %d\n", property->type, property->len, *devicetype);
            break;
        /* 6 */            
        case BT_PROPERTY_SERVICE_RECORD:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* 7 */
        case BT_PROPERTY_ADAPTER_SCAN_MODE: 
            mode = (bt_scan_mode_t *)property->val;
            
            BTUT_Logi("[GAP] type = %d, len = %d, scan mode = %d\n", property->type, property->len, (uint32_t)*mode);
            break;
        /* 8 */
        case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] type = %d, len = %d, bonded_addr = %02X:%02X:%02X:%02X:%02X:%02X\n", property->type, property->len, 
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
            break;
        /* 9 */
        case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
            disc_timeout = (unsigned int *)property->val;
            
            BTUT_Logi("[GAP] type = %d, len = %d, disc_timeout = %d\n", property->type, property->len, *disc_timeout);
            break;
        /* 10 */
        case BT_PROPERTY_REMOTE_FRIENDLY_NAME:            
            name = (char *)property->val;
            BTUT_Logi("[GAP] type = %d, len = %d, bdname = %s\n", property->type, property->len, name);

            break;
        /* 11 */
        case BT_PROPERTY_REMOTE_RSSI:
            rssi = (short *)property->val;
                
            BTUT_Logi("[GAP] type = %d, len = %d, rssi = %d\n", property->type, property->len, *rssi);
            break;
        /* 12 */
        case BT_PROPERTY_REMOTE_VERSION_INFO:            
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        /* FF */
        case BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP:            
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        default:
            BTUT_Logi("[GAP] type = %d, len = %d, Others.\n", property->type, property->len);
            break;
        }
    }

    BTUT_Logi("==========================================End==================================================\n");
}


// Callback functions implementation
static void btut_gap_state_changed_cb(bt_state_t state)
{
    BTUT_Logd("[GAP] %s() state: %d\n", __FUNCTION__, state);

    switch (state)
    {
        case BT_STATE_OFF:
            BTUT_Logi("[GAP] BT STATE OFF\n"); 
            break;
        case BT_STATE_ON:            
            BTUT_Logi("[GAP] BT STATE ON\n");

            if (default_scan_mode)
            {
                btut_gap_set_scan_mode(default_scan_mode);
            }
            btut_a2dp_snk_init();
            //btut_a2dp_src_init();
            btut_gatt_init();
            //btut_hid_init();
            break;
        default:
            break;
    }
}

static void btut_gap_properties_cb(bt_status_t status,
                                       int num_properties,
                                       bt_property_t *properties)
{
    BTUT_Logd("[GAP] %s() status: %d\n", __FUNCTION__, status);

    btut_gap_show_properties_info(num_properties, properties);
}

static void btut_gap_remote_device_properties_cb(bt_status_t status,
                                                        bt_bdaddr_t *bd_addr,
                                                        int num_properties,
                                                        bt_property_t *properties)
{
    BTUT_Logd("[GAP] %s() status: %d\n", __FUNCTION__, status);

    btut_gap_show_properties_debug(num_properties, properties);
}


static void btut_gap_device_found_cb(int num_properties,
                                           bt_property_t *properties)
{
    int i;
    char *name;        
    bt_bdaddr_t *btaddr;    
    bt_property_t *property;    

    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    BTUT_Logi("----------------------------------------------------------\n");

    for (i = 0; i < num_properties; i++)
    {
        property = &properties[i];
    
        switch (property->type)
        {
        /* 1 */
        case BT_PROPERTY_BDNAME:
            name = (char *)property->val;
            BTUT_Logi("[GAP] BDNAME = %s\n", name);
            
            break;
        /* 2 */
        case BT_PROPERTY_BDADDR:
            btaddr = (bt_bdaddr_t *)property->val;

            BTUT_Logi("[GAP] BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);

            break;
        default:
            break;
        }
    }

    btut_gap_show_properties_debug(num_properties, properties);
}


static void btut_gap_discovery_state_changed_cb(bt_discovery_state_t state)
{
    BTUT_Logd("[GAP] %s() state: %d\n", __FUNCTION__, state);

    switch (state)
    {
        case BT_DISCOVERY_STOPPED:            
            BTUT_Logi("[GAP] BT Search Device Stop.\n");
            break;
        case BT_DISCOVERY_STARTED:
            BTUT_Logi("[GAP] BT Search Device Start...\n"); 
            break;            
        default:
            break;
    }
}

static void btut_gap_pin_request_cb(bt_bdaddr_t *remote_bd_addr,
                                         bt_bdname_t *bd_name, uint32_t cod)
{
	bt_pin_code_t pin;
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    memset(&pin, 0, 16);
	memset(&pin, '0', 4);
	g_bt_interface->pin_reply(remote_bd_addr, 1, 4, &pin);
}


static void btut_gap_ssp_request_cb(bt_bdaddr_t *remote_bd_addr,
                                                bt_bdname_t *bd_name, uint32_t cod,
                                                bt_ssp_variant_t pairing_variant,
                                                uint32_t pass_key)
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);

    // Iverson debug
    if (remote_bd_addr)
    {        
        bt_bdaddr_t *btaddr = remote_bd_addr;    
            
        BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
            btaddr->address[0], btaddr->address[1], btaddr->address[2],
            btaddr->address[3], btaddr->address[4], btaddr->address[5]);
    }

    if (bd_name)
    {        
        BTUT_Logd("[GAP] BDNAME = %s\n", bd_name);
    }
    BTUT_Logd("[GAP] cod = 0x%08X, pairing_variant = %d, pass_key = %d.\n", cod, pairing_variant, pass_key);
    BTUT_Logi("[GAP] pass_key = %d.\n", pass_key);
    
    g_bt_interface->ssp_reply(remote_bd_addr, pairing_variant, 1, pass_key);
}

static void btut_gap_bond_state_changed_cb(bt_status_t status,
                                                         bt_bdaddr_t *remote_bd_addr,
                                                         bt_bond_state_t state)
{
    BTUT_Logd("[GAP] %s(), status = %d, state = %d\n", __FUNCTION__, status, state);

    switch (status)
    {
        case BT_STATUS_SUCCESS:            
            BTUT_Logi("[GAP] BT bond status is successful(%d), ", status);
            break;
        default:            
            BTUT_Logi("[GAP] BT bond status is failed(%d), ", status);
            break;
    }

    switch (state)
    {
        case BT_BOND_STATE_NONE:            
            BTUT_Logi("state is none.\n");
            break;
        case BT_BOND_STATE_BONDING:            
            BTUT_Logi("state is bonding.\n");
            break;
        case BT_BOND_STATE_BONDED:            
            BTUT_Logi("state is bonded.\n");
            break;            
        default:
            break;
    }

    if (remote_bd_addr)
    {        
        bt_bdaddr_t *btaddr = remote_bd_addr;    
            
        if ( (status == BT_STATUS_SUCCESS) && (state == BT_BOND_STATE_BONDED) )
        {
            BTUT_Logi("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
        else
        {            
            BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
    }
}

static void btut_gap_acl_state_changed_cb(bt_status_t status, bt_bdaddr_t *remote_bd_addr,
                                            bt_acl_state_t state)
{
    BTUT_Logd("[GAP] %s(), status = %d, state = %d\n", __FUNCTION__, status, state);

    switch (status)
    {
        case BT_STATUS_SUCCESS:            
            BTUT_Logi("[GAP] BT bond status is successful(%d), ", status);
            break;
        default:            
            BTUT_Logi("[GAP] BT bond status is failed(%d), ", status);
            break;
    }

    switch (state)
    {
        case BT_ACL_STATE_CONNECTED:            
            BTUT_Logi("acl is connected.\n");
            break;
        case BT_ACL_STATE_DISCONNECTED:            
            BTUT_Logi("acl is disconnected.\n");
            break;         
        default:
            break;
    }

    if (remote_bd_addr)
    {        
        bt_bdaddr_t *btaddr = remote_bd_addr;    
            
        if(status == BT_STATUS_SUCCESS)
        {
            BTUT_Logi("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
        else
        {
            BTUT_Logd("[GAP] REMOTE BDADDR = %02X:%02X:%02X:%02X:%02X:%02X\n",
                btaddr->address[0], btaddr->address[1], btaddr->address[2],
                btaddr->address[3], btaddr->address[4], btaddr->address[5]);
        }
    }
}

static void btut_gap_reset_ind_cb(void)
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);
}

static void btut_gap_thread_event_cb(bt_cb_thread_evt evt)
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);
}

static void btut_gap_dut_mode_recv_cb(uint16_t opcode, uint8_t *buf, uint8_t len)
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);
}

static void btut_gap_le_test_mode_cb(bt_status_t status, uint16_t num_packets)
{
    BTUT_Logd("[GAP] %s()\n", __FUNCTION__);
}


// For handling incoming commands from CLI.
int btut_gap_cmd_handler(int argc, char **argv)
{
    BTUT_CLI *cmd, *match = NULL;    
    int ret = 0;
    int count;

    count = 0;    
    cmd = bt_gap_cli_commands;
    
    BTUT_Logd("[GAP] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        
        btut_print_cmd_help(CMD_KEY_GAP, bt_gap_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
}

void *btut_gap_get_profile_interface(const char *profile_id)
{
    if (g_bt_interface != NULL)
    {
        return g_bt_interface->get_profile_interface(profile_id);
    }

    return NULL;
}

int btut_gap_init()
{
    int ret = 0;
    BTUT_MOD gap_mod = {0};

    // Init bluetooth interface.
    open_bluetooth_stack(NULL, "BlueAngel Linux", &g_bt_device);
    if (g_bt_device == NULL)
    {
        BTUT_Loge("[GAP] Failed to open Bluetooth stack.\n");
        return -1;
    }

    g_bt_interface = g_bt_device->get_bluetooth_interface();
    if (g_bt_device == NULL)
    {
        BTUT_Loge("[GAP] Failed to get Bluetooth interface\n");
        return -1;
    }

    g_bt_interface->init(&g_bt_callbacks);

    // Register to btut_cli.
    gap_mod.mod_id = BTUT_MOD_GAP;
    strncpy(gap_mod.cmd_key, CMD_KEY_GAP, sizeof(gap_mod.cmd_key));
    gap_mod.cmd_handler = btut_gap_cmd_handler;
    gap_mod.cmd_tbl = bt_gap_cli_commands;

    ret = btut_register_mod(&gap_mod);
    BTUT_Logd("[GAP] btut_register_mod() returns: %d\n", ret);

    // Enable Virtual Sniffer Log.
    //g_bt_interface->config_hci_snoop_log(1);

    //default gap enable    
    g_bt_interface->enable();

    return ret;
}

int btut_gap_deinit_profiles()
{
    BTUT_Logi("%s()\n", __FUNCTION__);

    btut_a2dp_snk_deinit();
    BTUT_Logi("%s() btut_a2dp_snk_deinit() done\n", __FUNCTION__);

    //btut_a2dp_src_deinit();
    //BTUT_Logi("%s() btut_a2dp_src_deinit() done\n", __FUNCTION__);
    
    btut_gatt_deinit();
    BTUT_Logi("%s() btut_gatt_deinit() done\n", __FUNCTION__);

    //btut_hid_deinit();
    //BTUT_Logi("%s() btut_hid_deinit() done\n", __FUNCTION__);
}

int btut_gap_deinit()
{
    g_bt_interface->disable();
    g_bt_interface->cleanup();
    return 0;
}

