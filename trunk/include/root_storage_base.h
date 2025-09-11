#ifndef __ROOT_STORAGE_BASE_H__
#define __ROOT_STORAGE_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h>  // IWYU pragma: keep

// 分区名称
#ifndef PARTITION_ALL_IN_ONE_NAME
#define PARTITION_ALL_IN_ONE_NAME                                               "all_in_one"
#define PARTITION_PHY_INIT_NAME                                                 "phy_init"
#define PARTITION_OTA_UPGRADE_NAME                                              "ota_upgrade"
#define PARTITION_USERDATA_NAME                                                 "userdata"
#define PARTITION_FDS_NAME                                                      "littleFs"
#define PARTITION_DEV_INFO_NAME                                               "dev_info"
#endif

// 文件系统分区定义.
#ifndef FS_PARTTION_USERDATA
#define FS_PARTTION_USERDATA                                                  "userdata"
#define FS_PATH_ROOT                                                          "/data"
#define FS_PATH_RES                                                           FS_PATH_ROOT"/res"
#define FS_PATH_AUDIO                                                         FS_PATH_ROOT"/audio"
#define FS_PATH_CONFIG                                                        FS_PATH_ROOT"/config"
#define FS_PATH_GIFT                                                          FS_PATH_ROOT"/gift"
#define FS_PATH_THEMES                                                        FS_PATH_ROOT"/themes"
#define FS_PATH_TMP                                                           FS_PATH_ROOT"/tmp"
#define FS_PATH_MUSIC                                                         FS_PATH_ROOT"/music"
#endif

// store magic
#ifndef STORAGE_DATA_MAGIC_NUM
#define STORAGE_DATA_MAGIC_NUM                                                 (0xAFFA5880)
#endif

#ifndef STORAGE_MAC_ADDR_MAGIC_NUM
#define STORAGE_MAC_ADDR_MAGIC_NUM                                             (0xAFFA5881)
#endif

#ifndef STORAGE_FAC_FLAG_MAGIC_NUM
#define STORAGE_FAC_FLAG_MAGIC_NUM                                             (0xAFFA5882)
#endif

#ifndef STORAGE_IMU_CALIB_MAGIC_NUM
#define STORAGE_IMU_CALIB_MAGIC_NUM                                            (0xAFFA5883)
#endif

#ifndef STORAGE_FAC_DEVINFO_MAGIC_NUM
#define STORAGE_FAC_DEVINFO_MAGIC_NUM                                          (0xAFFA5885)
#endif

#ifndef STORAGE_SYS_INFO_MAGIC_NUM
#define STORAGE_SYS_INFO_MAGIC_NUM                                             (0xAFFA5886)
#endif

#ifndef RES_SOUND_MAGIC_NUM
#define RES_SOUND_MAGIC_NUM                                                    (0xAFFA5888)
#endif

#ifndef STORAGE_SYS_CONFIG_MAGIC_NUM
#define STORAGE_SYS_CONFIG_MAGIC_NUM                                           (0xAFFA588B)
#endif

#ifndef STORAGE_SYS_STATE_MAGIC_NUM
#define STORAGE_SYS_STATE_MAGIC_NUM                                            (0xAFFA588C)
#endif

#ifndef STORAGE_BREATH_PUMP_MAGIC_NUM
#define STORAGE_BREATH_PUMP_MAGIC_NUM                                      (0xAFFA5884)
#endif

#ifndef STORAGE_USER_BIND_MAGIC_NUM
#define STORAGE_USER_BIND_MAGIC_NUM                                            (0xAFFA588D)
#endif

#ifndef STORAGE_AUDIO_PACKET_MAGIC_NUM
#define STORAGE_AUDIO_PACKET_MAGIC_NUM                                         (0xAFFA5889)
#endif

#ifndef STORAGE_THEME_PACKET_MAGIC_NUM
#define STORAGE_THEME_PACKET_MAGIC_NUM                                         (0xAFFA588A)
#endif

#ifndef SYS_INFO_JSON_HEAD_MAGIC
#define SYS_INFO_JSON_HEAD_MAGIC                                               (0xAFFA5887)
#endif

#ifndef STORAGE_AUDIO_THEME_MAGIC_NUM
#define STORAGE_AUDIO_THEME_MAGIC_NUM                                          (0xAFFA5888)
#endif

#ifndef STORAGE_BREATH_PUMP_DATA_HEAD_MAGIC_NUM
#define STORAGE_BREATH_PUMP_DATA_HEAD_MAGIC_NUM                                (0xAFFA588E)
#endif

#ifndef STORAGE_BREATH_PUMP_DATA_BODY_MAGIC_NUM
#define STORAGE_BREATH_PUMP_DATA_BODY_MAGIC_NUM                                (0xAFFA588F)
#endif

#ifndef STORAGE_UTC_TIMESTAMP_MAGIC_NUM
#define STORAGE_UTC_TIMESTAMP_MAGIC_NUM                                        (0xAFFA5890)
#endif

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */


#ifdef __cplusplus
}
#endif

#endif 


