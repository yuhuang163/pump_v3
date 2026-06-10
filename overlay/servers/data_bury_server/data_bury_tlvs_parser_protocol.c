/*
 * Device Management Service (0x01) TLV 解析与应答
 */

#include "root_common_api.h"
#include "data_bury_tlvs_parser_protocol.h"
#include "app_data_bury_protocol_builder.h"

#include <string.h>

#define DATA_BURY_CONN_MFS_MIN    20u
#define DATA_BURY_CONN_MFS_MAX    APP_DATA_BURY_PROTOCOL_APP_PAYLOAD_MAX

typedef struct {
    uint8_t  has_prot_version;
    uint8_t  prot_version;
    uint8_t  has_mfs;
    uint16_t mfs;
    uint8_t  has_mtu;
    uint16_t mtu;
    uint8_t  has_tx_interval;
    uint16_t tx_interval;
    uint8_t  has_arq_mode;
    uint8_t  arq_mode;
    uint8_t  has_ack_timeout;
    uint32_t ack_timeout_ms;
    uint8_t  has_max_retransmit;
    uint8_t  max_retransmit;
} data_bury_conn_param_t;

static uint16_t s_negotiated_mfs = APP_DATA_BURY_PROTOCOL_MFS_DEFAULT;
static data_bury_conn_param_t s_negotiated_conn;

static uint16_t data_bury_read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t data_bury_read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int data_bury_tlvs_parser_send_tlv(uint8_t svc_id, uint8_t cmd_id,
                                       const uint8_t *tlv, uint16_t tlv_len)
{
    uint16_t app_len = (uint16_t)(2u + tlv_len);

    if (app_len > s_negotiated_mfs) {
        return app_data_bury_protocol_frame_builder_send_split(svc_id, cmd_id, tlv, tlv_len, s_negotiated_mfs);
    }
    return app_data_bury_protocol_frame_builder_send(svc_id, cmd_id, tlv, tlv_len);
}

static int data_bury_tlvs_parser_send_error_rsp(uint8_t svc_id, uint8_t cmd_id, uint32_t err_code)
{
    uint8_t tlv[6];
    uint16_t tlv_len;

    tlv_len = app_data_bury_protocol_tlv_put_u32(tlv, (uint16_t)sizeof(tlv),
                                                 APP_DATA_BURY_PROTOCOL_TLV_TYPE_ERROR, err_code);
    if (tlv_len == 0u) {
        return -1;
    }
    return data_bury_tlvs_parser_send_tlv(svc_id, cmd_id, tlv, tlv_len);
}

static void data_bury_conn_param_set_defaults(data_bury_conn_param_t *param)
{
    memset(param, 0, sizeof(*param));
    param->has_prot_version = 1u;
    param->prot_version = 0x01u;
    param->has_mfs = 1u;
    param->mfs = APP_DATA_BURY_PROTOCOL_MFS_DEFAULT;
    param->has_mtu = 1u;
    param->mtu = 23u;
    param->has_tx_interval = 1u;
    param->tx_interval = 0u;
    param->has_arq_mode = 1u;
    param->arq_mode = 0u;
}

static uint16_t data_bury_conn_param_build_tlvs(const data_bury_conn_param_t *param,
                                                uint8_t *buf, uint16_t cap)
{
    uint16_t pos = 0u;
    uint16_t n;

    if (!param || !buf) {
        return 0u;
    }
    if (param->has_prot_version) {
        n = app_data_bury_protocol_tlv_put_u8(&buf[pos], (uint16_t)(cap - pos),
                                              APP_BURY_DEV_TLV_PROT_VERSION, param->prot_version);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    if (param->has_mfs) {
        n = app_data_bury_protocol_tlv_put_u16(&buf[pos], (uint16_t)(cap - pos),
                                               APP_BURY_DEV_TLV_MAX_FRAME_SIZE, param->mfs);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    if (param->has_mtu) {
        n = app_data_bury_protocol_tlv_put_u16(&buf[pos], (uint16_t)(cap - pos),
                                               APP_BURY_DEV_TLV_MTU, param->mtu);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    if (param->has_tx_interval) {
        n = app_data_bury_protocol_tlv_put_u16(&buf[pos], (uint16_t)(cap - pos),
                                               APP_BURY_DEV_TLV_TX_INTERVAL, param->tx_interval);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    if (param->has_arq_mode) {
        n = app_data_bury_protocol_tlv_put_u8(&buf[pos], (uint16_t)(cap - pos),
                                              APP_BURY_DEV_TLV_ARQ_MODE, param->arq_mode);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    if (param->has_ack_timeout) {
        n = app_data_bury_protocol_tlv_put_u32(&buf[pos], (uint16_t)(cap - pos),
                                               APP_BURY_DEV_TLV_ACK_TIMEOUT_MS, param->ack_timeout_ms);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    if (param->has_max_retransmit) {
        n = app_data_bury_protocol_tlv_put_u8(&buf[pos], (uint16_t)(cap - pos),
                                              APP_BURY_DEV_TLV_MAX_RETRANSMIT, param->max_retransmit);
        if (n == 0u) {
            return 0u;
        }
        pos = (uint16_t)(pos + n);
    }
    return pos;
}

static int data_bury_parse_conn_param_tlvs(const uint8_t *tlv, uint16_t tlv_len,
                                           data_bury_conn_param_t *req)
{
    uint16_t pos = 0u;

    if (!tlv || !req) {
        return -1;
    }
    memset(req, 0, sizeof(*req));

    while (pos + APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE <= tlv_len) {
        uint8_t type = tlv[pos];
        uint8_t vlen = tlv[pos + 1u];
        const uint8_t *val = &tlv[pos + 2u];

        if ((uint16_t)(pos + APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + vlen) > tlv_len) {
            return -1;
        }

        switch (type) {
        case APP_BURY_DEV_TLV_PROT_VERSION:
            if (vlen != 1u) {
                return -1;
            }
            req->has_prot_version = 1u;
            req->prot_version = val[0];
            break;
        case APP_BURY_DEV_TLV_MAX_FRAME_SIZE:
            if (vlen != 2u) {
                return -1;
            }
            req->has_mfs = 1u;
            req->mfs = data_bury_read_be16(val);
            break;
        case APP_BURY_DEV_TLV_MTU:
            if (vlen != 2u) {
                return -1;
            }
            req->has_mtu = 1u;
            req->mtu = data_bury_read_be16(val);
            break;
        case APP_BURY_DEV_TLV_TX_INTERVAL:
            if (vlen != 2u) {
                return -1;
            }
            req->has_tx_interval = 1u;
            req->tx_interval = data_bury_read_be16(val);
            break;
        case APP_BURY_DEV_TLV_ARQ_MODE:
            if (vlen != 1u) {
                return -1;
            }
            req->has_arq_mode = 1u;
            req->arq_mode = val[0];
            break;
        case APP_BURY_DEV_TLV_ACK_TIMEOUT_MS:
            if (vlen != 4u) {
                return -1;
            }
            req->has_ack_timeout = 1u;
            req->ack_timeout_ms = data_bury_read_be32(val);
            break;
        case APP_BURY_DEV_TLV_MAX_RETRANSMIT:
            if (vlen != 1u) {
                return -1;
            }
            req->has_max_retransmit = 1u;
            req->max_retransmit = val[0];
            break;
        case APP_BURY_DEV_TLV_RANDOM_VALUE:
            break;
        default:
            break;
        }
        pos = (uint16_t)(pos + APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + vlen);
    }

    if (pos != tlv_len) {
        return -1;
    }
    return 0;
}

static int data_bury_negotiate_conn_param(const data_bury_conn_param_t *req,
                                          data_bury_conn_param_t *rsp)
{
    if (!req || !rsp) {
        return -1;
    }

    memset(rsp, 0, sizeof(*rsp));

    if (req->has_prot_version) {
        if (req->prot_version != 0x01u) {
            return -1;
        }
        rsp->has_prot_version = 1u;
        rsp->prot_version = req->prot_version;
    }
    if (req->has_mfs) {
        uint16_t mfs = req->mfs;

        if (mfs < DATA_BURY_CONN_MFS_MIN) {
            mfs = DATA_BURY_CONN_MFS_MIN;
        }
        if (mfs > DATA_BURY_CONN_MFS_MAX) {
            mfs = DATA_BURY_CONN_MFS_MAX;
        }
        rsp->has_mfs = 1u;
        rsp->mfs = mfs;
        s_negotiated_mfs = mfs;
    }
    if (req->has_mtu) {
        uint16_t mtu = req->mtu;

        if (mtu < 23u) {
            mtu = 23u;
        }
        if (mtu > 512u) {
            mtu = 512u;
        }
        rsp->has_mtu = 1u;
        rsp->mtu = mtu;
    }
    if (req->has_tx_interval) {
        rsp->has_tx_interval = 1u;
        rsp->tx_interval = req->tx_interval;
    }

    rsp->has_arq_mode = 1u;
    rsp->arq_mode = 0u;

    if (req->has_ack_timeout && rsp->arq_mode != 0u) {
        rsp->has_ack_timeout = 1u;
        rsp->ack_timeout_ms = req->ack_timeout_ms;
    }
    if (req->has_max_retransmit && rsp->arq_mode != 0u) {
        rsp->has_max_retransmit = 1u;
        rsp->max_retransmit = req->max_retransmit;
    }

    s_negotiated_conn = *rsp;
    return 0;
}

static int data_bury_handle_conn_param_negotiate(const uint8_t *tlv, uint16_t tlv_len)
{
    data_bury_conn_param_t req;
    data_bury_conn_param_t rsp;
    uint8_t rsp_tlv[64];
    uint16_t rsp_len;
    int ret;

    if (data_bury_parse_conn_param_tlvs(tlv, tlv_len, &req) != 0) {
        LOGE("data bury conn param tlv parse fail");
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_PARAM);
    }
    if (!req.has_mfs && !req.has_mtu) {
        LOGE("data bury conn param missing mfs/mtu");
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_PARAM);
    }

    if (data_bury_negotiate_conn_param(&req, &rsp) != 0) {
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_PARAM);
    }

    rsp_len = data_bury_conn_param_build_tlvs(&rsp, rsp_tlv, (uint16_t)sizeof(rsp_tlv));
    if (rsp_len == 0u) {
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_UNKNOWN);
    }

    ret = data_bury_tlvs_parser_send_tlv(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                      APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE,
                                      rsp_tlv, rsp_len);
    LOG_BURY("data bury conn param negotiated mfs=%u mtu=%u arq=%u ret=%d",
             (unsigned)rsp.mfs, (unsigned)rsp.mtu, (unsigned)rsp.arq_mode, ret);
    return ret;
}

static int data_bury_parse_set_time_tlvs(const uint8_t *tlv, uint16_t tlv_len,
                                         uint32_t *utc_time, int16_t *time_offset,
                                         uint8_t *has_time, uint8_t *has_offset)
{
    uint16_t pos = 0u;

    if (!tlv || !utc_time || !time_offset || !has_time || !has_offset) {
        return -1;
    }
    *has_time = 0u;
    *has_offset = 0u;

    while (pos + APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE <= tlv_len) {
        uint8_t type = tlv[pos];
        uint8_t vlen = tlv[pos + 1u];
        const uint8_t *val = &tlv[pos + 2u];

        if ((uint16_t)(pos + APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + vlen) > tlv_len) {
            return -1;
        }

        switch (type) {
        case APP_BURY_DEV_TLV_TIME:
            if (vlen != 4u) {
                return -1;
            }
            *has_time = 1u;
            *utc_time = data_bury_read_be32(val);
            break;
        case APP_BURY_DEV_TLV_TIME_OFFSET:
            if (vlen < 2u) {
                return -1;
            }
            *has_offset = 1u;
            *time_offset = (int16_t)data_bury_read_be16(val);
            break;
        default:
            break;
        }
        pos = (uint16_t)(pos + APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + vlen);
    }

    if (pos != tlv_len) {
        return -1;
    }
    return 0;
}

static void data_bury_apply_time_offset(int16_t time_offset)
{
    storage_manager_context_t *st;
    int8_t hour;
    int ret;

    hour = (int8_t)(time_offset >> 8);
    st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (!st || !st->put_store_ram_val) {
        return;
    }
    ret = st->put_store_ram_val(STORE_ID_SYS_TIME_ZONE, hour, 0, NULL);
    if (ret != 0) {
        LOGW("data bury save time_offset hour=%d fail ret=%d", (int)hour, ret);
    }
}

static int data_bury_handle_set_device_time(const uint8_t *tlv, uint16_t tlv_len)
{
    uint32_t utc_time = 0u;
    int16_t time_offset = 0;
    uint8_t has_time = 0u;
    uint8_t has_offset = 0u;
    time_system_t *time_sys;

    if (data_bury_parse_set_time_tlvs(tlv, tlv_len, &utc_time, &time_offset,
                                      &has_time, &has_offset) != 0) {
        LOGE("data bury set time tlv parse fail");
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_SET_DEVICE_TIME,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_PARAM);
    }
    if (!has_time || !has_offset) {
        LOGE("data bury set time missing tlv time=%u offset=%u",
             (unsigned)has_time, (unsigned)has_offset);
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_SET_DEVICE_TIME,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_PARAM);
    }

    time_sys = (time_system_t *)find_module(ID_MODULE_TIME);
    if (!time_sys || !time_sys->sync_utc_time) {
        LOGE("data bury set time: time module unavailable");
        return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                 APP_BURY_DEV_MGMT_CMD_SET_DEVICE_TIME,
                                                 APP_DATA_BURY_PROTOCOL_ERROR_UNKNOWN);
    }

    time_sys->sync_utc_time(utc_time);
    data_bury_apply_time_offset(time_offset);
    LOG_BURY("data bury set device time utc=%u offset=0x%04x",
             (unsigned)utc_time, (unsigned)time_offset);

    return data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                               APP_BURY_DEV_MGMT_CMD_SET_DEVICE_TIME,
                                               APP_DATA_BURY_PROTOCOL_ERROR_SUCCESS);
}

void data_bury_tlvs_parser_protocol_init(void)
{
    s_negotiated_mfs = APP_DATA_BURY_PROTOCOL_MFS_DEFAULT;
    data_bury_conn_param_set_defaults(&s_negotiated_conn);
}

uint16_t data_bury_tlvs_parser_get_negotiated_mfs(void)
{
    return s_negotiated_mfs;
}

void data_bury_tlvs_parser_handle_frame(const app_data_bury_protocol_frame_node_t *frame)
{
    int ret;

    if (!frame) {
        return;
    }

    switch (frame->cmd_id) {
    case APP_BURY_DEV_MGMT_CMD_CONN_PARAM_NEGOTIATE:
        ret = data_bury_handle_conn_param_negotiate(frame->payload, frame->payload_len);
        break;
    case APP_BURY_DEV_MGMT_CMD_SET_DEVICE_TIME:
        ret = data_bury_handle_set_device_time(frame->payload, frame->payload_len);
        break;
    default:
        LOGW("data bury unsupported dev mgmt cmd=0x%02x", frame->cmd_id);
        ret = data_bury_tlvs_parser_send_error_rsp(APP_DATA_BURY_PROTOCOL_SVC_DEVICE_MGMT,
                                                frame->cmd_id,
                                                APP_DATA_BURY_PROTOCOL_ERROR_CMD);
        break;
    }
    if (ret != 0) {
        LOGW("data bury dev mgmt rsp fail cmd=0x%02x ret=%d", frame->cmd_id, ret);
    }
}
