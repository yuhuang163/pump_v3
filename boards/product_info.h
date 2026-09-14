#ifndef __PRODUCT_INFO_H__
#define __PRODUCT_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define ROOT_DEVICE_NAME                              "V3Pro"
#else
#define ROOT_DEVICE_NAME                              "V3"
#endif

/*
* 主固件版本类型:
* 'P' 正式版本
* 'D' 开发版本
* 'T' 测试版本
* 'C' 定制版本
*/
#define ROOT_FIRMWARE_TYPE                            'D'

/*
* 蓝牙协议版本号
*/
#define ROOT_BLE_PROTOCOL_VERSION                      100
#define ROOT_OTA_UPGRADE_TYPE                          4

/**
 * 硬件版本从1-999
*/
#define ROOT_HARDWARE_VERSION                          4
#define ROOT_HW_VERSION_STR                            "04"


/**
 * 芯片平台从10-90
*/
#define ROOT_SOC_PLAT                                  10
#define ROOT_SOC_PLAT_STR                              "10"

/**
 * 正式版本从100-900
*/
#if (ROOT_PRODUCT_ID == ROOT_PRODUCT_V3PRO_ID)
#define ROOT_FIRMWARE_VERSION                          316
#define ROOT_FW_VERSION_STR                            "316"
#else
#define ROOT_FIRMWARE_VERSION                          402
#define ROOT_FW_VERSION_STR                            "402"
#endif

/**
 * 资源版本号定义(major*100 + minor)
 * major版本表示固件支持的资源版本. 
 * minor表示资源差异化的版本.
*/
#if (ROOT_UI_VERSION == ROOT_UI_V2)
#define ROOT_RES_PATH                                  "V2" /// 资源路径
#define ROOT_RES_VERSION                               (103)
#elif (ROOT_UI_VERSION == ROOT_UI_V3)   
#define ROOT_RES_PATH                                  "V3" /// 资源路径
#define ROOT_RES_VERSION                               (104)
#elif (ROOT_UI_VERSION == ROOT_UI_V4)
#define ROOT_RES_PATH                                  "V4" /// 资源路径
#define ROOT_RES_VERSION                               (105)
#elif (ROOT_UI_VERSION == ROOT_UI_V5)
#define ROOT_RES_PATH                                  "V5" /// 资源路径
#define ROOT_RES_VERSION                               (105)
#elif (ROOT_UI_VERSION == ROOT_UI_V6)
#define ROOT_RES_PATH                                  "V6" /// 资源路径
#define ROOT_RES_VERSION                               (106)
#elif (ROOT_UI_VERSION == ROOT_UI_V7)
#define ROOT_RES_PATH                                  "V7" /// 资源路径
#define ROOT_RES_VERSION                               (107)
#else
#define ROOT_RES_PATH                                  "V4" /// 资源路径
#define ROOT_RES_VERSION                               (105)
#endif

#define ROOT_RES_MAJOR_VERSION                         (ROOT_RES_VERSION / 100)
#define ROOT_RES_MINOR_VERSION                         (ROOT_RES_VERSION % 100)

/**
 * 算法采测版本号:
 * R分支,初始版本从100定义,与主版本号无直接联系
*/
#define ALGO_TEST_FIRMWARE_TYPE                        'R'
#define ALGO_TEST_FIRMWARE_VERSION                     100

/**
 * 产品外观色
*/
#define PRODUCT_COLOR_BLACK                            "00"
#define PRODUCT_COLOR_GOLDEN                           "01"
#define PRODUCT_COLOR_BLUE                             "02"
#define PRODUCT_SUB_ID_INDEX                           48

/**
 * 产品语言（编译期默认语 / 无外部语言包时的回退）
 * Makefile: make PRODUCT_LANGUAGE=en|zh
 * 取值必须与 sys_language_e 前两项一致：EN=0, ZH=1
 * 未从命令行传入时默认英文，保证 IDE / 单独编译与现网一致
*/
#define PRODUCT_LANGUAGE_EN                             0x00
#define PRODUCT_LANGUAGE_ZH                             0x01
#ifndef PRODUCT_LANGUAGE_USED
#define PRODUCT_LANGUAGE_USED                           PRODUCT_LANGUAGE_EN
#endif

/**
 * 产品固件编译时间戳
*/
#define PRODUCT_BUILD_TIMESTAMP                        1757400154

#ifdef __cplusplus
}
#endif

#endif

