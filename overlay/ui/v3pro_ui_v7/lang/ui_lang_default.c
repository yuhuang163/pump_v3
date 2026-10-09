/* 固件默认语言表（无外部包或包失效时使用）。由 gen_ui_lang_tables.py 生成，不要手改。
 * 数值来自当前 PRODUCT_LANGUAGE 的 strings.h / layout.h / fonts.h。 */
#include "product_info.h"
#include "ui_lang_runtime.h"
#include "lv_user_font.h"

#if (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_EN)
#include "en/strings.h"
#include "en/layout.h"
#include "en/fonts.h"
#elif (PRODUCT_LANGUAGE_USED == PRODUCT_LANGUAGE_ZH)
#include "zh/strings.h"
#include "zh/layout.h"
#include "zh/fonts.h"
#else
#error "Unsupported PRODUCT_LANGUAGE_USED"
#endif

const char *const g_ui_str_default[UI_STR_ID_COUNT] = {
#ifdef UI_STR_MODE_STIMULATION
    [UI_STR_ID_MODE_STIMULATION] = UI_STR_MODE_STIMULATION,
#endif
#ifdef UI_STR_MODE_EXPRESSION
    [UI_STR_ID_MODE_EXPRESSION] = UI_STR_MODE_EXPRESSION,
#endif
#ifdef UI_STR_MODE_MIXED
    [UI_STR_ID_MODE_MIXED] = UI_STR_MODE_MIXED,
#endif
#ifdef UI_STR_MODE_MILK_BOOST
    [UI_STR_ID_MODE_MILK_BOOST] = UI_STR_MODE_MILK_BOOST,
#endif
#ifdef UI_STR_MODE_POWER_PUMPING
    [UI_STR_ID_MODE_POWER_PUMPING] = UI_STR_MODE_POWER_PUMPING,
#endif
#ifdef UI_STR_MODE_CUSTOM
    [UI_STR_ID_MODE_CUSTOM] = UI_STR_MODE_CUSTOM,
#endif
#ifdef UI_STR_MODE_COZY_FLOW
    [UI_STR_ID_MODE_COZY_FLOW] = UI_STR_MODE_COZY_FLOW,
#endif
#ifdef UI_STR_MODE_DENSE_MILK
    [UI_STR_ID_MODE_DENSE_MILK] = UI_STR_MODE_DENSE_MILK,
#endif
#ifdef UI_STR_MODE_HIGH_EFFICIENCY
    [UI_STR_ID_MODE_HIGH_EFFICIENCY] = UI_STR_MODE_HIGH_EFFICIENCY,
#endif
#ifdef UI_STR_MODE_LACTATION_STARTER
    [UI_STR_ID_MODE_LACTATION_STARTER] = UI_STR_MODE_LACTATION_STARTER,
#endif
#ifdef UI_STR_MODE_DEEP_EMPTYING
    [UI_STR_ID_MODE_DEEP_EMPTYING] = UI_STR_MODE_DEEP_EMPTYING,
#endif
#ifdef UI_STR_MODE_LET_DOWN_RESPONSE
    [UI_STR_ID_MODE_LET_DOWN_RESPONSE] = UI_STR_MODE_LET_DOWN_RESPONSE,
#endif
#ifdef UI_STR_MODE_BEFORE_BED
    [UI_STR_ID_MODE_BEFORE_BED] = UI_STR_MODE_BEFORE_BED,
#endif
#ifdef UI_STR_MODE_QUICK_REFILL
    [UI_STR_ID_MODE_QUICK_REFILL] = UI_STR_MODE_QUICK_REFILL,
#endif
#ifdef UI_STR_MODE_POST_NURSING
    [UI_STR_ID_MODE_POST_NURSING] = UI_STR_MODE_POST_NURSING,
#endif
#ifdef UI_STR_MODE_RELAX
    [UI_STR_ID_MODE_RELAX] = UI_STR_MODE_RELAX,
#endif
#ifdef UI_STR_SIDE_L
    [UI_STR_ID_SIDE_L] = UI_STR_SIDE_L,
#endif
#ifdef UI_STR_SIDE_R
    [UI_STR_ID_SIDE_R] = UI_STR_SIDE_R,
#endif
#ifdef UI_STR_SPEED
    [UI_STR_ID_SPEED] = UI_STR_SPEED,
#endif
#ifdef UI_STR_SWITCH_TO_FMT
    [UI_STR_ID_SWITCH_TO_FMT] = UI_STR_SWITCH_TO_FMT,
#endif
#ifdef UI_STR_DURATION_APPROX_MIN_FMT
    [UI_STR_ID_DURATION_APPROX_MIN_FMT] = UI_STR_DURATION_APPROX_MIN_FMT,
#endif
#ifdef UI_STR_COUNTDOWN_SEC_FMT
    [UI_STR_ID_COUNTDOWN_SEC_FMT] = UI_STR_COUNTDOWN_SEC_FMT,
#endif
#ifdef UI_STR_LEVEL_OVER_LIMIT
    [UI_STR_ID_LEVEL_OVER_LIMIT] = UI_STR_LEVEL_OVER_LIMIT,
#endif
#ifdef HOME_MAIN_HINT_TEXT
    [UI_STR_ID_HOME_MAIN_HINT_TEXT] = HOME_MAIN_HINT_TEXT,
#endif
#ifdef HOME_MAIN_HINT_TEXT_LOW_BAT
    [UI_STR_ID_HOME_MAIN_HINT_TEXT_LOW_BAT] = HOME_MAIN_HINT_TEXT_LOW_BAT,
#endif
#ifdef HOME_MAIN_HINT_TEXT_FORBID_START
    [UI_STR_ID_HOME_MAIN_HINT_TEXT_FORBID_START] = HOME_MAIN_HINT_TEXT_FORBID_START,
#endif
#ifdef HOME_SAVER_HINT_TEXT
    [UI_STR_ID_HOME_SAVER_HINT_TEXT] = HOME_SAVER_HINT_TEXT,
#endif
#ifdef HOME_LEVEL_DIR_LEFT_TEXT
    [UI_STR_ID_HOME_LEVEL_DIR_LEFT_TEXT] = HOME_LEVEL_DIR_LEFT_TEXT,
#endif
#ifdef HOME_LEVEL_DIR_RIGHT_TEXT
    [UI_STR_ID_HOME_LEVEL_DIR_RIGHT_TEXT] = HOME_LEVEL_DIR_RIGHT_TEXT,
#endif
#ifdef HOME_LEVEL_OVER_LIMIT_TEXT
    [UI_STR_ID_HOME_LEVEL_OVER_LIMIT_TEXT] = HOME_LEVEL_OVER_LIMIT_TEXT,
#endif
#ifdef HOME_LEVEL_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_HOME_LEVEL_MODE_SWITCH_TEXT_FMT] = HOME_LEVEL_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef HOME_BOTH_LEVEL_DIR_LEFT_TEXT
    [UI_STR_ID_HOME_BOTH_LEVEL_DIR_LEFT_TEXT] = HOME_BOTH_LEVEL_DIR_LEFT_TEXT,
#endif
#ifdef HOME_BOTH_LEVEL_DIR_RIGHT_TEXT
    [UI_STR_ID_HOME_BOTH_LEVEL_DIR_RIGHT_TEXT] = HOME_BOTH_LEVEL_DIR_RIGHT_TEXT,
#endif
#ifdef HOME_BOTH_LEVEL_OVER_LIMIT_TEXT
    [UI_STR_ID_HOME_BOTH_LEVEL_OVER_LIMIT_TEXT] = HOME_BOTH_LEVEL_OVER_LIMIT_TEXT,
#endif
#ifdef HOME_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_HOME_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT] = HOME_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef HOME_SPEED_LABEL_TEXT
    [UI_STR_ID_HOME_SPEED_LABEL_TEXT] = HOME_SPEED_LABEL_TEXT,
#endif
#ifdef HOME_SPEED_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_HOME_SPEED_MODE_SWITCH_TEXT_FMT] = HOME_SPEED_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef HOME_LOW_POWER_PAGE_TEXT
    [UI_STR_ID_HOME_LOW_POWER_PAGE_TEXT] = HOME_LOW_POWER_PAGE_TEXT,
#endif
#ifdef PUMP_HOME_MAIN_HINT_TEXT
    [UI_STR_ID_PUMP_HOME_MAIN_HINT_TEXT] = PUMP_HOME_MAIN_HINT_TEXT,
#endif
#ifdef PUMP_HOME_MAIN_SEAL_CHECKING_TEXT
    [UI_STR_ID_PUMP_HOME_MAIN_SEAL_CHECKING_TEXT] = PUMP_HOME_MAIN_SEAL_CHECKING_TEXT,
#endif
#ifdef PUMP_HOME_MAIN_SEAL_RESULT_TEXT
    [UI_STR_ID_PUMP_HOME_MAIN_SEAL_RESULT_TEXT] = PUMP_HOME_MAIN_SEAL_RESULT_TEXT,
#endif
#ifdef PUMP_HOME_MAIN_SEAL_FAIL_TEXT
    [UI_STR_ID_PUMP_HOME_MAIN_SEAL_FAIL_TEXT] = PUMP_HOME_MAIN_SEAL_FAIL_TEXT,
#endif
#ifdef PUMP_HOME_MAIN_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_PUMP_HOME_MAIN_MODE_SWITCH_TEXT_FMT] = PUMP_HOME_MAIN_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef PUMP_HOME_CUSTOM_TIME_H_TEXT
    [UI_STR_ID_PUMP_HOME_CUSTOM_TIME_H_TEXT] = PUMP_HOME_CUSTOM_TIME_H_TEXT,
#endif
#ifdef PUMP_HOME_CUSTOM_TIME_M_TEXT
    [UI_STR_ID_PUMP_HOME_CUSTOM_TIME_M_TEXT] = PUMP_HOME_CUSTOM_TIME_M_TEXT,
#endif
#ifdef PUMP_LEVEL_DIR_LEFT_TEXT
    [UI_STR_ID_PUMP_LEVEL_DIR_LEFT_TEXT] = PUMP_LEVEL_DIR_LEFT_TEXT,
#endif
#ifdef PUMP_LEVEL_DIR_RIGHT_TEXT
    [UI_STR_ID_PUMP_LEVEL_DIR_RIGHT_TEXT] = PUMP_LEVEL_DIR_RIGHT_TEXT,
#endif
#ifdef PUMP_LEVEL_OVER_LIMIT_TEXT
    [UI_STR_ID_PUMP_LEVEL_OVER_LIMIT_TEXT] = PUMP_LEVEL_OVER_LIMIT_TEXT,
#endif
#ifdef PUMP_LEVEL_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_PUMP_LEVEL_MODE_SWITCH_TEXT_FMT] = PUMP_LEVEL_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef PUMP_BOTH_LEVEL_DIR_LEFT_TEXT
    [UI_STR_ID_PUMP_BOTH_LEVEL_DIR_LEFT_TEXT] = PUMP_BOTH_LEVEL_DIR_LEFT_TEXT,
#endif
#ifdef PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT
    [UI_STR_ID_PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT] = PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT,
#endif
#ifdef PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT
    [UI_STR_ID_PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT] = PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT,
#endif
#ifdef PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT] = PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef PUMP_SPEED_LABEL_TEXT
    [UI_STR_ID_PUMP_SPEED_LABEL_TEXT] = PUMP_SPEED_LABEL_TEXT,
#endif
#ifdef PUMP_SPEED_MODE_SWITCH_TEXT_FMT
    [UI_STR_ID_PUMP_SPEED_MODE_SWITCH_TEXT_FMT] = PUMP_SPEED_MODE_SWITCH_TEXT_FMT,
#endif
#ifdef PUMP_STOP_HOLD_TITLE_TEXT
    [UI_STR_ID_PUMP_STOP_HOLD_TITLE_TEXT] = PUMP_STOP_HOLD_TITLE_TEXT,
#endif
#ifdef PUMP_PAUSE_TIMEOUT_TITLE1_TEXT
    [UI_STR_ID_PUMP_PAUSE_TIMEOUT_TITLE1_TEXT] = PUMP_PAUSE_TIMEOUT_TITLE1_TEXT,
#endif
#ifdef PUMP_PAUSE_TIMEOUT_TITLE2_TEXT
    [UI_STR_ID_PUMP_PAUSE_TIMEOUT_TITLE2_TEXT] = PUMP_PAUSE_TIMEOUT_TITLE2_TEXT,
#endif
#ifdef PUMP_FINISH_TIMEOUT_TITLE1_TEXT
    [UI_STR_ID_PUMP_FINISH_TIMEOUT_TITLE1_TEXT] = PUMP_FINISH_TIMEOUT_TITLE1_TEXT,
#endif
#ifdef PUMP_FINISH_TIMEOUT_TITLE2_TEXT
    [UI_STR_ID_PUMP_FINISH_TIMEOUT_TITLE2_TEXT] = PUMP_FINISH_TIMEOUT_TITLE2_TEXT,
#endif
#ifdef PUMP_WILL_FINISH_TITLE1_TEXT
    [UI_STR_ID_PUMP_WILL_FINISH_TITLE1_TEXT] = PUMP_WILL_FINISH_TITLE1_TEXT,
#endif
#ifdef PUMP_WILL_FINISH_TITLE2_TEXT
    [UI_STR_ID_PUMP_WILL_FINISH_TITLE2_TEXT] = PUMP_WILL_FINISH_TITLE2_TEXT,
#endif
#ifdef PUMP_FINISH_TITLE_TEXT
    [UI_STR_ID_PUMP_FINISH_TITLE_TEXT] = PUMP_FINISH_TITLE_TEXT,
#endif
#ifdef PUMP_FINISH_CONTENT_TEXT
    [UI_STR_ID_PUMP_FINISH_CONTENT_TEXT] = PUMP_FINISH_CONTENT_TEXT,
#endif
#ifdef PUMP_FINISH_CHARGE_ERROR_TITLE1_TEXT
    [UI_STR_ID_PUMP_FINISH_CHARGE_ERROR_TITLE1_TEXT] = PUMP_FINISH_CHARGE_ERROR_TITLE1_TEXT,
#endif
#ifdef PUMP_FINISH_CHARGE_ERROR_TITLE2_TEXT
    [UI_STR_ID_PUMP_FINISH_CHARGE_ERROR_TITLE2_TEXT] = PUMP_FINISH_CHARGE_ERROR_TITLE2_TEXT,
#endif
#ifdef PUMP_RUNING_30M_TIP_TEXT
    [UI_STR_ID_PUMP_RUNING_30M_TIP_TEXT] = PUMP_RUNING_30M_TIP_TEXT,
#endif
#ifdef PUMP_REST_MODE_ADJUST_TIP_TEXT
    [UI_STR_ID_PUMP_REST_MODE_ADJUST_TIP_TEXT] = PUMP_REST_MODE_ADJUST_TIP_TEXT,
#endif
#ifdef RHYTHM_CONFIRM_TITLE_TEXT
    [UI_STR_ID_RHYTHM_CONFIRM_TITLE_TEXT] = RHYTHM_CONFIRM_TITLE_TEXT,
#endif
#ifdef RHYTHM_CONFIRM_SUBTITLE_TEXT
    [UI_STR_ID_RHYTHM_CONFIRM_SUBTITLE_TEXT] = RHYTHM_CONFIRM_SUBTITLE_TEXT,
#endif
#ifdef SETTINGS_SUCCESS_TITLE_TEXT
    [UI_STR_ID_SETTINGS_SUCCESS_TITLE_TEXT] = SETTINGS_SUCCESS_TITLE_TEXT,
#endif
#ifdef SETTINGS_SUCCESS_PROGRAM_TITLE_TEXT
    [UI_STR_ID_SETTINGS_SUCCESS_PROGRAM_TITLE_TEXT] = SETTINGS_SUCCESS_PROGRAM_TITLE_TEXT,
#endif
#ifdef SETTINGS_SUCCESS_PROGRAM_SUBTITLE_TEXT
    [UI_STR_ID_SETTINGS_SUCCESS_PROGRAM_SUBTITLE_TEXT] = SETTINGS_SUCCESS_PROGRAM_SUBTITLE_TEXT,
#endif
#ifdef POWER_OFF_HOLD_TITLE_TEXT
    [UI_STR_ID_POWER_OFF_HOLD_TITLE_TEXT] = POWER_OFF_HOLD_TITLE_TEXT,
#endif
#ifdef POWER_OFF_TEXT_POWERING_OFF_STR
    [UI_STR_ID_POWER_OFF_TEXT_POWERING_OFF_STR] = POWER_OFF_TEXT_POWERING_OFF_STR,
#endif
#ifdef POWER_OFF_TEXT_AUTO_SHUT_OFF_STR
    [UI_STR_ID_POWER_OFF_TEXT_AUTO_SHUT_OFF_STR] = POWER_OFF_TEXT_AUTO_SHUT_OFF_STR,
#endif
#ifdef POWER_OFF_TEXT_LOW_POWER_STR
    [UI_STR_ID_POWER_OFF_TEXT_LOW_POWER_STR] = POWER_OFF_TEXT_LOW_POWER_STR,
#endif
#ifdef RESTARTING_TEXT
    [UI_STR_ID_RESTARTING_TEXT] = RESTARTING_TEXT,
#endif
#ifdef POWER_OFF_CHARGE_TEXT
    [UI_STR_ID_POWER_OFF_CHARGE_TEXT] = POWER_OFF_CHARGE_TEXT,
#endif
#ifdef LOW_POWER_PAGE_TEXT
    [UI_STR_ID_LOW_POWER_PAGE_TEXT] = LOW_POWER_PAGE_TEXT,
#endif
#ifdef POWER_ON_HOLD_LINE1_TEXT
    [UI_STR_ID_POWER_ON_HOLD_LINE1_TEXT] = POWER_ON_HOLD_LINE1_TEXT,
#endif
#ifdef POWER_ON_HOLD_LINE2_TEXT
    [UI_STR_ID_POWER_ON_HOLD_LINE2_TEXT] = POWER_ON_HOLD_LINE2_TEXT,
#endif
#ifdef AIRPLANE_MODE_TITLE_TEXT
    [UI_STR_ID_AIRPLANE_MODE_TITLE_TEXT] = AIRPLANE_MODE_TITLE_TEXT,
#endif
#ifdef AIRPLANE_MODE_HINT_TEXT
    [UI_STR_ID_AIRPLANE_MODE_HINT_TEXT] = AIRPLANE_MODE_HINT_TEXT,
#endif
#ifdef TRAVEL_LOCK_TITLE_TEXT
    [UI_STR_ID_TRAVEL_LOCK_TITLE_TEXT] = TRAVEL_LOCK_TITLE_TEXT,
#endif
#ifdef LOW_POWER_POPUP_TITLE_TEXT
    [UI_STR_ID_LOW_POWER_POPUP_TITLE_TEXT] = LOW_POWER_POPUP_TITLE_TEXT,
#endif
#ifdef CHARGE_ADAPTER_TITLE_TEXT
    [UI_STR_ID_CHARGE_ADAPTER_TITLE_TEXT] = CHARGE_ADAPTER_TITLE_TEXT,
#endif
#ifdef CHARGE_ADAPTER_SUBTITLE_TEXT
    [UI_STR_ID_CHARGE_ADAPTER_SUBTITLE_TEXT] = CHARGE_ADAPTER_SUBTITLE_TEXT,
#endif
#ifdef OTA_RES_UPGRADING_TEXT
    [UI_STR_ID_OTA_RES_UPGRADING_TEXT] = OTA_RES_UPGRADING_TEXT,
#endif
#ifdef OTA_FW_UPGRADING_TEXT
    [UI_STR_ID_OTA_FW_UPGRADING_TEXT] = OTA_FW_UPGRADING_TEXT,
#endif
#ifdef OTA_HW_UPGRADING_TEXT
    [UI_STR_ID_OTA_HW_UPGRADING_TEXT] = OTA_HW_UPGRADING_TEXT,
#endif
#ifdef OTA_UPGRADE_FAILED_TEXT
    [UI_STR_ID_OTA_UPGRADE_FAILED_TEXT] = OTA_UPGRADE_FAILED_TEXT,
#endif
#ifdef OTA_UPGRADE_COMPLETED_TEXT
    [UI_STR_ID_OTA_UPGRADE_COMPLETED_TEXT] = OTA_UPGRADE_COMPLETED_TEXT,
#endif
#ifdef FACTORY_RESET_CONFIRM_TITLE_TEXT
    [UI_STR_ID_FACTORY_RESET_CONFIRM_TITLE_TEXT] = FACTORY_RESET_CONFIRM_TITLE_TEXT,
#endif
#ifdef FACTORY_RESET_CONFIRM_HINT_TEXT
    [UI_STR_ID_FACTORY_RESET_CONFIRM_HINT_TEXT] = FACTORY_RESET_CONFIRM_HINT_TEXT,
#endif
#ifdef FACTORY_RESET_PROGRESS_TEXT
    [UI_STR_ID_FACTORY_RESET_PROGRESS_TEXT] = FACTORY_RESET_PROGRESS_TEXT,
#endif
#ifdef SYS_ERROR_TITLE_TEXT
    [UI_STR_ID_SYS_ERROR_TITLE_TEXT] = SYS_ERROR_TITLE_TEXT,
#endif
#ifdef SYS_ERROR_REPLUG_TEXT
    [UI_STR_ID_SYS_ERROR_REPLUG_TEXT] = SYS_ERROR_REPLUG_TEXT,
#endif
#ifdef TEMP_ERROR_TITLE_TEXT
    [UI_STR_ID_TEMP_ERROR_TITLE_TEXT] = TEMP_ERROR_TITLE_TEXT,
#endif
#ifdef TEMP_ERROR_CONTENT_TEXT
    [UI_STR_ID_TEMP_ERROR_CONTENT_TEXT] = TEMP_ERROR_CONTENT_TEXT,
#endif
#ifdef BLE_CONNECT_TEXT
    [UI_STR_ID_BLE_CONNECT_TEXT] = BLE_CONNECT_TEXT,
#endif
#ifdef BLE_DISCONNECT_TEXT
    [UI_STR_ID_BLE_DISCONNECT_TEXT] = BLE_DISCONNECT_TEXT,
#endif
#ifdef BLE_CONNECTING_TEXT
    [UI_STR_ID_BLE_CONNECTING_TEXT] = BLE_CONNECTING_TEXT,
#endif
#ifdef THEME_GUIDE_TITLE_TEXT
    [UI_STR_ID_THEME_GUIDE_TITLE_TEXT] = THEME_GUIDE_TITLE_TEXT,
#endif
#ifdef THEME_GUIDE_DESC_SELECT_TEXT
    [UI_STR_ID_THEME_GUIDE_DESC_SELECT_TEXT] = THEME_GUIDE_DESC_SELECT_TEXT,
#endif
#ifdef THEME_GUIDE_DESC_CONFIRM_TEXT
    [UI_STR_ID_THEME_GUIDE_DESC_CONFIRM_TEXT] = THEME_GUIDE_DESC_CONFIRM_TEXT,
#endif
#ifdef THEME_GUIDE_OPTION_MINIMAL_TEXT
    [UI_STR_ID_THEME_GUIDE_OPTION_MINIMAL_TEXT] = THEME_GUIDE_OPTION_MINIMAL_TEXT,
#endif
#ifdef THEME_GUIDE_OPTION_IP_TEXT
    [UI_STR_ID_THEME_GUIDE_OPTION_IP_TEXT] = THEME_GUIDE_OPTION_IP_TEXT,
#endif
#ifdef GUIDE_INTRO_TITLE_TEXT
    [UI_STR_ID_GUIDE_INTRO_TITLE_TEXT] = GUIDE_INTRO_TITLE_TEXT,
#endif
#ifdef GUIDE_QRCODE_DESC_TEXT
    [UI_STR_ID_GUIDE_QRCODE_DESC_TEXT] = GUIDE_QRCODE_DESC_TEXT,
#endif
#ifdef GUIDE_FINISH_QRCODE_TEXT
    [UI_STR_ID_GUIDE_FINISH_QRCODE_TEXT] = GUIDE_FINISH_QRCODE_TEXT,
#endif
#ifdef GUIDE_FINISH_DESC_TEXT
    [UI_STR_ID_GUIDE_FINISH_DESC_TEXT] = GUIDE_FINISH_DESC_TEXT,
#endif
#ifdef GUIDE_MODE_SWITCH_GUIDE_TITLE_TEXT
    [UI_STR_ID_GUIDE_MODE_SWITCH_GUIDE_TITLE_TEXT] = GUIDE_MODE_SWITCH_GUIDE_TITLE_TEXT,
#endif
#ifdef GUIDE_PUMP_START_GUIDE_TITLE_TEXT
    [UI_STR_ID_GUIDE_PUMP_START_GUIDE_TITLE_TEXT] = GUIDE_PUMP_START_GUIDE_TITLE_TEXT,
#endif
#ifdef GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT
    [UI_STR_ID_GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT] = GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT,
#endif
#ifdef GUIDE_PUMP_FINISH_GUIDE_TITLE_TEXT
    [UI_STR_ID_GUIDE_PUMP_FINISH_GUIDE_TITLE_TEXT] = GUIDE_PUMP_FINISH_GUIDE_TITLE_TEXT,
#endif
#ifdef GUIDE_PUMP_HOME_MODE_TITLE_TEXT
    [UI_STR_ID_GUIDE_PUMP_HOME_MODE_TITLE_TEXT] = GUIDE_PUMP_HOME_MODE_TITLE_TEXT,
#endif
#ifdef GUIDE_PUMP_START_MODE_TITLE_TEXT
    [UI_STR_ID_GUIDE_PUMP_START_MODE_TITLE_TEXT] = GUIDE_PUMP_START_MODE_TITLE_TEXT,
#endif
#ifdef GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT
    [UI_STR_ID_GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT] = GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT,
#endif
#ifdef GUIDE_SPEED_LABEL_TEXT
    [UI_STR_ID_GUIDE_SPEED_LABEL_TEXT] = GUIDE_SPEED_LABEL_TEXT,
#endif
#ifdef GUIDE_SPEED_GUIDE_PRESS_TEXT
    [UI_STR_ID_GUIDE_SPEED_GUIDE_PRESS_TEXT] = GUIDE_SPEED_GUIDE_PRESS_TEXT,
#endif
#ifdef GUIDE_SPEED_GUIDE_ADJUST_TEXT
    [UI_STR_ID_GUIDE_SPEED_GUIDE_ADJUST_TEXT] = GUIDE_SPEED_GUIDE_ADJUST_TEXT,
#endif
#ifdef GUIDE_LEVEL_SIDE_GUIDE_PRESS_TEXT
    [UI_STR_ID_GUIDE_LEVEL_SIDE_GUIDE_PRESS_TEXT] = GUIDE_LEVEL_SIDE_GUIDE_PRESS_TEXT,
#endif
#ifdef GUIDE_LEVEL_SIDE_GUIDE_OR_TEXT
    [UI_STR_ID_GUIDE_LEVEL_SIDE_GUIDE_OR_TEXT] = GUIDE_LEVEL_SIDE_GUIDE_OR_TEXT,
#endif
#ifdef GUIDE_LEVEL_SIDE_GUIDE_ADJUST_TEXT
    [UI_STR_ID_GUIDE_LEVEL_SIDE_GUIDE_ADJUST_TEXT] = GUIDE_LEVEL_SIDE_GUIDE_ADJUST_TEXT,
#endif
#ifdef GUIDE_LEVEL_BOTH_GUIDE_PRESS_TEXT
    [UI_STR_ID_GUIDE_LEVEL_BOTH_GUIDE_PRESS_TEXT] = GUIDE_LEVEL_BOTH_GUIDE_PRESS_TEXT,
#endif
#ifdef GUIDE_LEVEL_BOTH_GUIDE_AND_TEXT
    [UI_STR_ID_GUIDE_LEVEL_BOTH_GUIDE_AND_TEXT] = GUIDE_LEVEL_BOTH_GUIDE_AND_TEXT,
#endif
#ifdef GUIDE_LEVEL_BOTH_GUIDE_ADJUST_TEXT
    [UI_STR_ID_GUIDE_LEVEL_BOTH_GUIDE_ADJUST_TEXT] = GUIDE_LEVEL_BOTH_GUIDE_ADJUST_TEXT,
#endif
#ifdef GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT
    [UI_STR_ID_GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT] = GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT,
#endif
#ifdef GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT
    [UI_STR_ID_GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT] = GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT,
#endif
#ifdef GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT
    [UI_STR_ID_GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT] = GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT,
#endif
#ifdef GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT
    [UI_STR_ID_GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT] = GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT,
#endif
#ifdef GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT
    [UI_STR_ID_GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT] = GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT,
#endif
#ifdef GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT
    [UI_STR_ID_GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT] = GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT,
#endif
#ifdef PUMP_SEAL_LEAK_TITLE_AIR_SEAL
    [UI_STR_ID_PUMP_SEAL_LEAK_TITLE_AIR_SEAL] = PUMP_SEAL_LEAK_TITLE_AIR_SEAL,
#endif
#ifdef PUMP_SEAL_LEAK_TITLE_FIT
    [UI_STR_ID_PUMP_SEAL_LEAK_TITLE_FIT] = PUMP_SEAL_LEAK_TITLE_FIT,
#endif
#ifdef PUMP_SEAL_LEAK_TITLE_CUP
    [UI_STR_ID_PUMP_SEAL_LEAK_TITLE_CUP] = PUMP_SEAL_LEAK_TITLE_CUP,
#endif
#ifdef PUMP_SEAL_LEAK_TITLE_COMPLETE
    [UI_STR_ID_PUMP_SEAL_LEAK_TITLE_COMPLETE] = PUMP_SEAL_LEAK_TITLE_COMPLETE,
#endif
#ifdef PUMP_SEAL_LEAK_DESC_TUBING
    [UI_STR_ID_PUMP_SEAL_LEAK_DESC_TUBING] = PUMP_SEAL_LEAK_DESC_TUBING,
#endif
#ifdef PUMP_SEAL_LEAK_DESC_FLANGE
    [UI_STR_ID_PUMP_SEAL_LEAK_DESC_FLANGE] = PUMP_SEAL_LEAK_DESC_FLANGE,
#endif
#ifdef PUMP_SEAL_LEAK_DESC_CUP
    [UI_STR_ID_PUMP_SEAL_LEAK_DESC_CUP] = PUMP_SEAL_LEAK_DESC_CUP,
#endif
#ifdef PUMP_SEAL_LEAK_DESC_COMPLETE
    [UI_STR_ID_PUMP_SEAL_LEAK_DESC_COMPLETE] = PUMP_SEAL_LEAK_DESC_COMPLETE,
#endif
#ifdef PUMP_FIT_NOT_DETECTED_TITLE
    [UI_STR_ID_PUMP_FIT_NOT_DETECTED_TITLE] = PUMP_FIT_NOT_DETECTED_TITLE,
#endif
#ifdef PUMP_FIT_NOT_DETECTED_DESC
    [UI_STR_ID_PUMP_FIT_NOT_DETECTED_DESC] = PUMP_FIT_NOT_DETECTED_DESC,
#endif
#ifdef PRODUCT_TESTS_SCAN_QR_TEXT
    [UI_STR_ID_PRODUCT_TESTS_SCAN_QR_TEXT] = PRODUCT_TESTS_SCAN_QR_TEXT,
#endif
#ifdef PRODUCT_TESTS_HOME_BATTERY_FMT
    [UI_STR_ID_PRODUCT_TESTS_HOME_BATTERY_FMT] = PRODUCT_TESTS_HOME_BATTERY_FMT,
#endif
#ifdef PRODUCT_TESTS_BATTERY_FMT
    [UI_STR_ID_PRODUCT_TESTS_BATTERY_FMT] = PRODUCT_TESTS_BATTERY_FMT,
#endif
#ifdef PRODUCT_TESTS_AGING_TIME_FMT
    [UI_STR_ID_PRODUCT_TESTS_AGING_TIME_FMT] = PRODUCT_TESTS_AGING_TIME_FMT,
#endif
#ifdef PRODUCT_TESTS_AGING_COMPLETE_TEXT
    [UI_STR_ID_PRODUCT_TESTS_AGING_COMPLETE_TEXT] = PRODUCT_TESTS_AGING_COMPLETE_TEXT,
#endif
#ifdef PRODUCT_TESTS_AGING_RUNNING_TEXT
    [UI_STR_ID_PRODUCT_TESTS_AGING_RUNNING_TEXT] = PRODUCT_TESTS_AGING_RUNNING_TEXT,
#endif
#ifdef UI_STR_MODE_PROGRAM
    [UI_STR_ID_MODE_PROGRAM] = UI_STR_MODE_PROGRAM,
#endif
#ifdef PUMP_HOME_MAIN_SEAL_INCREASING_TEXT
    [UI_STR_ID_PUMP_HOME_MAIN_SEAL_INCREASING_TEXT] = PUMP_HOME_MAIN_SEAL_INCREASING_TEXT,
#endif
};

const ui_widget_style_t g_ui_widget_default[UI_W_COUNT] = {
    [UI_W_PROGRESS_ARC_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PROGRESS_ARC_TEXT_LINE_SPACE
            (int8_t)(PROGRESS_ARC_TEXT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_MODE_TITLE_Y
            (int16_t)(HOME_MAIN_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_HINT_Y
            (int16_t)(HOME_MAIN_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_HINT_TEXT_LOW_BAT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef HOME_MAIN_HINT_TEXT_LOW_BAT_LINE_SPACE
            (int8_t)(HOME_MAIN_HINT_TEXT_LOW_BAT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_SAVER_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_SAVER_HINT_Y
            (int16_t)(HOME_SAVER_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_STIM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_STIM_ICON_Y
            (int16_t)(HOME_MODE_PAGE_STIM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_STIM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_STIM_TITLE_Y
            (int16_t)(HOME_MODE_PAGE_STIM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_STIM_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_STIM_DURATION_Y
            (int16_t)(HOME_MODE_PAGE_STIM_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_EXPR_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_EXPR_ICON_Y
            (int16_t)(HOME_MODE_PAGE_EXPR_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_EXPR_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_EXPR_TITLE_Y
            (int16_t)(HOME_MODE_PAGE_EXPR_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_EXPR_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_EXPR_DURATION_Y
            (int16_t)(HOME_MODE_PAGE_EXPR_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_RHYTHM_1L_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_RHYTHM_1L_ICON_Y
            (int16_t)(HOME_MODE_PAGE_RHYTHM_1L_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_RHYTHM_1L_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_RHYTHM_1L_TITLE_Y
            (int16_t)(HOME_MODE_PAGE_RHYTHM_1L_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_RHYTHM_1L_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_RHYTHM_1L_DURATION_Y
            (int16_t)(HOME_MODE_PAGE_RHYTHM_1L_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_RHYTHM_2L_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_RHYTHM_2L_ICON_Y
            (int16_t)(HOME_MODE_PAGE_RHYTHM_2L_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_RHYTHM_2L_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_RHYTHM_2L_TITLE_Y
            (int16_t)(HOME_MODE_PAGE_RHYTHM_2L_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_RHYTHM_2L_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MODE_PAGE_RHYTHM_2L_DURATION_Y
            (int16_t)(HOME_MODE_PAGE_RHYTHM_2L_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MODE_PAGE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
#ifdef HOME_MODE_PAGE_TITLE_MAX_W
            (uint16_t)(HOME_MODE_PAGE_TITLE_MAX_W)
#else
            0
#endif
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_LEVEL_DIR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_LEVEL_DIR_Y
            (int16_t)(HOME_LEVEL_DIR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef HOME_LEVEL_DIR_LINE_SPACE
            (int8_t)(HOME_LEVEL_DIR_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_DIR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_BOTH_LEVEL_DIR_Y
            (int16_t)(HOME_BOTH_LEVEL_DIR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_OVER_LIMIT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_BOTH_LEVEL_OVER_LIMIT_Y
            (int16_t)(HOME_BOTH_LEVEL_OVER_LIMIT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef HOME_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE
            (int8_t)(HOME_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_SPEED_LABEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_SPEED_LABEL_Y
            (int16_t)(HOME_SPEED_LABEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_STATUS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_STATUS_Y
            (int16_t)(HOME_MAIN_STATUS_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_SPEED_ICON_Y
            (int16_t)(HOME_MAIN_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_SPEED_VALUE_Y
            (int16_t)(HOME_MAIN_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_LEFT_LEVEL_X
            (int16_t)(HOME_MAIN_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef HOME_MAIN_LEFT_LEVEL_Y
            (int16_t)(HOME_MAIN_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_RIGHT_LEVEL_X
            (int16_t)(HOME_MAIN_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef HOME_MAIN_RIGHT_LEVEL_Y
            (int16_t)(HOME_MAIN_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_2SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_TIME_2SEG_Y
            (int16_t)(HOME_MAIN_TIME_2SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_2SEG_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_2SEG_COLON_X
            (int16_t)(HOME_MAIN_TIME_2SEG_COLON_X)
#else
            0
#endif
        ,
        .y =
#ifdef HOME_MAIN_TIME_2SEG_COLON_Y
            (int16_t)(HOME_MAIN_TIME_2SEG_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_3SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_TIME_3SEG_Y
            (int16_t)(HOME_MAIN_TIME_3SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_COLON_3SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_MAIN_TIME_COLON_3SEG_Y
            (int16_t)(HOME_MAIN_TIME_COLON_3SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_MODE_TITLE_Y
            (int16_t)(HOME_CUSTOM_MAIN_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_HINT_Y
            (int16_t)(HOME_CUSTOM_MAIN_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_STATUS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_STATUS_Y
            (int16_t)(HOME_CUSTOM_MAIN_STATUS_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_SPEED_ICON_Y
            (int16_t)(HOME_CUSTOM_MAIN_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_SPEED_VALUE_Y
            (int16_t)(HOME_CUSTOM_MAIN_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_LEFT_LEVEL_Y
            (int16_t)(HOME_CUSTOM_MAIN_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_CUSTOM_MAIN_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_CUSTOM_MAIN_RIGHT_LEVEL_Y
            (int16_t)(HOME_CUSTOM_MAIN_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_MODE_TITLE_Y
            (int16_t)(PUMP_HOME_MAIN_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_HINT_Y
            (int16_t)(PUMP_HOME_MAIN_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_SEAL_CHECK] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_SEAL_CHECK_Y
            (int16_t)(PUMP_HOME_MAIN_SEAL_CHECK_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_MODE_SWITCH] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_MODE_SWITCH_Y
            (int16_t)(PUMP_HOME_MAIN_MODE_SWITCH_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_HOME_MAIN_MODE_SWITCH_LINE_SPACE
            (int8_t)(PUMP_HOME_MAIN_MODE_SWITCH_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_STIM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_STIM_ICON_Y
            (int16_t)(PUMP_MODE_PAGE_STIM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_STIM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_STIM_TITLE_Y
            (int16_t)(PUMP_MODE_PAGE_STIM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_STIM_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_STIM_DURATION_Y
            (int16_t)(PUMP_MODE_PAGE_STIM_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_EXPR_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_EXPR_ICON_Y
            (int16_t)(PUMP_MODE_PAGE_EXPR_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_EXPR_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_EXPR_TITLE_Y
            (int16_t)(PUMP_MODE_PAGE_EXPR_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_EXPR_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_EXPR_DURATION_Y
            (int16_t)(PUMP_MODE_PAGE_EXPR_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_RHYTHM_1L_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_RHYTHM_1L_ICON_Y
            (int16_t)(PUMP_MODE_PAGE_RHYTHM_1L_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_RHYTHM_1L_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_RHYTHM_1L_TITLE_Y
            (int16_t)(PUMP_MODE_PAGE_RHYTHM_1L_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_RHYTHM_1L_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_RHYTHM_1L_DURATION_Y
            (int16_t)(PUMP_MODE_PAGE_RHYTHM_1L_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_RHYTHM_2L_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_RHYTHM_2L_ICON_Y
            (int16_t)(PUMP_MODE_PAGE_RHYTHM_2L_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_RHYTHM_2L_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_RHYTHM_2L_TITLE_Y
            (int16_t)(PUMP_MODE_PAGE_RHYTHM_2L_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_RHYTHM_2L_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_MODE_PAGE_RHYTHM_2L_DURATION_Y
            (int16_t)(PUMP_MODE_PAGE_RHYTHM_2L_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_MODE_PAGE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
#ifdef PUMP_MODE_PAGE_TITLE_MAX_W
            (uint16_t)(PUMP_MODE_PAGE_TITLE_MAX_W)
#else
            0
#endif
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_LEVEL_DIR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_LEVEL_DIR_Y
            (int16_t)(PUMP_LEVEL_DIR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_LEVEL_DIR_LINE_SPACE
            (int8_t)(PUMP_LEVEL_DIR_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_BOTH_LEVEL_DIR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_BOTH_LEVEL_DIR_Y
            (int16_t)(PUMP_BOTH_LEVEL_DIR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_BOTH_LEVEL_OVER_LIMIT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_BOTH_LEVEL_OVER_LIMIT_Y
            (int16_t)(PUMP_BOTH_LEVEL_OVER_LIMIT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE
            (int8_t)(PUMP_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_BOTH_LEVEL_MODE_SWITCH] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_BOTH_LEVEL_MODE_SWITCH_Y
            (int16_t)(PUMP_BOTH_LEVEL_MODE_SWITCH_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SPEED_LABEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SPEED_LABEL_Y
            (int16_t)(PUMP_SPEED_LABEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_SPEED_LABEL_LINE_SPACE
            (int8_t)(PUMP_SPEED_LABEL_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_STOP_HOLD_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_STOP_HOLD_TITLE_Y
            (int16_t)(PUMP_STOP_HOLD_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_PAUSE_TIMEOUT_TITLE1] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_PAUSE_TIMEOUT_TITLE1_Y
            (int16_t)(PUMP_PAUSE_TIMEOUT_TITLE1_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_TIMEOUT_TITLE1] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_TIMEOUT_TITLE1_Y
            (int16_t)(PUMP_FINISH_TIMEOUT_TITLE1_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_WILL_FINISH_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_WILL_FINISH_ICON_Y
            (int16_t)(PUMP_WILL_FINISH_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_WILL_FINISH_TITLE1] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_WILL_FINISH_TITLE1_Y
            (int16_t)(PUMP_WILL_FINISH_TITLE1_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_ICON_Y
            (int16_t)(PUMP_FINISH_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_TITLE_Y
            (int16_t)(PUMP_FINISH_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_CONTENT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_CONTENT_Y
            (int16_t)(PUMP_FINISH_CONTENT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_CHARGE_ERROR_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_CHARGE_ERROR_ICON_Y
            (int16_t)(PUMP_FINISH_CHARGE_ERROR_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_CHARGE_ERROR_TITLE1] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_CHARGE_ERROR_TITLE1_Y
            (int16_t)(PUMP_FINISH_CHARGE_ERROR_TITLE1_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FINISH_CHARGE_ERROR_TITLE2] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FINISH_CHARGE_ERROR_TITLE2_Y
            (int16_t)(PUMP_FINISH_CHARGE_ERROR_TITLE2_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_RUNING_30M_TIP_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_RUNING_30M_TIP_ICON_Y
            (int16_t)(PUMP_RUNING_30M_TIP_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_RUNING_30M_TIP_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_RUNING_30M_TIP_TITLE_Y
            (int16_t)(PUMP_RUNING_30M_TIP_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_RUNING_30M_TIP_TITLE_LINE_SPACE
            (int8_t)(PUMP_RUNING_30M_TIP_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_REST_MODE_ADJUST_TIP_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_REST_MODE_ADJUST_TIP_ICON_Y
            (int16_t)(PUMP_REST_MODE_ADJUST_TIP_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_REST_MODE_ADJUST_TIP_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_REST_MODE_ADJUST_TIP_TITLE_Y
            (int16_t)(PUMP_REST_MODE_ADJUST_TIP_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_REST_MODE_ADJUST_TIP_TITLE_LINE_SPACE
            (int8_t)(PUMP_REST_MODE_ADJUST_TIP_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_RHYTHM_CONFIRM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef RHYTHM_CONFIRM_ICON_Y
            (int16_t)(RHYTHM_CONFIRM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_RHYTHM_CONFIRM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef RHYTHM_CONFIRM_TITLE_Y
            (int16_t)(RHYTHM_CONFIRM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_RHYTHM_CONFIRM_SUBTITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef RHYTHM_CONFIRM_SUBTITLE_Y
            (int16_t)(RHYTHM_CONFIRM_SUBTITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_ICON_Y
            (int16_t)(SETTINGS_SUCCESS_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_TITLE_Y
            (int16_t)(SETTINGS_SUCCESS_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_PROGRAM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_PROGRAM_ICON_Y
            (int16_t)(SETTINGS_SUCCESS_PROGRAM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_PROGRAM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_PROGRAM_TITLE_Y
            (int16_t)(SETTINGS_SUCCESS_PROGRAM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_PROGRAM_SUBTITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_PROGRAM_SUBTITLE_Y
            (int16_t)(SETTINGS_SUCCESS_PROGRAM_SUBTITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POWER_OFF_HOLD_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POWER_OFF_HOLD_ICON_Y
            (int16_t)(POWER_OFF_HOLD_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POWER_OFF_HOLD_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POWER_OFF_HOLD_TITLE_Y
            (int16_t)(POWER_OFF_HOLD_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef POWER_OFF_HOLD_TITLE_LINE_SPACE
            (int8_t)(POWER_OFF_HOLD_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POWER_ON_HOLD_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POWER_ON_HOLD_ICON_Y
            (int16_t)(POWER_ON_HOLD_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POWER_ON_HOLD_LINE1] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POWER_ON_HOLD_LINE1_Y
            (int16_t)(POWER_ON_HOLD_LINE1_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_AIRPLANE_MODE_LOCK_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef AIRPLANE_MODE_LOCK_ICON_Y
            (int16_t)(AIRPLANE_MODE_LOCK_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_AIRPLANE_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef AIRPLANE_MODE_TITLE_Y
            (int16_t)(AIRPLANE_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_AIRPLANE_MODE_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef AIRPLANE_MODE_HINT_Y
            (int16_t)(AIRPLANE_MODE_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef AIRPLANE_MODE_HINT_LINE_SPACE
            (int8_t)(AIRPLANE_MODE_HINT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_AIRPLANE_MODE_STIM_KEY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef AIRPLANE_MODE_STIM_KEY_ICON_X
            (int16_t)(AIRPLANE_MODE_STIM_KEY_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef AIRPLANE_MODE_STIM_KEY_ICON_Y
            (int16_t)(AIRPLANE_MODE_STIM_KEY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_AIRPLANE_MODE_ADD_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef AIRPLANE_MODE_ADD_ICON_X
            (int16_t)(AIRPLANE_MODE_ADD_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef AIRPLANE_MODE_ADD_ICON_Y
            (int16_t)(AIRPLANE_MODE_ADD_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_AIRPLANE_MODE_EXPR_KEY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef AIRPLANE_MODE_EXPR_KEY_ICON_X
            (int16_t)(AIRPLANE_MODE_EXPR_KEY_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef AIRPLANE_MODE_EXPR_KEY_ICON_Y
            (int16_t)(AIRPLANE_MODE_EXPR_KEY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_TRAVEL_LOCK_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef TRAVEL_LOCK_TITLE_Y
            (int16_t)(TRAVEL_LOCK_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef TRAVEL_LOCK_TITLE_LINE_SPACE
            (int8_t)(TRAVEL_LOCK_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_TRAVEL_LOCK_STIM_KEY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef TRAVEL_LOCK_STIM_KEY_ICON_X
            (int16_t)(TRAVEL_LOCK_STIM_KEY_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef TRAVEL_LOCK_STIM_KEY_ICON_Y
            (int16_t)(TRAVEL_LOCK_STIM_KEY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_TRAVEL_LOCK_ADD_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef TRAVEL_LOCK_ADD_ICON_X
            (int16_t)(TRAVEL_LOCK_ADD_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef TRAVEL_LOCK_ADD_ICON_Y
            (int16_t)(TRAVEL_LOCK_ADD_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_TRAVEL_LOCK_EXPR_KEY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef TRAVEL_LOCK_EXPR_KEY_ICON_X
            (int16_t)(TRAVEL_LOCK_EXPR_KEY_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef TRAVEL_LOCK_EXPR_KEY_ICON_Y
            (int16_t)(TRAVEL_LOCK_EXPR_KEY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_LOW_POWER_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef LOW_POWER_TITLE_Y
            (int16_t)(LOW_POWER_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POWER_OFF_CHARGE_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POWER_OFF_CHARGE_TEXT_Y
            (int16_t)(POWER_OFF_CHARGE_TEXT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_LOW_POWER_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef LOW_POWER_TEXT_Y
            (int16_t)(LOW_POWER_TEXT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_CHARGE_ADAPTER_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef CHARGE_ADAPTER_ICON_Y
            (int16_t)(CHARGE_ADAPTER_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_CHARGE_ADAPTER_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef CHARGE_ADAPTER_TITLE_Y
            (int16_t)(CHARGE_ADAPTER_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_CHARGE_ADAPTER_SUBTITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef CHARGE_ADAPTER_SUBTITLE_Y
            (int16_t)(CHARGE_ADAPTER_SUBTITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_DOWNLOAD_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_DOWNLOAD_ICON_Y
            (int16_t)(OTA_DOWNLOAD_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_DOWNLOAD_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_DOWNLOAD_TITLE_Y
            (int16_t)(OTA_DOWNLOAD_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_DOWNLOAD_PERCENT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_DOWNLOAD_PERCENT_Y
            (int16_t)(OTA_DOWNLOAD_PERCENT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_DOWNLOAD_SIZE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_DOWNLOAD_SIZE_Y
            (int16_t)(OTA_DOWNLOAD_SIZE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_SUCCESS_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_SUCCESS_ICON_Y
            (int16_t)(OTA_SUCCESS_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_SUCCESS_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_SUCCESS_TITLE_Y
            (int16_t)(OTA_SUCCESS_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_SUCCESS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef OTA_SUCCESS_LINE_SPACE
            (int8_t)(OTA_SUCCESS_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_ERROR_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_ERROR_ICON_Y
            (int16_t)(OTA_ERROR_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_ERROR_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_ERROR_TITLE_Y
            (int16_t)(OTA_ERROR_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_ERROR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef OTA_ERROR_LINE_SPACE
            (int8_t)(OTA_ERROR_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_FACTORY_RESET_CONFIRM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef FACTORY_RESET_CONFIRM_ICON_Y
            (int16_t)(FACTORY_RESET_CONFIRM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_FACTORY_RESET_CONFIRM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef FACTORY_RESET_CONFIRM_TITLE_Y
            (int16_t)(FACTORY_RESET_CONFIRM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_FACTORY_RESET_CONFIRM_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef FACTORY_RESET_CONFIRM_HINT_Y
            (int16_t)(FACTORY_RESET_CONFIRM_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_FACTORY_RESET_PROGRESS_ARC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef FACTORY_RESET_PROGRESS_ARC_Y
            (int16_t)(FACTORY_RESET_PROGRESS_ARC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_CONNECT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_CONNECT_ICON_Y
            (int16_t)(POPUP_WINDOW_BLE_CONNECT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_CONNECT_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_CONNECT_TEXT_Y
            (int16_t)(POPUP_WINDOW_BLE_CONNECT_TEXT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_DISCONNECT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_DISCONNECT_ICON_Y
            (int16_t)(POPUP_WINDOW_BLE_DISCONNECT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_DISCONNECT_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_DISCONNECT_TEXT_Y
            (int16_t)(POPUP_WINDOW_BLE_DISCONNECT_TEXT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_DISCONNECT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef POPUP_WINDOW_BLE_DISCONNECT_LINE_SPACE
            (int8_t)(POPUP_WINDOW_BLE_DISCONNECT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_CONNECTING_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_CONNECTING_ICON_Y
            (int16_t)(POPUP_WINDOW_BLE_CONNECTING_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_CONNECTING_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_CONNECTING_TEXT_Y
            (int16_t)(POPUP_WINDOW_BLE_CONNECTING_TEXT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SEAL_LEAK_TITLE_Y
            (int16_t)(PUMP_SEAL_LEAK_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_FIRST_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SEAL_LEAK_FIRST_TITLE_Y
            (int16_t)(PUMP_SEAL_LEAK_FIRST_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_DIR_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SEAL_LEAK_DIR_ICON_Y
            (int16_t)(PUMP_SEAL_LEAK_DIR_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SEAL_LEAK_DESC_Y
            (int16_t)(PUMP_SEAL_LEAK_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_SEAL_LEAK_DESC_LINE_SPACE
            (int8_t)(PUMP_SEAL_LEAK_DESC_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_FIRST_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SEAL_LEAK_FIRST_DESC_Y
            (int16_t)(PUMP_SEAL_LEAK_FIRST_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_COMPLETE_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_SEAL_LEAK_COMPLETE_DESC_Y
            (int16_t)(PUMP_SEAL_LEAK_COMPLETE_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_SEAL_LEAK_COMPLETE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_SEAL_LEAK_COMPLETE_TITLE_LINE_SPACE
            (int8_t)(PUMP_SEAL_LEAK_COMPLETE_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FIT_NOT_DETECTED_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FIT_NOT_DETECTED_TITLE_Y
            (int16_t)(PUMP_FIT_NOT_DETECTED_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_FIT_NOT_DETECTED_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_FIT_NOT_DETECTED_DESC_Y
            (int16_t)(PUMP_FIT_NOT_DETECTED_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_FIT_NOT_DETECTED_DESC_LINE_SPACE
            (int8_t)(PUMP_FIT_NOT_DETECTED_DESC_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef THEME_GUIDE_TITLE_Y
            (int16_t)(THEME_GUIDE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef THEME_GUIDE_DESC_Y
            (int16_t)(THEME_GUIDE_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef THEME_GUIDE_DESC_LINE_SPACE
            (int8_t)(THEME_GUIDE_DESC_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_LABEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef THEME_GUIDE_LABEL_Y
            (int16_t)(THEME_GUIDE_LABEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_INTRO_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_INTRO_TITLE_LINE_SPACE
            (int8_t)(GUIDE_INTRO_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_QRCODE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_QRCODE_ALIGN_Y
            (int16_t)(GUIDE_QRCODE_ALIGN_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_QRCODE_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_QRCODE_DESC_Y
            (int16_t)(GUIDE_QRCODE_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_QRCODE_DESC_LINE_SPACE
            (int8_t)(GUIDE_QRCODE_DESC_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_FINISH_QRCODE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_FINISH_QRCODE_ALIGN_Y
            (int16_t)(GUIDE_FINISH_QRCODE_ALIGN_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_FINISH_DESC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_FINISH_DESC_Y
            (int16_t)(GUIDE_FINISH_DESC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_FINISH_DESC_LINE_SPACE
            (int8_t)(GUIDE_FINISH_DESC_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_HOME_MODE_TITLE_Y
            (int16_t)(GUIDE_MODE_HOME_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_SWITCH_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_SWITCH_ICON_Y
            (int16_t)(GUIDE_MODE_SWITCH_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_SWITCH_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_SWITCH_TITLE_Y
            (int16_t)(GUIDE_MODE_SWITCH_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_SWITCH_DURATION] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_SWITCH_DURATION_Y
            (int16_t)(GUIDE_MODE_SWITCH_DURATION_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_SWITCH_GUIDE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_SWITCH_GUIDE_TITLE_Y
            (int16_t)(GUIDE_MODE_SWITCH_GUIDE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_MODE_SWITCH_GUIDE_TITLE_LINE_SPACE
            (int8_t)(GUIDE_MODE_SWITCH_GUIDE_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_SWITCH_GUIDE_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_SWITCH_GUIDE_ICON_Y
            (int16_t)(GUIDE_MODE_SWITCH_GUIDE_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_MODE_TITLE_Y
            (int16_t)(GUIDE_PUMP_HOME_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_MODE_TITLE_Y
            (int16_t)(GUIDE_PUMP_START_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_MODE_TITLE_Y
            (int16_t)(GUIDE_PUMP_PAUSE_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_GUIDE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_GUIDE_TITLE_Y
            (int16_t)(GUIDE_PUMP_START_GUIDE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_PUMP_START_GUIDE_TITLE_LINE_SPACE
            (int8_t)(GUIDE_PUMP_START_GUIDE_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_GUIDE_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_GUIDE_ICON_Y
            (int16_t)(GUIDE_PUMP_START_GUIDE_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_GUIDE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y
            (int16_t)(GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_PUMP_PAUSE_GUIDE_TITLE_LINE_SPACE
            (int8_t)(GUIDE_PUMP_PAUSE_GUIDE_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_GUIDE_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_GUIDE_ICON_Y
            (int16_t)(GUIDE_PUMP_PAUSE_GUIDE_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_FINISH_GUIDE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_FINISH_GUIDE_TITLE_Y
            (int16_t)(GUIDE_PUMP_FINISH_GUIDE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_PUMP_FINISH_GUIDE_TITLE_LINE_SPACE
            (int8_t)(GUIDE_PUMP_FINISH_GUIDE_TITLE_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_LABEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_LABEL_Y
            (int16_t)(GUIDE_SPEED_LABEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_MODE_TITLE_Y
            (int16_t)(GUIDE_SPEED_DONE_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_GUIDE_PRESS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_SPEED_GUIDE_PRESS_X
            (int16_t)(GUIDE_SPEED_GUIDE_PRESS_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_SPEED_GUIDE_PRESS_Y
            (int16_t)(GUIDE_SPEED_GUIDE_PRESS_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_SPEED_GUIDE_PRESS_LINE_SPACE
            (int8_t)(GUIDE_SPEED_GUIDE_PRESS_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_GUIDE_ADJUST] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_GUIDE_ADJUST_Y
            (int16_t)(GUIDE_SPEED_GUIDE_ADJUST_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_SPEED_GUIDE_ADJUST_LINE_SPACE
            (int8_t)(GUIDE_SPEED_GUIDE_ADJUST_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_GUIDE_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef GUIDE_SPEED_GUIDE_ICON_X
            (int16_t)(GUIDE_SPEED_GUIDE_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_SPEED_GUIDE_ICON_Y
            (int16_t)(GUIDE_SPEED_GUIDE_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DIR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DIR_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DIR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_LEVEL_SIDE_DIR_LINE_SPACE
            (int8_t)(GUIDE_LEVEL_SIDE_DIR_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_GUIDE_PRESS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_PRESS_Y
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_PRESS_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_GUIDE_OR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_OR_Y
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_OR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_GUIDE_ADJUST] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_ADJUST_Y
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_ADJUST_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_GUIDE_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_TEXT_LINE_SPACE
            (int8_t)(GUIDE_LEVEL_SIDE_GUIDE_TEXT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_DIR] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_BOTH_LEVEL_DIR_Y
            (int16_t)(GUIDE_BOTH_LEVEL_DIR_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_OVER_LIMIT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_BOTH_LEVEL_OVER_LIMIT_Y
            (int16_t)(GUIDE_BOTH_LEVEL_OVER_LIMIT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE
            (int8_t)(GUIDE_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_MODE_TITLE_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_GUIDE_PRESS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_PRESS_X
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_PRESS_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_PRESS_Y
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_PRESS_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_PRESS_LINE_SPACE
            (int8_t)(GUIDE_LEVEL_BOTH_GUIDE_PRESS_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_GUIDE_AND] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_AND_X
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_AND_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_AND_Y
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_AND_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_AND_LINE_SPACE
            (int8_t)(GUIDE_LEVEL_BOTH_GUIDE_AND_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_GUIDE_ADJUST] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_ADJUST_Y
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_ADJUST_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_ADJUST_LINE_SPACE
            (int8_t)(GUIDE_LEVEL_BOTH_GUIDE_ADJUST_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_X
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_Y
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_X
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_Y
            (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_SEAL_FAIL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
#ifdef PUMP_HOME_MAIN_SEAL_FAIL_LINE_SPACE
            (int8_t)(PUMP_HOME_MAIN_SEAL_FAIL_LINE_SPACE)
#else
            0
#endif
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_PREVIEW_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_PREVIEW_LEFT_X
            (int16_t)(THEME_GUIDE_PREVIEW_LEFT_X)
#else
            0
#endif
        ,
        .y =
#ifdef THEME_GUIDE_PREVIEW_LEFT_Y
            (int16_t)(THEME_GUIDE_PREVIEW_LEFT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_PREVIEW_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_PREVIEW_RIGHT_X
            (int16_t)(THEME_GUIDE_PREVIEW_RIGHT_X)
#else
            0
#endif
        ,
        .y =
#ifdef THEME_GUIDE_PREVIEW_RIGHT_Y
            (int16_t)(THEME_GUIDE_PREVIEW_RIGHT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_RING_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_RING_LEFT_X
            (int16_t)(THEME_GUIDE_RING_LEFT_X)
#else
            0
#endif
        ,
        .y =
#ifdef THEME_GUIDE_RING_LEFT_Y
            (int16_t)(THEME_GUIDE_RING_LEFT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_RING_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_RING_RIGHT_X
            (int16_t)(THEME_GUIDE_RING_RIGHT_X)
#else
            0
#endif
        ,
        .y =
#ifdef THEME_GUIDE_RING_RIGHT_Y
            (int16_t)(THEME_GUIDE_RING_RIGHT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_RADIO_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_RADIO_LEFT_X
            (int16_t)(THEME_GUIDE_RADIO_LEFT_X)
#else
            0
#endif
        ,
        .y =
#ifdef THEME_GUIDE_RADIO_LEFT_Y
            (int16_t)(THEME_GUIDE_RADIO_LEFT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_RADIO_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_RADIO_RIGHT_X
            (int16_t)(THEME_GUIDE_RADIO_RIGHT_X)
#else
            0
#endif
        ,
        .y =
#ifdef THEME_GUIDE_RADIO_RIGHT_Y
            (int16_t)(THEME_GUIDE_RADIO_RIGHT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_LABEL_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_LABEL_LEFT_X
            (int16_t)(THEME_GUIDE_LABEL_LEFT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_THEME_GUIDE_LABEL_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef THEME_GUIDE_LABEL_RIGHT_X
            (int16_t)(THEME_GUIDE_LABEL_RIGHT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_2SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_TIME_2SEG_Y
            (int16_t)(PUMP_HOME_MAIN_TIME_2SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_2SEG_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_2SEG_COLON_X
            (int16_t)(PUMP_HOME_MAIN_TIME_2SEG_COLON_X)
#else
            0
#endif
        ,
        .y =
#ifdef PUMP_HOME_MAIN_TIME_2SEG_COLON_Y
            (int16_t)(PUMP_HOME_MAIN_TIME_2SEG_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_3SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_TIME_3SEG_Y
            (int16_t)(PUMP_HOME_MAIN_TIME_3SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_COLON_3SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_TIME_COLON_3SEG_Y
            (int16_t)(PUMP_HOME_MAIN_TIME_COLON_3SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_SPEED_ICON_Y
            (int16_t)(PUMP_HOME_MAIN_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_SPEED_VALUE_Y
            (int16_t)(PUMP_HOME_MAIN_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_LEFT_LEVEL_X
            (int16_t)(PUMP_HOME_MAIN_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef PUMP_HOME_MAIN_LEFT_LEVEL_Y
            (int16_t)(PUMP_HOME_MAIN_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_RIGHT_LEVEL_X
            (int16_t)(PUMP_HOME_MAIN_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef PUMP_HOME_MAIN_RIGHT_LEVEL_Y
            (int16_t)(PUMP_HOME_MAIN_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_PLAY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_PLAY_ICON_Y
            (int16_t)(PUMP_HOME_MAIN_PLAY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_WAVE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_WAVE_Y
            (int16_t)(PUMP_HOME_MAIN_WAVE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_MODE_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_MODE_TITLE_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_MODE_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_HINT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_HINT_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_HINT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_TIME_2SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_COLON_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_TIME_2SEG_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_TIME_3SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_TIME_3SEG_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_TIME_3SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_TIME_COLON_3SEG] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_TIME_COLON_3SEG_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_TIME_COLON_3SEG_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_SPEED_ICON_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_SPEED_VALUE_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_LEFT_LEVEL_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_RIGHT_LEVEL_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_PLAY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_PLAY_ICON_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_PLAY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_WAVE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_CUSTOM_MAIN_WAVE_X
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_WAVE_X)
#else
            0
#endif
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_WAVE_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_WAVE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_2SEG_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_2SEG_M_X
            (int16_t)(HOME_MAIN_TIME_2SEG_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_2SEG_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_2SEG_S_X
            (int16_t)(HOME_MAIN_TIME_2SEG_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_3SEG_H] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_3SEG_H_X
            (int16_t)(HOME_MAIN_TIME_3SEG_H_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_3SEG_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_3SEG_M_X
            (int16_t)(HOME_MAIN_TIME_3SEG_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_3SEG_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_3SEG_S_X
            (int16_t)(HOME_MAIN_TIME_3SEG_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_COLON_HM] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_COLON_HM_X
            (int16_t)(HOME_MAIN_TIME_COLON_HM_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_MAIN_TIME_COLON_MS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_MAIN_TIME_COLON_MS_X
            (int16_t)(HOME_MAIN_TIME_COLON_MS_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_2SEG_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_2SEG_M_X
            (int16_t)(PUMP_HOME_MAIN_TIME_2SEG_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_2SEG_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_2SEG_S_X
            (int16_t)(PUMP_HOME_MAIN_TIME_2SEG_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_3SEG_H] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_3SEG_H_X
            (int16_t)(PUMP_HOME_MAIN_TIME_3SEG_H_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_3SEG_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_3SEG_M_X
            (int16_t)(PUMP_HOME_MAIN_TIME_3SEG_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_3SEG_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_3SEG_S_X
            (int16_t)(PUMP_HOME_MAIN_TIME_3SEG_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_COLON_HM] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_COLON_HM_X
            (int16_t)(PUMP_HOME_MAIN_TIME_COLON_HM_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_TIME_COLON_MS] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef PUMP_HOME_MAIN_TIME_COLON_MS_X
            (int16_t)(PUMP_HOME_MAIN_TIME_COLON_MS_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_BASE_BLE_UI] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef BASE_BLE_UI_X
            (int16_t)(BASE_BLE_UI_X)
#else
            0
#endif
        ,
        .y =
#ifdef BASE_BLE_UI_Y
            (int16_t)(BASE_BLE_UI_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_BASE_PUMP_HOME_BLE_UI] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef BASE_PUMP_HOME_BLE_UI_X
            (int16_t)(BASE_PUMP_HOME_BLE_UI_X)
#else
            0
#endif
        ,
        .y =
#ifdef BASE_PUMP_HOME_BLE_UI_Y
            (int16_t)(BASE_PUMP_HOME_BLE_UI_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_CUSTOM_BLE_UI] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef CUSTOM_BLE_UI_X
            (int16_t)(CUSTOM_BLE_UI_X)
#else
            0
#endif
        ,
        .y =
#ifdef CUSTOM_BLE_UI_Y
            (int16_t)(CUSTOM_BLE_UI_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_CUSTOM_PUMP_HOME_BLE_UI] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef CUSTOM_PUMP_HOME_BLE_UI_X
            (int16_t)(CUSTOM_PUMP_HOME_BLE_UI_X)
#else
            0
#endif
        ,
        .y =
#ifdef CUSTOM_PUMP_HOME_BLE_UI_Y
            (int16_t)(CUSTOM_PUMP_HOME_BLE_UI_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef HOME_BOTH_LEVEL_VALUE_Y
            (int16_t)(HOME_BOTH_LEVEL_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_VALUE_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_BOTH_LEVEL_VALUE_LEFT_X
            (int16_t)(HOME_BOTH_LEVEL_VALUE_LEFT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_VALUE_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_BOTH_LEVEL_VALUE_RIGHT_X
            (int16_t)(HOME_BOTH_LEVEL_VALUE_RIGHT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_DIR_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_BOTH_LEVEL_DIR_LEFT_X
            (int16_t)(HOME_BOTH_LEVEL_DIR_LEFT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_HOME_BOTH_LEVEL_DIR_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef HOME_BOTH_LEVEL_DIR_RIGHT_X
            (int16_t)(HOME_BOTH_LEVEL_DIR_RIGHT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PROGRESS_ARC] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PROGRESS_ARC_Y
            (int16_t)(PROGRESS_ARC_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PAGE_NUM_GAP] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PAGE_NUM_GAP_X
            (int16_t)(GUIDE_PAGE_NUM_GAP_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PAGE_NUM_CURRENT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PAGE_NUM_CURRENT_Y
            (int16_t)(GUIDE_PAGE_NUM_CURRENT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PAGE_NUM_TOTAL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PAGE_NUM_TOTAL_Y
            (int16_t)(GUIDE_PAGE_NUM_TOTAL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_TIME_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_MODE_HOME_TIME_M_X
            (int16_t)(GUIDE_MODE_HOME_TIME_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_TIME_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_MODE_HOME_TIME_S_X
            (int16_t)(GUIDE_MODE_HOME_TIME_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_TIME] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_HOME_TIME_Y
            (int16_t)(GUIDE_MODE_HOME_TIME_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_TIME_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_HOME_TIME_COLON_Y
            (int16_t)(GUIDE_MODE_HOME_TIME_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_HOME_SPEED_ICON_Y
            (int16_t)(GUIDE_MODE_HOME_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_MODE_HOME_SPEED_VALUE_Y
            (int16_t)(GUIDE_MODE_HOME_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_MODE_HOME_LEFT_LEVEL_X
            (int16_t)(GUIDE_MODE_HOME_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_MODE_HOME_LEFT_LEVEL_Y
            (int16_t)(GUIDE_MODE_HOME_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_MODE_HOME_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_MODE_HOME_RIGHT_LEVEL_X
            (int16_t)(GUIDE_MODE_HOME_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_MODE_HOME_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_MODE_HOME_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_TIME_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_HOME_TIME_M_X
            (int16_t)(GUIDE_PUMP_HOME_TIME_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_TIME_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_HOME_TIME_S_X
            (int16_t)(GUIDE_PUMP_HOME_TIME_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_TIME] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_TIME_Y
            (int16_t)(GUIDE_PUMP_HOME_TIME_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_TIME_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_TIME_COLON_Y
            (int16_t)(GUIDE_PUMP_HOME_TIME_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_SPEED_ICON_Y
            (int16_t)(GUIDE_PUMP_HOME_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_SPEED_VALUE_Y
            (int16_t)(GUIDE_PUMP_HOME_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_HOME_LEFT_LEVEL_X
            (int16_t)(GUIDE_PUMP_HOME_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_LEFT_LEVEL_Y
            (int16_t)(GUIDE_PUMP_HOME_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_HOME_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_HOME_RIGHT_LEVEL_X
            (int16_t)(GUIDE_PUMP_HOME_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_PUMP_HOME_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_PUMP_HOME_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_TIME_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_START_TIME_M_X
            (int16_t)(GUIDE_PUMP_START_TIME_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_TIME_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_START_TIME_S_X
            (int16_t)(GUIDE_PUMP_START_TIME_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_TIME] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_TIME_Y
            (int16_t)(GUIDE_PUMP_START_TIME_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_TIME_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_TIME_COLON_Y
            (int16_t)(GUIDE_PUMP_START_TIME_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_WAVE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_WAVE_Y
            (int16_t)(GUIDE_PUMP_START_WAVE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_SPEED_ICON_Y
            (int16_t)(GUIDE_PUMP_START_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_START_SPEED_VALUE_Y
            (int16_t)(GUIDE_PUMP_START_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_START_LEFT_LEVEL_X
            (int16_t)(GUIDE_PUMP_START_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_PUMP_START_LEFT_LEVEL_Y
            (int16_t)(GUIDE_PUMP_START_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_START_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_START_RIGHT_LEVEL_X
            (int16_t)(GUIDE_PUMP_START_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_PUMP_START_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_PUMP_START_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_WAVE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_WAVE_Y
            (int16_t)(GUIDE_PUMP_PAUSE_WAVE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_PLAY_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_PLAY_ICON_Y
            (int16_t)(GUIDE_PUMP_PAUSE_PLAY_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_SPEED_ICON_Y
            (int16_t)(GUIDE_PUMP_PAUSE_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_SPEED_VALUE_Y
            (int16_t)(GUIDE_PUMP_PAUSE_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_PAUSE_LEFT_LEVEL_X
            (int16_t)(GUIDE_PUMP_PAUSE_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_LEFT_LEVEL_Y
            (int16_t)(GUIDE_PUMP_PAUSE_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_PAUSE_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_PUMP_PAUSE_RIGHT_LEVEL_X
            (int16_t)(GUIDE_PUMP_PAUSE_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_PUMP_PAUSE_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_PUMP_PAUSE_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_PUMP_FINISH_GUIDE_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_PUMP_FINISH_GUIDE_ICON_Y
            (int16_t)(GUIDE_PUMP_FINISH_GUIDE_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_TIME_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_SPEED_DONE_TIME_M_X
            (int16_t)(GUIDE_SPEED_DONE_TIME_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_TIME_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_SPEED_DONE_TIME_S_X
            (int16_t)(GUIDE_SPEED_DONE_TIME_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_TIME] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_TIME_Y
            (int16_t)(GUIDE_SPEED_DONE_TIME_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_TIME_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_TIME_COLON_Y
            (int16_t)(GUIDE_SPEED_DONE_TIME_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_SPEED_ICON_Y
            (int16_t)(GUIDE_SPEED_DONE_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_SPEED_VALUE_Y
            (int16_t)(GUIDE_SPEED_DONE_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_SPEED_DONE_LEFT_LEVEL_X
            (int16_t)(GUIDE_SPEED_DONE_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_LEFT_LEVEL_Y
            (int16_t)(GUIDE_SPEED_DONE_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_DONE_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_SPEED_DONE_RIGHT_LEVEL_X
            (int16_t)(GUIDE_SPEED_DONE_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_SPEED_DONE_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_SPEED_DONE_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_SPEED_VALUE_Y
            (int16_t)(GUIDE_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_TIME_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_SIDE_DONE_TIME_M_X
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_TIME_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_TIME_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_SIDE_DONE_TIME_S_X
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_TIME_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_TIME] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_TIME_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_TIME_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_TIME_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_TIME_COLON_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_TIME_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_X
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_X
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_GUIDE_LEFT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_LEFT_ICON_X
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_LEFT_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_LEFT_ICON_Y
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_LEFT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_GUIDE_RIGHT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_RIGHT_ICON_X
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_RIGHT_ICON_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_GUIDE_RIGHT_ICON_Y
            (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_RIGHT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_SIDE_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_SIDE_VALUE_Y
            (int16_t)(GUIDE_LEVEL_SIDE_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_TIME_M] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_DONE_TIME_M_X
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_TIME_M_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_TIME_S] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_DONE_TIME_S_X
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_TIME_S_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_TIME] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_TIME_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_TIME_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_TIME_COLON] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_TIME_COLON_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_TIME_COLON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_SPEED_ICON_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_SPEED_VALUE_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_X
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_X
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_X)
#else
            0
#endif
        ,
        .y =
#ifdef GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_Y
            (int16_t)(GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef GUIDE_BOTH_LEVEL_VALUE_Y
            (int16_t)(GUIDE_BOTH_LEVEL_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_VALUE_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_BOTH_LEVEL_VALUE_LEFT_X
            (int16_t)(GUIDE_BOTH_LEVEL_VALUE_LEFT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_VALUE_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_BOTH_LEVEL_VALUE_RIGHT_X
            (int16_t)(GUIDE_BOTH_LEVEL_VALUE_RIGHT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_DIR_LEFT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_BOTH_LEVEL_DIR_LEFT_X
            (int16_t)(GUIDE_BOTH_LEVEL_DIR_LEFT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_GUIDE_BOTH_LEVEL_DIR_RIGHT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef GUIDE_BOTH_LEVEL_DIR_RIGHT_X
            (int16_t)(GUIDE_BOTH_LEVEL_DIR_RIGHT_X)
#else
            0
#endif
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_BATTERY_UI_BLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef BATTERY_UI_BLE_Y
            (int16_t)(BATTERY_UI_BLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_BASE_BATTERY_UI] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef BASE_BATTERY_UI_X
            (int16_t)(BASE_BATTERY_UI_X)
#else
            0
#endif
        ,
        .y =
#ifdef BASE_BATTERY_UI_Y
            (int16_t)(BASE_BATTERY_UI_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_BASE_PUMP_HOME_BATTERY_UI] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 1,
        .x =
#ifdef BASE_PUMP_HOME_BATTERY_UI_X
            (int16_t)(BASE_PUMP_HOME_BATTERY_UI_X)
#else
            0
#endif
        ,
        .y =
#ifdef BASE_PUMP_HOME_BATTERY_UI_Y
            (int16_t)(BASE_PUMP_HOME_BATTERY_UI_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_BATTERY_CAP] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
            0
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
#ifdef BATTERY_CAP_MAX_W
            (uint16_t)(BATTERY_CAP_MAX_W)
#else
            0
#endif
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_PAUSE_SPEED_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_PAUSE_SPEED_ICON_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_PAUSE_SPEED_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_PAUSE_SPEED_VALUE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_PAUSE_SPEED_VALUE_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_PAUSE_SPEED_VALUE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_PAUSE_LEFT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_PAUSE_LEFT_LEVEL_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_PAUSE_LEFT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_CUSTOM_MAIN_PAUSE_RIGHT_LEVEL] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_CUSTOM_MAIN_PAUSE_RIGHT_LEVEL_Y
            (int16_t)(PUMP_HOME_CUSTOM_MAIN_PAUSE_RIGHT_LEVEL_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_CUSTOM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_CUSTOM_ICON_Y
            (int16_t)(SETTINGS_SUCCESS_CUSTOM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_CUSTOM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_CUSTOM_TITLE_Y
            (int16_t)(SETTINGS_SUCCESS_CUSTOM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_CUSTOM_PROGRAM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_CUSTOM_PROGRAM_ICON_Y
            (int16_t)(SETTINGS_SUCCESS_CUSTOM_PROGRAM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_CUSTOM_PROGRAM_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_CUSTOM_PROGRAM_TITLE_Y
            (int16_t)(SETTINGS_SUCCESS_CUSTOM_PROGRAM_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_SETTINGS_SUCCESS_CUSTOM_PROGRAM_SUBTITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef SETTINGS_SUCCESS_CUSTOM_PROGRAM_SUBTITLE_Y
            (int16_t)(SETTINGS_SUCCESS_CUSTOM_PROGRAM_SUBTITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_CUSTOM_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_CUSTOM_ICON_Y
            (int16_t)(POPUP_WINDOW_BLE_CUSTOM_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_POPUP_WINDOW_BLE_CUSTOM_TEXT] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef POPUP_WINDOW_BLE_CUSTOM_TEXT_Y
            (int16_t)(POPUP_WINDOW_BLE_CUSTOM_TEXT_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_CUSTOM_RESULT_ICON] = {
        .kind = UI_WIDGET_KIND_ICON,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_CUSTOM_RESULT_ICON_Y
            (int16_t)(OTA_CUSTOM_RESULT_ICON_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 0,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_OTA_CUSTOM_RESULT_TITLE] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef OTA_CUSTOM_RESULT_TITLE_Y
            (int16_t)(OTA_CUSTOM_RESULT_TITLE_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
    [UI_W_PUMP_HOME_MAIN_SEAL_INCREASING] = {
        .kind = UI_WIDGET_KIND_TEXT,
        .obj_align = 2,
        .x =
            0
        ,
        .y =
#ifdef PUMP_HOME_MAIN_SEAL_INCREASING_Y
            (int16_t)(PUMP_HOME_MAIN_SEAL_INCREASING_Y)
#else
            0
#endif
        ,
        .opa = 255,
        .text_align = 2,
        .line_space =
            0
        ,
        .font_role = 0xFF,
        .max_w =
            0
        ,
        .color_rgb = 0,
        .flags = UI_STYLE_F_COLOR_INHERIT | UI_STYLE_F_OPA_INHERIT,
    },
};

const int16_t g_ui_layout_default[UI_LAY_COUNT] = {
#ifdef PUMP_PAUSE_TIMEOUT_TITLE_GAP
    [UI_LAY_PUMP_PAUSE_TIMEOUT_TITLE_GAP] = (int16_t)(PUMP_PAUSE_TIMEOUT_TITLE_GAP),
#endif
#ifdef PUMP_FINISH_TIMEOUT_TITLE_GAP
    [UI_LAY_PUMP_FINISH_TIMEOUT_TITLE_GAP] = (int16_t)(PUMP_FINISH_TIMEOUT_TITLE_GAP),
#endif
#ifdef PUMP_WILL_FINISH_TITLE_GAP
    [UI_LAY_PUMP_WILL_FINISH_TITLE_GAP] = (int16_t)(PUMP_WILL_FINISH_TITLE_GAP),
#endif
#ifdef POWER_ON_HOLD_LINE_GAP
    [UI_LAY_POWER_ON_HOLD_LINE_GAP] = (int16_t)(POWER_ON_HOLD_LINE_GAP),
#endif
#ifdef AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y
    [UI_LAY_AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y] = (int16_t)(AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y),
#endif
#ifdef POWER_OFF_TEXT_LOW_POWER_SHIFT_Y
    [UI_LAY_POWER_OFF_TEXT_LOW_POWER_SHIFT_Y] = (int16_t)(POWER_OFF_TEXT_LOW_POWER_SHIFT_Y),
#endif
#ifdef FACTORY_RESET_PROGRESS_TEXT_GAP
    [UI_LAY_FACTORY_RESET_PROGRESS_TEXT_GAP] = (int16_t)(FACTORY_RESET_PROGRESS_TEXT_GAP),
#endif
#ifdef SYS_ABNORMAL_ICON_Y_SYS_ERROR
    [UI_LAY_SYS_ABNORMAL_ICON_Y_SYS_ERROR] = (int16_t)(SYS_ABNORMAL_ICON_Y_SYS_ERROR),
#endif
#ifdef SYS_ABNORMAL_TITLE_Y_SYS_ERROR
    [UI_LAY_SYS_ABNORMAL_TITLE_Y_SYS_ERROR] = (int16_t)(SYS_ABNORMAL_TITLE_Y_SYS_ERROR),
#endif
#ifdef SYS_ABNORMAL_CONTENT_Y_SYS_ERROR
    [UI_LAY_SYS_ABNORMAL_CONTENT_Y_SYS_ERROR] = (int16_t)(SYS_ABNORMAL_CONTENT_Y_SYS_ERROR),
#endif
#ifdef SYS_ABNORMAL_ICON_Y_TEMP_ERROR
    [UI_LAY_SYS_ABNORMAL_ICON_Y_TEMP_ERROR] = (int16_t)(SYS_ABNORMAL_ICON_Y_TEMP_ERROR),
#endif
#ifdef SYS_ABNORMAL_TITLE_Y_TEMP_ERROR
    [UI_LAY_SYS_ABNORMAL_TITLE_Y_TEMP_ERROR] = (int16_t)(SYS_ABNORMAL_TITLE_Y_TEMP_ERROR),
#endif
#ifdef SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR
    [UI_LAY_SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR] = (int16_t)(SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR),
#endif
#ifdef PUMP_HOME_MAIN_TIME_MODE_SWITCH_SHIFT_Y
    [UI_LAY_PUMP_HOME_MAIN_TIME_MODE_SWITCH_SHIFT_Y] = (int16_t)(PUMP_HOME_MAIN_TIME_MODE_SWITCH_SHIFT_Y),
#endif
#ifdef PUMP_HOME_CUSTOM_LEVEL_SHIFT_Y
    [UI_LAY_PUMP_HOME_CUSTOM_LEVEL_SHIFT_Y] = (int16_t)(PUMP_HOME_CUSTOM_LEVEL_SHIFT_Y),
#endif
#ifdef HOME_BOTH_LEVEL_VALUE_W
    [UI_LAY_HOME_BOTH_LEVEL_VALUE_W] = (int16_t)(HOME_BOTH_LEVEL_VALUE_W),
#endif
#ifdef HOME_BOTH_LEVEL_VALUE_H
    [UI_LAY_HOME_BOTH_LEVEL_VALUE_H] = (int16_t)(HOME_BOTH_LEVEL_VALUE_H),
#endif
#ifdef HOME_BOTH_LEVEL_DIR_W
    [UI_LAY_HOME_BOTH_LEVEL_DIR_W] = (int16_t)(HOME_BOTH_LEVEL_DIR_W),
#endif
#ifdef HOME_BOTH_LEVEL_DIR_H
    [UI_LAY_HOME_BOTH_LEVEL_DIR_H] = (int16_t)(HOME_BOTH_LEVEL_DIR_H),
#endif
#ifdef GUIDE_QRCODE_SIZE
    [UI_LAY_GUIDE_QRCODE_SIZE] = (int16_t)(GUIDE_QRCODE_SIZE),
#endif
#ifdef GUIDE_QRCODE_PAD
    [UI_LAY_GUIDE_QRCODE_PAD] = (int16_t)(GUIDE_QRCODE_PAD),
#endif
#ifdef GUIDE_QRCODE_TIP_H
    [UI_LAY_GUIDE_QRCODE_TIP_H] = (int16_t)(GUIDE_QRCODE_TIP_H),
#endif
#ifdef GUIDE_FINISH_QRCODE_SIZE
    [UI_LAY_GUIDE_FINISH_QRCODE_SIZE] = (int16_t)(GUIDE_FINISH_QRCODE_SIZE),
#endif
#ifdef GUIDE_FINISH_QRCODE_PAD
    [UI_LAY_GUIDE_FINISH_QRCODE_PAD] = (int16_t)(GUIDE_FINISH_QRCODE_PAD),
#endif
#ifdef GUIDE_FINISH_TIP_H
    [UI_LAY_GUIDE_FINISH_TIP_H] = (int16_t)(GUIDE_FINISH_TIP_H),
#endif
#ifdef GUIDE_MODE_HOME_TIME_SEG_WIDTH
    [UI_LAY_GUIDE_MODE_HOME_TIME_SEG_WIDTH] = (int16_t)(GUIDE_MODE_HOME_TIME_SEG_WIDTH),
#endif
#ifdef GUIDE_MODE_HOME_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_MODE_HOME_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_MODE_HOME_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_MODE_HOME_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_MODE_HOME_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_MODE_HOME_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_MODE_HOME_LEFT_LEVEL_W
    [UI_LAY_GUIDE_MODE_HOME_LEFT_LEVEL_W] = (int16_t)(GUIDE_MODE_HOME_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_MODE_HOME_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_MODE_HOME_RIGHT_LEVEL_W] = (int16_t)(GUIDE_MODE_HOME_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_MODE_SWITCH_GUIDE_TIP_H
    [UI_LAY_GUIDE_MODE_SWITCH_GUIDE_TIP_H] = (int16_t)(GUIDE_MODE_SWITCH_GUIDE_TIP_H),
#endif
#ifdef GUIDE_MODE_SWITCH_CIRCLE_GAP
    [UI_LAY_GUIDE_MODE_SWITCH_CIRCLE_GAP] = (int16_t)(GUIDE_MODE_SWITCH_CIRCLE_GAP),
#endif
#ifdef GUIDE_MODE_SWITCH_TIP_H
    [UI_LAY_GUIDE_MODE_SWITCH_TIP_H] = (int16_t)(GUIDE_MODE_SWITCH_TIP_H),
#endif
#ifdef GUIDE_PUMP_HOME_TIME_SEG_WIDTH
    [UI_LAY_GUIDE_PUMP_HOME_TIME_SEG_WIDTH] = (int16_t)(GUIDE_PUMP_HOME_TIME_SEG_WIDTH),
#endif
#ifdef GUIDE_PUMP_HOME_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_PUMP_HOME_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_PUMP_HOME_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_PUMP_HOME_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_PUMP_HOME_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_PUMP_HOME_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_PUMP_HOME_LEFT_LEVEL_W
    [UI_LAY_GUIDE_PUMP_HOME_LEFT_LEVEL_W] = (int16_t)(GUIDE_PUMP_HOME_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_PUMP_HOME_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_PUMP_HOME_RIGHT_LEVEL_W] = (int16_t)(GUIDE_PUMP_HOME_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_PUMP_START_TIME_SEG_WIDTH
    [UI_LAY_GUIDE_PUMP_START_TIME_SEG_WIDTH] = (int16_t)(GUIDE_PUMP_START_TIME_SEG_WIDTH),
#endif
#ifdef GUIDE_PUMP_START_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_PUMP_START_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_PUMP_START_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_PUMP_START_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_PUMP_START_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_PUMP_START_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_PUMP_START_LEFT_LEVEL_W
    [UI_LAY_GUIDE_PUMP_START_LEFT_LEVEL_W] = (int16_t)(GUIDE_PUMP_START_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_PUMP_START_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_PUMP_START_RIGHT_LEVEL_W] = (int16_t)(GUIDE_PUMP_START_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_PUMP_PAUSE_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_PUMP_PAUSE_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_PUMP_PAUSE_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_PUMP_PAUSE_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_PUMP_PAUSE_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_PUMP_PAUSE_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_PUMP_PAUSE_LEFT_LEVEL_W
    [UI_LAY_GUIDE_PUMP_PAUSE_LEFT_LEVEL_W] = (int16_t)(GUIDE_PUMP_PAUSE_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_PUMP_PAUSE_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_PUMP_PAUSE_RIGHT_LEVEL_W] = (int16_t)(GUIDE_PUMP_PAUSE_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_SPEED_DONE_TIME_SEG_WIDTH
    [UI_LAY_GUIDE_SPEED_DONE_TIME_SEG_WIDTH] = (int16_t)(GUIDE_SPEED_DONE_TIME_SEG_WIDTH),
#endif
#ifdef GUIDE_SPEED_DONE_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_SPEED_DONE_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_SPEED_DONE_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_SPEED_DONE_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_SPEED_DONE_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_SPEED_DONE_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_SPEED_DONE_LEFT_LEVEL_W
    [UI_LAY_GUIDE_SPEED_DONE_LEFT_LEVEL_W] = (int16_t)(GUIDE_SPEED_DONE_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_SPEED_DONE_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_SPEED_DONE_RIGHT_LEVEL_W] = (int16_t)(GUIDE_SPEED_DONE_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_SPEED_GUIDE_TIP_H
    [UI_LAY_GUIDE_SPEED_GUIDE_TIP_H] = (int16_t)(GUIDE_SPEED_GUIDE_TIP_H),
#endif
#ifdef GUIDE_SPEED_TIP_H
    [UI_LAY_GUIDE_SPEED_TIP_H] = (int16_t)(GUIDE_SPEED_TIP_H),
#endif
#ifdef GUIDE_LEVEL_SIDE_DONE_TIME_SEG_WIDTH
    [UI_LAY_GUIDE_LEVEL_SIDE_DONE_TIME_SEG_WIDTH] = (int16_t)(GUIDE_LEVEL_SIDE_DONE_TIME_SEG_WIDTH),
#endif
#ifdef GUIDE_LEVEL_SIDE_DONE_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_LEVEL_SIDE_DONE_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_LEVEL_SIDE_DONE_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_W
    [UI_LAY_GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_W] = (int16_t)(GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_W] = (int16_t)(GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_LEVEL_SIDE_GUIDE_TIP_H
    [UI_LAY_GUIDE_LEVEL_SIDE_GUIDE_TIP_H] = (int16_t)(GUIDE_LEVEL_SIDE_GUIDE_TIP_H),
#endif
#ifdef GUIDE_LEVEL_SIDE_TIP_H
    [UI_LAY_GUIDE_LEVEL_SIDE_TIP_H] = (int16_t)(GUIDE_LEVEL_SIDE_TIP_H),
#endif
#ifdef GUIDE_LEVEL_BOTH_DONE_TIME_SEG_WIDTH
    [UI_LAY_GUIDE_LEVEL_BOTH_DONE_TIME_SEG_WIDTH] = (int16_t)(GUIDE_LEVEL_BOTH_DONE_TIME_SEG_WIDTH),
#endif
#ifdef GUIDE_LEVEL_BOTH_DONE_SPEED_GROUP_WIDTH
    [UI_LAY_GUIDE_LEVEL_BOTH_DONE_SPEED_GROUP_WIDTH] = (int16_t)(GUIDE_LEVEL_BOTH_DONE_SPEED_GROUP_WIDTH),
#endif
#ifdef GUIDE_LEVEL_BOTH_DONE_SPEED_ICON_VALUE_GAP
    [UI_LAY_GUIDE_LEVEL_BOTH_DONE_SPEED_ICON_VALUE_GAP] = (int16_t)(GUIDE_LEVEL_BOTH_DONE_SPEED_ICON_VALUE_GAP),
#endif
#ifdef GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_W
    [UI_LAY_GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_W] = (int16_t)(GUIDE_LEVEL_BOTH_DONE_LEFT_LEVEL_W),
#endif
#ifdef GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_W
    [UI_LAY_GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_W] = (int16_t)(GUIDE_LEVEL_BOTH_DONE_RIGHT_LEVEL_W),
#endif
#ifdef GUIDE_LEVEL_BOTH_GUIDE_TIP_H
    [UI_LAY_GUIDE_LEVEL_BOTH_GUIDE_TIP_H] = (int16_t)(GUIDE_LEVEL_BOTH_GUIDE_TIP_H),
#endif
#ifdef GUIDE_BOTH_LEVEL_VALUE_W
    [UI_LAY_GUIDE_BOTH_LEVEL_VALUE_W] = (int16_t)(GUIDE_BOTH_LEVEL_VALUE_W),
#endif
#ifdef GUIDE_BOTH_LEVEL_VALUE_H
    [UI_LAY_GUIDE_BOTH_LEVEL_VALUE_H] = (int16_t)(GUIDE_BOTH_LEVEL_VALUE_H),
#endif
#ifdef GUIDE_BOTH_LEVEL_DIR_W
    [UI_LAY_GUIDE_BOTH_LEVEL_DIR_W] = (int16_t)(GUIDE_BOTH_LEVEL_DIR_W),
#endif
#ifdef GUIDE_BOTH_LEVEL_DIR_H
    [UI_LAY_GUIDE_BOTH_LEVEL_DIR_H] = (int16_t)(GUIDE_BOTH_LEVEL_DIR_H),
#endif
#ifdef GUIDE_BOTH_LEVEL_OVER_LIMIT_W
    [UI_LAY_GUIDE_BOTH_LEVEL_OVER_LIMIT_W] = (int16_t)(GUIDE_BOTH_LEVEL_OVER_LIMIT_W),
#endif
#ifdef GUIDE_BOTH_LEVEL_OVER_LIMIT_H
    [UI_LAY_GUIDE_BOTH_LEVEL_OVER_LIMIT_H] = (int16_t)(GUIDE_BOTH_LEVEL_OVER_LIMIT_H),
#endif
#ifdef GUIDE_BOTH_LEVEL_TIP_H
    [UI_LAY_GUIDE_BOTH_LEVEL_TIP_H] = (int16_t)(GUIDE_BOTH_LEVEL_TIP_H),
#endif
#ifdef PUMP_HOME_CUSTOM_TIME_2SEG_SEP_SHIFT_Y
    [UI_LAY_PUMP_HOME_CUSTOM_TIME_2SEG_SEP_SHIFT_Y] = (int16_t)(PUMP_HOME_CUSTOM_TIME_2SEG_SEP_SHIFT_Y),
#endif
#ifdef PUMP_HOME_CUSTOM_TIME_3SEG_SEP_SHIFT_Y
    [UI_LAY_PUMP_HOME_CUSTOM_TIME_3SEG_SEP_SHIFT_Y] = (int16_t)(PUMP_HOME_CUSTOM_TIME_3SEG_SEP_SHIFT_Y),
#endif
#ifdef BATTERY_UI_ICON_GAP
    [UI_LAY_BATTERY_UI_ICON_GAP] = (int16_t)(BATTERY_UI_ICON_GAP),
#endif
#ifdef BATTERY_UI_BLE_ICON_W
    [UI_LAY_BATTERY_UI_BLE_ICON_W] = (int16_t)(BATTERY_UI_BLE_ICON_W),
#endif
#ifdef BATTERY_UI_Y_WITH_PEER
    [UI_LAY_BATTERY_UI_Y_WITH_PEER] = (int16_t)(BATTERY_UI_Y_WITH_PEER),
#endif
#ifdef BATTERY_UI_Y_ALONE
    [UI_LAY_BATTERY_UI_Y_ALONE] = (int16_t)(BATTERY_UI_Y_ALONE),
#endif
#ifdef BATTERY_FRAME_W
    [UI_LAY_BATTERY_FRAME_W] = (int16_t)(BATTERY_FRAME_W),
#endif
#ifdef BATTERY_FRAME_H
    [UI_LAY_BATTERY_FRAME_H] = (int16_t)(BATTERY_FRAME_H),
#endif
#ifdef BATTERY_CAP_H
    [UI_LAY_BATTERY_CAP_H] = (int16_t)(BATTERY_CAP_H),
#endif
#ifdef BATTERY_CAP_X_OFFSET
    [UI_LAY_BATTERY_CAP_X_OFFSET] = (int16_t)(BATTERY_CAP_X_OFFSET),
#endif
#ifdef BATTERY_CAP_Y_OFFSET
    [UI_LAY_BATTERY_CAP_Y_OFFSET] = (int16_t)(BATTERY_CAP_Y_OFFSET),
#endif
#ifdef PUMP_HOME_MAIN_TIME_SEAL_INCREASING_SHIFT_Y
    [UI_LAY_PUMP_HOME_MAIN_TIME_SEAL_INCREASING_SHIFT_Y] = (int16_t)(PUMP_HOME_MAIN_TIME_SEAL_INCREASING_SHIFT_Y),
#endif
#ifdef PUMP_HOME_CUSTOM_TIME_SEAL_INCREASING_SHIFT_Y
    [UI_LAY_PUMP_HOME_CUSTOM_TIME_SEAL_INCREASING_SHIFT_Y] = (int16_t)(PUMP_HOME_CUSTOM_TIME_SEAL_INCREASING_SHIFT_Y),
#endif
};

const lv_font_t *const g_ui_font_default[UI_FONT_ROLE_COUNT] = {
    [UI_FONT_ROLE_HINT] = UI_FONT_HINT_FW,
    [UI_FONT_ROLE_BODY_SM] = UI_FONT_BODY_SM_FW,
    [UI_FONT_ROLE_TITLE] = UI_FONT_TITLE_FW,
    [UI_FONT_ROLE_TITLE_MD] = UI_FONT_TITLE_MD_FW,
    [UI_FONT_ROLE_TITLE_LG] = UI_FONT_TITLE_LG_FW,
    [UI_FONT_ROLE_LATIN_19] = UI_FONT_AEONIK_19_FW,
    [UI_FONT_ROLE_LATIN_26] = UI_FONT_AEONIK_FW,
    [UI_FONT_ROLE_LATIN_29] = UI_FONT_AEONIK_29_FW,
    [UI_FONT_ROLE_NUM_MD] = UI_FONT_NUM_MD_FW,
    [UI_FONT_ROLE_NUM_LG] = UI_FONT_NUM_LG_FW,
};
