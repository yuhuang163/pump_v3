#ifndef __BSP_CHARGE_PD_H__
#define __BSP_CHARGE_PD_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "bsp_internal.h" // IWYU pragma: keep

/**@brief Firmware information define*/

/// PD状态寄存器
#define CHARGE_PD_STATUS_REG                     0x09
/// EPR激活状态
#define EPR_ACTIVE_STATUS_POS                    4
typedef enum {
    EPR_ACTIVE_STATUS_DISABLE = 0x00 << EPR_ACTIVE_STATUS_POS, /**< 关闭 */
    EPR_ACTIVE_STATUS_ENABLE  = 0x01 << EPR_ACTIVE_STATUS_POS, /**< 开启 */
} epr_active_status_e;

/// PD激活状态
#define PD_ACTIVE_STATUS_POS                     3
typedef enum {
    PD_ACTIVE_STATUS_DISABLE = 0x00 << PD_ACTIVE_STATUS_POS, /**< 关闭 */
    PD_ACTIVE_STATUS_ENABLE  = 0x01 << PD_ACTIVE_STATUS_POS, /**< 开启 */
} pd_active_status_e;

/// QC3.0激活状态
#define QC3_0_ACTIVE_STATUS_POS                  2
typedef enum {
    QC3_0_ACTIVE_STATUS_DISABLE = 0x00 << QC3_0_ACTIVE_STATUS_POS, /**< 关闭 */
    QC3_0_ACTIVE_STATUS_ENABLE  = 0x01 << QC3_0_ACTIVE_STATUS_POS, /**< 开启 */
} qc3_0_active_status_e;

/// QC2.0激活状态
#define QC2_0_ACTIVE_STATUS_POS                  1
typedef enum {
    QC2_0_ACTIVE_STATUS_DISABLE = 0x00 << QC2_0_ACTIVE_STATUS_POS, /**< 关闭 */
    QC2_0_ACTIVE_STATUS_ENABLE  = 0x01 << QC2_0_ACTIVE_STATUS_POS, /**< 开启 */
} qc2_0_active_status_e;

/// BC激活状态
#define BC_ACTIVE_STATUS_POS                     0
typedef enum {
    BC_ACTIVE_STATUS_DISABLE = 0x00 << BC_ACTIVE_STATUS_POS, /**< 关闭 */
    BC_ACTIVE_STATUS_ENABLE  = 0x01 << BC_ACTIVE_STATUS_POS, /**< 开启 */
} bc_active_status_e;

/// PD电压选择寄存器
#define CHARGE_PD_VOLTAGE_REG                    0x0A
typedef enum {
    CHARGE_PD_VOLTAGE_5V   = 1, /**< 5V */
    CHARGE_PD_VOLTAGE_9V   = 2, /**< 9V */
    CHARGE_PD_VOLTAGE_12V  = 3, /**< 12V */
    CHARGE_PD_VOLTAGE_15V  = 4, /**< 15V */
    CHARGE_PD_VOLTAGE_20V  = 5, /**< 20V */
    CHARGE_PD_VOLTAGE_28V  = 6, /**< 28V */
    CHARGE_PD_PPS_MODE     = 7, /**< PPS Mode */
    CHARGE_PD_AVS_MODE     = 8, /**< AVS Mode */
} charge_pd_voltage_e;

/// PD电流数据寄存器
#define CHARGE_PD_OUTPUT_CURRENT_REG            0x50

/// AVS电压数据寄存器
#define CHARGE_PD_AVS_VOL_H_REG                 0x51
#define CHARGE_PD_AVS_VOL_L_REG                 0x52

/// PPS电压数据寄存器
#define CHARGE_PD_PPS_VOLTAGE_REG               0x53

/// PD电源数据寄存器 0x60-0x8F
#define CHARGE_PD_POWER_DATA_REG                0x60

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
void bsp_charge_pd_init(void);
void charge_pd_status_poll(void);
int bsp_charge_pd_is_ready(void);
int charge_pd_current_get(void);
int bsp_charge_pd_is_mismatch(void);
#ifdef __cplusplus
}
#endif

#endif 
