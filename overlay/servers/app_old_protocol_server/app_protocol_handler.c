/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"
#include "protocol_manager_api.h"
#include "servers_manager_api.h"
#include "storage_manager_api.h"

/*
* DEFINES
*****************************************************************************************
*/
static app_old_protocol_handler_context_t ctx_app_old_protocol_handler = {0};

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void app_old_comm_poll_post_task_handler(hdr_post_args_t *args)
{
    //ota升级时，不上报设备状态
    ota_manager_context_t *ota_mod = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_VOID(ota_mod);
    if (ota_mod->is_ota_upgrading() == true) {
        LOG_SRV(" OTA upgrading, skip reporting device status");
        return;
    }
    LOG_SRV(" Poll timer triggered - reporting device status");
    app_old_comm_report_device_status(APP_OLD_COMM_LEFT_CT, OLD_PUBLISH_DATA_POLL_REPORT, NULL);
    app_old_comm_report_device_status(APP_OLD_COMM_RIGHT_CT, OLD_PUBLISH_DATA_POLL_REPORT, NULL);
    POST_TASK_HANDLER_DELAYED_ARG0(app_old_comm_poll_post_task_handler, 30000, "app_old_comm_poll_post_task");
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
// Handle request commands (APP->device)
void app_old_comm_handle_request_cmd(app_old_comm_cmd_id_e cid, uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling %s request (data len: %d)",
           app_old_comm_get_cmd_id_str(cid), cal);

    // Print command data if available
    if (cal > 0 && cab != NULL) {
        LOG_SRV(" Command data: ");
        LOG_HEX("app_recv_data hex", cab, cal);
    }
#if 0
    // 对除了传输加密key和测试以外的所有指令数据进行加密
    if (ctx_app_old_protocol_handler.encrypt_status == 1) {
        if ((cid != APP_OLD_COMM_CID_GET_ENCRYPT) && (cid != APP_OLD_COMM_CID_GET_DEV_INFO)) {
            app_old_protocol_data_cryption(ctx_app_old_protocol_handler.encrypt_key, cab, cal); //解密密ble数据
        }
    }
    LOG_HEX("app_recv_data cryption hex", cab, cal);
#endif
    switch (cid) {
        case APP_OLD_COMM_CID_HEARTBEAT:
            app_old_comm_handle_heartbeat_req();
            break;
        case APP_OLD_COMM_CID_GET_DEV_INFO:
            app_old_comm_handle_get_dev_info_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_DEV_SN:
        case APP_OLD_COMM_CID_GET_SN:
            app_old_comm_handle_get_sn_req();
        case APP_OLD_COMM_CID_SET_SN:
            app_old_comm_handle_set_sn_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_RTC:
            app_old_comm_handle_set_rtc_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SYS_CTRL:
            app_old_comm_handle_sys_ctrl_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_ENCRYPT:
            app_old_comm_handle_get_encrypt_req();
            break;
        case APP_OLD_COMM_CID_GET_STATUS:
            app_old_comm_handle_get_status_req(APP_OLD_COMM_LEFT_CT);
            app_old_comm_handle_get_status_req(APP_OLD_COMM_RIGHT_CT);
            break;
        case APP_OLD_COMM_CID_CTRL_DEV:
            app_old_comm_handle_ctrl_dev_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_CTRL_DEV_STATUS:
            app_old_comm_handle_ctrl_dev_sys_status_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_BIND:
            app_old_comm_handle_set_bind_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_USER_DEFINE:
            app_old_comm_handle_set_user_define_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_PARA_DOWNLOAD:
            app_old_comm_handle_set_trail_user_define_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_TRAVEL_LOCK:
            app_old_comm_handle_set_travel_lock_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_TRIPLE:
            app_old_comm_handle_get_triple_req();
            break;
        case APP_OLD_COMM_CID_SET_PRODUCTID:
            app_old_comm_handle_set_productid_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_DEVICEID:
            app_old_comm_handle_set_deviceid_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_ENCRYPTIONKEY:
            app_old_comm_handle_set_encryption_key_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_TASTE_CTRL:
            app_old_comm_handle_ctrl_taste_dev_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_RESET_STATUS:
            app_old_comm_handle_ctrl_reset_dev_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_TASTE_MODE_STATE:
            app_old_comm_handle_get_taste_dev_status_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_RECORD_NUM:
            app_old_comm_handle_get_record_num_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_RECORD_DATA:
            app_old_comm_handle_get_record_data_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_PUMP_BLE_RGB_SET_CTRL:
            app_old_comm_handle_pump_ble_rgb_set_ctrl_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_CLEAR_CUSTOM_MODE:
            app_old_comm_handle_clear_trail_custom_mode_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_SET_BILATERAL_PUMP_PARAM:
            app_old_comm_handle_set_bilateral_pump_param_req(cal, cab);
            break;
        case APP_OLD_COMM_CID_GET_ABNORMAL_INFOR:
            app_old_comm_handle_dev_abnormal_app_req(cal, cab);
            break;
        default:
            LOG_SRV(" Unknown request command 0x%02X", cid);
            break;
    }
}

// Handle report commands (device->APP)
void app_old_comm_handle_response_cmd(app_old_comm_cmd_id_e cid, uint8_t cal, void *cab)
{
    // LOG_SRV(" Handling %s (data len: %d)", app_old_comm_get_cmd_id_str(cid), cal);
    servers_publish_data_args_t *args_temp = (servers_publish_data_args_t *)cab;
    switch (cid) {
        case APP_OLD_COMM_CID_NOTIFY_STATUS: {
            ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
            EXCEPTION_CHECK_VOID(ble_ctx);
            if (ble_ctx->ble_is_connected() == true) {
                app_old_comm_report_device_status(APP_OLD_COMM_LEFT_CT, args_temp->arg1, args_temp->val);
                app_old_comm_report_device_status(APP_OLD_COMM_RIGHT_CT, args_temp->arg1, args_temp->val);
            }
        } break;
        case APP_OLD_COMM_CID_TASTE_REPORT: {
            ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
            EXCEPTION_CHECK_VOID(ble_ctx);
            if (ble_ctx->ble_is_connected() == true) {
                app_old_comm_handle_report_taste_dev_status(args_temp->arg1, args_temp->val);
            }
        } break;
        case APP_OLD_COMM_CID_NOTIFY_V3_DEV_ABNORMAL: {
            ble_manager_context_t *ble_ctx = (ble_manager_context_t *)find_module(ID_MODULE_BLE);
            EXCEPTION_CHECK_VOID(ble_ctx);
            if (ble_ctx->ble_is_connected() == true) {
                app_old_comm_handle_dev_abnormal_report(args_temp);
            }
        } break;
        case APP_OLD_COMM_CID_NOTIFY_BIND: //没用到
            app_old_comm_report_bind_status(1, 0);
            break;
        case APP_OLD_COMM_CID_NOTIFY_DISCON: {
            if (args_temp->arg1 == true) //连接
            {
                LOG_SRV(" ble connect");
                POST_TASK_HANDLER_DELAYED_ARG0(app_old_comm_poll_post_task_handler, 30000, "app_old_comm_poll_post_task");
                /* code */
            } else if (args_temp->arg1 == false) //断连接
            {
                LOG_SRV(" ble disconnect");
                app_old_comm_report_discon_status();
                REMOVE_TASK_HANDLER_DIRECTLY("app_old_comm_poll_post_task");
            }
#if 0
                ctx_app_old_protocol_handler->encrypt_status = 0; //断开或连接后重置加密状态
                if (args_temp->arg1) {
                    do {
                        ctx_app_old_protocol_handler->encrypt_key = (uint8_t)rand();
                    } while ((ctx_app_old_protocol_handler->encrypt_key == 0) || (ctx_app_old_protocol_handler->encrypt_key == 0xFF)); // 不能为0或者0xFF
                } else {
                    ;
                }
                LOG_SRV("ctx_app_old_protocol_handler->encrypt_key %d", ctx_app_old_protocol_handler->encrypt_key);
#endif
        } break;
        default:
            LOG_SRV(" Unhandled response command %s (0x%02X)",
                   app_old_comm_get_cmd_id_str(cid), cid);
            break;
    }
}

void app_old_protocol_hander_frame_builder_send(app_old_commcmd_type_e cmdtype, app_old_comm_cmd_id_e cid,
                                                uint8_t data_len, uint8_t *data)
{
    EXCEPTION_CHECK_VOID(ctx_app_old_protocol_handler.protocol_ctx);
#if 0
    // 对除了传输加密key和测试以外的所有指令数据进行加密
    if (ctx_app_old_protocol_handler.encrypt_status == 1) {
        if ((cid != APP_OLD_COMM_CID_GET_ENCRYPT) && (cid != APP_OLD_COMM_CID_GET_DEV_INFO)) {
            app_old_protocol_data_cryption(ctx_app_old_protocol_handler.encrypt_key, data, data_len); //解密密ble数据
        }
    }
    LOG_HEX("app_recv_data cryption hex", data, data_len);
#endif
    ctx_app_old_protocol_handler.protocol_ctx->app_old_protocol_frame_builder_send(cmdtype, cid, data_len, data);
}

void app_old_protocol_ctx_init(void)
{
    memset(&ctx_app_old_protocol_handler, 0, sizeof(app_old_protocol_handler_context_t));
    ctx_app_old_protocol_handler.protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
}

app_old_protocol_handler_context_t *app_old_protocol_handler_ctx_get(void)
{
    return &ctx_app_old_protocol_handler;
}
