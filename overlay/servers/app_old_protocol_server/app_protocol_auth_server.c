#include "root_common_api.h" // IWYU pragma: keep
#include "app_protocol_handler.h"

/*
* DEFINES
*****************************************************************************************
*/

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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void app_old_comm_handle_get_triple_req(void)
{
    LOG_SRV(" Handling GET_TRIPLE request");

    // TODO: Handle GET_TRIPLE request
    factory_dev_info_t factory_dev_info;
	uint8_t size_buf= 30;
    uint8_t *tmp_buf = (uint8_t *)u_malloz(size_buf);
    EXCEPTION_CHECK_VOID(tmp_buf);
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    /// 产测完成标识若未置位，重新置位，防止产测漏掉，后续app连上后会自动清掉
    factory_flag_info_t flag_info = {0};
    ctx->get_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
    if (flag_info.deploy_flag == 0) {
        flag_info.deploy_flag = 1;
        ctx->put_store_val(STORE_ID_FACTORY_FLAG_INFO, 0, 0, &flag_info);
        LOG_SRV("factory complete flag re-set");
    }
#if 0
	uint8_t product_id[] = "x77MDK";

	// cf5b895919e3412d
	uint8_t *devid = get_deviceID(); 

	uint8_t keysecret[] = "cf5b895919e3412d";
	uint8_t send_secret[] = "19e3412d";

	// 做一个flash校验，校验成功之后再进行复制；
	//  获取实际数据长度（排除结束符）
	const size_t len_prod = sizeof(product_id) - 1;
	const size_t len_keysecret = sizeof(keysecret) - 1;
	const size_t len_sec = sizeof(send_secret) - 1;
	const size_t len_devid = 16; // FFFF+16

#else
    const size_t len_prod = 6;
	const size_t len_sec = 8;
	const size_t len_devid = 16;
	uint8_t *product_id = factory_dev_info.product_id;
	uint8_t *devid = factory_dev_info.device_id;
	uint8_t send_secret[8] = {0};
	for (int i = 8; i < 16; i++)
	{
		send_secret[i - 8] = factory_dev_info.enctyption_key[i];
	}
    LOG_HEX("product_id:", factory_dev_info.product_id, factory_dev_info.product_id_len);
    LOG_HEX("device_id:", factory_dev_info.device_id, factory_dev_info.device_id_len);
    LOG_HEX("enctyption_key:", factory_dev_info.enctyption_key, factory_dev_info.enctyption_key_len);
#endif

	// 加密
	rc4(factory_dev_info.enctyption_key, 16, send_secret, len_sec);
	// 解密测试
	// rc4(keysecret, len_keysecret, send_secret, len_sec);

	// 创建目标数组
	// uint8_t combined[len_prod + len_devid + len_sec];
	memset(tmp_buf, 0, size_buf);	// 确保tmp_buf被清零
	memcpy(tmp_buf, product_id, len_prod);
	memcpy(tmp_buf + len_prod, devid, len_devid);
	memcpy((tmp_buf + len_prod + len_devid), send_secret, len_sec);

    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_TRIPLE,  30, tmp_buf);
	u_free(tmp_buf);
	tmp_buf = NULL;
}

void app_old_comm_handle_set_productid_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling SET_PRODUCTID request (data len: %d)", cal);

    // TODO: Handle setting product ID
    factory_dev_info_t factory_dev_info;
    EXCEPTION_CHECK_VOID(cab);
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);

	if (cal >= DEVICE_PRODUCT_ID_LEN)
	{
		factory_dev_info.product_id_len = DEVICE_PRODUCT_ID_LEN;
	}
	else
	{
		factory_dev_info.product_id_len = cal;
	}

	memset(factory_dev_info.product_id, 0, factory_dev_info.product_id_len);
	memcpy(factory_dev_info.product_id, cab, factory_dev_info.product_id_len);
	ctx->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_PRODUCTID, 0, NULL);
}

void app_old_comm_handle_set_deviceid_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling SET_DEVICEID request (data len: %d)", cal);

    // TODO: Handle setting device ID
    factory_dev_info_t factory_dev_info;
    EXCEPTION_CHECK_VOID(cab);
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);

	if (cal >= DEVICE_DEVICE_ID_LEN)
	{
		factory_dev_info.device_id_len = DEVICE_DEVICE_ID_LEN;
	}
	else
	{
		factory_dev_info.device_id_len = cal;
	}

	memset(factory_dev_info.device_id, 0, factory_dev_info.device_id_len);
	memcpy(factory_dev_info.device_id, cab, factory_dev_info.device_id_len);
	ctx->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_DEVICEID, 0, NULL);
}

void app_old_comm_handle_set_encryption_key_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling SET_ENCRYPTIONKEY request (data len: %d)", cal);

    // TODO: Handle setting encryption key
    factory_dev_info_t factory_dev_info;
    EXCEPTION_CHECK_VOID(cab);
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);

	if (cal >= DEVICE_ENCTYPTION_KEY_LEN)
	{
		factory_dev_info.enctyption_key_len = DEVICE_ENCTYPTION_KEY_LEN;
	}
	else
	{
		factory_dev_info.enctyption_key_len = cal;
	}

	memset(factory_dev_info.enctyption_key, 0, factory_dev_info.enctyption_key_len);
	memcpy(factory_dev_info.enctyption_key, cab, factory_dev_info.enctyption_key_len);
	ctx->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_ENCRYPTIONKEY, 0, NULL);
}

void app_old_comm_handle_get_sn_req(void)
{
    LOG_SRV(" Handling get SN request");

    factory_dev_info_t factory_dev_info;
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);

    EXCEPTION_CHECK_VOID(ctx);
    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    LOG_HEX("SN:", factory_dev_info.sn, factory_dev_info.sn_len);

    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_GET_SN, factory_dev_info.sn_len, factory_dev_info.sn);
}

void app_old_comm_handle_set_sn_req(uint8_t cal, uint8_t *cab)
{
    LOG_SRV(" Handling set SN request");

    factory_dev_info_t factory_dev_info;
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(cab);

    ctx->get_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);

	if (cal >= DEVICE_UNIQUE_SN_LEN)
	{
		factory_dev_info.sn_len = DEVICE_UNIQUE_SN_LEN;
	}
	else
	{
		factory_dev_info.sn_len = cal;
	}

	memset(factory_dev_info.sn, 0, factory_dev_info.sn_len);
	memcpy(factory_dev_info.sn, cab, factory_dev_info.sn_len);
	ctx->put_store_val(STORE_ID_FACTORY_DEV_INFO, 0, 0, &factory_dev_info);
    app_old_protocol_hander_frame_builder_send(APP_OLD_COMM_CT_ACK, APP_OLD_COMM_CID_SET_SN, 0, NULL);
}
