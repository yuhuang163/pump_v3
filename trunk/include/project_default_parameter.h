#ifndef __PROJECT_DEFAULT_PARAM_H__
#define __PROJECT_DEFAULT_PARAM_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/

#ifndef BREATH_PUMP_HEAD_UI_START_DATE
#define BREATH_PUMP_HEAD_UI_START_DATE                                       (85)
#endif

#ifndef BREATH_PUMP_HEAD_UI_MAX_DAYS
#define BREATH_PUMP_HEAD_UI_MAX_DAYS                                         (90)
#endif

#ifndef BREATH_PUMP_HEAD_REMAIN_DAYS
#define BREATH_PUMP_HEAD_REMAIN_DAYS                                         (BREATH_PUMP_HEAD_UI_MAX_DAYS - BREATH_PUMP_HEAD_UI_START_DATE)
#endif

#ifdef __cplusplus
}
#endif

#endif 


