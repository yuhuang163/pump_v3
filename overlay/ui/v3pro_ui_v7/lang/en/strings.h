#ifndef __UI_LANG_EN_STRINGS_H__
#define __UI_LANG_EN_STRINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Shared
 ******************************************************************************/
#define UI_STR_MODE_STIMULATION                         "Stimulation"
#define UI_STR_MODE_EXPRESSION                          "Expression"
#define UI_STR_MODE_MIXED                               "Mixed"
#define UI_STR_MODE_MILK_BOOST                          "Milk Boost"
#define UI_STR_MODE_POWER_PUMPING                       "Power Pumping"
#define UI_STR_MODE_CUSTOM                              "Customize"
#define UI_STR_MODE_COZY_FLOW                           "Cozy Flow"
#define UI_STR_MODE_DENSE_MILK                          "Power Pumping"
#define UI_STR_MODE_HIGH_EFFICIENCY                     "Milk Boost"
#define UI_STR_MODE_LACTATION_STARTER                   "Lactation Starter"
#define UI_STR_MODE_DEEP_EMPTYING                       "Deep Emptying"
#define UI_STR_MODE_LET_DOWN_RESPONSE                   "Let-down Response"
#define UI_STR_MODE_BEFORE_BED                          "Before Bed"
#define UI_STR_MODE_QUICK_REFILL                        "Quick Refill Relief"
#define UI_STR_MODE_POST_NURSING                        "Post-Nursing"
#define UI_STR_MODE_RELAX                               "Relax"
#define UI_STR_SIDE_L                                   "L"
#define UI_STR_SIDE_R                                   "R"
#define UI_STR_SPEED                                    "Speed"
#define UI_STR_SWITCH_TO_FMT                            "Switch to %s"
#define UI_STR_DURATION_APPROX_MIN_FMT                  "≈ %dmin"
#define UI_STR_COUNTDOWN_SEC_FMT                        "%u s"
#define UI_STR_LEVEL_OVER_LIMIT                         "For safety,\nUp to 3 levels at a time."

/*******************************************************************************
 * Home
 ******************************************************************************/
#define HOME_MAIN_HINT_TEXT                             "Press to start pumping"
#define HOME_MAIN_HINT_TEXT_LOW_BAT                     "Low battery, please charge."
#define HOME_MAIN_HINT_TEXT_FORBID_START                "Battery too low to pump"
#define HOME_SAVER_HINT_TEXT                            "Press anywhere to wake up"
#define HOME_LEVEL_DIR_LEFT_TEXT                        "Left Level"
#define HOME_LEVEL_DIR_RIGHT_TEXT                       "Right Level"
#define HOME_LEVEL_OVER_LIMIT_TEXT                      UI_STR_LEVEL_OVER_LIMIT
#define HOME_LEVEL_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define HOME_BOTH_LEVEL_DIR_LEFT_TEXT                   UI_STR_SIDE_L
#define HOME_BOTH_LEVEL_DIR_RIGHT_TEXT                  UI_STR_SIDE_R
#define HOME_BOTH_LEVEL_OVER_LIMIT_TEXT                 UI_STR_LEVEL_OVER_LIMIT
#define HOME_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT            UI_STR_SWITCH_TO_FMT
#define HOME_SPEED_LABEL_TEXT                           UI_STR_SPEED
#define HOME_SPEED_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define HOME_LOW_POWER_PAGE_TEXT                        "Battery too low\nCannot start pumping"

/*******************************************************************************
 * Pump home / adjust
 ******************************************************************************/
#define PUMP_HOME_MAIN_HINT_TEXT                        "Press to resume"
#define PUMP_HOME_MAIN_SEAL_CHECKING_TEXT               "Checking seal..."
#define PUMP_HOME_MAIN_SEAL_RESULT_TEXT                 "Seal check passed"
#define PUMP_HOME_MAIN_MODE_SWITCH_TEXT_FMT             "Switched to %s.\nLevel and speed may changed."
#define PUMP_LEVEL_DIR_LEFT_TEXT                        "Left Level"
#define PUMP_LEVEL_DIR_RIGHT_TEXT                       "Right Level"
#define PUMP_LEVEL_OVER_LIMIT_TEXT                      UI_STR_LEVEL_OVER_LIMIT
#define PUMP_LEVEL_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define PUMP_BOTH_LEVEL_DIR_LEFT_TEXT                   UI_STR_SIDE_L
#define PUMP_BOTH_LEVEL_DIR_RIGHT_TEXT                  UI_STR_SIDE_R
#define PUMP_BOTH_LEVEL_OVER_LIMIT_TEXT                 UI_STR_LEVEL_OVER_LIMIT
#define PUMP_BOTH_LEVEL_MODE_SWITCH_TEXT_FMT            UI_STR_SWITCH_TO_FMT
#define PUMP_SPEED_LABEL_TEXT                           UI_STR_SPEED
#define PUMP_SPEED_MODE_SWITCH_TEXT_FMT                 UI_STR_SWITCH_TO_FMT
#define PUMP_STOP_HOLD_TITLE_TEXT                       "Press and hold to\nend pumping."
#define PUMP_PAUSE_TIMEOUT_TITLE1_TEXT                  "Pumping Ended"
#define PUMP_PAUSE_TIMEOUT_TITLE2_TEXT                  "Automatically\nending..."
#define PUMP_FINISH_TIMEOUT_TITLE1_TEXT                 "Pumping Ended"
#define PUMP_FINISH_TIMEOUT_TITLE2_TEXT                 "Automatically\nending..."
#define PUMP_WILL_FINISH_TITLE1_TEXT                    "Your pumping program is\nabout to end."
#define PUMP_WILL_FINISH_TITLE2_TEXT                    "Press the knob to\ncontinue pumping."
#define PUMP_FINISH_TITLE_TEXT                          "Pumping complete."
#define PUMP_FINISH_CONTENT_TEXT                        "Open the App to log your\nmilk output."
#define PUMP_FINISH_CHARGE_ERROR_TITLE1_TEXT            "Incompatible adapter"
#define PUMP_FINISH_CHARGE_ERROR_TITLE2_TEXT            "Ending automatically"
#define PUMP_RUNING_30M_TIP_TEXT                        "Pumping over 30 min.\nTake a short break."
#define PUMP_REST_MODE_ADJUST_TIP_TEXT                  "In rest mode\nCannot adjust."

/*******************************************************************************
 * Rhythm / settings
 ******************************************************************************/
#define RHYTHM_CONFIRM_TITLE_TEXT                       "Switch program?"
#define RHYTHM_CONFIRM_SUBTITLE_TEXT                    "The timer will reset"
#define SETTINGS_SUCCESS_TITLE_TEXT                     "Success"
#define SETTINGS_SUCCESS_PROGRAM_TITLE_TEXT             "Program switched"
#define SETTINGS_SUCCESS_PROGRAM_SUBTITLE_TEXT          "Timer reset"

/*******************************************************************************
 * Power / lock / airplane
 ******************************************************************************/
#define POWER_OFF_HOLD_TITLE_TEXT                       "Press & hold"
#define POWER_OFF_TEXT_POWERING_OFF_STR                 "Powering off..."
#define POWER_OFF_TEXT_AUTO_SHUT_OFF_STR                "Auto Powering Off..."
#define POWER_OFF_TEXT_LOW_POWER_STR                    "Battery to low to\npower off..."
#define RESTARTING_TEXT                                 "Restarting..."
#define POWER_OFF_CHARGE_TEXT                           "Charging..."
#define LOW_POWER_PAGE_TEXT                             "Battery too low\nConnect to a charger"
#define POWER_ON_HOLD_LINE1_TEXT                        "Press & hold the"
#define POWER_ON_HOLD_LINE2_TEXT                        "knob to power on"
#define AIRPLANE_MODE_TITLE_TEXT                        "Device locked"
#define AIRPLANE_MODE_HINT_TEXT                         "Press & hold to unlock"
#define TRAVEL_LOCK_TITLE_TEXT                          "Press & hold\nto lock device"
#define LOW_POWER_POPUP_TITLE_TEXT                      "Powering off in"

/*******************************************************************************
 * Charge / OTA / factory
 ******************************************************************************/
#define CHARGE_ADAPTER_TITLE_TEXT                       "Incompatible adapter"
#define CHARGE_ADAPTER_SUBTITLE_TEXT                    "Unable to charge"
#define OTA_RES_UPGRADING_TEXT                          "Resource upgrading"
#define OTA_FW_UPGRADING_TEXT                           "Firmware upgrading"
#define OTA_HW_UPGRADING_TEXT                           "Hardware upgrading"
#define OTA_UPGRADE_FAILED_TEXT                         "Upgrade failed"
#define OTA_UPGRADE_COMPLETED_TEXT                      "Upgrade completed"
#define FACTORY_RESET_CONFIRM_TITLE_TEXT                "Reset settings"
#define FACTORY_RESET_CONFIRM_HINT_TEXT                 "Press & hold knob"
#define FACTORY_RESET_PROGRESS_TEXT                     "Reset settings..."

/*******************************************************************************
 * System / BLE popup
 ******************************************************************************/
#define SYS_ERROR_TITLE_TEXT                            "System error"
#define SYS_ERROR_REPLUG_TEXT                           "Press & hold knob 10s\nto restart"
#define TEMP_ERROR_TITLE_TEXT                           "High battery\ntemperature"
#define TEMP_ERROR_CONTENT_TEXT                         "Device suspended"
#define BLE_CONNECT_TEXT                                "Device Connected"
#define BLE_DISCONNECT_TEXT                             "Device disconnected"
#define BLE_CONNECTING_TEXT                             "Device Connecting"

/*******************************************************************************
 * Theme guide
 ******************************************************************************/
#define THEME_GUIDE_TITLE_TEXT                          "System settings"
#define THEME_GUIDE_DESC_SELECT_TEXT                    "Turn the knob to select style"
#define THEME_GUIDE_DESC_CONFIRM_TEXT                   "Press          to confirm"
#define THEME_GUIDE_OPTION_MINIMAL_TEXT                 "Minimal"
#define THEME_GUIDE_OPTION_IP_TEXT                      "IP"

/*******************************************************************************
 * Guide
 ******************************************************************************/
#define GUIDE_INTRO_TITLE_TEXT                          "Let's get started!"
#define GUIDE_QRCODE_DESC_TEXT                          "Scan the QR code to\ndownload the app and\nconnect your pump."
#define GUIDE_FINISH_QRCODE_TEXT                        "https://support.momcozy.com/product/bpv-3"
#define GUIDE_FINISH_DESC_TEXT                          "Tutorial complete. Scan the\nQR code anytime to view it\nagain."
#define GUIDE_MODE_SWITCH_GUIDE_TITLE_TEXT              "Turn the knob to\nexplore more options."
#define GUIDE_PUMP_START_GUIDE_TITLE_TEXT               "Press the knob to\nstart."
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_TEXT               "Press the knob to\npause."
#define GUIDE_PUMP_FINISH_GUIDE_TITLE_TEXT              "Press and hold the\nknob to end pumping."
#define GUIDE_PUMP_HOME_MODE_TITLE_TEXT                 UI_STR_MODE_STIMULATION
#define GUIDE_PUMP_START_MODE_TITLE_TEXT                UI_STR_MODE_STIMULATION
#define GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT                UI_STR_MODE_STIMULATION
#define GUIDE_SPEED_LABEL_TEXT                          UI_STR_SPEED
#define GUIDE_SPEED_GUIDE_PRESS_TEXT                    "Press"
#define GUIDE_SPEED_GUIDE_ADJUST_TEXT                   "to adjust cycle speed."
#define GUIDE_LEVEL_SIDE_GUIDE_PRESS_TEXT               "Press"
#define GUIDE_LEVEL_SIDE_GUIDE_OR_TEXT                  "or"
#define GUIDE_LEVEL_SIDE_GUIDE_ADJUST_TEXT              "to adjust left/right\nsuction."
#define GUIDE_LEVEL_BOTH_GUIDE_PRESS_TEXT               "Press"
#define GUIDE_LEVEL_BOTH_GUIDE_AND_TEXT                 "and"
#define GUIDE_LEVEL_BOTH_GUIDE_ADJUST_TEXT              "together to adjust\nboth sides."
#define GUIDE_LEVEL_SIDE_DIR_LEFT_TEXT                  "Left Level"
#define GUIDE_LEVEL_SIDE_DIR_RIGHT_TEXT                 "Right Level"
#define GUIDE_LEVEL_SIDE_OVER_LIMIT_TEXT                UI_STR_LEVEL_OVER_LIMIT
#define GUIDE_BOTH_LEVEL_DIR_LEFT_TEXT                  UI_STR_SIDE_L
#define GUIDE_BOTH_LEVEL_DIR_RIGHT_TEXT                 UI_STR_SIDE_R
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_TEXT                UI_STR_LEVEL_OVER_LIMIT

/*******************************************************************************
 * Seal leak (Pro)
 ******************************************************************************/
#define PUMP_SEAL_LEAK_TITLE_AIR_SEAL                   "Check Air Seal"
#define PUMP_SEAL_LEAK_TITLE_FIT                        "Check Fit"
#define PUMP_SEAL_LEAK_TITLE_CUP                        "Check Cup"
#define PUMP_SEAL_LEAK_TITLE_COMPLETE                   "Self-Check\nComplete"
#define PUMP_SEAL_LEAK_DESC_TUBING                      "Reinsert tubing firmly"
#define PUMP_SEAL_LEAK_DESC_FLANGE                      "Center the nipple\nSeal flange firmly"
#define PUMP_SEAL_LEAK_DESC_CUP                         "Snap rim shut\nSeat duckbill valve"
#define PUMP_SEAL_LEAK_DESC_COMPLETE                    "Press to resume"
#define PUMP_FIT_NOT_DETECTED_TITLE                     "Fit Not Detected"
#define PUMP_FIT_NOT_DETECTED_DESC                      "Check cup placement\nPress to resume"

/*******************************************************************************
 * Product tests
 ******************************************************************************/
#define PRODUCT_TESTS_SCAN_QR_TEXT                      "Please scan the QR code\nto quickly connect the device"
#define PRODUCT_TESTS_HOME_BATTERY_FMT                  "battery: %d%%"
#define PRODUCT_TESTS_BATTERY_FMT                       "battery-%d%%"
#define PRODUCT_TESTS_AGING_TIME_FMT                    "aging-time-%02luh-%02lum-%02lus"
#define PRODUCT_TESTS_AGING_COMPLETE_TEXT               "aging-complete"
#define PRODUCT_TESTS_AGING_RUNNING_TEXT                "aging-running"

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_EN_STRINGS_H__
