/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __MMC_CARD_H__
#define __MMC_CARD_H__

#include "host.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CARD_WRITE                      0
#define CARD_READ                       1

#define CARD_XFER_SIZE_MAX              (65535 * 512U)  //31.9995Mbytes

/*
*   Card Type Definition
*/
#define CARD_CEATA                      0
#define CARD_SD                         DEV_TYPE_SD
#define CARD_SDIO                       DEV_TYPE_SDIO
#define CARD_MMC                        DEV_TYPE_EMMC

#define mmc_card_type(x)                ((x==CARD_MMC)?"MMC":(x==CARD_SD)?"SD":(x==CARD_SDIO)?"SDIO":(x==CARD_CEATA)?"CEATA":"UNKNOWN")

#define MMC_LOW_CAPACITY_CARD           ((uint32_t)0x00000000U)   //mmc card capacity <=2Gbytes
#define MMC_HIGH_CAPACITY_CARD          ((uint32_t)0x00000001U)   //mmc card capacity >2Gbytes

/*
*   Card Response Definition
*/
#define CARD_RESPONSE_NONE              0
#define CARD_RESPONSE_R1                1
#define CARD_RESPONSE_R1B               2
#define CARD_RESPONSE_R2                3
#define CARD_RESPONSE_R3                4
#define CARD_RESPONSE_R4                5
#define CARD_RESPONSE_R5                6
#define CARD_RESPONSE_R5B               7
#define CARD_RESPONSE_R6                8
#define CARD_RESPONSE_R7                9

/*
*   Card Status Definition
*/
#define CARD_IDLE_STATE                 0
#define CARD_READY_STATE                1
#define CARD_IDENT_STATE                2
#define CARD_STBY_STATE                 3
#define CARD_TRAN_STATE                 4
#define CARD_DATA_STATE                 5
#define CARD_RCV_STATE                  6
#define CARD_PRG_STATE                  7
#define CARD_DIS_STATE                  8
#define CARD_BTST_STATE                 9
#define CARD_SLP_STATE                  10

#define CARD_STATUS_SWITCH_ERROR        (1 << 7)
#define CARD_STATUS_RDY_FOR_DATA        (1 << 8)
#define CARD_STATUS_CURR_STATE          (0xf << 9)
#define CARD_STATUS_ERROR               (1 << 19)
#define CARD_STATE_TRANS                (4 << 9)

/*
*   SD card VHS
*/
#define SDVHS_2_7_TO_3_6                0x01
#define SDVHSCHECKPATTERN               0xAA

/*
*   The OCR Register Fields
*/
#define CARD_CAPACITY_STATUS            ((uint32_t)(1 << 30))       // CCS
#define SWITCH_1_8V_ACCEPTED            ((uint32_t)(1 << 24))       // S18A
#define CARD_POWER_UP_STATUS            ((uint32_t)(1 << 31))

/*
*   The OCR Register Fields
*/
#define ACCESS_MODE                     0x60000000
#define ACCESS_MODE__BYTE_MODE          0x00000000
#define ACCESS_MODE__SECTOR_MODE        0x40000000
#define ACCESS_MODE_S18A                0x01000000
/*
*   The RCA Field
*/
#define MMC_RCA_MASK                    0xFFFF
#define MMC_RCA_VALUE_DEFAULT           0xBE01

/*
*   EXT_CSD
*/
#define MMC_EXT_CSD_LENGTH              512
//----------------------------------------------------------------------------------------------------------
#define EXT_CSD__S_CMD_SET              504     // Supported Command Sets
#define EXT_CSD__HPI_FEATURES           503     // HPI Features
#define EXT_CSD__BKOPS_SUPPORT          502     // Background Operations Support
#define EXT_CSD__GENERIC_CMD6_TIME      248     // Generic CMD6 timeout
#define EXT_CSD__POWER_OFF_LONG_TIME    247     // Power off notification(long) timeout
#define EXT_CSD__BKOPS_STATUS           246     // Background Operations Status
#define EXT_CSD__CORRECTLY_PRG_SECTORS_NUM  242 // Number of Correctly Programmed Sectors (4 Bytes)
#define EXT_CSD__INI_TIMEOUT_AP         241     // 1st Initialization Time After Partitioning
#define EXT_CSD__PWR_CL_DDR_52_360      239     // Power class for 52MHz, DDR at 3.6V
#define EXT_CSD__PWR_CL_DDR_52_195      238     // Power class for 52MHz, DDR at 1.95V
#define EXT_CSD__MIN_PERF_DDR_W_8_52    235     // Minimum Write Performance for 8-bit at 52MHz in DDR Mode
#define EXT_CSD__MIN_PERF_DDR_R_8_52    234     // Minimum Read  Performance for 8-bit at 52MHz in DDR Mode
#define EXT_CSD__TRIM_MULT              232     // TRIM Multiplier
#define EXT_CSD__SEC_FEATURE_SUPPORT    231     // Secure Feature Support
#define EXT_CSD__SEC_ERASE_MULT         230     // Secure Erase Multiplier
#define EXT_CSD__SEC_TRIM_MULT          229     // Secure TRIM Multiplier
#define EXT_CSD__BOOT_INFO              228     // Boot Information
#define EXT_CSD__BOOT_SIZE_MULTI        226     // Boot Partition Size
#define EXT_CSD__ACC_SIZE               225     // Access Size
#define EXT_CSD__HC_ERASE_GRP_SIZE      224     // High-Capacity Erase Unit Size
#define EXT_CSD__ERASE_TIMEOUT_MULTI    223     // High-Capacity Erase Timeout
#define EXT_CSD__REL_WR_SEC_C           222     // Reliable Write Sector Count
#define EXT_CSD__HC_WP_GRP_SIZE         221     // High-Capacity Write Protect Group Size
#define EXT_CSD__S_C_VCC                220     // Sleep Current (VCC)
#define EXT_CSD__S_C_VCCQ               219     // Sleep Current (VCCQ)
#define EXT_CSD__S_A_TIMEOUT            217     // Sleep/Awake Timeout
#define EXT_CSD__SLEEP_NOTIFY_TIME      216     // Sleep Notification Timeout
#define EXT_CSD__SEC_COUNT              212     // Sector Count (4 Bytes)
#define EXT_CSD__MIN_PERF_W_8_52        210     // Minimum Write Performance for 8bit @ 52MHz
#define EXT_CSD__MIN_PERF_R_8_52        209     // Minimum Read Performance for 8bit @ 52MHz
#define EXT_CSD__MIN_PERF_W_8_26_4_52   208     // Minimum Write Performance for 8bit @ 26MHz / 4bit @ 52MHz
#define EXT_CSD__MIN_PERF_R_8_26_4_52   207     // Minimum Read Performance for 8bit @ 26MHz / 4bit @ 52MHz
#define EXT_CSD__MIN_PERF_W_4_26        206     // Minimum Write Performance for 4bit @ 26MHz
#define EXT_CSD__MIN_PERF_R_4_26        205     // Minimum Read Performance for 4bit @ 26MHz
#define EXT_CSD__PWR_CL_26_360          203     // Power Class for 26MHz @ 3.6V
#define EXT_CSD__PWR_CL_52_360          202     // Power Class for 52MHz @ 3.6V
#define EXT_CSD__PWR_CL_26_195          201     // Power Class for 26MHz @ 1.95V
#define EXT_CSD__PWR_CL_52_195          200     // Power Class for 52MHz @ 1.95V
#define EXT_CSD__PARTITION_SWITCH_TIME  199     // Partition Switching Timing
#define EXT_CSD__OUT_OF_INTERRUPT_TIME  198     // Out-of-Interrupt Busy Timing
#define EXT_CSD__CARD_TYPE              196     // Card Type
#define EXT_CSD__CSD_STRUCTURE          194     // CSD Structure Version
#define EXT_CSD__EXT_CSD_REV            192     // Extended CSD Revision

// Read-Write Fields
#define EXT_CSD__CMD_SET                191     // R/W - Command Set
#define EXT_CSD__CMD_SET_REV            189     // RO  - Command Set Revision
#define EXT_CSD__POWER_CLASS            187     // R/W - Power Class
#define EXT_CSD__HS_TIMING              185     // R/W - High Speed Interface Timing
#define EXT_CSD__BUS_WIDTH              183     // WO  - Bus Width Mode
#define EXT_CSD__ERASED_MEM_COUNT       181     // RO  - Erased Memory Content
#define EXT_CSD__PARTITION_CONFIG       179     // R/W - Partition Configuration
#define EXT_CSD__BOOT_CONFIG_PROT       178     // R/W - Boot Config Protection
#define EXT_CSD__BOOT_BUS_WIDTH         177     // R/W - Boot Bus Width
#define EXT_CSD__ERASE_GROUP_DEF        175     // R/W - High-Density Erase Group Definition
#define EXT_CSD__BOOT_WP                173     // R/W - Boot Area Write Protection Register
#define EXT_CSD__USER_WP                171     // R/W - User Area Write Protection Register
#define EXT_CSD__FW_CONFIG              169     // R/W - FW Configuration
#define EXT_CSD__RPMB_SIZE_MULT         168     // RO  - RPMB Size
#define EXT_CSD__WR_REL_SET             167     // R/W - Write Reliability Setting Register
#define EXT_CSD__WR_REL_PARAM           166     // RO  - Write Reliability Parameter Register
#define EXT_CSD__BKOPS_START            164     // WO  - Manually Start Background Operations
#define EXT_CSD__BKOPS_EN               163     // R/W - Enabled Background Operations Handshake
#define EXT_CSD__RST_n_FUNCTION         162     // R/W - H/W Reset Function
#define EXT_CSD__HPI_MGMT               161     // R/W - HPI Management
#define EXT_CSD__PARTITIONING_SUPPORT   160     // RO  - Partitioning Support
#define EXT_CSD__MAX_ENH_SIZE_MULT      157     // RO  - Max Enhanced Area Size (3 Bytes)
#define EXT_CSD__PARTITIONS_ATTRIBUTE   156     // R/W - Partitions Attribute
#define EXT_CSD__PARTITION_SETTING_COMPLETED 155// R/W - Partitioning Setting
#define EXT_CSD__GP_SIZE_MULT           143     // R/W - General Purpose Partition Size (12 Bytes)
#define EXT_CSD__ENH_SIZE_MULT          140     // R/W - Enhanced User Data Area Size (3 Bytes)
#define EXT_CSD__ENH_START_ADDR         136     // R/W - Enhanced User Data Start Address (4 Bytes)
#define EXT_CSD__SEC_BAD_BLK_MGMNT      134     // R/W - Bad Block Management Mode
#define EXT_CSD__PWR_OFF_NOTIFY         34      // R/W - Power Off Notification
#define EXT_CSD__CACHE_CTRL             33      // R/W - Control to turn the Cache ON/OFF
#define EXT_CSD__FLUSH_CACHE            32      // W   - Flushing of the cache
//----------------------------------------------------------------------------------------------------------

#define CMD6_CMD_SET                    0
#define CMD6_SET_BITS                   1
#define CMD6_CLR_BITS                   2
#define CMD6_WR_BYTE                    3

//7.4.2 S_CMD_SET [504]
#define EXT_CSD__S_CMD_SET__STD_MMC     0
#define EXT_CSD__S_CMD_SET__ATA_BY_MMCA 1

#define CARD_BLOCK_LENGTH               512 //always 512 when emmc card
#define CARD_RPMB_DATA_LENGTH           256 //256 when emmc RPMB

/*
*   ERASE arguments
*/
#define MMC_ERASE_ARG                   0x00000000
#define MMC_SECURE_ERASE_ARG            0x80000000
#define MMC_TRIM_ARG                    0x00000001
#define MMC_DISCARD_ARG                 0x00000003
#define MMC_SECURE_TRIM1_ARG            0x80000001
#define MMC_SECURE_TRIM2_ARG            0x80008000

/*
*   RPMB
*/
#define RPMB_REQ_KEY                    1
#define RPMB_REQ_WCOUNTER               2
#define RPMB_REQ_WRITE_DATA             3
#define RPMB_REQ_READ_DATA              4
#define RPMB_REQ_STATUS                 5

#define RPMB_SZ_NONCE                   16
#define RPMB_SZ_DATA                    256
#define RPMB_SZ_MAC                     32
#define RPMB_SZ_STUFF                   196

enum EMMC_PARTITION_ACCESS {
    NO_ACCESS_BOOT_DEFAULT = 0,
    EMMC_BOOT_PART_1,
    EMMC_BOOT_PART_2,
    EMMC_RPMB_PART,
    EMMC_GENERAL_PART_1,
    EMMC_GENERAL_PART_2,
    EMMC_GENERAL_PART_3,
    EMMC_GENERAL_PART_4,
};

/*
*   Card Registers Definition
*/
struct mmc_cid {
    unsigned char   mid;    // Manufacturer ID
    unsigned char   cbx;    // Card/BGA
    unsigned short  oid;    // OEM/Application ID
    unsigned char   pnm[6]; // Product Name
    unsigned char   prv;    // Product Revision
    unsigned int    psn;    // Product Serial Number
    unsigned short  mdt;    // Manufacturing date
};

struct mmc_csd {
    unsigned char  csd_structure;           // CSD Structure
    unsigned char  spec_vers;               // System specification version
    unsigned char  taac;                    // data read access-time - 1
    unsigned char  nsac;                    // data read access-time - 2 in CLK cycles (nsac * 100)
    unsigned char  tran_speed;              // max. data transfer rate
    unsigned short ccc;                     // card command classes
    unsigned char  read_bl_len;             // max. read data block length
    unsigned char  read_bl_partial;         // partial blocks for read allowed
    unsigned char  write_blk_misalign;      // write block misalignment
    unsigned char  read_blk_misalign;       // read block misalignment
    unsigned char  dsr_imp;                 // DSR implemented
    unsigned int   c_size;                  // device size
    unsigned char  vdd_r_curr_min;          // max. read current @ VDD min
    unsigned char  vdd_r_curr_max;          // max. read current @ VDD max
    unsigned char  vdd_w_curr_min;          // max. write current @ VDD min
    unsigned char  vdd_w_curr_max;          // max. write current @ VDD max
    unsigned char  c_size_mult;             // device size multiplier
    unsigned char  erase_grp_size;          // erase group size
    unsigned char  erase_grp_mult;          // erase group size multiplier
    unsigned char  erase_blk_en;            // erase single block enable
    unsigned char  sector_size;             // erase sector size
    unsigned char  wp_grp_size;             // write protect group size
    unsigned char  wp_grp_enable;           // write protect group enable
    unsigned char  default_ecc;             // manufacturer default ECC
    unsigned char  r2w_factor;              // write speed factor
    unsigned char  write_bl_len;            // max. write data block length
    unsigned char  write_bl_partial;        // partial blocks for write allowed
    unsigned char  content_prot_app;        // Content protection application
    unsigned char  file_format_grp;         // File format group
    unsigned char  copy;                    // copy flag (OTP)
    unsigned char  perm_write_protect;      // permanent write protection
    unsigned char  tmp_write_protect;       // temporary write protection
    unsigned char  file_format;             // File format
    unsigned char  ecc;                     // ECC code
};

struct mmc_card {
    unsigned int type;
    unsigned int ocr;
    unsigned int sd_vhs;
    unsigned int rca;
    unsigned int r1;
    unsigned int r5;

    unsigned int raw_data[4];
    struct mmc_cid cid;
    struct mmc_csd csd;
    unsigned char ext_csd[512];

    unsigned char high_capacity;
    unsigned char has_sleep;            /* card sleep status: 1=sleep, 0=awake */
    unsigned int read_bl_len;
    unsigned int write_bl_len;
    unsigned char can_trim;
    unsigned int erase_grp_size_card;   /* in 512-byte sectors */

    unsigned int timing;
    unsigned int width;
};

struct mmc_card_info {
    unsigned int cardtype;                     // card Type
    unsigned int cardclass;                    // class of the card class
    unsigned int relcardadd;                   // Relative Card Address
    unsigned int blocknbr;                     // Card Capacity in blocks
    unsigned int blocksize;                    // one block size in bytes
    unsigned int logblocknbr;                  // Card logical Capacity in blocks
    unsigned int logllocksize;                 // logical block size in bytes
    unsigned int ext_csd_rev;                  // extended CSD revision
    unsigned int firmware_version_h32;         // firmware version[261:258]
    unsigned int firmware_version;             // firmware version[257:254]
    unsigned int device_version;               // device version
    unsigned int cache_size;                   // emmc cache size
    unsigned int cache_ctrl;                   // indicates whether the cache of emmc is open
    unsigned int cache_flush_policy;           // cache flush policy (read only)
    unsigned int generic_cmd6_time;            // generic CMD6 timeout
    struct mmc_cid cid;
    struct mmc_csd csd;
};

struct mmc_rpmb_frame {
    unsigned char stuff[RPMB_SZ_STUFF];        //Stuff Bytes
    unsigned char mac[RPMB_SZ_MAC];            //Authentication Key / Message Authentication Code (MAC)
    unsigned char data[RPMB_SZ_DATA];          //data
    unsigned char nonce[RPMB_SZ_NONCE];        //Nonce
    unsigned int write_counter;                //Write Counter
    unsigned short address;                    //Data Address
    unsigned short block_count;                //Block Count
    unsigned short result;                     //Operation Result
    unsigned short request;                    //Request (to the memory), Response (from the memory)
};

/*
*   Common Card Operation Functions
*/
int detect_card(struct mmc_host *host);
int init_card(struct mmc_host *host);
void init_card_info(struct mmc_host *host);
int config_card(struct mmc_host *host, unsigned int timing, unsigned int khz, unsigned int width);

/*******************************************************************************
Note: The following card_offset are block addresses rather than physical byte addresses
*******************************************************************************/
//dma + blocking mode(original mode)
int write_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);
int read_card(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);
int write_card_ext(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);
int read_card_ext(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);

//polling/blocking mode
int write_card_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);
int read_card_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);

//dma mode,if there is a callback, it is non-blocking mode, otherwise it is blocking mode
int write_card_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);
int read_card_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int data_size);

//polling/blocking mode
int write_card_blks_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt);
int read_card_blks_blocking_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt);

//dma mode,if there is a callback, it is non-blocking mode, otherwise it is blocking mode
int write_card_blks_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt);
int read_card_blks_dma_mode(struct mmc_host *host, unsigned int card_offset, void *buf, unsigned int block_cnt);

int read_write_card_blocking_mode(struct mmc_host *host, uint32_t arg, uint8_t *buf);
int read_write_card_dma_mode(struct mmc_host *host, uint32_t arg, uint8_t *buf);
int read_write_card_stuff_blocking_mode(struct mmc_host *host, uint32_t stuff_bits, uint8_t read1_write0, uint8_t *buf);
int read_write_card_stuff_dma_mode(struct mmc_host *host, uint32_t stuff_bits, uint8_t read1_write0, uint8_t *buf);
int send_custom_data(struct mmc_host *host, uint32_t data);

int switch_card(struct mmc_host *host, unsigned int type, unsigned int index, unsigned int value, unsigned int cmdset);
int erase_card(struct mmc_host *host, uintptr_t start_blk, uintptr_t blk_cnt);
int sleep_card(struct mmc_host *host);
int awake_card(struct mmc_host *host);
int select_card(struct mmc_host *host);
int deselect_card(struct mmc_host *host);
int get_card_info(struct mmc_card_info *pcard_info);
int get_card_status(struct mmc_host *host);
int card_cache_control(struct mmc_host *host, uint8_t enable_disable);
int card_cache_flush(struct mmc_host *host);

void card_controller_clk_continue(struct mmc_host *host);
void card_controller_clk_pause(struct mmc_host *host);

#ifdef __cplusplus
}
#endif

#endif
