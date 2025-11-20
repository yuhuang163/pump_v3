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
static void ble_event_algo_real_time(void *context, void *data, int data_len);
static void ble_event_algo_remind_data(void *context, void *data, int data_len);
static void ble_event_device_remind_data(void *context, void *data, int data_len);
static void ble_event_airplane_mode_changed(void *context, void *data, int data_len);
static void ble_event_breast_pump_state_change(void *context, void *data, int data_len);
static void ble_event_breast_pump_mode_change(void *context, void *data, int data_len);
static void ble_event_notify_app_settings(void *context, void *data, int data_len);
static void ble_event_master_send_data(void *context, void *data, int data_len);
static void ble_event_master_recive_data(void *context, void *data, int data_len);
static void ble_event_fac_send_data(void *context, void *data, int data_len);
static void ble_event_fac_recive_data(void *context, void *data, int data_len);
static void ble_event_remot_send_data(void *context, void *data, int data_len);
static void ble_event_remot_recive_data(void *context, void *data, int data_len);
static ble_msg_process_handle_t s_ble_msg_process_list[] = {
    {BLE_MSG_SET_ADV_UPDATE,                 ble_event_opt_adv_param_update},
    {BLE_MSG_SET_ADV_START,                  ble_event_opt_adv_start},
    {BLE_MSG_SET_ADV_STOP,                   ble_event_opt_adv_stop},
    {BLE_MSG_USER_BIND_SUCC,                 ble_event_user_bind_succ},
    {BLE_MSG_SUSPEND,                        ble_event_suspend},
    {BLE_MSG_RESUME,                         ble_event_resume},
    {BLE_MSG_PERIPH_DISCONN,                 ble_event_periph_disconnect},
    {BLE_MSG_PERIPH_CONNECT_SUC,             ble_event_periph_connect_suc},
    {BLE_MSG_PERIPH_DISCONN_SUC,             ble_event_periph_disconnect_suc},
    {BLE_MSG_SEND_PUMP_REALTIME_INFO,       ble_event_algo_real_time},
    {BLE_MSG_SEND_ALGO_REMIND_DATA,         ble_event_algo_remind_data},
    {BLE_MSG_SEND_DEVICE_REMIND_DATA,       ble_event_device_remind_data},
    {BLE_MSG_AIRPLANE_MODE_CHANGE,          ble_event_airplane_mode_changed},
    {BLE_MSG_BREAST_PUMP_STATE_CHANGE,      ble_event_breast_pump_state_change},
    {BLE_MSG_BREAST_PUMP_MODE_CHANGE,       ble_event_breast_pump_mode_change},
    {BLE_MSG_NOTIY_APP_SETTINGS,            ble_event_notify_app_settings},
    {BLE_MSG_MASTER_SEND_DATA,              ble_event_master_send_data},
    {BLE_MSG_MASTER_RECIVE_DATA,            ble_event_master_recive_data},
    {BLE_MSG_FAC_SEND_DATA,                 ble_event_fac_send_data},
    {BLE_MSG_FAC_RECIVE_DATA,               ble_event_fac_recive_data},
    {BLE_MSG_REMOT_SEND_DATA,               ble_event_remot_send_data},
    {BLE_MSG_REMOT_RECIVE_DATA,             ble_event_remot_recive_data},
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
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
        if (pble->get_adv_state()) {
            LOGW("ble adv opened, stop it");
            DEVICE_IOCTL(pble->priv.ble_dev, OP_CODE_BLE_ADV_CLOSE);
        }

        uint32_t args[] = {(BLE_OPT_ADV_SLOW == opt_id) ? 0 : 1};
        DEVICE_IOCTL_ARGS(pble->priv.ble_dev, OP_CODE_BLE_ADV_SPEED_SET, args);
        pble->set_adv_speed(opt_id);
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
    uint16_t conn_id = *(uint16_t *)data;
    LOG_BLE("ble connect succ, conn_id: %d", conn_id);
    pble->priv.connect_idx = conn_id;
    pble->priv.ble_connect_state = 1;
    pble->set_adv_state(0);
    pble->on_connect_status_change(pble, pble->priv.ble_connect_state, 0);
}

static void ble_event_periph_disconnect_suc(void *context, void *data, int data_len)
{
    ble_manager_context_t *pble = context;
    pble->set_adv_state(0);
    pble->priv.ble_connect_state = 0;
    if (BLE_BIND_STATE_COMPLETE != pble->priv.ble_bind_state) {
        pble->priv.ble_bind_state = BLE_BIND_STATE_INIT;
    }
    LOG_BLE("adv_speed_flag: %d", pble->priv.adv_speed_flag);
    if (pble->get_adv_speed() != BLE_OPT_ADV_STOP) {
        if (pble->priv.ble_dev->ioctl) {
            uint32_t args[] = {(BLE_OPT_ADV_SLOW == pble->get_adv_speed()) ? 0 : 1};
            DEVICE_IOCTL_ARGS(pble->priv.ble_dev, OP_CODE_BLE_ADV_SPEED_SET, args);
        }
    }
    pble->on_connect_status_change(pble, pble->priv.ble_connect_state, 0);
}

static void ble_event_algo_real_time(void *context, void *data, int data_len)
{

}

static void ble_event_algo_remind_data(void *context, void *data, int data_len)
{

}

static void ble_event_device_remind_data(void *context, void *data, int data_len)
{

}

static void ble_event_airplane_mode_changed(void *context, void *data, int data_len)
{

}

static void ble_event_breast_pump_state_change(void *context, void *data, int data_len)
{

}

static void ble_event_breast_pump_mode_change(void *context, void *data, int data_len)
{

}

static void ble_event_notify_app_settings(void *context, void *data, int data_len)
{

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
    CHECK_IF_VALID(pble->priv.master_cb, {
        pble->priv.master_cb(pble, data, data_len);
    });
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
    // LOG_BLE("fac send data size %d", data_len);
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
    CHECK_IF_VALID(pble->priv.factory_cb, {
        pble->priv.factory_cb(pble, data, data_len);
    });
}

static void ble_event_remot_send_data(void *context, void *data, int data_len)
{

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