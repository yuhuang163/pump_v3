#ifndef __PROJECT_DEFAULT_PARAM_H__
#define __PROJECT_DEFAULT_PARAM_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/

#ifndef BREAST_PUMP_HEAD_UI_START_DATE
#define BREAST_PUMP_HEAD_UI_START_DATE                                       (85)
#endif

#ifndef BREAST_PUMP_HEAD_UI_MAX_DAYS
#define BREAST_PUMP_HEAD_UI_MAX_DAYS                                         (90)
#endif

#ifndef BREAST_PUMP_HEAD_REMAIN_DAYS
#define BREAST_PUMP_HEAD_REMAIN_DAYS                                         (BREAST_PUMP_HEAD_UI_MAX_DAYS - BREAST_PUMP_HEAD_UI_START_DATE)
#endif

#ifdef __cplusplus
}
#endif

#endif 


