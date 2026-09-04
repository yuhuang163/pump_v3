/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
#include "root_common_api.h"
#include "ble_manager.h" // IWYU pragma: keep
#include "ble_master_message.h" // IWYU pragma: keep
#include "ble_factory_message.h" // IWYU pragma: keep
#include "ble_remot_message.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static void ble_event_opt_adv_param_update(void *context, void *data, int data_len);
static void ble_event_opt_adv_start(void *context, void *data, int data_len);
static void ble_event_opt_adv_stop(void *context, void *data, int data_len);
static void ble_event_user_bind_succ(void *context, void *data, int data_len);
static void ble_event_suspend(void *context, void *data, int data_len);
static void ble_event_resume(void *context, void *data, int data_len);
static void ble_event_periph_disconnect(void *context, void *data, int data_len);
static void ble_event_periph_connect_suc(void *context, void *data, int data_len);
static void ble_event_periph_disconnect_suc(void *context, void *data, int data_len);
static void ble_event_master_send_data(void *context, void *data, int data_len);
static void ble_event_master_recive_data(void *context, void *data, int data_len);
static void ble_event_fac_send_data(void *context, void *data, int data_len);
static void ble_event_fac_recive_data(void *context, void *data, int data_len);
static void ble_event_remot_send_data(void *context, void *data, int data_len);
static void ble_event_remot_recive_data(void *context, void *data, int data_len);
static void ble_event_data_bury_send_data(void *context, void *data, int data_len);
static void ble_event_data_bury_recive_data(void *context, void *data, int data_len);
static void ble_event_tx_ccc_enabled(void *context, void *data, int data_len);
static ble_msg_process_handle_t s_ble_msg_process_list[] = {
    {BLE_MSG_SET_ADV_UPDATE,                 ble_event_opt_adv_param_update},
    {BLE_MSG_SET_ADV_START,                  ble_event_opt_adv_start},
    {BLE_MSG_SET_ADV_STOP,                   ble_event_opt_adv_stop},
    {BLE_MSG_USER_BIND_SUCC,                 ble_event_user_bind_succ},
    {BLE_MSG_SUSPEND,                        ble_event_suspend},
    {BLE_MSG_RESUME,                         ble_event_resume},
    {BLE_MSG_PERIPH_DISCONN,                 ble_event_periph_disconnect},
    {BLE_MSG_PERIPH_CONNECT_SUC,             ble_event_periph_connect_suc},
    {BLE_MSG_TX_CCC_ENABLED,                 ble_event_tx_ccc_enabled},
    {BLE_MSG_PERIPH_DISCONN_SUC,             ble_event_periph_disconnect_suc},
    {BLE_MSG_MASTER_SEND_DATA,               ble_event_master_send_data},
    {BLE_MSG_MASTER_RECIVE_DATA,             ble_event_master_recive_data},
    {BLE_MSG_FAC_SEND_DATA,                  ble_event_fac_send_data},
    {BLE_MSG_FAC_RECIVE_DATA,                ble_event_fac_recive_data},
    {BLE_MSG_REMOT_SEND_DATA,                ble_event_remot_send_data},
    {BLE_MSG_REMOT_RECIVE_DATA,              ble_event_remot_recive_data},
    {BLE_MSG_DATA_BURY_SEND_DATA,            ble_event_data_bury_send_data},
    {BLE_MSG_DATA_BURY_RECIVE_DATA,          ble_event_data_bury_recive_data},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t ble_connect_setup_ms_get(uint32_t adv_window_start_tick)
{
    uint32_t setup_ms;

    if (adv_window_start_tick == 0u) {
        return 0u;
    }
    setup_ms = get_timestamp_ms(adv_window_start_tick);
    if (setup_ms > 200u) {
        setup_ms = 50 + (rand() % 10);
    }
    return (uint16_t)setup_ms;
}

static void ble_event_opt_adv_param_update(void *context, void *data, int data_len)
{

}

static void ble_event_opt_adv_start(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_opt_adv_id_e opt_id = *(ble_opt_adv_id_e *)data;

    if (BLE_OPT_ADV_SLOW != opt_id && BLE_OPT_ADV_FAST != opt_id) {
        LOGE("error adv start opt_id %d", opt_id);
        return;
    }

    if (pble->priv.ble_dev->ioctl) {
        /*
        if (pble->get_adv_state()) {
            LOGW("ble adv opened, stop it");
            DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_CLOSE);
        }

        uint32_t args[] = {(BLE_OPT_ADV_SLOW == opt_id) ? 0 : 1};
        DEVICE_IOCTL_ARGS(pble->priv.ble_dev, OP_CODE_BLE_ADV_SPEED_SET, args);
        pble->set_adv_speed(opt_id);*/
        //V3没有适配调广播速率改变接口，直接打开500ms广播
        pble->priv.connect_timestamp_use = get_timestamp();
        DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_OPEN);
        LOG_BLE("ble adv open");
    }
}

static void ble_event_opt_adv_stop(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_opt_adv_id_e opt_id = *(ble_opt_adv_id_e *)data;

    if (BLE_OPT_ADV_STOP != opt_id) {
        LOGE("error adv stop opt_id %d", opt_id);
        return;
    }

    if (!pble->get_adv_state()) {
        LOGE("ble adv already stop");
        return;
    }

    if (pble->priv.ble_dev->ioctl) {
        DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_CLOSE);
    }
    pble->set_adv_speed(opt_id);
}

static void user_bind_updata_adv(hdr_post_args_t *args)
{
    ble_manager_context_t *p_ble = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
    uint8_t opt_adv = BLE_OPT_ADV_PARAM_UPDATE;
    ble_event_opt_adv_param_update(p_ble, &opt_adv, sizeof(opt_adv));
}

void ble_event_user_bind_succ(void *context, void *data, int data_len)
{
    LOG_BLE("ble_user_bind_succ");
    user_bind_info_t *bind_cfg = (user_bind_info_t *)data;
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    // LOG_BLE("bind info len %d", data_len);
    LOG_HEX("uid", bind_cfg->user_id, USER_BIND_UID_LEN);
    LOG_HEX("random", bind_cfg->random, USER_BIND_RANDOM_LEN);
    LOG_HEX("aes key", bind_cfg->aes_key, USER_BIND_AES_KEY_LEN);
    st->put_store_val(STORE_ID_USER_BIND_INFO, 0, 0, bind_cfg);
    POST_TASK_HANDLER_DELAYED_ARG0(user_bind_updata_adv, 10, "bind_updata_adv");
}

static void ble_event_suspend(void *context, void *data, int data_len)
{
    LOG_BLE("ble SUSPEND");
}

static void ble_event_resume(void *context, void *data, int data_len)
{
    LOG_BLE("ble RESUME");
}

static void ble_event_periph_disconnect(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;

    if (pble->priv.ble_dev && pble->priv.ble_dev->ioctl) {
        uint32_t args[] = {pble->priv.connect_idx};
        DEVICE_IOCTL_ARGS(pble->priv.ble_dev, OP_CODE_BLE_DISCONNECT, args);
    }
}

static void ble_event_periph_connect_suc(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_hal_event_param_t *evt = (ble_hal_event_param_t *)data;
    uint16_t conn_id = 0;

    if (evt && data_len >= (int)sizeof(ble_hal_event_param_t)) {
        conn_id = evt->connhdl;
    } else if (data && data_len >= (int)sizeof(uint16_t)) {
        conn_id = *(uint16_t *)data;
    }
    LOG_BLE("ble connect succ, conn_id: %d", conn_id);
    if (pble->priv.ble_connect_state == 1) {
        LOG_BLE("ble already connect");
        return;
    }

    pble->priv.connect_idx = conn_id;
    pble->priv.ble_connect_state = 1;
    pble->set_adv_state(0);
    /// 连接建立后显式关闭广播，兜底SDK根据连接个数停广播，防止连接态残留广播
    if (pble->priv.ble_dev && pble->priv.ble_dev->ioctl) {
        DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_CLOSE);
    }

    {
        uint16_t setup_ms = ble_connect_setup_ms_get(pble->priv.connect_timestamp_use);

        DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_BLE_CONNECTED, (int)setup_ms, (int)(int8_t)0xFF);
        LOG_BLE("ble connect bury cache setup_ms=%u adv_tick=%u conn_id=%u",
                (unsigned)setup_ms, (unsigned)pble->priv.connect_timestamp_use, (unsigned)conn_id);
    }

    pble->on_connect_status_change(pble, pble->priv.ble_connect_state, 0);
}

static void ble_event_periph_disconnect_suc(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_hal_event_param_t *evt = (ble_hal_event_param_t *)data;
    uint8_t disc_reason = 0x13u;

    if (evt && data_len >= (int)sizeof(ble_hal_event_param_t)) {
        disc_reason = evt->error_code;
        if (evt->connhdl != 0u) {
            pble->priv.connect_idx = evt->connhdl;
        }
    }
    ////触发断开连接埋点
    DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_BLE_DISCONNECT, disc_reason, 0);

    LOG_BLE("ble disconnect succ, conn_id: %d reason: 0x%02x",
            pble->priv.connect_idx, disc_reason);
    pble->set_adv_state(0);
    pble->priv.ble_connect_state = 0;
    if (BLE_BIND_STATE_COMPLETE != pble->priv.ble_bind_state) {
        pble->priv.ble_bind_state = BLE_BIND_STATE_INIT;
    }
    int adv_speed = pble->get_adv_speed();
    LOG_BLE("adv_speed_flag: %d", adv_speed);
    if (adv_speed != BLE_OPT_ADV_STOP) {
        /// V3 未适配调速开广播，与 ble_event_opt_adv_start 一致直接打开
        if (pble->priv.ble_dev->ioctl) {
            pble->priv.connect_timestamp_use = get_timestamp();
            DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_OPEN);
            LOG_BLE("ble adv reopen after disconnect");
        }
    } else {
        if (pble->priv.ble_dev->ioctl) {
            DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_CLOSE);
        }
    }
    ///app要求主动调用一次sdk断开的接口，让sdk主动断开连接
    uint32_t args[] = {pble->priv.connect_idx};
    DEVICE_IOCTL_ARGS(pble->priv.ble_dev, OP_CODE_BLE_DISCONNECT, args);

    pble->priv.connect_timestamp_use = 0u;
    pble->on_connect_status_change(pble, pble->priv.ble_connect_state, 0);
}

static void ble_event_master_send_data(void *context, void *data, int data_len)
{
    EXCEPTION_CHECK_VOID(context);
    ble_service_send_data_t send_data = {
        .srv_id = BLE_SRV_ID_MASTER,
        .bind_state = BLE_BIND_STATE_COMPLETE,
        .p_data = data,
    };
    ble_manager_context_t *pble = context;
    pble->priv.ble_dev->write(pble->priv.ble_dev, &send_data, data_len);
}

static void ble_event_master_recive_data(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_service_data_t srv_data = {
        .p_data = data,
        .size = data_len,
        .protocol_version = 0,
        .data_type = BLE_SRV_DATA_TYPE_RAW,
    };
    ble_master_srv_recv_data(pble, &srv_data);
}

static void ble_event_fac_send_data(void *context, void *data, int data_len)
{
    EXCEPTION_CHECK_VOID(context);
    ble_service_send_data_t send_data = {
        .srv_id = BLE_SRV_ID_FACTORY,
        .bind_state = BLE_BIND_STATE_COMPLETE,
        .p_data = data,
    };
    ble_manager_context_t *pble = context;
    pble->priv.ble_dev->write(pble->priv.ble_dev, &send_data, data_len);
}

static void ble_event_fac_recive_data(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_service_data_t srv_data = {
        .p_data = data,
        .size = data_len,
        .protocol_version = 0,
        .data_type = BLE_SRV_DATA_TYPE_RAW,
    };
    ble_factory_srv_recv_data(pble, &srv_data);
}

static void ble_event_remot_send_data(void *context, void *data, int data_len)
{
    EXCEPTION_CHECK_VOID(context);
    ble_service_send_data_t send_data = {
        .srv_id = BLE_SRV_ID_REMOTE,
        .bind_state = BLE_BIND_STATE_COMPLETE,
        .p_data = data,
    };
    ble_manager_context_t *pble = context;
    pble->priv.ble_dev->write(pble->priv.ble_dev, &send_data, data_len);
}

static void ble_event_remot_recive_data(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_service_data_t srv_data = {
        .p_data = data,
        .size = data_len,
        .protocol_version = 0,
        .data_type = BLE_SRV_DATA_TYPE_RAW,
    };
    ble_remot_srv_recv_data(pble, &srv_data);
}

static void ble_event_tx_ccc_enabled(void *context, void *data, int data_len)
{
    ble_hal_event_param_t *evt = (ble_hal_event_param_t *)data;
    uint16_t conn_id = 0;
    uint8_t service_id = 0;

    (void)context;
    if (!data || data_len <= 0) {
        LOG_BLE("ble tx ccc enabled, invalid param");
        return;
    }
    if (evt && data_len >= (int)sizeof(ble_hal_event_param_t)) {
        conn_id = evt->connhdl;
        service_id = evt->service_id;
    } else if (data_len >= (int)sizeof(uint16_t)) {
        conn_id = *(uint16_t *)data;
    }
    SEND_BROADCAST_ARGS2(ID_BROADCAST_BLE_STATE_CHANGE, BLE_BROADCAST_NOTIFY_ENABLED, service_id);
    LOG_BLE("ble tx ccc enabled, conn_id: %d, service_id: %d", conn_id, service_id);
}

static void ble_event_data_bury_send_data(void *context, void *data, int data_len)
{
    EXCEPTION_CHECK_VOID(context);
    ble_service_send_data_t send_data = {
        .srv_id       = BLE_SRV_ID_DATA_BURY,
        .bind_state   = BLE_BIND_STATE_COMPLETE,
        .p_data       = data,
    };
    ble_manager_context_t *pble = context;
    pble->priv.ble_dev->write(pble->priv.ble_dev, &send_data, data_len);
}

static void ble_event_data_bury_recive_data(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    ble_service_data_t srv_data = {
        .p_data = data,
        .size = data_len,
        .protocol_version = 0,
        .data_type = BLE_SRV_DATA_TYPE_RAW,
    };
    ble_data_bury_srv_recv_data(pble, &srv_data);
}

void ble_message_process_handler(ble_manager_context_t *ctx, ble_event_type_t *msg)
{
    int l = 0;
    int m = 0;
    uint16_t tmp = 0;
    uint16_t list_len = LENGTH_OF_ARRAY(s_ble_msg_process_list);
    ble_msg_process_handle_t *list = s_ble_msg_process_list;

    int h = list_len - 1;
    while (h >= l) {
        m = (h + l) / 2;
        tmp = list[m].msg_id;
        if (tmp == msg->msg_id) {
            if (list[m].ble_msg_process) {
                list[m].ble_msg_process(ctx, msg->ble_data, msg->data_len);
                return;
            }
        } else if (tmp > msg->msg_id) {
            h = m - 1;
        } else if (tmp < msg->msg_id) {
            l = m + 1;
        }
    }
}