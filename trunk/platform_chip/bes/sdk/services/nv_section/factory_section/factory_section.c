/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "cmsis.h"
#include "plat_types.h"
#include "tgt_hardware.h"
#include "string.h"
#include "crc32_c.h"
#include "factory_section.h"
#include "pmu.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "heap_api.h"

extern uint32_t __factory_start[];
extern uint32_t __factory_end[];

static factory_section_t *factory_section_p = NULL;
static factory_section_wifi_t *factory_section_wifi_p = NULL;
static uint8_t nv_record_dev_rev = nvrec_current_version;
static uint8_t nv_record_dev_rev_wifi = nvrec_current_version;
static int rf_band_select = 0x0;//1-5G,0-2G

static uint8_t factory_memrory[FACTORY_SECTOR_SIZE];

static uint8_t* factory_get_mem(void)
{
    return factory_memrory;
}

#ifndef RAM_NV_RECORD
static void factory_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;

    opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    TRACE(5,"%s:type = %d, addr = 0x%x,len = 0x%x,result = %d,suspend_num=%d.",
                __func__,
                opera_result->type,
                opera_result->addr,
                opera_result->len,
                opera_result->result,
                opera_result->suspend_num);
}
#endif


static bool factory_init = false;
void factory_section_init(void)
{
    if (factory_init == true) {
        TRACE(0, "factory section has registered! So do not register again");
        return ;
    }
    factory_init = true;
#ifndef RAM_NV_RECORD
    enum NORFLASH_API_RET_T result;
    enum HAL_FLASH_ID_T flash_id;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint32_t page_size = 0;

    flash_id = norflash_api_get_dev_id_by_addr((uint32_t)__factory_start);
    hal_norflash_get_size(flash_id,
               NULL,
               &block_size,
               &sector_size,
               &page_size);
    result = norflash_api_register(NORFLASH_API_MODULE_ID_FACTORY,
                    flash_id,
                    (uint32_t)__factory_start,
                    (uint32_t)__factory_end - (uint32_t)__factory_start,
                    block_size,
                    sector_size,
                    page_size,
                    FACTORY_SECTOR_SIZE,
                    factory_callback
                    );
    ASSERT(result == NORFLASH_API_OK,"nv_record_init: module register failed! result = %d.",result);
#endif
}

int factory_section_wifi_open()
{
    uint8_t wifi_addr[6] = {0x00, 0x80, 0xe1, 0x30, 0x50, 0x60};

    factory_section_wifi_p = (factory_section_wifi_t *)((uint32_t)__factory_start + FACTORY_SECTION_WIFI_OFFSET);

    //DUMP8("%02x ", factory_section_wifi_p, sizeof(factory_section_wifi_t));
    //TRACE(0, "\nfactory_section_wifi_p:%p, len:%d", factory_section_wifi_p, sizeof(factory_section_wifi_t));

    if (factory_section_wifi_p->head.magic != nvrec_dev_magic)
    {
        TRACE(0, "wifi factory magic error:0x%x", factory_section_wifi_p->head.magic);
        factory_section_wifi_p = NULL;
        return -1;
    }
    if ((factory_section_wifi_p->head.version < nvrec_mini_version) ||
        (factory_section_wifi_p->head.version > nvrec_current_version))
    {
        TRACE(0, "wifi factory version error:%d", factory_section_wifi_p->head.version);
        factory_section_wifi_p = NULL;
        return -1;
    }

    nv_record_dev_rev_wifi = factory_section_wifi_p->head.version;
    if (factory_section_wifi_p->head.crc !=
        crc32_c(0,(unsigned char *)(&(factory_section_wifi_p->data)), sizeof(factory_section_wifi_data_t)))
    {
        TRACE(0, "wifi factory crc error.");
        factory_section_wifi_p = NULL;
        return -1;
    }

    //TRACE(0, "open wifi factory section success");
    memcpy(wifi_addr, factory_section_wifi_p->data.mac_addr, 6);

    DUMP8("%02x ", wifi_addr, 6);
    return 0;
}

#ifdef WIFI_FACTORY_EN
void factory_section_reset(void)
{
    uint32_t lock;
    unsigned char ble_addr[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    unsigned char bt_addr[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    factory_section_t* bt_section = NULL;
    enum NORFLASH_API_RET_T ret;
    uint8_t *mempool = NULL;

    mempool = factory_get_mem();
    if (mempool == NULL) {
        TRACE(0, "%s syspool get memory fail",__func__);
        return ;
    }
    memset(mempool, 0, FACTORY_SECTOR_SIZE);

    bt_section = (factory_section_t *)(mempool);
    factory_section_p = (factory_section_t *)__factory_start;

    if (factory_section_p->head.magic != nvrec_dev_magic || factory_section_p->head.version != nvrec_current_version) {
        TRACE(0, "%s bt factory is not right,so create it",__func__);

        bt_section->head.magic = nvrec_dev_magic;
        bt_section->head.version = nvrec_current_version;

        memcpy((uint8_t*)bt_section->data.rev2_ble_addr, (uint8_t*)ble_addr, 6);
        memcpy((uint8_t*)bt_section->data.rev2_bt_addr, (uint8_t*)bt_addr, 6);

        memcpy(bt_section->data.rev2_bt_name, "cust_bt", strlen("cust_bt"));
        memcpy(bt_section->data.rev2_ble_name, "bes_ble", strlen("bes_ble"));

        bt_section->head.crc = crc32_c(0, (unsigned char *)(&(bt_section->data)), sizeof(factory_section_t));
    } else {
        memcpy(bt_section, factory_section_p, FACTORY_SECTOR_SIZE / 2);
    }

    factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool + FACTORY_SECTION_WIFI_OFFSET);
    factory_section_wifi_p = (factory_section_wifi_t *)((uint32_t)__factory_start + FACTORY_SECTION_WIFI_OFFSET);

    wifi_factory->head.magic = nvrec_dev_magic;
    wifi_factory->head.version = nvrec_current_version;

    wifi_factory->data.mac_addr[0] = 0x00;
    wifi_factory->data.mac_addr[1] = 0x80;
    wifi_factory->data.mac_addr[2] = 0x43;
    wifi_factory->data.mac_addr[3] = 0x55;
    wifi_factory->data.mac_addr[4] = 0x78;
    wifi_factory->data.mac_addr[5] = 0x45;

    wifi_factory->data.tx_power_type = 0xba;
    wifi_factory->data.tx_power_flags_5G = 0;

    wifi_factory->data.bt_tx_power[0] = 0x00000055;
    wifi_factory->data.bt_tx_power[1] = 0x0000340d;
    wifi_factory->data.bt_tx_power[2] = 0x00000005;
    wifi_factory->data.bt_tx_power[3] = 0x00000014;

    nv_record_dev_rev_wifi = wifi_factory->head.version;
    wifi_factory->head.crc =
        crc32_c(0,(unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));

    lock = int_lock_global();

    ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
    ASSERT(ret == NORFLASH_API_OK, "factory_section_set_contry_code: erase failed! ret = %d.", ret);
    ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
    ASSERT(ret == NORFLASH_API_OK, "factory_section_set_contry_code: write failed! ret = %d.", ret);

    int_unlock_global(lock);

    TRACE(0, "%s The factory create by software success!",__func__);
}
#endif

int factory_section_open(void)
{
    factory_section_p = (factory_section_t *)__factory_start;

#ifdef RAM_NV_RECORD
    factory_section_set_bt_address(bt_global_addr);
    factory_section_set_ble_address(ble_global_addr);
#else
#ifdef WIFI_FACTORY_EN
    if (factory_section_wifi_open())
    {
        TRACE(0, "There is no factory data, rebuild it");
        factory_section_reset();
        factory_section_wifi_open();
    }
#endif

    if (factory_section_p->head.magic != nvrec_dev_magic)
    {
        factory_section_p = NULL;
        return -1;
    }
    if ((factory_section_p->head.version < nvrec_mini_version) ||
        (factory_section_p->head.version > nvrec_current_version))
    {
        factory_section_p = NULL;
        return -1;
    }

    nv_record_dev_rev = factory_section_p->head.version;

    if (1 == nv_record_dev_rev)
    {
        if (factory_section_p->head.crc !=
            crc32_c(0,(unsigned char *)(&(factory_section_p->head.reserved0)),
            sizeof(factory_section_t)-2-2-4-(5+63+2+2+2+1+8)*sizeof(int))){
            factory_section_p = NULL;
            return -1;
        }

        memcpy(bt_global_addr, factory_section_p->data.bt_address, 6);
        memcpy(ble_global_addr, factory_section_p->data.ble_address, 6);
        TRACE(2,"%s sucess btname:%s", __func__, factory_section_p->data.device_name);
    }
    else
    {
        // check the data length
        if (((uint32_t)(&((factory_section_t *)0)->data.rev2_reserved0)+
            factory_section_p->data.rev2_data_len) > 4096)
        {
            TRACE(1,"nv rec dev data len %d has exceeds the facory sector size!.",
                factory_section_p->data.rev2_data_len);
            return -1;
        }

        if (factory_section_p->data.rev2_crc !=
            crc32_c(0,(unsigned char *)(&(factory_section_p->data.rev2_reserved0)),
            factory_section_p->data.rev2_data_len)){
            factory_section_p = NULL;
            return -1;
        }

        memcpy(bt_global_addr, factory_section_p->data.rev2_bt_addr, 6);
        memcpy(ble_global_addr, factory_section_p->data.rev2_ble_addr, 6);
        TRACE(2,"%s sucess btname:%s", __func__, (char *)factory_section_p->data.rev2_bt_name);
    }
#endif
    DUMP8("%02x ", bt_global_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    DUMP8("%02x ", ble_global_addr, BT_ADDR_OUTPUT_PRINT_NUM);
    return 0;
}

uint8_t* factory_section_get_bt_address(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.bt_address);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_bt_addr);
        }
    }
    else
    {
        return bt_global_addr;
    }
}

uint8_t* factory_section_get_ble_address(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.ble_address);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_ble_addr);
        }
    }
    else
    {
        return ble_global_addr;
    }
}

uint8_t* factory_section_get_default_peer_bt_address(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.default_peer_bt_address);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_default_peer_bt_address);
        }
    }
    else
    {
        return NULL;
    }
}

uint8_t factory_section_get_default_bt_nv_role(void)
{
    uint8_t bt_nv_role = NV_ROLE_IBRT_UNKNOWN;

    if (factory_section_p) {
        if (1 == nv_record_dev_rev) {
            TRACE(0, "WARNING! bt nv role only used in rev 2");
        } else {
            bt_nv_role = factory_section_p->data.rev2_default_bt_nv_role;

            if ((bt_nv_role != NV_ROLE_IBRT_MASTER) && (bt_nv_role != NV_ROLE_IBRT_SLAVE)) {
                TRACE(0, "WARNING! nvrecord bt role invalid: %d!", bt_nv_role);
                bt_nv_role = NV_ROLE_IBRT_UNKNOWN;
            }
        }
    }

    return bt_nv_role;
}

int factory_section_set_bt_address(uint8_t* btAddr)
{
    uint32_t lock;
    uint8_t* tmpBuf = NULL;
    if (factory_section_p){
        TRACE(0, "Update bt addr as:");
        DUMP8("%02x ", btAddr, BT_ADDR_OUTPUT_PRINT_NUM);

        tmpBuf = factory_get_mem();
        if (tmpBuf == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }

        lock = int_lock_global();
        memcpy(tmpBuf, factory_section_p, FACTORY_SECTOR_SIZE);
        if (1 == nv_record_dev_rev)
        {
            memcpy(((factory_section_t *)tmpBuf)->data.bt_address,
                btAddr, 6);
            ((factory_section_t *)tmpBuf)->head.crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            memcpy((uint8_t *)&(((factory_section_t *)tmpBuf)->data.rev2_bt_addr), btAddr, 6);
            ((factory_section_t *)tmpBuf)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->data.rev2_reserved0)),
                factory_section_p->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, tmpBuf, FACTORY_SECTOR_SIZE);
#else
        enum NORFLASH_API_RET_T ret;
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK,"%s: erase failed! ret = %d.", __FUNCTION__, ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start, (uint8_t *)tmpBuf, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK,"%s: write failed! ret = %d.", __FUNCTION__, ret);
#endif
        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}

int factory_section_set_ble_address(uint8_t* bleAddr)
{
    uint32_t lock;
    uint8_t* tmpBuf = NULL;

    if (factory_section_p){
        TRACE(0, "Update ble addr as:");
        DUMP8("%02x ", bleAddr, BT_ADDR_OUTPUT_PRINT_NUM);

        tmpBuf = factory_get_mem();
        if (tmpBuf == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }

        lock = int_lock_global();
        memcpy(tmpBuf, factory_section_p, FACTORY_SECTOR_SIZE);
        if (1 == nv_record_dev_rev)
        {
            memcpy(((factory_section_t *)tmpBuf)->data.ble_address,
                bleAddr, 6);
            ((factory_section_t *)tmpBuf)->head.crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->head.reserved0)), sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            memcpy((uint8_t *)&(((factory_section_t *)tmpBuf)->data.rev2_ble_addr),
                bleAddr, 6);
            ((factory_section_t *)tmpBuf)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)tmpBuf)->data.rev2_reserved0)),
                factory_section_p->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, tmpBuf, FACTORY_SECTOR_SIZE);
#else
        enum NORFLASH_API_RET_T ret;
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK,"%s: erase failed! ret = %d.",__FUNCTION__,ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY,(uint32_t)__factory_start, (uint8_t*)tmpBuf,FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK,"%s: write failed! ret = %d.",__FUNCTION__,ret);
#endif
        int_unlock_global(lock);

        return 0;
    }else{
        return -1;
    }
}

uint8_t* factory_section_get_bt_name(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)&(factory_section_p->data.device_name);
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_bt_name);
        }
    }
    else
    {
        return (uint8_t *)BT_LOCAL_NAME;
    }
}

int factory_section_set_bt_name(const char *name,int len)
{
    uint8_t *mempool = NULL;
    if (factory_section_p)
    {
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        if (1 == nv_record_dev_rev)
        {
            TRACE(2,"%s [OLD] %s -> [NEW1] %s", __func__, factory_section_p->data.device_name,name);
            memcpy(((factory_section_t *)mempool)->data.device_name, name, len);
            ((factory_section_t *)mempool)->head.crc = crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            TRACE(2,"%s [OLD] %s -> [NEW2] %s", __func__, (char *)factory_section_p->data.rev2_bt_name,name);
            memcpy(((factory_section_t *)mempool)->data.rev2_bt_name, name, len);
            ((factory_section_t *)mempool)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->data.rev2_reserved0)),
                ((factory_section_t *)mempool)->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_xtal_fcap_set: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t*)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_xtal_fcap_set: write failed! ret = %d.", ret);
        int_unlock_global(lock);
#endif
        memcpy(factory_section_p->data.rev2_bt_name, name, len);
        return 0;
    }
    else
    {
        return -1;
    }
}

int factory_section_set_ble_name(const char *name)
{
    uint8_t *mempool = NULL;
    uint8_t len = strlen(name) + 1;

    if (factory_section_p)
    {
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        if (1 == nv_record_dev_rev)
        {
            TRACE(2,"%s [OLD] %s -> [NEW1] %s", __func__, factory_section_p->data.device_name, name);
            memcpy(((factory_section_t *)mempool)->data.device_name, name, len);
            ((factory_section_t *)mempool)->head.crc = crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            TRACE(2,"%s [OLD] %s -> [NEW2] %s", __func__, (char *)factory_section_p->data.rev2_ble_name, name);
            memcpy(((factory_section_t *)mempool)->data.rev2_ble_name, name, len);
            ((factory_section_t *)mempool)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->data.rev2_reserved0)),
                ((factory_section_t *)mempool)->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();
        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_xtal_fcap_set: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t*)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_xtal_fcap_set: write failed! ret = %d.", ret);
        int_unlock_global(lock);
#endif
        memcpy(factory_section_p->data.rev2_ble_name, name, len);
        return 0;
    }
    else
    {
        return -1;
    }
}

uint8_t* factory_section_get_ble_name(void)
{
    if (factory_section_p)
    {
        if (1 == nv_record_dev_rev)
        {
            return (uint8_t *)BLE_DEFAULT_NAME;
        }
        else
        {
            return (uint8_t *)&(factory_section_p->data.rev2_ble_name);
        }
    }
    else
    {
        return (uint8_t *)BLE_DEFAULT_NAME;
    }
}

uint32_t factory_section_get_version(void)
{
    if (factory_section_p)
    {
        return nv_record_dev_rev;
    }

    return 0;
}

int factory_section_xtal_fcap_get(unsigned int *xtal_fcap)
{
#ifdef IS_USE_DEFAULT_XTAL_FCAP
    return DEFAULT_XTAL_FCAP;
#endif
    if (factory_section_p){
        if (1 == nv_record_dev_rev)
        {
            *xtal_fcap = factory_section_p->data.xtal_fcap;
        }
        else
        {
            *xtal_fcap = factory_section_p->data.rev2_xtal_fcap;
        }
        return 0;
    }else{
        return -1;
    }
}

int factory_section_xtal_fcap_set(unsigned int xtal_fcap)
{
    uint8_t *mempool = NULL;

    if (factory_section_p){
        TRACE(1,"factory_section_xtal_fcap_set:%d", xtal_fcap);
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);
        if (1 == nv_record_dev_rev)
        {
            ((factory_section_t *)mempool)->data.xtal_fcap = xtal_fcap;
            ((factory_section_t *)mempool)->head.crc = crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->head.reserved0)),sizeof(factory_section_t)-2-2-4);
        }
        else
        {
            ((factory_section_t *)mempool)->data.rev2_xtal_fcap = xtal_fcap;
            ((factory_section_t *)mempool)->data.rev2_crc =
                crc32_c(0,(unsigned char *)(&(((factory_section_t *)mempool)->data.rev2_reserved0)),
                factory_section_p->data.rev2_data_len);
        }
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_xtal_fcap_set: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start,(uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_xtal_fcap_set: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }else{
        return -1;
    }
}

void factory_section_original_btaddr_get(uint8_t *btAddr)
{
    if(factory_section_p){
        TRACE(0,"get factory_section_p");
        if (1 == nv_record_dev_rev)
        {
            memcpy(btAddr, factory_section_p->data.bt_address, 6);
        }
        else
        {
            memcpy(btAddr, factory_section_p->data.rev2_bt_addr, 6);
        }
    }else{
        TRACE(0,"get bt_addr");
        memcpy(btAddr, bt_global_addr, 6);
    }
}

int factory_section_wifi_band_status(int band_type)
{
    rf_band_select = band_type;
    return 0;
}

int factory_section_get_wifi_band()
{
    return rf_band_select;
}

uint8_t* factory_section_get_wifi_address(void)
{
    if (factory_section_wifi_p)
    {
        if ((factory_section_wifi_p->data.mac_addr[0] & 0x1) != 1)
        {
            return (uint8_t *)&(factory_section_wifi_p->data.mac_addr);
        }
    }
    return NULL;
}

int factory_section_set_wifi_address(uint8_t *wifi_addr)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_wifi_address");
        DUMP8("%02x ", wifi_addr, 6);
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);
        memcpy(wifi_factory->data.mac_addr, wifi_addr, 6);
        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
        /*
         * warning: falsh erase may run into busy loop if another CPU access FLASH.
         * CP doesn't use FLASH running application, so this only occur when MCU erase FLASH while CP is copying text and data from FLASH in CP initialization.
         * I have moved cp_server_init a little earlier in apps_init to prevent this situation.
         */
        ///TODO: check cp_accel_init_done()
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

uint8_t* factory_section_get_wifi_tx_power(void)
{
    if (factory_section_wifi_p)
    {
        int i;
        for (i = 0; i < ARRAY_SIZE(factory_section_wifi_p->data.tx_power_ch); ++i)
            if (factory_section_wifi_p->data.tx_power_ch[i] != 0xFF &&
                factory_section_wifi_p->data.tx_power_ch[i] != 0)
                break;
        if (i != ARRAY_SIZE(factory_section_wifi_p->data.tx_power_ch))
            return (uint8_t *)&(factory_section_wifi_p->data.tx_power_ch);
    }
    return NULL;
}

int  factory_section_set_wifi_tx_power_ch(int power,int index)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_wifi_tx_power_ch");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail", __func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        if (rf_band_select == 1)
        {
            wifi_factory->data.tx_power_ch_5G[index] = power;
            TRACE(0, "  tx_power_ch_5G = 0x%04x", wifi_factory->data.tx_power_ch_5G[index]);
        }
        else if (rf_band_select == 0)
        {
            wifi_factory->data.tx_power_ch[index] = power;
            TRACE(0, "  tx_power_ch = 0x%04x", wifi_factory->data.tx_power_ch[index]);
        }
        else
        {
            TRACE(0, "%s The band set error", __func__);
        }

        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_power: erase failed! ret = %d.", ret);

        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_power: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

int  factory_section_set_wifi_data(factory_section_wifi_data_t *wifi_data)
{
    uint8_t *mempool = NULL;

    if (wifi_data == NULL)
    {
        return -1;
    }

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_wifi_data");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);
        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        memcpy(&wifi_factory->data, wifi_data , sizeof(factory_section_wifi_data_t));

        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }

}

factory_section_t* factory_section_get_base_addr()
{
    if (factory_section_p)
    {
        return factory_section_p;
    }
    return NULL;
}

factory_section_wifi_t* factory_section_get_wifi_data()
{
    if (factory_section_wifi_p)
    {
        return factory_section_wifi_p;
    }
    return NULL;
}

int factory_section_set_wifi_temp(int temp)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_wifi_temp");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        if (rf_band_select == 1)
        {
            wifi_factory->data.temperature_5G = temp;
        }
        else if (rf_band_select == 0)
        {
            wifi_factory->data.temperature = temp;
        }
        else
        {
            TRACE(0, "%s The band set error",__func__);
        }

        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

uint32_t* factory_section_get_bt_tx_power(void)
{
    if (factory_section_wifi_p) {
        return factory_section_wifi_p->data.bt_tx_power;
    } else {
        return NULL;
    }
}

int  factory_section_set_bt_tx_power(uint8_t index, uint32_t div_value, uint32_t txPower)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_bt_tx_power");

        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        if ((index+1) > (ARRAY_SIZE(factory_section_wifi_p->data.bt_tx_power)-1))
        {
            TRACE(0, "index invalid:%d", index);
            return -1;
        }
        wifi_factory->data.bt_tx_power[index] = div_value;
        wifi_factory->data.bt_tx_power[index+1] = txPower;

        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start , FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start , (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        TRACE(0, "factory_section error!");
        return -1;
    }
}

int  factory_section_set_wifi_xtal(unsigned short xtal)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_wifi_xtal");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        wifi_factory->data.freq_cal = xtal;

        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_xtal: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_xtal: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

int  factory_section_set_wifi_power_type(unsigned short powerType)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_wifi_powe_type");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        if (rf_band_select == 1)
        {
            wifi_factory->data.tx_power_flags_5G = powerType;
        }
        else if (rf_band_select == 0)
        {
            wifi_factory->data.tx_power_type = powerType;
        }
        else
        {
            TRACE(0, "%s The band set error",__func__);
        }

        TRACE(0, "wifi_factory->data.tx_power_type = %d" ,wifi_factory->data.tx_power_type);
        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_wifi_address: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

int  factory_section_set_country_code(char* counryCode)
{
    uint8_t *mempool = NULL;

    if (counryCode == NULL)
    {
        return -1;
    }

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_country_code");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        strcpy((char *)wifi_factory->data.country , counryCode);

        TRACE(0, "wifi_factory->data.contry = %s" ,wifi_factory->data.country);
        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_contry_code: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_contry_code: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

char* factory_section_get_country_code(void)
{
    if (factory_section_wifi_p)
    {
        if ((factory_section_wifi_p->data.country[0]) != 0)
            return (char *)(factory_section_wifi_p->data.country);
    }
    return NULL;
}

int  factory_section_set_ble_power_control(unsigned char ble_power_control)
{
    uint8_t *mempool = NULL;

    if (factory_section_p && factory_section_wifi_p)
    {
        TRACE(0, "factory_section_set_ble_power_control");
        mempool = factory_get_mem();
        if (mempool == NULL) {
            TRACE(0, "%s syspool get memory fail",__func__);
            return -1;
        }
        memcpy(mempool, factory_section_p, FACTORY_SECTOR_SIZE);

        factory_section_wifi_t *wifi_factory = (factory_section_wifi_t *)(mempool+FACTORY_SECTION_WIFI_OFFSET);

        wifi_factory->data.ble_power_control = ble_power_control;

        TRACE(0, "wifi_factory->data.ble_power_control = %d" ,wifi_factory->data.ble_power_control);
        wifi_factory->head.crc =
            crc32_c(0, (unsigned char *)(&(wifi_factory->data)), sizeof(factory_section_wifi_data_t));
#ifdef RAM_NV_RECORD
        memcpy(factory_section_p, mempool, FACTORY_SECTOR_SIZE);
#else
        uint32_t lock;
        enum NORFLASH_API_RET_T ret;
        lock = int_lock_global();

        ret = norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_ble_power_control: erase failed! ret = %d.", ret);
        ret = norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY, (uint32_t)__factory_start, (uint8_t *)mempool, FACTORY_SECTOR_SIZE, false);
        ASSERT(ret == NORFLASH_API_OK, "factory_section_set_ble_power_control: write failed! ret = %d.", ret);

        int_unlock_global(lock);
#endif
        return 0;
    }
    else
    {
        return -1;
    }
}

unsigned char factory_section_get_ble_power_control()
{
    if (factory_section_wifi_p)
    {
        if ((factory_section_wifi_p->data.ble_power_control) != 0)
        {
            return (factory_section_wifi_p->data.ble_power_control);
        }
        else
        {
            return 0;//not write calib value
        }
    }
    return 0;//default value
}
