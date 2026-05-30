/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "bsp_config.h"
#include "bsp_common.h"
#include "bsp_i2c.h"
#include "bsp_hyn.h"
#include <stdint.h>

#if IS_SUPPORT_TOUCH_IC_OTA
#include "hyn_firmware.h"

/*
* DEFINES
*****************************************************************************************
*/
#define HYN_KEY_RETRY_COUNT             (3)
#ifndef HYN_OTA_VERBOSE_LOG
#define HYN_OTA_VERBOSE_LOG             (0)
#endif

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t tp_hrs_write_bytes(uint16_t reg_addr, uint8_t *buf, uint16_t len)
{
#if HYN_OTA_VERBOSE_LOG
	LOG_PBUS("tp_hrs_write_bytes: reg_addr: 0x%04x, len: %d", reg_addr, len);
#endif
    uint8_t addr_buf[2] = {(uint8_t)(reg_addr >> 8), (uint8_t)(reg_addr & 0xFF)};
    i2c_master_data_t data = {
        .dev_address = HYN_TOUCH_BOOT_I2C_ADDR,
        .reg_addr = addr_buf,
        .reg_addr_len = 2,
        .data = buf,
        .len = len,
    };
    return ((bsp_i2c_write_byte(TOUCH_KEY_I2C_ID, &data) == BSP_OK) ? HYN_TRUE : HYN_FAULT);
}

static int tp_hrs_read_bytes(uint16_t reg_addr, uint8_t *buf, uint16_t len)
{
#if HYN_OTA_VERBOSE_LOG
    LOG_PBUS("tp_hrs_read_bytes: reg_addr: 0x%04x, len: %d", reg_addr, len);
#endif
    uint8_t addr_buf[2] = {(uint8_t)(reg_addr >> 8), (uint8_t)(reg_addr & 0xFF)};
    i2c_master_data_t data = {
        .dev_address = HYN_TOUCH_BOOT_I2C_ADDR,
        .reg_addr = addr_buf,
        .reg_addr_len = 2,
        .data = buf,
        .len = len,
    };
    return ((bsp_i2c_read_byte(TOUCH_KEY_I2C_ID, &data) == BSP_OK) ? HYN_TRUE : HYN_FAULT);
}

static int hyn_ic_enter_boot_mode(void)//使触摸IC进入boot模式
{
	uint8_t retry_cnt = 0;
	
	for (retry_cnt = 0; retry_cnt < 10; retry_cnt++) {
		uint8_t cmd[3];
		uint8_t read_buf[2];

		bsp_touch_key_reset();
		
		msleep(5 + retry_cnt);//5~20ms 这个延时必须准确
		
		cmd[0] = 0xAB;//0xAB:CST816T/CST816D/CST816S/HYN19X2/CSK0XX		//	 0xAA；CST716    
		if (tp_hrs_write_bytes(0xA001, &cmd[0], 1) == HYN_FAULT) {
			continue;
		}

		msleep(1);
		if (tp_hrs_read_bytes(0XA003, read_buf, 1) == HYN_FAULT) {
			continue;
		}

		if (read_buf[0] != 0xC1) {//0xC1:CST816T、CST816D、CSK0XT    //   0x55: CST716 、CST816S、CSK0XS 
			LOG_PBUS("read_buf[0] != 0xC1: %02x", read_buf[0]);
			continue;
		} else {
			return HYN_TRUE;
		}
	}
	if (retry_cnt >= 10) return HYN_FAULT;

    return HYN_TRUE;
}

static uint8_t hyn_ic_exit_boot_mode(void)
{
	uint8_t cmd = 0xEE;
	if (tp_hrs_write_bytes(0xA006, &cmd, 1) == HYN_FAULT) {
		return HYN_FAULT;
	}
	return HYN_TRUE;
}

static uint16_t  hyn_read_checksum(void)//读取IC里面的校验和
{
	union{
		uint16_t sum;
		uint8_t buf[2];
	} checksum;

	uint8_t cmd[3];
	uint8_t retry_cnt = 50;
	uint8_t checksum_ready_flag = 0;
	uint8_t retry = 1;
	uint8_t result = HYN_TRUE;

	for (retry = 1; retry < 6; retry++) {
		if (hyn_ic_enter_boot_mode() == HYN_FAULT) {
			LOG_PBUS("hyn_ic_enter_bootmode fail!\r\n");
			continue;
		}		
		cmd[0] = 0x00;
		result = tp_hrs_write_bytes(0xA003, &cmd[0], 1);//写数据--开始计算校验和
		if (result == HYN_FAULT) continue;

		msleep(100*retry); // 要求100ms以上

		checksum.sum = 0;			
		retry_cnt = 50;
		while(retry_cnt--) {
			result = tp_hrs_read_bytes(0xA000, &checksum_ready_flag, 1);
			if (result == HYN_FAULT) continue;

			if (checksum_ready_flag == 0x01) { //checksum计算完成
				break;
			}
			msleep(10);
		}
		result = tp_hrs_read_bytes(0xA008, checksum.buf, 2);//读出校验和
		if (result == HYN_FAULT) continue;

		return checksum.sum;
	}	

	if (retry >= 6) {
		return HYN_FAULT;
	}
	return checksum.sum;
}

static uint8_t hyn_ic_update(uint16_t start_addr, uint16_t len, uint8_t *src_data, uint8_t retry)//升级固件
{
	uint16_t written_len;
    uint8_t last_reported_progress = 0;
	uint8_t cmd[4];
	uint8_t read_buf[2];
	uint8_t retry_cnt = 50;
	
	if (hyn_ic_enter_boot_mode()) {
		return HYN_FAULT;
	}
	written_len = 0;
    do {
		if (written_len >= len) {
			return HYN_TRUE;
		}
		cmd[0] = start_addr & 0xFF;
		cmd[1] = start_addr >> 8;

		if (tp_hrs_write_bytes(0xA014, &cmd[0], 2) == HYN_FAULT) return HYN_FAULT;
		 
#if 0   // 0xA018 缓存数据起始地址 长度512
			#if 0  //512字节分成4组，每组128字节
			{
				uint8_t i=0;
				for (i=0; i<4; i++){
					if (tp_hrs_write_bytes(0xA018+(i*128), src_data+(i*128), PER_LEN/4)== HYN_FAULT) return HYN_FAULT;
				}
			}
			#else   
			{		//512字节分成16组，每组32字节
				uint8_t i=0;
				for (i=0; i<16; i++){
					if (tp_hrs_write_bytes(0xA018+(i*32), src_data+(i*32), PER_LEN/16) == HYN_FAULT) return HYN_FAULT;
				}
			}
			
//			{ //512字节分成128组，每组4字节
//				uint8_t i=0;
//				for(i=0; i<128; i++){
//					if (tp_hrs_write_bytes(0xA018+(i*4), src_data+(i*4), PER_LEN/128) == HYN_FAULT) return HYN_FAULT;
//				}
//			}
		   #endif 
#else
		if (tp_hrs_write_bytes(0xA018, src_data, PER_LEN) == HYN_FAULT) return HYN_FAULT;//写入512字节数据
#endif

		msleep(1);
		cmd[0] = 0xEE;
		if (tp_hrs_write_bytes(0xA004, &cmd[0], 1) == HYN_FAULT) return HYN_FAULT;////在执行这个命令之前，必须写够512个字节！！！

		msleep(200*retry); //这个时间不能省  unit=1ms    >=100
		
		retry_cnt = 50;
		while(retry_cnt--) {
			msleep(5);			
			if (tp_hrs_read_bytes(0xA005, read_buf, 1) == HYN_FAULT) continue;						 
			if (read_buf[0] == 0x55) {// success
				msleep(1);
				break;
			}
		}
		
		if (retry_cnt == 0) return HYN_FAULT;
		
		start_addr += PER_LEN;
		src_data += PER_LEN;
		written_len += PER_LEN;
		/// 上报升级进度
		uint32_t progress = (written_len * 100) / len;
        uint8_t progress_u8 = (uint8_t)(progress > 100 ? 100 : progress);
        if ((progress_u8 >= 100) || (progress_u8 >= (uint8_t)(last_reported_progress + 5))) {
            last_reported_progress = progress_u8;
            bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_PROGRESS, len, written_len);
        }
    } while(len);

	cmd[0] = 0x00;
	if (tp_hrs_write_bytes(0xA003, &cmd[0], 1) == HYN_FAULT) return HYN_FAULT;
	return HYN_TRUE;
}

/**
 * @param bin  固件镜像（含 6 字节头：addr_lo,addr_hi,len_lo,len_hi,cs_lo,cs_hi + payload）
 * @param bin_size 镜像总长度
 */
static uint8_t touch_fw_update_with_bin(const uint8_t *bin, uint32_t bin_size, bool force_update)
{
	uint16_t ic_checksum = 0;

    LOG_PBUS("touch IC start OTA");
	if (hyn_ic_enter_boot_mode() != HYN_TRUE) {
        LOGE("hynitron enter boot mode fail! \r\n");
		return HYN_FAULT;
	}

    if (bin_size > 10U) {
        uint16_t start_addr = bin[1];
        uint16_t length = bin[3];
        uint16_t checksum = bin[5];
        uint8_t retry = 1;

        start_addr <<= 8;
        start_addr |= bin[0];
        length <<= 8;
        length |= bin[2];
        checksum <<= 8;
        checksum |= bin[4];
        LOG_PBUS("hynitron enter boot mode success.\r\n");
        LOG_PBUS("hex_file_checksum: 0x%02x \r\n", checksum);
        ic_checksum = hyn_read_checksum();
        LOG_PBUS("IC_checksum: 0x%02x \r\n", ic_checksum);
		
		if ((start_addr | length) == 0x3C00 &&
            (force_update || (ic_checksum != checksum))) {
            for (retry = 1; retry < 4; retry++) {
                if (hyn_ic_update(start_addr, length, (uint8_t *)(uintptr_t)(bin + 6), retry) == HYN_FAULT) {
                    continue;
                }
                break;
            }
            if (retry >= 4) {
                hyn_ic_exit_boot_mode();
                bsp_touch_key_reset();
                return HYN_FAULT;
            }

            ic_checksum = hyn_read_checksum();
            LOG_PBUS("after update IC_checksum: 0x%02x \r\n", ic_checksum);

            if (ic_checksum != checksum) {
                LOGE("hynitron update fail! \r\n");
            }
        }
    }

    hyn_ic_exit_boot_mode();
    bsp_touch_key_reset();
    LOG_PBUS("touch IC OTA over");

    return HYN_TRUE;
}

uint8_t touch_fw_update_handler(void)
{
    return touch_fw_update_with_bin(app_bin, (uint32_t)sizeof(app_bin), false);
}

uint8_t touch_fw_update_handler_force(bool force_update)
{
	bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_START, sizeof(app_bin), 0);
	uint8_t ret = touch_fw_update_with_bin(app_bin, (uint32_t)sizeof(app_bin), force_update);
    if (ret != 0) {
        msleep(1000);
        bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_FAIL, sizeof(app_bin), 0);
    } else {
        bsp_touch_key_ota_event_cb(KEY_OTA_EVENT_UPDATE_SUCCESS, sizeof(app_bin), sizeof(app_bin));
    }
    return ret;
}

bool touch_firmware_is_lower_than_image(int current_version)
{
    return current_version < TOUCH_IC_FW_VERSION;
}

int touch_firmware_target_version(void)
{
    return TOUCH_IC_FW_VERSION;
}
#endif