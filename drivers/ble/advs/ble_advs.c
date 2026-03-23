/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_common.h"  // IWYU pragma: keep
#include "ble_advs.h"
#include "gap_service.h"
#include "bes_gap_api.h"
#include "app_ble.h" // IWYU pragma: keep
#include "ble_core_common.h"
#include "factory_section.h"
/*
* DEFINES
*****************************************************************************************
*/
#define CUSTOM_ADV_SLOW_INTERVAL         100
#define CUSTOM_ADV_FAST_INTERVAL         100

#define IS_SUPPORT_OLD_ADV_DATA          1

#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define ADV_MAC_OFFSET 8
#define RSP_MAC_OFFSET 8
#else
#define ADV_MAC_OFFSET 5
#define RSP_MAC_OFFSET 5
#endif


/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_OLD_ADV_DATA
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
static uint8_t s_adv_rsp_data_set[] = {
    0x0A,                              /// length
    0xFF,                              /// manufacturer type 1
    0xAA,                              /// ROOT company flag
    ROOT_PRODUCT_ID,                  /// pump product code
    0x30,                              /// pump model code
    0x34,0x25,0x00,0x10,0xC5,0xDA, /// MAC(little endian)
    0x07,
    BES_GAP_AD_TYPE_COMPLETE_NAME,      /// type
    0x56, 0x33, 0x20, 0x50, 0x52, 0x4F, // name, V3 PRO
};

static uint8_t s_adv_data_set[] = {
    0x0A,                              /// length
    0xFF,                              /// manufacturer type 1
    0xAA,                              /// ROOT company flag
    ROOT_PRODUCT_ID,                  /// pump product code
    0x30,                              /// pump model code
    0x34,0x25,0x00,0x10,0xC5,0xDA, /// MAC(little endian)
    0x07,
    BES_GAP_AD_TYPE_COMPLETE_NAME,      /// type
    0x56, 0x33, 0x20, 0x50, 0x52, 0x4F, // name, V3 PRO
};
#else
static uint8_t s_adv_rsp_data_set[] = {
    0x0A,                              /// length
    0xFF,                              /// manufacturer type 1
    0xAA,                              /// ROOT company flag
    ROOT_PRODUCT_ID,                  /// pump product code
    0x30,                              /// pump model code
    0x34,0x25,0x00,0x10,0xC5,0xDA, /// MAC(little endian)
    0x03,
    BES_GAP_AD_TYPE_COMPLETE_NAME,      /// type
    0x56, 0x33, /// name, V3
};

static uint8_t s_adv_data_set[] = {
    0x0A,                              /// length
    0xFF,                              /// manufacturer type 1
    0xAA,                              /// ROOT company flag
    ROOT_PRODUCT_ID,                  /// pump product code
    0x30,                              /// pump model code
    0x34,0x25,0x00,0x10,0xC5,0xDA, /// MAC(little endian)
    0x03,
    BES_GAP_AD_TYPE_COMPLETE_NAME,      /// type
    0x56, 0x33, /// name, V3
};
#endif
#else
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
static uint8_t s_adv_data_set[] = {
    0x0D,                               // length
    BES_GAP_AD_TYPE_COMPLETE_NAME,      // type
    0x56, 0x33, 0x20, 0x50, 0x52, 0x4F, // name, V3 PRO
    0x2D,                               // '-'
    0x41, 0x41, 0x3A, 0x42, 0x42       // MAC, AA:BB
};

static uint8_t s_adv_rsp_data_set[] = {
    0x0D,                               // length
    BES_GAP_AD_TYPE_COMPLETE_NAME,      // type
    0x56, 0x33, 0x20, 0x50, 0x52, 0x4F, // name, V3 PRO
    0x2D,                               // '-'
    0x41, 0x41, 0x3A, 0x42, 0x42       // MAC, AA:BB
};
#else
static uint8_t s_adv_rsp_data_set[] = {
    0x09,                               // length
    BES_GAP_AD_TYPE_COMPLETE_NAME,      // type
    0x56, 0x33, // name, V3PRO
    0x2D,                               // '-'
    0x41, 0x41, 0x3A, 0x42, 0x42       // MAC, AA:BB
};
static uint8_t s_adv_data_set[] = {
    0x09,                               // length
    BES_GAP_AD_TYPE_COMPLETE_NAME,      // type
    0x56, 0x33, // name, V3PRO
    0x2D,                               // '-'
    0x41, 0x41, 0x3A, 0x42, 0x42       // MAC, AA:BB
};
#endif

#endif

static uint8_t s_mac_addr[6] = {0x20, 0x19, 0x07, 0xEC, 0x01, 0xFF};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ble_custom_mac_sync(void)
{
    /// TODO: 需要从storage中读取mac地址
    uint8_t *ble_addr = factory_section_get_ble_address();
    LOG_PBUS("factory mac address: %02x:%02x:%02x:%02x:%02x:%02x", 
        ble_addr[0], ble_addr[1], ble_addr[2], ble_addr[3], ble_addr[4], ble_addr[5]);
    memcpy(s_mac_addr, ble_addr, 6);
}

static void ble_custom_adv_data_update(void)
{

#if IS_SUPPORT_OLD_ADV_DATA
    memcpy(&s_adv_data_set[5], s_mac_addr, sizeof(s_mac_addr));
    // memcpy(&s_adv_rsp_data_set[5], s_mac_addr, sizeof(s_mac_addr));
#else
    char ascii_mac[8] = {0};
    LOG_PBUS("local mac address: %02x:%02x:%02x:%02x:%02x:%02x", 
        s_mac_addr[0], s_mac_addr[1], s_mac_addr[2], s_mac_addr[3], s_mac_addr[4], s_mac_addr[5]);
    HEX_TO_ASCII(s_mac_addr, 2, ascii_mac);
    s_adv_data_set[ADV_MAC_OFFSET]     = ascii_mac[0];
    s_adv_data_set[ADV_MAC_OFFSET + 1] = ascii_mac[1];
    s_adv_data_set[ADV_MAC_OFFSET + 2] = ':';
    s_adv_data_set[ADV_MAC_OFFSET + 3] = ascii_mac[2];
    s_adv_data_set[ADV_MAC_OFFSET + 4] = ascii_mac[3];

    s_adv_rsp_data_set[RSP_MAC_OFFSET]     = ascii_mac[0];
    s_adv_rsp_data_set[RSP_MAC_OFFSET + 1] = ascii_mac[1];
    s_adv_rsp_data_set[RSP_MAC_OFFSET + 2] = ':';
    s_adv_rsp_data_set[RSP_MAC_OFFSET + 3] = ascii_mac[2];
    s_adv_rsp_data_set[RSP_MAC_OFFSET + 4] = ascii_mac[3];
#endif

}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ble_custom_adv_init(gap_adv_param_t *adv_param, BLE_ADV_PARAM_T *legacy_param)
{
    ble_custom_mac_sync();
    ble_custom_adv_data_update();
    adv_param->connectable = true;
    adv_param->scannable = true;
    adv_param->use_legacy_pdu = true;
    adv_param->include_tx_power_data = true;
    adv_param->has_custom_adv_timing = true;
    adv_param->use_custom_local_addr = true;
    adv_param->own_addr_use_rpa = true;
    adv_param->own_addr_type = BT_ADDR_TYPE_RANDOM;
    memcpy(adv_param->custom_local_addr.address, s_mac_addr, 6);
    adv_param->fast_advertising = false;
    adv_param->adv_timing.min_adv_fast_interval_ms = CUSTOM_ADV_FAST_INTERVAL;
    adv_param->adv_timing.max_adv_fast_interval_ms = CUSTOM_ADV_FAST_INTERVAL;
    adv_param->adv_timing.min_adv_slow_interval_ms = CUSTOM_ADV_SLOW_INTERVAL;
    adv_param->adv_timing.max_adv_slow_interval_ms = CUSTOM_ADV_SLOW_INTERVAL;
    memcpy(legacy_param->advData, s_adv_data_set, sizeof(s_adv_data_set));
    legacy_param->advDataLen = sizeof(s_adv_data_set);
    memcpy(legacy_param->scanRspData, s_adv_rsp_data_set, sizeof(s_adv_rsp_data_set));
    #if IS_SUPPORT_OLD_ADV_DATA
    legacy_param->scanRspDataLen = 0;
    #else
    legacy_param->scanRspDataLen = sizeof(s_adv_rsp_data_set);
    #endif

}

void ble_custom_gap_advs_start(void)
{
    ble_global_t *g = ble_get_global();
    if (g->ble_stub_adv_enable) {
        LOG_PBUS("stub adv is already enabled");
        return;
    }
    ble_core_enable_stub_adv();
    app_ble_start_connectable_adv(500);
}

void ble_custom_gap_advs_stop(void)
{
    ble_global_t *g = ble_get_global();
    if (!g->ble_stub_adv_enable) {
        LOG_PBUS("stub adv is already disabled");
        return; 
    }
    ble_core_disable_stub_adv();
}

int ble_custom_gap_mac_addr_set(uint8_t *mac_addr)
{
    memcpy(s_mac_addr, mac_addr, 6);
    int ret = factory_section_set_ble_address(mac_addr);
    if (ret != 0) {
        LOG_PBUS("set ble address failed");
        return -1;
    }
    return ret;
}

int ble_custom_gap_mac_addr_get(uint8_t *mac_addr)
{
    uint8_t *ble_addr = factory_section_get_ble_address();
    if (ble_addr && mac_addr) {
        memcpy(mac_addr, ble_addr, 6);
        return 0;
    }
    return -1;
}
