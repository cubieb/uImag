
#include <string.h>

#include "bluetooth.h"
#include "bt_av.h"
#include "bt_rc.h"

#include "bt_a2dp_android.h"

#include "btut_cli.h"
#include "btut_debug.h"
#include "btut_gap_if.h"
#include "btut_a2dp_snk_if.h"
#include <sys/time.h>
extern void *btut_gap_get_profile_interface(const char *profile_id);

#define MAX_AV_DEVICE       4

// For AVRCP Controller.
#define AVRCP_POP_PLAY              0x44
#define AVRCP_POP_STOP              0x45
#define AVRCP_POP_PAUSE             0x46
#define AVRCP_POP_FORWARD           0x4B
#define AVRCP_POP_BACKWARD          0x4C

typedef struct _av_dev_contex_t
{
    int in_use;
    bt_bdaddr_t bt_addr;
    // A2DP stream status.
    int audio_state;
    // The right to play music.
    int audio_flag;
    // Curret key sent via AVRCP.
    int cur_key;
} bt_av_dev_context;

void btrc_connect_tg(unsigned char *bd_addr);
void btrc_disconnect_tg(void);

static void btut_avrcp_ctl_passthrough_rsp_cb(bt_bdaddr_t *bd_addr, int id, int key_state);
static void btut_avrcp_ctl_connection_state_cb(bool state, bt_bdaddr_t *bd_addr);

static void btut_a2dp_snk_connection_state_cb(btav_connection_state_t state, bt_bdaddr_t *bd_addr);
static void btut_a2dp_snk_audio_state_cb(btav_audio_state_t state, bt_bdaddr_t *bd_addr);

static int btut_a2dp_snk_connect_int_handler(int argc, char *argv[]);
static int btut_a2dp_snk_disconnect_handler(int argc, char *argv[]);

// AVRCP Command Handler 
static int btut_avrcp_ctl_play_handler(int argc, char *argv[]);
static int btut_avrcp_ctl_stop_handler(int argc, char *argv[]);
static int btut_avrcp_ctl_pause_handler(int argc, char *argv[]);
static int btut_avrcp_ctl_forward_handler(int argc, char *argv[]);
static int btut_avrcp_ctl_backward_handler(int argc, char *argv[]);
static int btut_avrcp_ctl_keycode_handler(int argc, char *argv[]);

static bt_av_dev_context g_av_cntx[MAX_AV_DEVICE];

static btav_interface_t *g_bt_a2dp_snk_interface;
static btav_callbacks_t g_bt_a2dp_snk_callbacks = 
{
    sizeof(btav_callbacks_t),
    btut_a2dp_snk_connection_state_cb,
    btut_a2dp_snk_audio_state_cb,
};

static btrc_ctrl_interface_t *g_bt_avrcp_ctl_interface;
static btrc_ctrl_callbacks_t g_bt_avrcp_ctl_callbacks =
{
    sizeof(btrc_ctrl_callbacks_t),
    btut_avrcp_ctl_passthrough_rsp_cb,
    btut_avrcp_ctl_connection_state_cb,
};

static BTUT_CLI bt_a2dp_snk_cli_commands[] =
{
    {"connect", btut_a2dp_snk_connect_int_handler,
     " = connect"},
    {"disconnect", btut_a2dp_snk_disconnect_handler,
     " = disconnect"},
    {"play", btut_avrcp_ctl_play_handler,
     " = AVRCP ctrl play test"},
    {"pause", btut_avrcp_ctl_pause_handler,
     " = AVRCP ctrl pause test"},
    {"stop", btut_avrcp_ctl_stop_handler,
    " = AVRCP ctrl stop test"}, 
    {"forward", btut_avrcp_ctl_forward_handler,
    " = AVRCP ctrl forward test"},
    {"backward", btut_avrcp_ctl_backward_handler,
    " = AVRCP ctrl backward test"},
    {"keycode", btut_avrcp_ctl_keycode_handler,
     " = AVRCP ctrl keycode test"},
    {NULL, NULL, NULL},
};

// Pointer for keeping A2DP audio data related context.
void *g_a2dp_data;

static int _is_av_dev_linked()
{
    int i = 0;
    for (i = 0; i < MAX_AV_DEVICE; i++)
    {
        if (g_av_cntx[i].in_use != 0)
        {
            return 1;
        }
    }
    return 0;
}

static bt_av_dev_context *_get_available_av_dev()
{
    int i = 0;
    for (i = 0; i < MAX_AV_DEVICE; i++)
    {
        if (g_av_cntx[i].in_use == 0)
        {
            return &g_av_cntx[i];
        }
    }
    return NULL;
}

static bt_av_dev_context *_get_av_dev_by_addr(bt_bdaddr_t *bd_addr)
{
    int i = 0;
    for (i = 0; i < MAX_AV_DEVICE; i++)
    {
        if (g_av_cntx[i].in_use == 1 &&
            memcmp(&(g_av_cntx[i].bt_addr), bd_addr, sizeof(bt_bdaddr_t)) == 0)
        {
            return &g_av_cntx[i];
        }
    }
    return NULL;
}

static void do_audio_control(const char *cmd){
        pid_t pid;
        struct timeval tv = {0,0};
        int status;
        pid = fork();
        if(pid == 0){
            execlp("cmder", "cmder", cmd, NULL);
            exit(0);
        }else{
            gettimeofday(&tv,0);
            printf("before rplayer %s,resume %ld, %ld\n", cmd, tv.tv_sec, tv.tv_usec);
            wait(&status);
            gettimeofday(&tv,0);
            printf("after rplayer %s,resume %ld, %ld\n", cmd, tv.tv_sec, tv.tv_usec);
        }
}

static void btut_a2dp_snk_connection_state_cb(btav_connection_state_t state, bt_bdaddr_t *bd_addr)
{
#ifdef RPLAYER_TEST
    pid_t pid;
    int status, i = 0;
    struct timeval tv;
    bt_av_dev_context *av_dev = NULL;

    BTUT_Logi("%s() state: %d bd_addr %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__, state,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);

    if(state == BTAV_CONNECTION_STATE_DISCONNECTED){
        // Disconnect AVRCP Target.
        av_dev = _get_av_dev_by_addr(bd_addr);
        if (av_dev != NULL)
        {
            av_dev->in_use = 0;
            if(!_is_av_dev_linked()){
                do_audio_control("c");
            }
            btrc_disconnect_tg_addr(bd_addr);
        }
        else
        {
            BTUT_Logi("av_dev not found.");
        }    
    }

    if(state == BTAV_CONNECTION_STATE_CONNECTING){
        printf("+++ now bt connecting ...\n");
            
    }

    if(state == BTAV_CONNECTION_STATE_DISCONNECTING){
        printf("+++ now bt disconnecting ...\n");
    }

    if(state == BTAV_CONNECTION_STATE_CONNECTED){
        
        // Connect AVRCP Target.
        av_dev = _get_av_dev_by_addr(bd_addr);
        if (av_dev != NULL)
        {
            BTUT_Logi("av_dev already uses the same addr.");
        }
        else
        {
            av_dev = _get_available_av_dev();
            if (av_dev != NULL)
            {
                printf("To connect AVRCP\n");
                if(!_is_av_dev_linked()){
                    do_audio_control("o");
                }
                av_dev->in_use = 1;
                av_dev->audio_state = BTAV_AUDIO_STATE_STOPPED;
                memcpy(&(av_dev->bt_addr), bd_addr, sizeof(bt_bdaddr_t));
                btrc_connect_tg(bd_addr);
            }
        }
    }
    printf("state cb ===========> %d\n", state);

#endif
}

static void btut_a2dp_snk_audio_state_cb(btav_audio_state_t state, bt_bdaddr_t *bd_addr)
{
    bt_av_dev_context *tmp_dev = NULL, *playing_dev = NULL;
    int i = 0;

    BTUT_Logi("[A2DP_SNK] %s() state: %d bd_addr %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__, state,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
            bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);

    for (i = 0; i < MAX_AV_DEVICE; i++)
    {
        if (memcmp(&g_av_cntx[i].bt_addr, bd_addr, sizeof(bt_bdaddr_t)) == 0)
        {
            tmp_dev = &g_av_cntx[i];
        }

        if (g_av_cntx[i].audio_flag != 0)
        {
            playing_dev = &g_av_cntx[i];
        }
    }

    BTUT_Logi("[A2DP_SNK] %s() tmp_dev: 0x%x playing_dev: 0x%x",
        __FUNCTION__, tmp_dev, playing_dev);
    
    if(tmp_dev == NULL)
       return;

    tmp_dev->audio_state = state;
    switch (state)
    {
        case BTAV_AUDIO_STATE_STARTED:
            tmp_dev->audio_flag = 1;
            if (playing_dev != NULL && playing_dev != tmp_dev)
            {
                // Pause the current playing device.
                playing_dev->audio_flag = 0;
                playing_dev->cur_key = AVRCP_POP_PAUSE;
                g_bt_avrcp_ctl_interface->send_pass_through_cmd(&playing_dev->bt_addr, AVRCP_POP_PAUSE, 0);
            }
            break;

        case BTAV_AUDIO_STATE_REMOTE_SUSPEND:
        case BTAV_AUDIO_STATE_STOPPED:
            tmp_dev->audio_flag = 0;
            break;

        default:
            break;
    }
#if 0   
    // Do nothing for now.
    if(state == BTAV_AUDIO_STATE_STOPPED){
        printf("+++ now bt stopped ...\n");
        do_audio_control("p");
    }
    if(state == BTAV_AUDIO_STATE_REMOTE_SUSPEND)
        printf("+++ now remote suspend ...\n");
    if(state == BTAV_AUDIO_STATE_STARTED){
        printf("+++ now bt started ...\n");
        do_audio_control("g");
    }
#endif    
    printf("audio cb ===========> %d\n", state);
}

static void btut_avrcp_ctl_passthrough_rsp_cb(bt_bdaddr_t *bd_addr, int id, int key_state)
{
    bt_av_dev_context *tmp_dev = NULL;
    int i = 0;

    BTUT_Logi("[AVRCP] %s() id: %d, key_state: %d", __FUNCTION__, id, key_state);

    tmp_dev = _get_av_dev_by_addr(bd_addr);
    if (tmp_dev == NULL)
    {
        BTUT_Logw("[AVRCP] %s() No av_dev for %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__,
            bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
            bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);
        return;
    }

    if (key_state == 0)
    {
        // Here comes PRESS_CNF, send RELEASE
        if (g_bt_avrcp_ctl_interface == NULL)
        {
            BTUT_Loge("g_bt_avrcp_ctl_interface is NULL.");
            return;
        }

        // Release
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&tmp_dev->bt_addr, id, 1);
    }
}
static void btut_avrcp_ctl_connection_state_cb(bool state, bt_bdaddr_t *bd_addr)
{
    bt_av_dev_context *av_dev = NULL;

    BTUT_Logi("[AVRCP] %s() state: %d bd_addr %02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__, state,
        bd_addr->address[0], bd_addr->address[1], bd_addr->address[2],
        bd_addr->address[3], bd_addr->address[4], bd_addr->address[5]);

    av_dev = _get_av_dev_by_addr(bd_addr);
    if (state)
    {
        if (av_dev != NULL)
        {
            BTUT_Logi("[AVRCP] %s() already in use.", __FUNCTION__);
        }
        else
        {
            av_dev = _get_available_av_dev();
            if (av_dev != NULL)
            {
                BTUT_Logi("[AVRCP] %s() Incoming connection accepted.", __FUNCTION__);
                if(!_is_av_dev_linked()){
                    do_audio_control("o");
                }
                av_dev->in_use = 1;
                memcpy(&(av_dev->bt_addr), bd_addr, sizeof(bt_bdaddr_t));
            }
            else
            {
                BTUT_Logw("[AVRCP] No available av_dev.");
            }
        }
    }
    else
    {
        // Disconnected.
        if (av_dev != NULL)
        {
            BTUT_Logi("[AVRCP] %s() Disconnected", __FUNCTION__);
            av_dev->in_use = 0;
            if(!_is_av_dev_linked()){
                do_audio_control("c");
            }
            memset(&(av_dev->bt_addr), 0, sizeof(bt_bdaddr_t));
        }
        else
        {
            BTUT_Logi("[AVRCP] %s() av_dev not found.", __FUNCTION__);
        }
    }
}

static void btut_a2dp_snk_btaddr_stoh(char *btaddr_s, bt_bdaddr_t *bdaddr_h)
{
    int i;

    for (i = 0; i <6; i++)
    {
        bdaddr_h->address[i] = strtoul(btaddr_s, &btaddr_s, 16);
        btaddr_s++;
    }
}

static int btut_a2dp_snk_connect_int_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    printf("[A2DP_SNK] %s()\n", __FUNCTION__);

    if (argc < 1)
    {
        return -1;
    }
    
    ptr = argv[0];
    btut_a2dp_snk_btaddr_stoh(ptr, &bdaddr);
    printf("A2DP connected to %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_a2dp_snk_interface->connect(&bdaddr, 1);     
    return 0;

}

static int btut_a2dp_snk_disconnect_handler(int argc, char *argv[])
{
    char *ptr;
    bt_bdaddr_t bdaddr;

    printf("[A2DP_SNK] %s()\n", __FUNCTION__);

    if (argc < 1)
    {
        return -1;
    }
    
    ptr = argv[0];
    btut_a2dp_snk_btaddr_stoh(ptr, &bdaddr);
    printf("A2DP disconnected to %02X:%02X:%02X:%02X:%02X:%02X\n", 
        bdaddr.address[0], bdaddr.address[1], bdaddr.address[2], 
        bdaddr.address[3], bdaddr.address[4], bdaddr.address[5]);

    g_bt_a2dp_snk_interface->disconnect(&bdaddr); 
    return 0;
}

static int btut_avrcp_ctl_play_handler(int argc, char *argv[])
{
    int idx = 0;

    BTUT_Logi("[AVRCP] %s()", __FUNCTION__);

    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Logd("[AVRCP] g_bt_avrcp_ctl_interface is NULL.");
        return -1;
    }

    if (argc < 1)
    {        
        printf("Usage :\n");
        printf("  A2DP_SNK play [device index: 0 ~ 3]\n");

        return -1;
    }

    idx = atoi(argv[0]);
    if (idx >= 0 && idx < MAX_AV_DEVICE)
    {
        g_av_cntx[idx].cur_key = AVRCP_POP_PLAY;
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&(g_av_cntx[idx].bt_addr), AVRCP_POP_PLAY, 0);
        return 0;
    }
    else
    {
        printf("Usage :\n");
        printf("  A2DP_SNK play [device index: 0 ~ 3]\n");
        return -1;
    }

}

static int btut_avrcp_ctl_stop_handler(int argc, char *argv[])
{
    int idx = 0;

    BTUT_Logi("%s()", __FUNCTION__);

    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Logd("g_bt_avrcp_ctl_interface is NULL.");
        return -1;
    }

    if (argc < 1)
    {
            printf("Usage :\n");
            printf("  A2DP_SNK stop [device index: 0 ~ 3]\n");
    
            return -1;
    }

    idx = atoi(argv[0]);
    if (idx >= 0 && idx < MAX_AV_DEVICE)
    {
        g_av_cntx[idx].cur_key = AVRCP_POP_STOP;
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&(g_av_cntx[idx].bt_addr), AVRCP_POP_STOP, 0);
        return 0;
    }
    else
    {
        printf("Usage :\n");
        printf("  A2DP_SNK stop [device index: 0 ~ 3]\n");
        return -1;
    }

}

static int btut_avrcp_ctl_pause_handler(int argc, char *argv[])
{
    int idx = 0;

    BTUT_Logi("[AVRCP] %s()", __FUNCTION__);

    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Logd("[AVRCP] g_bt_avrcp_ctl_interface is NULL.");
        return -1;
    }

    if (argc < 1)
    {
        printf("Usage :\n");
        printf("  A2DP_SNK pause [device index: 0 ~ 3]\n");

        return -1;
    }

    idx = atoi(argv[0]);
    if (idx >= 0 && idx < MAX_AV_DEVICE)
    {
        g_av_cntx[idx].cur_key = AVRCP_POP_PAUSE;
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&(g_av_cntx[idx].bt_addr), AVRCP_POP_PAUSE, 0);
        return 0;
    }
    else
    {
        printf("Usage :\n");
        printf("  A2DP_SNK pause [device index: 0 ~ 3]\n");
        return -1;
    }

}

static int btut_avrcp_ctl_forward_handler(int argc, char *argv[])
{
    int idx = 0;

    BTUT_Logi("%s()", __FUNCTION__);

    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Logd("g_bt_avrcp_ctl_interface is NULL.");
        return -1;
    }

    if (argc < 1)
    {
        printf("Usage :\n");
        printf("  A2DP_SNK forward [device index: 0 ~ 3]\n");

        return -1;
    }

    idx = atoi(argv[0]);
    if (idx >= 0 && idx < MAX_AV_DEVICE)
    {
        g_av_cntx[idx].cur_key = AVRCP_POP_FORWARD;
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&(g_av_cntx[idx].bt_addr), AVRCP_POP_FORWARD, 0);
        return 0;
    }
    else
    {
        printf("Usage :\n");
        printf("  A2DP_SNK forward [device index: 0 ~ 3]\n");
        return -1;
    }

}

static int btut_avrcp_ctl_backward_handler(int argc, char *argv[])
{
    int idx = 0;

    BTUT_Logi("%s()", __FUNCTION__);

    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Logd("g_bt_avrcp_ctl_interface is NULL.");
        return -1;
    }

    if (argc < 1)
    {
        printf("Usage :\n");
        printf("  A2DP_SNK backward [device index: 0 ~ 3]\n");

        return -1;
    }

    idx = atoi(argv[0]);
    if (idx >= 0 && idx < MAX_AV_DEVICE)
    {
        g_av_cntx[idx].cur_key = AVRCP_POP_BACKWARD;
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&(g_av_cntx[idx].bt_addr), AVRCP_POP_BACKWARD, 0);
        return 0;
    }
    else
    {
        printf("Usage :\n");
        printf("  A2DP_SNK backward [device index: 0 ~ 3]\n");
        return -1;
    }

}

static int btut_avrcp_ctl_keycode_handler(int argc, char *argv[])
{
    int idx = 0;
    uint8_t cmd = 0x0;

    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Logd("g_bt_avrcp_ctl_interface is NULL.");
        return -1;
    }

    if (argc < 2)
    {
        printf("Usage :\n");
        printf("  A2DP_SNK keycode [key in hex] [device index: 0 ~ 3]\n");

        return -1;
    }

    cmd = (uint8_t) strtol( argv[0], NULL, 16);
    idx = atoi(argv[1]);
    if (idx >= 0 && idx < MAX_AV_DEVICE)
    {
        printf("%s() %d %d %s\n", __FUNCTION__, cmd, argc, argv[1]);
        g_av_cntx[idx].cur_key = cmd;
        g_bt_avrcp_ctl_interface->send_pass_through_cmd(&(g_av_cntx[idx].bt_addr), cmd, 0);
        return 0;
    }
    else
    {
        printf("Usage :\n");
        printf("  A2DP_SNK backward [device index: 0 ~ 3]\n");
        return -1;
    }

}

// For handling incoming commands from CLI.
int btut_a2dp_snk_cmd_handler(int argc, char **argv)
{
#ifdef RPLAYER_TEST    
    BTUT_CLI *cmd, *match = NULL;    
    int ret = 0;
    int count;

    count = 0;    
    cmd = bt_a2dp_snk_cli_commands;
    
    BTUT_Logd("[A2DP_SNK] argc: %d, argv[0]: %s\n", argc, argv[0]);

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
        BTUT_Logd("[A2DP_SNK] Unknown command '%s'\n", argv[0]);        
        
        btut_print_cmd_help(CMD_KEY_A2DP_SNK, bt_a2dp_snk_cli_commands);
        ret = -1;
    }
    else
    {
        match->handler(argc - 1, &argv[1]);
    }

    return ret;
#else 
    return 0;
#endif    
}


// The data callback is simply included by audio.a2dp.blueangel.so as an extern function.
// The audio data interface shall be refined.
void btut_a2dp_snk_data_callback(unsigned char *data, unsigned short len)
{
    BTUT_Logd("[A2DP_SNK] Received data. len: %d\n", len);
}


int btut_a2dp_snk_init()
{
    int ret = 0, i = 0;
    BTUT_MOD a2dp_mod = {0};

    // Get A2DP interface
    g_bt_a2dp_snk_interface = (btav_interface_t *) btut_gap_get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
    if (g_bt_a2dp_snk_interface == NULL)
    {
        BTUT_Loge("[A2DP_SNK] Failed to get A2DP interface\n");
        return -1;
    }

    // Get AVRCP Ctrl interface
    g_bt_avrcp_ctl_interface = (btrc_ctrl_interface_t *) btut_gap_get_profile_interface(BT_PROFILE_AV_RC_CTRL_ID);
    if (g_bt_avrcp_ctl_interface == NULL)
    {
        BTUT_Loge("[A2DP_SNK] Failed to get AVRCP Ctrl interface\n");
        return -1;
    }

    // Init A2DP interface
    if (g_bt_a2dp_snk_interface->init(&g_bt_a2dp_snk_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[A2DP_SNK] Failed to init A2DP interface\n");
        return -1;
    }

    // Init AVRCP Ctrl interface
    if (g_bt_avrcp_ctl_interface->init(&g_bt_avrcp_ctl_callbacks) != BT_STATUS_SUCCESS)
    {
        BTUT_Loge("[A2DP_SNK] Failed to init AVRCP Ctrl interface\n");
        return -1;
    }

    // Init A/V device context.
    for (i = 0; i < MAX_AV_DEVICE; i ++)
    {
        memset(&g_av_cntx[i], 0, sizeof(bt_av_dev_context));
    }

#ifdef RPLAYER_TEST 
#else    
    // Init A2DP Data interface
    ret = a2dp_init(0, 0, &g_a2dp_data);
    BTUT_Logd("[A2DP_SNK] a2dp_init() returns: %d\n", ret);
    a2dp_snk_set_data_cb(btut_a2dp_snk_data_callback);
#endif

    // Register command to CLI
    a2dp_mod.mod_id = BTUT_MOD_A2DP_SNK;
    strncpy(a2dp_mod.cmd_key, CMD_KEY_A2DP_SNK, sizeof(a2dp_mod.cmd_key));
    a2dp_mod.cmd_handler = btut_a2dp_snk_cmd_handler;
    a2dp_mod.cmd_tbl = bt_a2dp_snk_cli_commands;
    
    ret = btut_register_mod(&a2dp_mod);
    BTUT_Logd("[A2DP_SNK] btut_register_mod() returns: %d\n", ret);

    return ret;
}

int btut_a2dp_snk_deinit()
{
    // Deinit A2DP interface
    if (g_bt_a2dp_snk_interface != NULL)
    {
        g_bt_a2dp_snk_interface->cleanup();
    }

    // Deinit AVRCP Ctrl interface
    if (g_bt_avrcp_ctl_interface != NULL)
    {
        g_bt_avrcp_ctl_interface->cleanup();
    }

#ifdef RPLAYER_TEST
#else    
    // Deinit A2DP data interface.
    a2dp_cleanup(g_a2dp_data);
#endif
    return 0;
}

