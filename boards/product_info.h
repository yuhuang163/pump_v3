#ifndef __PRODUCT_INFO_H__
#define __PRODUCT_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 产品型号
 * 说明: 为了兼容预处理器条件判断，增加数值宏定义
*/
typedef enum {
    PRODUCT_INFO_V3 = 0,
    PRODUCT_INFO_V3PRO = 1,
} product_info_t;

#define PRODUCT_INFO_V3_VALUE                         0
#define PRODUCT_INFO_V3PRO_VALUE                      1

/**
 * 产品信息
*/
#ifndef ROOT_PRODUCT_ID
#define ROOT_PRODUCT_ID                               PRODUCT_INFO_V3_VALUE
#endif

#if (ROOT_PRODUCT_ID == PRODUCT_INFO_V3PRO_VALUE)
#define DEFAULT_PID_CODE    0x09
#else
#define DEFAULT_PID_CODE    0x08
#endif

#if (ROOT_PRODUCT_ID == PRODUCT_INFO_V3PRO_VALUE)
#define ROOT_FW_NAME                                  "BREAST_PUMP_V3PRO"
#define ROOT_DEVICE_NAME                              "V3PRO"
#else
#define ROOT_FW_NAME                                  "BREAST_PUMP_V3"
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
 * 硬件版本从100-900
*/
#define ROOT_HARDWARE_VERSION                          100
#define ROOT_HW_VERSION_STR                            "100"


/**
 * 芯片平台从10-90
*/
#define ROOT_SOC_PLAT                                  10
#define ROOT_SOC_PLAT_STR                              "10"

/**
 * 正式版本从100-900
*/
#define ROOT_FIRMWARE_VERSION                          104
#define ROOT_FW_VERSION_STR                            "104"

/**
 * 资源版本号定义(major*100 + minor)
 * major版本表示固件支持的资源版本. 
 * minor表示资源差异化的版本.
*/
#if V3_UI_VERSION == V3_UI_V2
#define ROOT_RES_PATH                                  "V2" /// 资源路径
#else
#define ROOT_RES_PATH                                  "V1" /// 资源路径
#endif
#define ROOT_RES_VERSION                               (101) 
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
 * 产品固件编译时间戳
*/
#define PRODUCT_BUILD_TIMESTAMP                        1757400154

#ifdef __cplusplus
}
#endif

#endif

