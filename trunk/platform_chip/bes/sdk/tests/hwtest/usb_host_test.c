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
#include "hwtest.h"
#include "hal_cmu.h"
#include "hal_usbhost.h"
#include "string.h"
#ifdef USB_PMU_CFG
#include "hal_chipid.h"
#include "pmu.h"
#endif

// USBHOST_TEST
#define USBHOST_BUF_ALIGNED ALIGNED(4)
volatile int usbhost_port_changed = 0;
enum HAL_USBHOST_PORT_EVENT_T usbhost_port_event;
uint8_t USBHOST_BUF_ALIGNED usbhost_data_buf[64];
struct HAL_USBHOST_CHAN_TYPE_T usbhost_ep0_type;
struct HAL_USBHOST_SETUP_XFER_T usbhost_setup;

#ifdef SIMU
#define USBHOST_PORT_EN_DELAY_TIME      MS_TO_TICKS(1)
#else
#define USBHOST_PORT_EN_DELAY_TIME      MS_TO_TICKS(200)
#endif

#define  DESCRIPTOR_TYPE_DEVICE			1
#define  DESCRIPTOR_TYPE_CONFIGURATION	2
#define  DESCRIPTOR_TYPE_STRING			3
#define  DESCRIPTOR_TYPE_INTERFACE		4
#define  DESCRIPTOR_TYPE_ENDPOINT		5

#define DESCRIPTOR_TYPE_HID         0x21
#define DESCRIPTOR_TYPE_REPORT      0x22
#define DESCRIPTOR_TYPE_PHYSICAL    0x23
#define DESCRIPTOR_TYPE_HUB         0x29

enum USB_CLASS_CODE
{
	CLASS_DEVICE,
	CLASS_AUDIO,
	CLASS_COMM_AND_CDC_CONTROL,
	CLASS_HID,
	CLASS_PHYSICAL = 0x05,
	CLASS_STILL_IMAGING,
	CLASS_PRINTER,
	CLASS_MASS_STORAGE,
	CLASS_HUB,
	CLASS_CDC_DATA,
	CLASS_SMART_CARD,
	CLASS_CONTENT_SECURITY = 0x0D,
	CLASS_VIDEO = 0x0E,
	CLASS_DIAGNOSTIC_DEVICE = 0xDC,
	CLASS_WIRELESS_CONTROLLER = 0xE0,
	CLASS_MISCELLANEOUS = 0xEF,
	CLASS_APP_SPECIFIC = 0xFE,
	CLASS_VENDOR_SPECIFIC = 0xFF
};

#define  DEVICE_TO_HOST         0x80
#define  HOST_TO_DEVICE         0x00
#define  REQUEST_TYPE_CLASS     0x20
#define  RECIPIENT_DEVICE       0x00
#define  RECIPIENT_INTERFACE    0x01
#define  RECIPIENT_ENDPOINT		0x02
#define  RECIPIENT_OTHER		0x03

#define  GET_STATUS				0
#define  CLEAR_FEATURE			1
#define  SET_FEATURE			3
#define  SET_ADDRESS			5
#define  GET_DESCRIPTOR			6
#define  SET_DESCRIPTOR			7
#define  GET_CONFIGURATION		8
#define  SET_CONFIGURATION		9
#define  GET_INTERFACE			10
#define  SET_INTERFACE			11
#define  SYNCH_FRAME			11

typedef struct
{
	u8	bLength;
	u8	bDescriptorType;
	u16 bcdUSB;
	u8 bDeviceClass;
	u8 bDeviceSubClass;
	u8 bDeviceProtocol;
	u8 bMaxPacketSize;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;	// version
	u8 iManufacturer;
	u8 iProduct;
	u8 iSerialNumber;
	u8 bNumConfigurations;
} DeviceDescriptor;	// 16 bytes

typedef struct
{
	u8	bLength;
	u8	bDescriptorType;
	u16	wTotalLength;
	u8	bNumInterfaces;
	u8	bConfigurationValue;	// Value to use as an argument to select this configuration
	u8	iConfiguration;			// Index of String Descriptor describing this configuration
	u8	bmAttributes;			// Bitmap D7 Reserved, set to 1. (USB 1.0 Bus Powered),D6 Self Powered,D5 Remote Wakeup,D4..0 = 0
	u8	bMaxPower;				// Maximum Power Consumption in 2mA units
} ConfigurationDescriptor;

typedef struct
{
	u8	bLength;
	u8	bDescriptorType;
	u8  bInterfaceNumber;
	u8	bAlternateSetting;
	u8	bNumEndpoints;
	u8	bInterfaceClass;
	u8	bInterfaceSubClass;
	u8	bInterfaceProtocol;
	u8	iInterface;				// Index of String Descriptor Describing this interface
} InterfaceDescriptor;

typedef struct
{
	u8	bLength;
	u8	bDescriptorType;
	u8	bEndpointAddress;	// Bits 0:3 endpoint, Bits 7 Direction 0 = Out, 1 = In (Ignored for Control Endpoints)
	u8	bmAttributes;		// Bits 0:1 00 = Control, 01 = Isochronous, 10 = Bulk, 11 = Interrupt
	u16 wMaxPacketSize;
	u8	bInterval;			// Interval for polling endpoint data transfers.
} EndpointDescriptor;

DeviceDescriptor dev_desc;

static void usb_host_env_init(void)
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

void usbhost_port_handler(enum HAL_USBHOST_PORT_EVENT_T evt)
{
    usbhost_port_event = evt;
    usbhost_port_changed++;
}

void usbhost_test(void)
{
    int ret;
    bool get_dev_desc = false;
    bool port_en_det = false;
    int len;
    uint32_t time;
    uint32_t recv_len;

    usbhost_ep0_type.mps = 64;
    usbhost_ep0_type.ep_num = 0;
    usbhost_ep0_type.ep_in = 0;
    usbhost_ep0_type.ep_type = HAL_USBHOST_EP_CTRL;
    usbhost_ep0_type.dev_addr = 0;

    usbhost_setup.chan_in = HAL_USBHOST_CHAN_NONE;
    usbhost_setup.chan_out = HAL_USBHOST_CHAN_NONE;
    usbhost_setup.data_buf = usbhost_data_buf;
    usbhost_setup.buf_len = sizeof(usbhost_data_buf);
    usbhost_setup.next_stage = HAL_USBHOST_SETUP_DATA_IN;
    usbhost_setup.handler = NULL;

    usb_host_env_init();
    ret = hal_usbhost_open(usbhost_port_handler, NULL);
    ASSERT(ret == 0, "usbhost open failed: %d", ret);
    while (1) {
        if (usbhost_port_changed) {
            usbhost_port_changed = 0;
            port_en_det = false;
            if (usbhost_port_event == HAL_USBHOST_PORT_CONN) {
#ifdef SIMU
                hal_usbhost_port_reset(1);
#else
                hal_usbhost_port_reset(20);
#endif
            } else if (usbhost_port_event == HAL_USBHOST_PORT_EN_FS ||
                    usbhost_port_event == HAL_USBHOST_PORT_EN_HS) {
                ret = hal_usbhost_get_chan(&usbhost_setup.chan_out);
                ASSERT(ret == 0, "usbhost get out chan failed: %d", ret);
                ret = hal_usbhost_get_chan(&usbhost_setup.chan_in);
                ASSERT(ret == 0, "usbhost get in chan failed: %d", ret);
                if (usbhost_port_event == HAL_USBHOST_PORT_EN_HS) {
                    usbhost_ep0_type.mps = 64;
                } else {
                    usbhost_ep0_type.mps = 8;
                }
                usbhost_ep0_type.ep_in = 0;
                ret = hal_usbhost_init_chan(usbhost_setup.chan_out, &usbhost_ep0_type);
                ASSERT(ret == 0, "usbhost init out chan failed: %d", ret);
                usbhost_ep0_type.ep_in = 1;
                ret = hal_usbhost_init_chan(usbhost_setup.chan_in, &usbhost_ep0_type);
                ASSERT(ret == 0, "usbhost init in chan failed: %d", ret);
                time = hal_sys_timer_get();
                port_en_det = true;
            } else if (usbhost_port_event == HAL_USBHOST_PORT_DISCONN) {
                hal_usbhost_free_chan(usbhost_setup.chan_out);
                usbhost_setup.chan_out = HAL_USBHOST_CHAN_NONE;
                hal_usbhost_free_chan(usbhost_setup.chan_in);
                usbhost_setup.chan_in = HAL_USBHOST_CHAN_NONE;
            }
        }

        if (port_en_det) {
            if (hal_sys_timer_get() - time >= USBHOST_PORT_EN_DELAY_TIME) {
                port_en_det = false;
                get_dev_desc = true;
            }
        }

        if (get_dev_desc) {
            get_dev_desc = false;

            usbhost_setup.setup_pkt.bmRequestType = DEVICE_TO_HOST | RECIPIENT_DEVICE;
            usbhost_setup.setup_pkt.bRequest = GET_DESCRIPTOR;
            usbhost_setup.setup_pkt.wValue = (DESCRIPTOR_TYPE_DEVICE << 8) | 0;
            usbhost_setup.setup_pkt.wIndex = 0;
            usbhost_setup.setup_pkt.wLength = 8;
            ret = hal_usbhost_start_setup_xfer(&usbhost_setup, &recv_len);
            ASSERT(ret == 0, "usbhost start setup0 failed: %d", ret);

            len = MIN(sizeof(dev_desc), recv_len);
            memcpy(&dev_desc, &usbhost_data_buf[0], len);
            TRACE(2,"dev_desc: recv_len=%d, maxPktSize=%d", recv_len, dev_desc.bMaxPacketSize);

            ret = hal_usbhost_update_chan_mps(usbhost_setup.chan_out, dev_desc.bMaxPacketSize);
            ASSERT(ret == 0, "usbhost out update mps failed: %d", ret);
            ret = hal_usbhost_update_chan_mps(usbhost_setup.chan_in, dev_desc.bMaxPacketSize);
            ASSERT(ret == 0, "usbhost in update mps failed: %d", ret);

            usbhost_setup.setup_pkt.wLength = sizeof(dev_desc);
            ret = hal_usbhost_start_setup_xfer(&usbhost_setup, &recv_len);
            ASSERT(ret == 0, "usbhost start setup1 failed: %d", ret);

            memcpy(&dev_desc, &usbhost_data_buf[0], sizeof(dev_desc));
            TRACE(4,"dev_desc: recv_len=%d Class %02X found %04X:%04X",recv_len,dev_desc.bDeviceClass,dev_desc.idVendor,dev_desc.idProduct);
        }
    }
}


