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
#ifdef RTOS
    #include "cmsis_os.h"
#endif

#include "hwtest.h"
#include "hal_aud.h"
#include "hal_cmu.h"
#include "hal_usb.h"
#include "usb_cdc.h"
#include "usb_audio.h"
#include "usb_general.h"
#include "string.h"
#ifdef USB_MTP_TEST
    #include "usb_mtp.h"
    #ifdef RTOS
        #include "mtp_storage.h"
        #include "mbed_mount.h"
        #include "fs_device.h"
    #endif
#endif
#include "hwtimer_list.h"
#ifdef USB_PMU_CFG
    #include "hal_chipid.h"
    #include "pmu.h"
#endif
#ifdef USB_SLEEP_TEST
    #include "hal_sysfreq.h"
    #ifdef RTOS
        #include "cmsis_os.h"
    #endif
#endif

#define USB_TRANSFER_LENGTH                 (1024 * 32)
#define MULTI_BUFFER_NUM                    3
static uint32_t recv_buffer[MULTI_BUFFER_NUM][USB_TRANSFER_LENGTH / 4];
static uint32_t send_buffer[USB_TRANSFER_LENGTH / 4];

static void usb_serial_break_handler(uint16_t ms);

// Avoid linking error
void WEAK usbphy_open(void)
{
}

void WEAK usbphy_sleep(void)
{
}

void WEAK usbphy_wakeup(void)
{
}

static void usb_dev_env_init(void)
{
#ifdef USB_PMU_CFG
    pmu_open();
#endif

#ifdef ROM_BUILD
    enum HAL_CMU_USB_CLOCK_SEL_T src;

#ifdef USB_HIGH_SPEED
    src = HAL_CMU_USB_CLOCK_SEL_PLL;
#else
#ifdef USB_CLK_SRC_24M_X2
    src = HAL_CMU_USB_CLOCK_SEL_24M_X2;
#elif defined(USB_CLK_SRC_48M)
    src = HAL_CMU_USB_CLOCK_SEL_48M;
#elif defined(USB_CLK_SRC_26M_X2)
    src = HAL_CMU_USB_CLOCK_SEL_26M_X2;
#elif defined(USB_CLK_SRC_26M_X4)
    src = HAL_CMU_USB_CLOCK_SEL_26M_X4;
#else
    src = HAL_CMU_USB_CLOCK_SEL_PLL;
#endif
#endif
    hal_cmu_usb_rom_set_clock_source(src);
#endif
}

// USB_SERIAL_SPEED_TEST
#define USB_DATA_SIZE (80 * 1024)
#define USB_TEST_COUNT 100

static uint32_t usb_data[USB_DATA_SIZE / 4];
static uint32_t usb_time[USB_TEST_COUNT];
static uint32_t usb_test_cnt = 0;
static uint32_t usb_start_time = 0;
static volatile uint32_t usb_test_done = 0;

static const struct USB_SERIAL_CFG_T cdc_cfg = {
    .break_callback = usb_serial_break_handler,
    .mode = USB_SERIAL_API_BLOCKING,
};

static void usb_recv_done(const void *data, uint32_t size, int error)
{
    if (size != USB_DATA_SIZE || error) {
        TRACE(3, "USB recv error: cnt=%d size=%d err=%d", usb_test_cnt, size, error);
        usb_test_done = 2;
        return;
    }

    if (usb_test_cnt < USB_TEST_COUNT) {
        usb_time[usb_test_cnt] = hal_fast_sys_timer_get() - usb_start_time;
    }
    usb_test_done = 1;
    //TRACE(2,"USB recv done: cnt=%d size=%d", usb_test_cnt, size);
}

static void usb_serial_break_handler(uint16_t ms)
{
    TRACE(1, "Detect break: %u ms", ms);
}

void usb_serial_speed_test(void)
{
    int result;
    unsigned int i;

    usb_dev_env_init();
    usb_serial_open(&cdc_cfg);
    TRACE(1, "[0x%X] Open USB serial test speed", hal_sys_timer_get());

    usb_serial_flush_recv_buffer();
    usb_serial_init_xfer(USB_SERIAL_XFER_TYPE_ALL);

    while (1) {
        usb_test_cnt = 0;
        memset(usb_data, 0, sizeof(usb_data));
        memset(usb_time, 0, sizeof(usb_time));
        result = usb_serial_direct_recv((uint8_t *)&usb_data[0], 1024, 1, NULL, NULL);
        if (result != 0) {
            TRACE(1, "start usb_serial_direct_recv failed: ret=%d", result);
        } else {
            do {
                usb_test_done = 0;
                usb_start_time = hal_fast_sys_timer_get();
                result = usb_serial_direct_recv((uint8_t *)&usb_data[0], sizeof(usb_data),
                                                sizeof(usb_data), NULL, usb_recv_done);
                //TRACE(1,"usb_serial_direct_recv ret=%d", result);
                if (result != 0) {
                    break;
                }
                while (usb_test_done == 0);
                //TRACE(1,"usb_test_done: %d", usb_test_done);
            } while (++usb_test_cnt < USB_TEST_COUNT && usb_test_done != 2);

            usb_start_time = 0;
            for (i = 0; i < usb_test_cnt; i++) {
                //TRACE(3,"USB TIME [%03d]: %05d  0x%04x", i, usb_time[i], usb_time[i]);
                usb_start_time += usb_time[i];
            }
            //TRACE(1,"USB total bytes: %u", sizeof(usb_data) * usb_test_cnt);
            TRACE(1, "USB total bytes: %u,usb_test_cnt:%u", sizeof(usb_data) * usb_test_cnt, usb_test_cnt);
            TRACE(2, "USB total time: %u ticks, %u us", usb_start_time, FAST_TICKS_TO_US(usb_start_time));
            float speed = (float)sizeof(usb_data) * usb_test_cnt * 1000 /  FAST_TICKS_TO_US(usb_start_time);
            TRACE(2, "USB speed: %d.%02d KB/s", (uint32_t)speed, (uint32_t)((speed - (uint32_t)speed) * 100));
        }
    }
}

#define ALIGNED4                        ALIGNED(4)

static uint8_t ALIGNED4 usb_serial_data[1000];

static void usb_serial_recv_timeout(void *param)
{
    usb_serial_cancel_recv();
}

#ifdef CUSTOM_CDC_HID
#define LED_NUM_LOCK    (1 << 0)
#define LED_CAPS_LOCK   (1 << 1)
#define LED_SCROLL_LOCK (1 << 2)

static uint8_t keyboard_zero[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t keyboard_val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int keyboard_set_report_callback(const uint8_t *data, uint32_t size)
{
    uint8_t buffer[8];

    memcpy((void *)buffer, (void *)data, size);
    TRACE(0, "%s, data=%x", __func__, buffer[0]);

    if (buffer[0] & LED_CAPS_LOCK) {
        //todo turn on capslock led
    } else {
        //todo turn off capslock led
    }

    return true;
}
#endif

// USB_SERIAL_TEST
void usb_serial_test(void)
{
    int result;
    uint32_t i;
    HWTIMER_ID timer;

    timer = hwtimer_alloc(usb_serial_recv_timeout, NULL);
    ASSERT(timer, "Failed to alloc usb serial recv timer");

    usb_dev_env_init();
    usb_serial_open(&cdc_cfg);
#ifdef CUSTOM_CDC_HID
    usb_custom_cdc_hid_open(NULL, keyboard_set_report_callback); // implement callback function as needed
#endif
    TRACE(1, "[0x%X] Open USB serial", hal_sys_timer_get());

    usb_serial_flush_recv_buffer();
    usb_serial_init_xfer(USB_SERIAL_XFER_TYPE_ALL);

    while (1) {
        i = 0;

        result = usb_serial_recv(usb_serial_data + i, 1);
        if (result != 0) {
            TRACE(1, "Failed to receive from usb serial: ret=%d", result);
            break;
        }
        i++;

        while (i < ARRAY_SIZE(usb_serial_data)) {
#ifdef SIMU
            hwtimer_start(timer, US_TO_TICKS(500));
#else
            hwtimer_start(timer, MS_TO_TICKS(10));
#endif
            result = usb_serial_recv(usb_serial_data + i, 1);
            hwtimer_stop(timer);
            if (result != 0) {
                // Probably timeout
                usb_serial_init_xfer(USB_SERIAL_XFER_TYPE_ALL);
                break;
            }
            i++;
        }

        result = usb_serial_send(usb_serial_data, i);
        if (result != 0) {
            TRACE(1, "Failed to send to usb serial: ret=%d", result);
            break;
        }
#ifdef CUSTOM_CDC_HID
        //for hid keyboard test
        keyboard_val[2] = usb_serial_data[0];
        usb_custom_cdc_hid_set_data(keyboard_val, 8);
        usb_custom_cdc_hid_set_data(keyboard_zero, 8); // after set the keyboard value, need to send 0, thie can deal in epn_compl_handler
#endif
    }
}

static volatile int rx_done = 0;
static volatile int tx_done = 0;

#ifndef USB_SLEEP_TEST
static void usb_direct_rx_callback(const void *data, uint32_t size, int error)
{
    rx_done = 1;
    TRACE(4, "%s: data=%p size=%u error=%d", __func__, data, size, error);
}

static void usb_direct_tx_callback(const void *data, uint32_t size, int error)
{
    tx_done = 1;
    TRACE(4, "%s: data=%p size=%u error=%d", __func__, data, size, error);
}
#endif

#ifndef USB_SERIAL_DIRECT_XFER_SIZE
    #define USB_SERIAL_DIRECT_XFER_SIZE     64
#endif

// USB_SERIAL_DIRECT_XFER_TEST
void usb_serial_direct_xfer_test(void)
{
#ifndef USB_SLEEP_TEST
    int result;
#endif

    usb_dev_env_init();
    usb_serial_open(&cdc_cfg);
    TRACE(1, "[0x%X] Open USB serial", hal_sys_timer_get());

    usb_serial_flush_recv_buffer();
    usb_serial_init_xfer(USB_SERIAL_XFER_TYPE_ALL);

#ifdef USB_SLEEP_TEST
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    while (1) {
        osDelay(10000000);
    }
#else
    while (1) {
        rx_done = 0;
        result = usb_serial_direct_recv(usb_serial_data, ARRAY_SIZE(usb_serial_data), USB_SERIAL_DIRECT_XFER_SIZE, NULL, usb_direct_rx_callback);
        if (result != 0) {
            TRACE(1, "Failed to receive from usb serial: ret=%d", result);
            break;
        }
        while (rx_done == 0);

        tx_done = 0;
        result = usb_serial_direct_send(usb_serial_data, USB_SERIAL_DIRECT_XFER_SIZE, usb_direct_tx_callback);
        if (result != 0) {
            TRACE(1, "Failed to send to usb serial: ret=%d", result);
            break;
        }
        while (tx_done == 0);
    }
#endif
}

// USB_AUDIO_TEST
#define USB_AUDIO_PKT_SIZE                  (48000 / 1000 * 2 * 2)

static uint32_t usb_audio_data[2][USB_AUDIO_PKT_SIZE / 4];

static void recv_itf_handler(enum USB_AUDIO_ITF_CMD_T cmd)
{
    if (cmd == USB_AUDIO_ITF_STOP) {
        usb_audio_stop_recv();
    } else {
        usb_audio_start_recv((uint8_t *)usb_audio_data[0], 0, sizeof(usb_audio_data));
    }
}

static void send_itf_handler(enum USB_AUDIO_ITF_CMD_T cmd)
{
    if (cmd == USB_AUDIO_ITF_STOP) {
        usb_audio_stop_send();
    } else {
        usb_audio_start_send((uint8_t *)usb_audio_data[0], sizeof(usb_audio_data[0]), sizeof(usb_audio_data));
    }
}

static void data_recv_handler(const struct USB_AUDIO_XFER_INFO_T *info)
{
}

static void data_send_handler(const struct USB_AUDIO_XFER_INFO_T *info)
{
}

static void itf_handler(enum USB_AUDIO_ITF_ID_T id, enum USB_AUDIO_ITF_CMD_T cmd)
{
    if (id == USB_AUDIO_ITF_ID_RECV) {
        recv_itf_handler(cmd);
    } else {
        send_itf_handler(cmd);
    }
}

static void xfer_handler(enum USB_AUDIO_ITF_ID_T id, const struct USB_AUDIO_XFER_INFO_T *info)
{
    if (id == USB_AUDIO_ITF_ID_RECV) {
        data_recv_handler(info);
    } else {
        data_send_handler(info);
    }
}

void usb_audio_test(void)
{
    static const struct USB_AUDIO_CFG_T usb_audio_cfg = {
        .recv_sample_rate = AUD_SAMPRATE_48000,
        .send_sample_rate = AUD_SAMPRATE_48000,
        .itf_callback = itf_handler,
        .xfer_callback = xfer_handler,
    };

    usb_dev_env_init();
    usb_audio_open(&usb_audio_cfg);
}

// USB_MTP_TEST
#ifdef USB_MTP_TEST
#ifdef RTOS
extern int mbed_fs_mount(char *name);
extern int mbed_fs_umount(char *name);
#ifdef FS_LITTLEFS_ENABLE
char *mbed_mount_littlefs_device(fs_device_e dev)
{
    mbed_mount_littlefs_demo(dev);
    return mbed_name(FS_LITTLEFS, dev);
}
char *mbed_mount_littlefs_mem()
{
    mbed_fs_mount("lfsmem");
    return "lfsmem";
}
#endif
char *mbed_mount_fatfs_device(fs_device_e dev)
{
    mbed_mount_fatfs_demo(dev, 512);
    return mbed_name(FS_FATFS, dev);
}
#else
static int32_t mtp_obj_handles_num_get(uint32_t storage, uint32_t parent, uint16_t format)
{
    if (storage == 0) {
        TR_ERROR(0, "%s: storage id error, storage:%d", __FUNCTION__, storage);
        return -1;
    }
    return 0;
}

static int32_t mtp_obj_info_receive(mtp_obj_info_t *objinfo)
{
    if (objinfo->m_storage_id == 0) {
        TR_ERROR(0, "%s: storage id error, storage:%d", __FUNCTION__, objinfo->m_storage_id);
        return -1;
    }
    objinfo->m_handle = 1;
    TR_INFO(0, "%s: storage:%d parHandle:%d handle:%d",
            __FUNCTION__, objinfo->m_storage_id, objinfo->m_parent, objinfo->m_handle);
    return objinfo->m_handle;
}

static int32_t mtp_obj_info_read(uint32_t storage, uint32_t handle, mtp_obj_info_t *objinfo)
{
    if (handle == 0 || handle == 0xFFFFFFFF) {
        TR_ERROR(0, "%s: handle id error, storage:%d handle:%d", __FUNCTION__, storage, handle);
        return -1;
    }
    if (storage == 0) {
        TR_ERROR(0, "%s: storage id error, storage:%d", __FUNCTION__, storage);
        return -1;
    }
    static const char name[8] = {'t', 'e', 's', 't', '.', 't', 'x', 't'};
    objinfo->m_handle = handle;
    objinfo->m_storage_id = storage;
    objinfo->m_parent = 0;
    objinfo->m_compressed_size = 7;
    objinfo->m_format = Text;
    for (int i = 0; i < sizeof(name); i ++) {
        objinfo->m_name[i] = name[i];
    }

    TR_INFO(0, "%s: storage:%d handle:%d name:%s format:%x",
            __FUNCTION__, storage, handle, objinfo->m_name, objinfo->m_format);
    return 0;
}

static int32_t mtp_obj_content_read(uint32_t storage, uint32_t handle,
                                    uint32_t offset, char *buff, uint32_t length)
{
    if (buff == NULL || length <= 0) {
        TR_ERROR(0, "%s: param in error, storage:%d handle:%d", __FUNCTION__, storage, handle);
        return -1;
    }
    if (storage == 0) {
        TR_ERROR(0, "%s: storage id error, storage:%d", __FUNCTION__, storage);
        return -1;
    }
    static const char file[7] = {'m', 't', 'p', 't', 'e', 's', 't'};
    int32_t ret;
    ret = length;
    for (int i = 0; i < sizeof(file); i ++) {
        buff[i] = file[i];
    }
    TR_INFO(0, "%s: storage:%d handle:%d len:%d", __FUNCTION__, storage, handle, ret);
    return ret;
}

static int32_t mtp_obj_content_write(uint32_t storage, uint32_t handle,
                                     uint32_t offset, char *buff, uint32_t length)
{
    if (buff == NULL || length <= 0) {
        TR_ERROR(0, "%s: param in error, storage:%d handle:%d", __FUNCTION__, storage, handle);
        return -1;
    }
    if (storage == 0) {
        TR_ERROR(0, "%s: storage id error, storage:%d", __FUNCTION__, storage);
        return -1;
    }

    int32_t ret;
    ret = length;
    TR_INFO(0, "%s: storage:%d handle:%d len:%d", __FUNCTION__, storage, handle, ret);
    return ret;
}

static const struct USB_MTP_CALLBACKS callbacks = {
    .bind = NULL,
    .open_session = NULL,
    .close_session = NULL,
    .get_handlesnum = mtp_obj_handles_num_get,
    .get_handles = NULL,
    .destroy_handles = NULL,
    .get_storageinfo = NULL,
    .receive_objinfo = mtp_obj_info_receive,
    .read_objinfo = mtp_obj_info_read,
    .read_obj = mtp_obj_content_read,
    .write_obj = mtp_obj_content_write,
    .read_cancel = NULL,
    .write_cancel = NULL,
    .write_end = NULL,
    .removeobj = NULL,
    .rename_obj = NULL,
};

static int mtp_callback_register()
{
    int ret = 0;
    ret = usb_mtp_callback_register(&callbacks);
    if (ret != 0) {
        TR_ERROR(0, "%s: mtp callback register err %d\n", __func__, ret);
        return -1;
    }
    return 0;
}
#endif
void usb_mtp_test(void)
{
    char *fs_mount_point_mem = NULL;
    char *fs_mount_point_dev = NULL;
    const uint32_t fs_max_file_size = 1024 * 1024 * 1024;
    usb_dev_env_init();

#ifdef RTOS
#ifdef FS_LITTLEFS_ENABLE
    fs_mount_point_mem = mbed_mount_littlefs_mem();
    fs_mount_point_dev = mbed_mount_littlefs_device(DEVICE_EMMC);//1600
    //char *fs_mount_point_lfsdev = mbed_mount_littlefs_device(DEVICE_SDMMC);//1502
#endif
#ifdef FS_FATFS_ENABLE
    fs_mount_point_dev = mbed_mount_fatfs_device(DEVICE_EMMC);//1600
    //char *fs_mount_point_lfsdev = mbed_mount_littlefs_device(DEVICE_SDMMC);//1502
#endif
#endif

    int ret;
    int32_t storageid[STORAGE_MAX_NUM];
    int id_index = 0;

    POSSIBLY_UNUSED MTP_STORAGE_NAME storagename[STORAGE_MAX_NUM];

    ret = mtp_callback_register();
    ASSERT(ret == 0, "Failed to register mtp callback: %d", ret);
    ret = usb_mtp_open(USB_MTP_API_NONBLOCKING);
    ASSERT(ret == 0, "Failed to open usb mtp: %d", ret);

    //bind storage 1
    if (fs_mount_point_mem != NULL) {
        storageid[id_index] = usb_mtp_fs_bind(fs_mount_point_mem, fs_max_file_size);
        TR_INFO(0, "fs_bind return storage id[%d]:%x", id_index, storageid[id_index]);
        ASSERT(storageid[id_index] > 0, "Failed to bind mtp fs: %d", storageid[id_index]);
        storagename[id_index].namelen = strlen(fs_mount_point_mem);
        storagename[id_index].name = (uint8_t *)fs_mount_point_mem;
        id_index++;
    }

    //bind storage 2
    if (fs_mount_point_dev != NULL) {
        storageid[id_index] = usb_mtp_fs_bind(fs_mount_point_dev, fs_max_file_size);
        TR_INFO(0, "fs_bind return storage id[%d]:%x", id_index, storageid[id_index]);
        ASSERT(storageid[id_index] > 0, "Failed to bind mtp fs: %d", storageid[id_index]);
        storagename[id_index].namelen = strlen(fs_mount_point_dev);
        storagename[id_index].name = (uint8_t *)fs_mount_point_dev;
        id_index++;
    }

    usb_mtp_set_storage(id_index, storageid, NULL);

#ifndef RTOS
    hal_sys_timer_delay_us(1000000); //1s wait the mtp enumeration
    ret = usb_mtp_work_start();
    ASSERT(ret == 0, "Failed to start usb mtp work: %d", ret);
#endif
}
#endif

static bool usb_speed_test_send_callback(const uint8_t *data, uint32_t length, enum XFER_COMPL_STATE state)
{
    if (state == XFER_COMPL_SUCCESS) {
        hal_usb_send_epn(EP2BULK_IN, (uint8_t *)send_buffer, USB_TRANSFER_LENGTH - 1, ZLP_AUTO);
    } else {
        TRACE(1, "usb_speed_test_send_callback failed");
    }
    return true;
}

static bool usb_speed_test_recv_callback(const uint8_t *data, uint32_t length, enum XFER_COMPL_STATE state)
{
    static uint8_t buf_no = 0;
    if (state == XFER_COMPL_SUCCESS) {
        buf_no++;
        if (buf_no >= MULTI_BUFFER_NUM)
            buf_no = 0;
        hal_usb_recv_epn(EP2BULK_OUT, (uint8_t *)recv_buffer[buf_no], USB_TRANSFER_LENGTH);
    } else {
        TRACE(1, "usb_speed_test_recv_callback failed");
    }

    return true;
}

/*
 Chip 2005 usb2.0 high speed test:
 Test result with CPU frequence at 104M and default sram config is 48.5/44.6 MB/s, 80% of theoretical speed.
*/
extern struct HAL_USB_CALLBACKS u_general_callbacks;
void usb_general_speed_test(void)
{
    int ret;

    struct USB_GENERAL_CFG_T cfg = {0};
    u_general_callbacks.epn_recv_compl[EP2BULK_OUT - 1] = usb_speed_test_recv_callback;
    u_general_callbacks.epn_send_compl[EP2BULK_IN - 1] = usb_speed_test_send_callback;
    ret = usb_general_open(&cfg);
    TRACE(1, "Open USB General %d", ret);

    while (!hal_usb_configured()) {
        osDelay(10);
    }

    ret = hal_usb_recv_epn(EP2BULK_OUT, (uint8_t *)recv_buffer[0], USB_TRANSFER_LENGTH);
    if (ret) {
        TRACE(1, "usb recv transfer submit failed %d", ret);
    }
    ret = hal_usb_send_epn(EP2BULK_IN, (uint8_t *)send_buffer, USB_TRANSFER_LENGTH - 1, ZLP_AUTO);
    if (ret) {
        TRACE(1, "usb send transfer submit failed %d", ret);
    }
    TRACE(1, "Transfer submitted");

    while (1) {
        osDelay(10);
    }
}

