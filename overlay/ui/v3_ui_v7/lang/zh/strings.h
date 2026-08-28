#ifndef __UI_LANG_ZH_STRINGS_H__
#define __UI_LANG_ZH_STRINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Shared
 ******************************************************************************/
#define UI_STR_MODE_STIMULATION                         "刺激模式"
#define UI_STR_MODE_EXPRESSION                          "吸乳模式"
#define UI_STR_MODE_MIXED                               "混合模式"
#define UI_STR_MODE_MILK_BOOST                          "奶量提升"
#define UI_STR_MODE_POWER_PUMPING                       "追奶模式"
#define UI_STR_MODE_RELAX                               "放松"
#define UI_STR_SIDE_L                                   "L"
#define UI_STR_SIDE_R                                   "R"
#define UI_STR_SPEED                                    "吸奶频率"
#define UI_STR_SWITCH_TO_FMT                            "切换至%s"
#define UI_STR_DURATION_APPROX_MIN_FMT                  "约%d分钟"
#define UI_STR_COUNTDOWN_SEC_FMT                        "%u秒"
#define UI_STR_LEVEL_OVER_LIMIT                         "为了安心使用\n每次最多调节3档"

/*******************************************************************************
 * Home
 ******************************************************************************/
#define HOME_MAIN_HINT_TEXT                             "按下旋钮启动本次吸乳"
#define HOME_SAVER_HINT_WAKE_TEXT                       "点击任意处唤醒"
#define HOME_SAVER_HINT_UNLOCK_TEXT                     "按下旋钮解锁"
#define HOME_SAVER_HINT_TEXT                            HOME_SAVER_HINT_WAKE_TEXT
#define HOME_LEVEL_DIR_LEFT_TEXT                        "左侧力度"
#define HOME_LEVEL_DIR_RIGHT_TEXT                       "右侧力度"
#define HOME_LEVEL_OVER_LIMIT_TEXT                      UI_STR_LEVEL_OVER_LIMIT
#define HOME_LEVEL_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define HOME_BOTH_LEVEL_DIR_LEFT_TEXT                   UI_STR_SIDE_L
#define HOME_BOTH_LEVEL_DIR_RIGHT_TEXT                  UI_STR_SIDE_R
#define HOME_BOTH_LEVEL_OVER_LIMIT_TEXT                 UI_STR_LEVEL_OVER_LIMIT
#define HOME_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT            UI_STR_SWITCH_TO_FMT
#define HOME_SPEED_LABEL_TEXT                           UI_STR_SPEED
#define HOME_SPEED_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT

/*******************************************************************************
 * Pump home / adjust
 ******************************************************************************/
#define PUMP_HOME_MAIN_HINT_TEXT                        "按下继续"
#define PUMP_HOME_MAIN_MODE_SWITCH_TEXT_FMT \
    "已切换至%s。\n力度和速度可能已改变。"
#define PUMP_LEVEL_DIR_LEFT_TEXT                        "左侧力度"
#define PUMP_LEVEL_DIR_RIGHT_TEXT                       "右侧力度"
#define PUMP_LEVEL_OVER_LIMIT_TEXT                      UI_STR_LEVEL_OVER_LIMIT
#define PUMP_LEVEL_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define PUMP_BOTH_LEVEL_DIR_LEFT_TEXT                   UI_STR_SIDE_L
#define PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT                  UI_STR_SIDE_R
#define PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT                 UI_STR_LEVEL_OVER_LIMIT
#define PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT            UI_STR_SWITCH_TO_FMT
#define PUMP_SPEED_LABEL_TEXT                           UI_STR_SPEED
#define PUMP_SPEED_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define PUMP_STOP_HOLD_TITLE_TEXT                       "长按结束吸乳"
#define PUMP_PAUSE_TIMEOUT_TITLE1_TEXT                  "吸乳已结束"
#define PUMP_PAUSE_TIMEOUT_TITLE2_TEXT                  "即将自动结束..."
#define PUMP_FINISH_TIMEOUT_TITLE1_TEXT                 "吸乳已结束"
#define PUMP_FINISH_TIMEOUT_TITLE2_TEXT                 "即将自动结束..."
#define PUMP_WILL_FINISH_TITLE1_TEXT                    "当前程序即将结束"
#define PUMP_WILL_FINISH_TITLE2_TEXT                    "按下旋钮继续吸乳"
#define PUMP_FINISH_TITLE_TEXT                          "吸乳完成"
#define PUMP_FINISH_CONTENT_TEXT                        "打开 Momcozy App\n记录本次出奶量"
#define PUMP_FINISH_CHARGE_ERROR_TITLE1_TEXT            "充电器不兼容"
#define PUMP_FINISH_CHARGE_ERROR_TITLE2_TEXT            "即将自动结束"
#define PUMP_RUNING_30M_TIP_TEXT                        "已吸乳超过30分钟\n请稍作休息"
#define PUMP_REST_MODE_ADJUST_TIP_TEXT                  "休息模式中\n无法调节"

/*******************************************************************************
 * Rhythm / settings
 ******************************************************************************/
#define RHYTHM_CONFIRM_TITLE_TEXT                       "切换程序？"
#define RHYTHM_CONFIRM_SUBTITLE_TEXT                    "计时将重新开始"
#define SETTINGS_SUCCESS_TITLE_TEXT                     "设置成功"
#define SETTINGS_SUCCESS_PROGRAM_TITLE_TEXT             "程序已切换"
#define SETTINGS_SUCCESS_PROGRAM_SUBTITLE_TEXT          "计时已重置"

/*******************************************************************************
 * Power / lock / airplane
 ******************************************************************************/
#define POWER_OFF_HOLD_TITLE_TEXT                       "长按关机"
#define POWER_OFF_TEXT_POWERING_OFF_STR                 "正在关机..."
#define POWER_OFF_TEXT_AUTO_SHUT_OFF_STR                "即将自动关机..."
#define RESTARTING_TEXT                                 "正在重启..."
#define POWER_ON_HOLD_LINE1_TEXT                        "长按"
#define POWER_ON_HOLD_LINE2_TEXT                        "#E2637E 2秒# 开机"
#define AIRPLANE_MODE_TITLE_TEXT                        "设备已锁定"
#define AIRPLANE_MODE_HINT_TEXT                         "长按5秒解锁"
#define TRAVEL_LOCK_TITLE_TEXT                          "长按5秒锁定设备"
#define LOW_POWER_POPUP_TITLE_TEXT                      "即将关机"

/*******************************************************************************
 * Charge / OTA / factory
 ******************************************************************************/
#define CHARGE_ADAPTER_TITLE_TEXT                       "充电器不兼容"
#define CHARGE_ADAPTER_SUBTITLE_TEXT                    "请拔下电源"
#define OTA_RES_UPGRADING_TEXT                          "资源升级中..."
#define OTA_FW_UPGRADING_TEXT                           "固件升级中..."
#define OTA_HW_UPGRADING_TEXT                           "硬件升级中..."
#define OTA_UPGRADE_FAILED_TEXT                         "升级失败"
#define OTA_UPGRADE_COMPLETED_TEXT                      "升级完成"
#define FACTORY_RESET_CONFIRM_TITLE_TEXT                "重置设置"
#define FACTORY_RESET_CONFIRM_HINT_TEXT                 "长按旋钮3秒"
#define FACTORY_RESET_PROGRESS_TEXT                     "正在重置设置…"

/*******************************************************************************
 * System / BLE popup
 ******************************************************************************/
#define SYS_ERROR_TITLE_TEXT                            "系统异常"
#define SYS_ERROR_REPLUG_TEXT                           "重新插拔电源后重启"
#define TEMP_ERROR_TITLE_TEXT                           "电池温度过高"
#define TEMP_ERROR_CONTENT_TEXT                         "设备已暂停"
#define BLE_CONNECT_TEXT                                "设备已连接"
#define BLE_DISCONNECT_TEXT                             "设备已断开"
#define BLE_CONNECTING_TEXT                             "正在连接设备…"

/*******************************************************************************
 * Theme guide
 ******************************************************************************/
#define THEME_GUIDE_TITLE_TEXT                          "系统设置"
#define THEME_GUIDE_DESC_SELECT_TEXT                    "旋转旋钮选择风格"
#define THEME_GUIDE_DESC_CONFIRM_TEXT                   "按下          确认"
#define THEME_GUIDE_OPTION_MINIMAL_TEXT                 "极简"
#define THEME_GUIDE_OPTION_IP_TEXT                      "IP"

/*******************************************************************************
 * Guide
 ******************************************************************************/
#define GUIDE_INTRO_TITLE_TEXT                          "准备好了吗?\n开始你的吸乳之旅吧!"
#define GUIDE_QRCODE_DESC_TEXT                          "扫码下载 Momcozy App\n连接设备, 开启贴心吸乳体验"
#define GUIDE_FINISH_QRCODE_TEXT                        "https://support.momcozy.com/product/bpv-3"
#define GUIDE_FINISH_DESC_TEXT                          "新手引导已完成\n可扫码查看更详细的操作视频"
#define GUIDE_MODE_SWITCH_GUIDE_TITLE_TEXT              "转动旋钮\n查看更多选择"
#define GUIDE_PUMP_START_GUIDE_TITLE_TEXT               "按下旋钮启动"
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT               "按下旋钮暂停"
#define GUIDE_PUMP_FINISH_GUIDE_TITLE_TEXT              "长按旋钮结束吸乳"
#define GUIDE_PUMP_HOME_MODE_TITLE_TEXT                 UI_STR_MODE_STIMULATION
#define GUIDE_PUMP_START_MODE_TITLE_TEXT                UI_STR_MODE_STIMULATION
#define GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT                UI_STR_MODE_STIMULATION
#define GUIDE_SPEED_LABEL_TEXT                          UI_STR_SPEED
#define GUIDE_SPEED_GUIDE_PRESS_TEXT                    "按下"
#define GUIDE_SPEED_GUIDE_ADJUST_TEXT                   "找到适合你的吸乳节奏"
#define GUIDE_LEVEL_SIDE_GUIDE_PRESS_TEXT               "按下"
#define GUIDE_LEVEL_SIDE_GUIDE_OR_TEXT                  "或"
#define GUIDE_LEVEL_SIDE_GUIDE_ADJUST_TEXT              "进入对应侧吸力调节"
#define GUIDE_LEVEL_BOTH_GUIDE_PRESS_TEXT               "同时按下"
#define GUIDE_LEVEL_BOTH_GUIDE_AND_TEXT                 "和"
#define GUIDE_LEVEL_BOTH_GUIDE_ADJUST_TEXT              "一起调节两侧吸力"
#define GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT                  "左侧吸力档位"
#define GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT                 "右侧吸力档位"
#define GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT                UI_STR_LEVEL_OVER_LIMIT
#define GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT                  UI_STR_SIDE_L
#define GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT                 UI_STR_SIDE_R
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT                UI_STR_LEVEL_OVER_LIMIT

/*******************************************************************************
 * Product tests
 ******************************************************************************/
#define PRODUCT_TESTS_SCAN_QR_TEXT                      "请扫描二维码\n快速连接设备"
#define PRODUCT_TESTS_BATTERY_FMT                       "电量-%d%%"
#define PRODUCT_TESTS_AGING_TIME_FMT                    "老化-%02lu时-%02lu分-%02lu秒"
#define PRODUCT_TESTS_AGING_COMPLETE_TEXT               "老化完成"
#define PRODUCT_TESTS_AGING_RUNNING_TEXT                "老化中"

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_ZH_STRINGS_H__
