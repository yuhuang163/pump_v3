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
#if (defined TEST_SPI || defined TEST_SPI_SLAVE || defined TEST_SYSVIEW)
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_spi.h"
#include "hal_gpio.h"
#include "hal_sysfreq.h"
#include "test_spi.h"
#include "stdlib.h"

#ifndef TEST_SPI_RATE
#define TEST_SPI_RATE                       (6000000)
#endif

//#define SPI_LOOP_TEST
//#define SPI_USE_DMA

//////// BES SPI slave operation
#define MST_SPI_WR (0<<30)
#define MST_SPI_RD (1<<30)

#define MST_SPI_DUMMY_DATA (0xDEADBEEF)

#define MST_SPI_ADDR_WR(a) (((a)>>2)|MST_SPI_WR)
#define MST_SPI_ADDR_RD(a) (((a)>>2)|MST_SPI_RD)
#ifndef PLATFORM_BES2710IMP
#define BEST_SHARE_MEMORY_ADDRESS_POINTER     (0x20000000)
#else
#define BEST_SHARE_MEMORY_ADDRESS_POINTER     (0x20040000)
#endif

#define SPI_SLAVE_ACTION_RX_INDICATE       0
#define SPI_SLAVE_ACTION_TX_OK_CONFIRMED   1

#define MASTER_TX_INDICATE_GPIO_PIN          (HAL_GPIO_PIN_P1_0)
#define MASTER_RX_OK_GPIO_PIN                (HAL_GPIO_PIN_P1_1)

#define SLAVE_TX_INDICATE_GPIO_PIN           (HAL_GPIO_PIN_P1_2)
#define SLAVE_RX_OK_GPIO_PIN                 (HAL_GPIO_PIN_P1_3)

#ifdef MCU_SPI_SLAVE_SPECIFY_MEM
#define SPI_SLAVE_MEM_M_TX (0x200401bc)
#define SPI_SLAVE_MEM_S_TX (SPI_SLAVE_MEM_M_TX + MCU_SPI_SLAVE_BUF_SIZE)
#define SPI_SLAVE_MEM_SIZE  MCU_SPI_SLAVE_BUF_SIZE
#endif

// #define USE_SOFT_CS
#ifdef CHIP_SUBSYS_SENS
#define SOFT_CS_PIN HAL_IOMUX_PIN_P1_1
#else
#define SOFT_CS_PIN HAL_IOMUX_PIN_P9_1
#endif
typedef struct
{
    unsigned int r1; // r1 and r2 are used for SPI read align, 2 32bit dummy word
    unsigned int r2;
    bes_spi_share_memory_struct_t shm_struct;
} bes_spi_share_memory_struct_ext_t;

static bes_spi_share_memory_struct_ext_t _spi_shm;
static bes_spi_share_memory_struct_t *spi_shm = &_spi_shm.shm_struct;

#ifdef USE_BBK_SPI_IF
#define ADDR_SIZE   4
#define LEN_SIZE    4
#define CRC_SIZE    4
#define DUMY_SIZE   8
#define BUF_SIZE  (LEN_SIZE + 4096 + CRC_SIZE)
static uint8_t tx_buf[ADDR_SIZE + BUF_SIZE];
static uint8_t rx_buf[DUMY_SIZE + BUF_SIZE];
#else
#define CRC_SIZE    4
#define BUF_SIZE  (1024+CRC_SIZE)
static uint8_t tx_buf[BUF_SIZE+4];
static uint8_t rx_buf[BUF_SIZE+8];
#endif

void soft_cs_ctrl(uint8_t val)
{
#ifdef USE_SOFT_CS
    if(val)
        hal_gpio_pin_set(SOFT_CS_PIN);
    else
        hal_gpio_pin_clr(SOFT_CS_PIN);
#endif
}

void spi_wait_send_recv_done(int error)
{
    if(error!=0)
        TRACE(3,"spi send or recv error: %d",error);
#if (TEST_SPI_ID == 1)
    while(hal_spilcd_busy());
#else
    while (hal_spi_busy());
#endif
    soft_cs_ctrl(1);
}
#ifdef SPI_USE_DMA
void spi_dma_send_callback(int error)
{
    if(error!=0)
    {
        TRACE(3,"spi send error:%d",error);
    }
}
void spi_dma_recv_callback(int error)
{
    if(error!=0)
    {
        TRACE(3,"spi recv error:%d",error);
    }
}
#endif
#if 0
static void spi_invert_data_8bit(unsigned char *buff, unsigned int buff_len)
{
	int i = 0;
	unsigned char *p = (unsigned char *)buff, temp = 0;


	for (i = 0; i < buff_len; i += 4) {
		temp = p[i];
		p[i] = p[i+3];
		p[i+3] = temp;

		temp = p[i+1];
		p[i+1] = p[i+2];
		p[i+2] = temp;
	}
}

static void spi_invert_data(unsigned char *buff, unsigned int buff_len)
{
	spi_invert_data_8bit(buff, buff_len);
}
#endif
void spi_device_write_memory(unsigned int address, unsigned char *buf, unsigned int buf_len)
{
    int ret=0;

    buf[0] = ((MST_SPI_ADDR_WR(address))>>0)&0xFF;
    buf[1] = ((MST_SPI_ADDR_WR(address))>>8)&0xFF;
    buf[2] = ((MST_SPI_ADDR_WR(address))>>16)&0xFF;
    buf[3] = ((MST_SPI_ADDR_WR(address))>>24)&0xFF;

    //spi_invert_data(buf , buf_len);
    TRACE(3,"write_mem:buf=0x%x,len=%d", (uint32_t)buf, buf_len);
    soft_cs_ctrl(0);
#if (TEST_SPI_ID == 1)
#ifdef SPI_USE_DMA
    ret = hal_spilcd_dma_send((const void *)buf, buf_len , spi_dma_send_callback);
#else
    ret = hal_spilcd_send((const void *)buf, buf_len);
#endif
#elif (TEST_SPI_ID == 2)
#ifdef SPI_USE_DMA
    ret = hal_spidpd_dma_send((const void *)buf,buf_len,spi_dma_send_callback);
#else
    ret = hal_spidpd_send((const void *)buf, buf_len);
#endif
#else
#ifdef SPI_USE_DMA
    ret = hal_spi_dma_send((const void *)buf,buf_len,spi_dma_send_callback);
#else
    ret = hal_spi_send((const void *)buf, buf_len);
#endif
#endif
    TRACE(3 , "hal spi send ret = %d",ret);
    spi_wait_send_recv_done(ret);
}

void spi_device_read_memory(unsigned int address, unsigned char *buf, unsigned int buf_len)
{
	int ret = 0;

    unsigned int tx_buff[4];

    tx_buff[0] = MST_SPI_ADDR_RD(address);
    tx_buff[1] = MST_SPI_DUMMY_DATA;
    tx_buff[2] = MST_SPI_DUMMY_DATA;
    //spi_invert_data((uint8_t*)tx_buff , sizeof(tx_buff));
    soft_cs_ctrl(0);
#if (TEST_SPI_ID == 1)
#ifdef SPI_USE_DMA
    ret = hal_spilcd_dma_recv(&tx_buff, buf, buf_len,spi_dma_recv_callback);
#else
    ret = hal_spilcd_recv(&tx_buff, buf, buf_len);
#endif
#elif (TEST_SPI_ID == 2)
#ifdef SPI_USE_DMA
    ret = hal_spidpd_dma_recv(&tx_buff, buf, buf_len,spi_dma_recv_callback);
#else
    ret = hal_spidpd_recv(&tx_buff, buf, buf_len);
#endif
#else
#ifdef SPI_USE_DMA
    ret = hal_spi_dma_recv(&tx_buff, buf, buf_len,spi_dma_recv_callback);
#else
    ret = hal_spi_recv(&tx_buff, buf, buf_len);
#endif
#endif
    spi_wait_send_recv_done(ret);
}
static const unsigned long crc_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
};


unsigned long pal_crc32(unsigned long crc, const unsigned char *buf, unsigned int len)
{
    unsigned long c;

    c = crc ^ 0xffffffffL;

    while (len) {
        c = crc_table[(c ^ (*buf++)) & 0xff] ^ (c >> 8);
        len--;
    }

    return c ^ 0xffffffffL;
}

void spi_device_enable_slave(void)
{
    unsigned int buff[2]={0x00, 0x01};
    spi_device_write_memory(0xfffc0004, (unsigned char *)buff, sizeof(buff));
}

int spi_device_detect(void)
{
	int ret = 0, retry = 65535;
    unsigned int buff[2]={0};
    unsigned int ptr[3]={0};
    unsigned int spi_shm_cache[16];

_retry:
    ////////////////////
    // open SPI slave continuous read function
    buff[1] = 0x1;
    spi_device_write_memory(0xfffc0004, (unsigned char *)buff, sizeof(buff));

	spi_device_read_memory(BEST_SHARE_MEMORY_ADDRESS_POINTER, (unsigned char *)ptr, sizeof(ptr));
    TRACE(3,"read ptr=0x%x,0x%x,0x%x",ptr[0],ptr[1],ptr[2]);
    spi_device_read_memory(ptr[2], (unsigned char *)spi_shm_cache, sizeof(_spi_shm)+8);


    memcpy((void*)&_spi_shm , (void*)&(spi_shm_cache[0]) , sizeof(_spi_shm));
    TRACE(3,"spi_device_detect:get share memory struct OK.,0x%x,0x%x,0x%x,0x%x",\
        _spi_shm.shm_struct.master_tx_address,_spi_shm.shm_struct.slave_tx_address,_spi_shm.shm_struct.master_tx_len,_spi_shm.shm_struct.crc);

    ////////////////////
    // check CRC
    	unsigned int crc = pal_crc32(0, (unsigned char *)spi_shm, sizeof(*spi_shm)-4);

    	if ( (crc != spi_shm->crc) || (crc == 0) ) {
    		TRACE(3,"spi_device_detect:error,share memory struct CRC check failed!");
    		TRACE(3,"spi_device_detect:error,calc_crc=0x%x,spi_shm->crc=0x%x", crc, spi_shm->crc);
    		ret = -4;
    	}
    	else {
    		TRACE(3,"spi_device_detect:share memory struct CRC check OK.");
    	}

    ////////////////////
    // retry
    if ( (ret != 0) && (retry > 0) ) {
    	TRACE(3,"spi_device_detect:warning,ret=%d,retry=%d", ret, retry);
        ret = 0;
    	--retry;
    	goto _retry;
    }
	return ret;
}
#ifdef SPI_LOOP_TEST
void spi_loop_test(void)
{
    uint8_t tx_buff[4] , rx_buff[8];
    int ret;
    memset(tx_buff , 0xa5 , 4);
#if (TEST_SPI_ID == 1)
    ret = hal_spilcd_dma_recv(&tx_buff, rx_buff, 4,spi_dma_recv_callback);
#elif (TEST_SPI_ID == 2)
    ret = hal_spidpd_dma_recv(&tx_buff, rx_buff, 4,spi_dma_recv_callback);
#else
    ret = hal_spi_dma_recv(&tx_buff, rx_buff, 4,spi_dma_recv_callback);
#endif
    spi_wait_send_recv_done(ret);
    TRACE(3,"rx_buff=0x%02x %02x %02x %02x",rx_buff[0],rx_buff[1],rx_buff[2],rx_buff[3]);
}
#endif

POSSIBLY_UNUSED static void rand_generator(uint8_t *dest, uint32_t size)
{
    while (size--) {
        *dest++ = (uint8_t)rand();
    }
}

void spi_send_test(void)
{
    uint8_t tx_buff[4];
    int ret;
    uint32_t buf_len = 4;
    memset(tx_buff , 0xa5 , buf_len);
    soft_cs_ctrl(0);
#if (TEST_SPI_ID == 1)
#ifdef SPI_USE_DMA
    ret = hal_spilcd_dma_send((const void *)tx_buff, buf_len , spi_dma_send_callback);
#else
    ret = hal_spilcd_send((const void *)tx_buff, buf_len);
#endif
#elif (TEST_SPI_ID == 2)
#ifdef SPI_USE_DMA
    ret = hal_spidpd_dma_send((const void *)tx_buff, buf_len , spi_dma_send_callback);
#else
    ret = hal_spidpd_send((const void *)tx_buff, buf_len);
#endif
#else
#ifdef SPI_USE_DMA
    ret = hal_spi_dma_send((const void *)tx_buff,buf_len,spi_dma_send_callback);
#else
    ret = hal_spi_send((const void *)tx_buff, buf_len);
#endif
#endif
    TRACE(3 , "hal spi send ret = %d",ret);
    spi_wait_send_recv_done(ret);

}

void spi_master_tx_indicate(char set)
{
    if (set) {
        hal_gpio_pin_set(MASTER_TX_INDICATE_GPIO_PIN);
    } else {
        hal_gpio_pin_clr(MASTER_TX_INDICATE_GPIO_PIN);
    }
}

void spi_master_rx_ok(char set)
{
    if (set) {
        hal_gpio_pin_set(MASTER_RX_OK_GPIO_PIN);
    } else {
        hal_gpio_pin_clr(MASTER_RX_OK_GPIO_PIN);
    }
}

/**
 * new:
 * #defien ADDR_SIZE    4
 * #define LEN_SIZE    4
 * #define CRC_SIZE    4
 * #define DUMY_SIZE    8
 * #define BUF_SIZE  (LEN_SIZE + 1024 + CRC_SIZE)
 * static uint8_t tx_buf[ADDR_SIZE + BUF_SIZE];
 * static uint8_t rx_buf[DUMY_SIZE + BUF_SIZE];
 * 
 * len: sizeof(buf)
 * write:
 * addr + len + buf + crc
 * read:
 * dumy + len + buf + crc
 */
#ifdef USE_BBK_SPI_IF
void xtc_spi_master_write(uint8_t *buf, uint32_t len)
{
    uint32_t tx_crc;
    spi_master_tx_indicate(0);
    // memcpy buf to tx_buf
    memset(tx_buf, 0, sizeof(tx_buf));
    *(uint32_t *)(tx_buf + ADDR_SIZE) = len;
    memcpy(tx_buf + ADDR_SIZE + LEN_SIZE, buf, len);
    tx_crc = pal_crc32(0, (const uint8_t *)(tx_buf + ADDR_SIZE), (BUF_SIZE - CRC_SIZE));
    memcpy(tx_buf + ADDR_SIZE + BUF_SIZE - CRC_SIZE, &tx_crc, CRC_SIZE);
    spi_device_write_memory(_spi_shm.shm_struct.master_tx_address, tx_buf, ADDR_SIZE + BUF_SIZE);
    spi_master_tx_indicate(1);
}

void xtc_spi_master_read(void)
{
    uint32_t rx_crc;
    uint32_t tx_crc;
    spi_master_rx_ok(0);
    spi_device_read_memory(_spi_shm.shm_struct.slave_tx_address, rx_buf, DUMY_SIZE + BUF_SIZE);
    // DUMP8("%02X ", rx_buf, 32);
    rx_crc = pal_crc32(0, (const uint8_t *)rx_buf + DUMY_SIZE, (BUF_SIZE - CRC_SIZE));
    memcpy(&tx_crc, (rx_buf + DUMY_SIZE + BUF_SIZE - CRC_SIZE), CRC_SIZE);
    if (rx_crc == tx_crc) {
        TRACE(5, "---[m rx success]crc32:0x%08X,rx size:%d", rx_crc, BUF_SIZE - CRC_SIZE);
#ifdef TEST_SYSVIEW
        extern void app_user_gps_post_msg(uint8_t *buf, uint32_t len, uint8_t status);
        uint32_t len;
        memcpy(&len, (rx_buf + DUMY_SIZE), LEN_SIZE);
        if (len < 4096) {
            app_user_gps_post_msg((const uint8_t *)rx_buf + DUMY_SIZE + LEN_SIZE, len, 0x1);
        } else {
            app_user_gps_post_msg((const uint8_t *)rx_buf + DUMY_SIZE + LEN_SIZE, len, NULL);
        }
#endif
    } else {
        TRACE(6, "***[m rx fail]tx crc32:0x%08X,rx crc32:0x%08X,rx size:%d", tx_crc, rx_crc, BUF_SIZE - CRC_SIZE);
    }
    spi_master_rx_ok(1);
}

#else

static void spi_master_read(void)
{
    uint32_t rx_crc;
    uint32_t tx_crc;
    spi_master_rx_ok(0);
    spi_device_read_memory(_spi_shm.shm_struct.slave_tx_address, rx_buf, BUF_SIZE+8);
    // DUMP8("%02X ", rx_buf, 32);
    rx_crc = pal_crc32(0, (const uint8_t *)rx_buf + 8, (BUF_SIZE - CRC_SIZE));
    memcpy(&tx_crc, (rx_buf + 8 + BUF_SIZE - CRC_SIZE), CRC_SIZE);
    if (rx_crc == tx_crc) {
        TRACE(5, "---[m rx success]crc32:0x%08X,rx size:%d", rx_crc, BUF_SIZE - CRC_SIZE);
    } else {
        TRACE(6, "***[m rx fail]tx crc32:0x%08X,rx crc32:0x%08X,rx size:%d", tx_crc, rx_crc, BUF_SIZE - CRC_SIZE);
    }
    spi_master_rx_ok(1);
}

static void spi_master_write(void)
{
    uint32_t tx_crc;
    spi_master_tx_indicate(0);
    rand_generator(tx_buf + sizeof(uint32_t), (BUF_SIZE - CRC_SIZE));
    tx_crc = pal_crc32(0, (const uint8_t *)(tx_buf + sizeof(uint32_t)), (BUF_SIZE - CRC_SIZE));
    memcpy(tx_buf + sizeof(uint32_t) + BUF_SIZE - CRC_SIZE, &tx_crc, CRC_SIZE);
    // TRACE(3, "tx addr=%08x", _spi_shm.shm_struct.master_tx_address);
    spi_device_write_memory(_spi_shm.shm_struct.master_tx_address, tx_buf, BUF_SIZE + 4);
    spi_master_tx_indicate(1);
}

#endif

static void spi_slave_irq_handle(uint8_t status)
{
    if(status == SPI_SLAVE_ACTION_TX_OK_CONFIRMED) {
#ifdef USE_BBK_SPI_IF
    xtc_spi_master_read();
#else
    spi_master_read();
#endif
        
    } else if(status == SPI_SLAVE_ACTION_RX_INDICATE) {
        //enable loop write
        // spi_master_write();
    }
}

static void _spi_slave_gpio_irq(enum HAL_GPIO_PIN_T pin)
{
    if (pin == SLAVE_TX_INDICATE_GPIO_PIN) {
        spi_slave_irq_handle(SPI_SLAVE_ACTION_TX_OK_CONFIRMED);
    } else if (pin == SLAVE_RX_OK_GPIO_PIN) {
        spi_slave_irq_handle(SPI_SLAVE_ACTION_RX_INDICATE);
    }
}

POSSIBLY_UNUSED static void set_master_iomux(void)
{
    struct HAL_GPIO_IRQ_CFG_T gpiocfg;
    // gpio
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_trigger_master_pinmux[] = {
        {MASTER_TX_INDICATE_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {MASTER_RX_OK_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {SLAVE_TX_INDICATE_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
        {SLAVE_RX_OK_GPIO_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(gpio_trigger_master_pinmux, ARRAY_SIZE(gpio_trigger_master_pinmux));

    // input pin
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SLAVE_TX_INDICATE_GPIO_PIN, HAL_GPIO_DIR_IN, 1);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)SLAVE_RX_OK_GPIO_PIN, HAL_GPIO_DIR_IN, 1);

    // output pin
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)MASTER_TX_INDICATE_GPIO_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)MASTER_RX_OK_GPIO_PIN, HAL_GPIO_DIR_OUT, 0);

    gpiocfg.irq_enable = true;
    gpiocfg.irq_debounce = false;
    gpiocfg.irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING;
    gpiocfg.irq_handler = _spi_slave_gpio_irq;
    gpiocfg.irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE;
    hal_gpio_setup_irq(SLAVE_TX_INDICATE_GPIO_PIN, &gpiocfg);
    hal_gpio_setup_irq(SLAVE_RX_OK_GPIO_PIN, &gpiocfg);
}

#ifdef SPI_WAKEUP
#define WAKEUP_PIN HAL_GPIO_PIN_P0_0
static void wakeup_gpio_init(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
        {WAKEUP_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(WAKEUP_PIN, HAL_GPIO_DIR_OUT, 0);
}
#endif

// SPI_TEST
void test_spi(void)
{
#ifdef USE_SOFT_CS
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP soft_cs_pin[] = {
        {SOFT_CS_PIN, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_MEM, HAL_IOMUX_PIN_PULLUP_ENABLE}
    };
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(2,"%s used id: %s", __FUNCTION__,
                #if (TEST_SPI_ID == 1)
                    "SPI_ID_SLCD"
                #elif (TEST_SPI_ID == 2)
                    "SPI_ID_DPD"
                #else
                    "SPI_ID_0"
                #endif
            );

    /* return value. */
    int ret = 0;
    uint32_t spi_data = 0xdea9385b;

    struct HAL_SPI_CFG_T spi_cfg;
    memset(&spi_cfg, 0, sizeof(spi_cfg));

    spi_cfg.clk_delay_half = true;          /* clk phase */
    spi_cfg.clk_polarity = true;            /* clk polarity in steady state*/
    spi_cfg.slave = false;
#ifdef SPI_USE_DMA
    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;
#else
    spi_cfg.dma_rx = false;
    spi_cfg.dma_tx = false;
#endif
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
    spi_cfg.rate = TEST_SPI_RATE;
    spi_cfg.rx_bits = 32;
    spi_cfg.tx_bits = 32;
    spi_cfg.rx_frame_bits = 0;

#if (TEST_SPI_ID == 1)
    /* LCDSPI */
    hal_iomux_set_spilcd();
    ret = hal_spilcd_open(&spi_cfg);
    TRACE(2,"%s hal_spilcd_open ret[%d]", __FUNCTION__, ret);
#elif (TEST_SPI_ID == 2)
    /* LCDDPD */
    hal_iomux_set_spi2();
    ret = hal_spidpd_open(&spi_cfg);
    TRACE(2,"%s hal_spidpd_open ret[%d]", __FUNCTION__, ret);
#else
    /* SPI 0*/
    hal_iomux_set_spi();
    ret = hal_spi_open(&spi_cfg);
    TRACE(2,"%s hal_spi_open ret[%d]", __FUNCTION__, ret);
#endif
#ifdef USE_SOFT_CS
    hal_iomux_init(soft_cs_pin, 1);
    hal_gpio_pin_set_dir(SOFT_CS_PIN , HAL_GPIO_DIR_OUT , 1);
#endif
#ifdef SPI_LOOP_TEST
    while(1)
        spi_loop_test();
#endif

#ifdef SPI_WAKEUP
    wakeup_gpio_init();

#ifdef MCU_SPI_SLAVE_SPECIFY_MEM
    _spi_shm.shm_struct.master_tx_address = SPI_SLAVE_MEM_M_TX;
    _spi_shm.shm_struct.master_tx_len = SPI_SLAVE_MEM_SIZE;
    _spi_shm.shm_struct.slave_tx_address = SPI_SLAVE_MEM_S_TX;
    _spi_shm.shm_struct.slave_tx_len = SPI_SLAVE_MEM_SIZE;
#else
    hal_gpio_pin_set(WAKEUP_PIN);
    osDelay(2);
    spi_device_detect();
    hal_gpio_pin_clr(WAKEUP_PIN);
#endif

#else
    spi_device_detect();
#endif

    set_master_iomux();
    // spi_send_test();
    TRACE(2,"spi data=0x%x",spi_data);

#ifndef TEST_SYSVIEW
    do {
#if 0
        soft_cs_ctrl(0);
        #if (TEST_SPI_ID == 1)
            hal_spilcd_send(&spi_data, 4);
        #elif (TEST_SPI_ID == 2)
            hal_spidpd_send(&spi_data, 4);
        #else
            hal_spi_send(&spi_data, 4);         /* len: the number of BYTE. */
        #endif
        spi_wait_send_recv_done(0);
#endif

#ifdef SPI_WAKEUP
        hal_gpio_pin_set(WAKEUP_PIN);
        osDelay(2);
#ifdef MCU_SPI_SLAVE_SPECIFY_MEM
        spi_device_enable_slave();
#else
        spi_device_detect();
#endif 
        hal_gpio_pin_clr(WAKEUP_PIN);
#endif

#ifdef USE_BBK_SPI_IF
        static uint16_t cnt = 1;
        uint16_t len = cnt;
        uint8_t test_buf[500] = {0};
        memset(test_buf, cnt, sizeof(test_buf));
        xtc_spi_master_write(test_buf, sizeof(test_buf));
        cnt++;
#else
        spi_master_write();
#endif
        osDelay(5000);

    } while(1);
#endif
}
#endif

