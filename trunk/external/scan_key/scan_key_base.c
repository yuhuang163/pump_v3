

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "scan_key_base.h"
#include "scan_key.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define DE_SK_EVENT_CB(key, code_index)              ((scan_key_data_t *)key)->cb(((scan_key_data_t *)key), code_index)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void v_scan_key(scan_key_ctrl_data_t *key_ctrl, scan_key_data_t *key_data, uint8_t code_size)
{
    for (uint8_t i = 0;  i < code_size; i++) {
        if (key_data[i].status) {
            key_data[i].scan_count++;
        }
        // printf("key %d status:%d, event:%d, count:%d, triger:%d, hold:%d, reset:%d\n", i, key_data[i].status, key_data[i].event, 
        //         key_data[i].scan_count, key_ctrl[i].triger, key_ctrl[i].hold, key_ctrl[i].reset);
        switch (key_data[i].status) {
            case DE_SK_STATUS_START: /*开始状态*/
            {
                if (key_ctrl[i].triger) {
                    /*对应按键动作按下*/
                    key_data[i].debounce_tick = 1; /*开始抖动过滤检测*/
                    key_data[i].event = DE_SK_EVENT_DOWN; /*按下事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (key_ctrl[i].hold) {
                    /*对应按键保持按下*/
                    key_data[i].debounce_tick++;
                    if (key_data[i].debounce_tick >= DE_SK_DEBOUNCE_TICK)  {
                        /*通过抖动验证*/
                        key_data[i].scan_count = key_data[i].debounce_tick;
                        key_data[i].debounce_tick = 0;       /*方便确认下次有没有经过抖动验证*/
                        key_data[i].status = DE_SK_STATUS_DOWN;
                        key_data[i].event = DE_SK_EVENT_DOWN_YES;   /*确认按下0事件*/
                        DE_SK_EVENT_CB(&key_data[i], i);
                    }
                    /*按键是按下的,但没有通过抖动事件,继续抖动验证*/
                } else if (key_data[i].debounce_tick) {
                    /*按键没有按下,且经过抖动验证,说明没通过按键按键抖动验证*/
                    key_data[i].debounce_tick = 0;
                    key_data[i].event = DE_SK_EVENT_DOWN_NO;/*按键没有按下,抖动验证不通过事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                }
                /*按键是初始状态,一直没动*/
            }
            break;

            case DE_SK_STATUS_DOWN:
            {
                if (key_ctrl[i].reset) {
                    /*对应按键动作抬起,复位*/
                    key_data[i].debounce_tick = 1;/*开始抖动过滤检测*/
                    key_data[i].event = DE_SK_EVENT_UP;/*抬起事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (!key_ctrl[i].hold) {
                    /*对应按键保持抬起*/
                    key_data[i].debounce_tick++;
                    if (key_data[i].debounce_tick >= DE_SK_DEBOUNCE_TICK) {
                        key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                        key_data[i].double_start_tick = key_data[i].scan_count;/*用于双击抬起的时间判断*/
                        key_data[i].status = DE_SK_STATUS_UP;
                        key_data[i].event = DE_SK_EVENT_UP_YES;/*确认抬起0事件*/
                        DE_SK_EVENT_CB(&key_data[i], i);
                    }
                    /*按键是抬起的,但没有通过抖动事件,继续抖动验证*/
                } else if (key_data[i].debounce_tick) {
                    /*按键按下,且经过抖动验证,说明没通过按键按键抖动验证*/
                    key_data[i].debounce_tick = 0;
                    key_data[i].event = DE_SK_EVENT_UP_NO;/*按键没有抬起,抖动过滤成功事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (key_data[i].scan_count >= DE_SK_LONG_START_TICK) {
                    /*按键是一直按下的,且超过长按值*/
                    key_data[i].status = DE_SK_STATUS_DOWN_LONG;
                    key_data[i].event = DE_SK_EVENT_DOWN_LONG_YES;/*确认按下L事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                }
                /*按键是一直按下的,但没有超过长按值*/
            }
            break;

            case DE_SK_STATUS_UP:
            {
                if (key_ctrl[i].triger) { 
                    /*对应按键触发按下*/
                    key_data[i].debounce_tick = 1;/*开始抖动过滤检测*/
                    key_data[i].event = DE_SK_EVENT_DOWN_AGAIN;/*按下事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (key_ctrl[i].hold) {
                    /*对应按键保持按下*/
                    key_data[i].debounce_tick++;
                    if (key_data[i].debounce_tick >= DE_SK_DEBOUNCE_TICK) {
                        /*通过抖动验证*/
                        key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                        key_data[i].status = DE_SK_STATUS_DOWN_AGAIN;
                        key_data[i].event = DE_SK_EVENT_DOWN_AGAIN_YES;/*确认按下0事件*/
                        DE_SK_EVENT_CB(&key_data[i], i);
                    }
                } else if (key_data[i].debounce_tick) {
                    /*按键抬起,且经过抖动验证没有通过*/
                    key_data[i].debounce_tick = 0;
                    key_data[i].event = DE_SK_EVENT_DOWN_NO;/*按键没有按下,抖动过滤成功事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if ((key_data[i].scan_count - key_data[i].double_start_tick) >= DE_SK_INTERVEL_TICK) {
                    /*按键是抬起的,一直没动*/
                    key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                    key_data[i].status = DE_SK_STATUS_START;
                    key_data[i].event = DE_SK_EVENT_DOUBLE_NO;/*确认非双击事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                }
                /*按键是抬起的,一直没动,且抬起时间小于ClickTick*/
            }
            break;

            case DE_SK_STATUS_DOWN_AGAIN:
            {
                if (key_ctrl[i].reset) {
                    /*对应按键触发抬起,复位*/
                    key_data[i].debounce_tick = 1;/*开始抖动过滤检测*/
                    key_data[i].event = DE_SK_EVENT_UP_AGAIN;/*抬起事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (!key_ctrl[i].hold) {
                    /*对应按键是抬起的*/
                    key_data[i].debounce_tick++;
                    if (key_data[i].debounce_tick >= DE_SK_DEBOUNCE_TICK) {
                        /*通过抖动验证*/
                        key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                        key_data[i].status = DE_SK_STATUS_START/*SKStatusUp1Yes*/;
                        key_data[i].event = DE_SK_EVENT_UP_AGAIN_YES;/*确认抬起1事件*/
                        DE_SK_EVENT_CB(&key_data[i], i);
                    }
                    /*按键是抬起的,但没有通过抖动事件,继续抖动验证*/
                } else if (key_data[i].debounce_tick) {
                    /*按键按下,且经过抖动验证没有通过*/
                    key_data[i].debounce_tick = 0;
                    key_data[i].event = DE_SK_EVENT_UP_AGAIN_NO;/*按键没有抬起,抖动过滤成功事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                }
                /*按键是初始状态,一直没动*/
            }
            break;

            case DE_SK_STATUS_DOWN_LONG: /*长按确认*/
            {
                if (key_ctrl[i].reset) {
                    /*对应按键触发抬起,复位*/
                    key_data[i].debounce_tick = 1;/*开始抖动过滤检测*/
                    key_data[i].event = DE_SK_EVENT_UP_LONG;/*抬起事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (!key_ctrl[i].hold) {
                    /*对应按键是抬起的*/
                    key_data[i].debounce_tick++;
                    if (key_data[i].debounce_tick >= DE_SK_DEBOUNCE_TICK) {
                        /*通过抖动验证*/
                        key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                        key_data[i].status = DE_SK_STATUS_START/*SKStatusUp1Yes*/;
                        key_data[i].event = DE_SK_EVENT_UP_LONG_YES;/*确认抬起0事件*/
                        DE_SK_EVENT_CB(&key_data[i], i);
                    }
                    /*按键是抬起的,但没有通过抖动事件,继续抖动验证*/
                } else if (key_data[i].debounce_tick) {
                    /*按键按下,且经过抖动验证没有通过*/
                    key_data[i].debounce_tick = 0;
                    key_data[i].event = DE_SK_EVENT_UP_LONG_NO;/*按键没有抬起,抖动过滤成功事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (key_data[i].scan_count >= DE_SK_CONTINUE_START_TICK) {
                    /*按键是一直按下的,且超过长按值*/
                    // key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                    key_data[i].double_start_tick = key_data[i].scan_count;/*用于双击抬起的时间判断*/
                    key_data[i].status = DE_SK_STATUS_DOWN_CONTINUE;
                    key_data[i].event = DE_SK_EVENT_DOWN_CONTINUE_YES;/*确认按下L事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                }
                /*按键是一直按下的,但没到连击起始值*/
            }
            break;

            case DE_SK_STATUS_DOWN_CONTINUE: /*连击确认*/
            {
                if (key_ctrl[i].reset) {
                    /*对应按键触发抬起,复位*/
                    key_data[i].debounce_tick = 1;/*开始抖动过滤检测*/
                    key_data[i].event = DE_SK_EVENT_UP_CONTINUE;/*抬起事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if (!key_ctrl[i].hold) {
                    /*对应按键是抬起的*/
                    key_data[i].debounce_tick++;
                    if (key_data[i].debounce_tick >= DE_SK_DEBOUNCE_TICK) {
                        /*通过抖动验证*/
                        key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                        key_data[i].status = DE_SK_STATUS_START/*SKStatusUp1Yes*/;
                        key_data[i].event = DE_SK_EVENT_UP_CONTINUE_YES;/*确认抬起0事件*/
                        DE_SK_EVENT_CB(&key_data[i], i);
                    }
                    /*按键是抬起的,但没有通过抖动事件,继续抖动验证*/
                } else if (key_data[i].debounce_tick) {
                    /*按键按下,且经过抖动验证没有通过*/
                    key_data[i].debounce_tick = 0;
                    key_data[i].event = DE_SK_EVENT_UP_CONTINUE_NO;/*按键没有抬起,抖动过滤成功事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                } else if ((key_data[i].scan_count - key_data[i].double_start_tick) >= DE_SK_CONTINUE_TICK) {
                    /*按键是一直按下的,且超连击值*/
                    // key_data[i].debounce_tick = 0;/*方便确认下次有没有经过抖动验证*/
                    key_data[i].double_start_tick = key_data[i].scan_count;/*用于双击抬起的时间判断*/
                    // key_data[i].status = DE_SK_STATUS_DOWN_CONTINUE;
                    key_data[i].event = DE_SK_EVENT_DOWN_CONTINUE_YES;/*确认按下L事件*/
                    DE_SK_EVENT_CB(&key_data[i], i);
                }
                /*按键是一直按下的,但没到连击起始值*/
            }
            break;
        }
    }
}
