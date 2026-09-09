#ifndef __UI_LANG_ZH_LAYOUT_H__
#define __UI_LANG_ZH_LAYOUT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 中文坐标先与英文对齐。字号/行高不同时在这里改 Y / LINE_SPACE，不要改页面。
 */

/*******************************************************************************
 * Shared
 ******************************************************************************/
#define PROGRESS_ARC_TEXT_LINE_SPACE                    (8)

/*******************************************************************************
 * Home
 ******************************************************************************/
#define HOME_MAIN_MODE_TITLE_Y                          (62 + 4)
#define HOME_MAIN_HINT_Y                                (206 + 4)

#define HOME_MAIN_HINT_TEXT_LOW_BAT_LINE_SPACE          (4)
#define HOME_SAVER_HINT_Y                               (88)
#define HOME_MODE_PAGE_STIM_ICON_Y                      (107)
#define HOME_MODE_PAGE_STIM_TITLE_Y                     (213)
#define HOME_MODE_PAGE_STIM_DURATION_Y                  (241)
#define HOME_MODE_PAGE_EXPR_ICON_Y                      (107)
#define HOME_MODE_PAGE_EXPR_TITLE_Y                     (213)
#define HOME_MODE_PAGE_EXPR_DURATION_Y                  (241)
#define HOME_MODE_PAGE_RHYTHM_1L_ICON_Y                 (88)
#define HOME_MODE_PAGE_RHYTHM_1L_TITLE_Y                (194)
#define HOME_MODE_PAGE_RHYTHM_1L_DURATION_Y             (241)
#define HOME_MODE_PAGE_RHYTHM_2L_ICON_Y                 (67)
#define HOME_MODE_PAGE_RHYTHM_2L_TITLE_Y                (173)
#define HOME_MODE_PAGE_RHYTHM_2L_DURATION_Y             (263)
#define HOME_MODE_PAGE_TITLE_MAX_W                      (300)
#define HOME_LEVEL_DIR_Y                                (208)
#define HOME_LEVEL_DIR_LINE_SPACE                       (8)
#define HOME_BOTH_LEVEL_DIR_Y                           (206)
#define HOME_BOTH_LEVEL_OVER_LIMIT_Y                    (60)
#define HOME_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE           (8)
#define HOME_SPEED_LABEL_Y                              (208)

/*******************************************************************************
 * Pump home / adjust
 ******************************************************************************/
#define PUMP_HOME_MAIN_MODE_TITLE_Y                     (62 + 4)
#define PUMP_HOME_MAIN_HINT_Y                           (220)

#define PUMP_HOME_MAIN_SEAL_CHECK_Y                     (208)
#define PUMP_HOME_MAIN_MODE_SWITCH_Y                    (208)
#define PUMP_HOME_MAIN_MODE_SWITCH_LINE_SPACE           (8)
#define PUMP_MODE_PAGE_STIM_ICON_Y                      (107)
#define PUMP_MODE_PAGE_STIM_TITLE_Y                     (213)
#define PUMP_MODE_PAGE_STIM_DURATION_Y                  (241)
#define PUMP_MODE_PAGE_EXPR_ICON_Y                      (107)
#define PUMP_MODE_PAGE_EXPR_TITLE_Y                     (213)
#define PUMP_MODE_PAGE_EXPR_DURATION_Y                  (241)
#define PUMP_MODE_PAGE_RHYTHM_1L_ICON_Y                 (88)
#define PUMP_MODE_PAGE_RHYTHM_1L_TITLE_Y                (194)
#define PUMP_MODE_PAGE_RHYTHM_1L_DURATION_Y             (241)
#define PUMP_MODE_PAGE_RHYTHM_2L_ICON_Y                 (67)
#define PUMP_MODE_PAGE_RHYTHM_2L_TITLE_Y                (173)
#define PUMP_MODE_PAGE_RHYTHM_2L_DURATION_Y             (263)
#define PUMP_MODE_PAGE_TITLE_MAX_W                      (290)
#define PUMP_LEVEL_DIR_Y                                (208)
#define PUMP_LEVEL_DIR_LINE_SPACE                       (8)
#define PUMP_BOTH_LEVEL_DIR_Y                           (206)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_Y                    (60)
#define PUMP_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE           (8)
#define PUMP_BOTH_LEVEL_MODE_SWITCH_Y                   (84)
#define PUMP_SPEED_LABEL_Y                              (208)
#define PUMP_SPEED_LABEL_LINE_SPACE                     (8)
#define PUMP_STOP_HOLD_TITLE_Y                          (200)
#define PUMP_PAUSE_TIMEOUT_TITLE1_Y                     (193 + 4)
#define PUMP_PAUSE_TIMEOUT_TITLE_GAP                    (8)
#define PUMP_FINISH_TIMEOUT_TITLE1_Y                    (193 + 4)
#define PUMP_FINISH_TIMEOUT_TITLE_GAP                   (8)
#define PUMP_WILL_FINISH_ICON_Y                         (90)
#define PUMP_WILL_FINISH_TITLE1_Y                       (192 + 4)
#define PUMP_WILL_FINISH_TITLE_GAP                      (12)
#define PUMP_FINISH_ICON_Y                              (73)
#define PUMP_FINISH_TITLE_Y                             (177)
#define PUMP_FINISH_CONTENT_Y                           (221)
#define PUMP_FINISH_CHARGE_ERROR_ICON_Y                 (90)
#define PUMP_FINISH_CHARGE_ERROR_TITLE1_Y               (197)
#define PUMP_FINISH_CHARGE_ERROR_TITLE2_Y               (238)
#define PUMP_RUNING_30M_TIP_ICON_Y                      (113)
#define PUMP_RUNING_30M_TIP_TITLE_Y                     (182)
#define PUMP_RUNING_30M_TIP_TITLE_LINE_SPACE            (8)
#define PUMP_REST_MODE_ADJUST_TIP_ICON_Y                (112)
#define PUMP_REST_MODE_ADJUST_TIP_TITLE_Y               (182)
#define PUMP_REST_MODE_ADJUST_TIP_TITLE_LINE_SPACE      (8)

/*******************************************************************************
 * Rhythm / settings
 ******************************************************************************/
#define RHYTHM_CONFIRM_ICON_Y                           (90)
#define RHYTHM_CONFIRM_TITLE_Y                          (196)
#define RHYTHM_CONFIRM_SUBTITLE_Y                       (237)
#define SETTINGS_SUCCESS_ICON_Y                         (107)
#define SETTINGS_SUCCESS_TITLE_Y                        (213)
#define SETTINGS_SUCCESS_PROGRAM_ICON_Y                 (90)
#define SETTINGS_SUCCESS_PROGRAM_TITLE_Y                (196)
#define SETTINGS_SUCCESS_PROGRAM_SUBTITLE_Y             (238)

/*******************************************************************************
 * Power / lock / airplane
 ******************************************************************************/
#define POWER_OFF_HOLD_ICON_Y                           (94)
#define POWER_OFF_HOLD_TITLE_Y                          (200)
#define POWER_OFF_HOLD_TITLE_LINE_SPACE                 (8)
#define POWER_ON_HOLD_ICON_Y                            (76)
#define POWER_ON_HOLD_LINE1_Y                           (255)
#define POWER_ON_HOLD_LINE_GAP                          (8)
#define AIRPLANE_MODE_LOCK_ICON_Y                       (48)
#define AIRPLANE_MODE_TITLE_Y                           (154)
#define AIRPLANE_MODE_HINT_Y                            (194)
#define AIRPLANE_MODE_HINT_LINE_SPACE                   (8)
#define AIRPLANE_MODE_HOLDING_CONTENT_SHIFT_Y           (52)
#define AIRPLANE_MODE_STIM_KEY_ICON_X                   (99)
#define AIRPLANE_MODE_STIM_KEY_ICON_Y                   (248)
#define AIRPLANE_MODE_ADD_ICON_X                        (171)
#define AIRPLANE_MODE_ADD_ICON_Y                        (271)
#define AIRPLANE_MODE_EXPR_KEY_ICON_X                   (201)
#define AIRPLANE_MODE_EXPR_KEY_ICON_Y                   (248)
#define TRAVEL_LOCK_TITLE_Y                             (112)
#define TRAVEL_LOCK_TITLE_LINE_SPACE                    (4)
#define TRAVEL_LOCK_STIM_KEY_ICON_X                     AIRPLANE_MODE_STIM_KEY_ICON_X
#define TRAVEL_LOCK_STIM_KEY_ICON_Y                     (186)
#define TRAVEL_LOCK_ADD_ICON_X                          AIRPLANE_MODE_ADD_ICON_X
#define TRAVEL_LOCK_ADD_ICON_Y                          (209)
#define TRAVEL_LOCK_EXPR_KEY_ICON_X                     AIRPLANE_MODE_EXPR_KEY_ICON_X
#define TRAVEL_LOCK_EXPR_KEY_ICON_Y                     (186)
#define LOW_POWER_TITLE_Y                               (111)
#define POWER_OFF_TEXT_LOW_POWER_SHIFT_Y                (19)
#define POWER_OFF_CHARGE_TEXT_Y                         (217)
#define LOW_POWER_TEXT_Y                                (200)

/*******************************************************************************
 * Charge / OTA / factory
 ******************************************************************************/
#define CHARGE_ADAPTER_ICON_Y                           (90)
#define CHARGE_ADAPTER_TITLE_Y                          (196)
#define CHARGE_ADAPTER_SUBTITLE_Y                       (238)
#define OTA_DOWNLOAD_ICON_Y                             (90)
#define OTA_DOWNLOAD_TITLE_Y                            (195)
#define OTA_DOWNLOAD_PERCENT_Y                          (237)
#define OTA_DOWNLOAD_SIZE_Y                             (300)
#define OTA_SUCCESS_ICON_Y                              (107)
#define OTA_SUCCESS_TITLE_Y                             (212)
#define OTA_SUCCESS_LINE_SPACE                          (8)
#define OTA_ERROR_ICON_Y                                (107)
#define OTA_ERROR_TITLE_Y                               (212)
#define OTA_ERROR_LINE_SPACE                            (8)
#define FACTORY_RESET_CONFIRM_ICON_Y                    (90)
#define FACTORY_RESET_CONFIRM_TITLE_Y                   (196)
#define FACTORY_RESET_CONFIRM_HINT_Y                    (238)
#define FACTORY_RESET_PROGRESS_ARC_Y                    (121)
#define FACTORY_RESET_PROGRESS_TEXT_GAP                 (24)

/*******************************************************************************
 * System / BLE popup
 ******************************************************************************/
#define SYS_ABNORMAL_ICON_Y_SYS_ERROR                   (90)
#define SYS_ABNORMAL_TITLE_Y_SYS_ERROR                  (196)
#define SYS_ABNORMAL_CONTENT_Y_SYS_ERROR                (238)
#define SYS_ABNORMAL_ICON_Y_TEMP_ERROR                  (90)
#define SYS_ABNORMAL_TITLE_Y_TEMP_ERROR                 (196)
#define SYS_ABNORMAL_CONTENT_Y_TEMP_ERROR               (238)
#define POPUP_WINDOW_BLE_CONNECT_ICON_Y                 (109)
#define POPUP_WINDOW_BLE_CONNECT_TEXT_Y                 (215)
#define POPUP_WINDOW_BLE_DISCONNECT_ICON_Y              (109)
#define POPUP_WINDOW_BLE_DISCONNECT_TEXT_Y              (215)
#define POPUP_WINDOW_BLE_DISCONNECT_LINE_SPACE          (8)
#define POPUP_WINDOW_BLE_CONNECTING_ICON_Y              (109)
#define POPUP_WINDOW_BLE_CONNECTING_TEXT_Y              (215)

/*******************************************************************************
 * Seal leak (Pro)
 ******************************************************************************/
#define PUMP_SEAL_LEAK_TITLE_Y                          (46)
#define PUMP_SEAL_LEAK_FIRST_TITLE_Y                    (63)
#define PUMP_SEAL_LEAK_DIR_ICON_Y                      (105)
#define PUMP_SEAL_LEAK_DESC_Y                           (296)
#define PUMP_SEAL_LEAK_DESC_LINE_SPACE                  (4)
#define PUMP_SEAL_LEAK_FIRST_DESC_Y                     (314)
#define PUMP_SEAL_LEAK_COMPLETE_DESC_Y                  (314)
#define PUMP_SEAL_LEAK_COMPLETE_TITLE_LINE_SPACE         (8)
#define PUMP_FIT_NOT_DETECTED_TITLE_Y                   (48)
#define PUMP_FIT_NOT_DETECTED_DESC_Y                    (296)
#define PUMP_FIT_NOT_DETECTED_DESC_LINE_SPACE           (4)

/*******************************************************************************
 * Theme guide
 ******************************************************************************/
#define THEME_GUIDE_TITLE_Y                             (51)
#define THEME_GUIDE_DESC_Y                              (87)
#define THEME_GUIDE_LABEL_Y                             (289)

/*******************************************************************************
 * Guide
 ******************************************************************************/
#define GUIDE_INTRO_TITLE_LINE_SPACE                    (8)
#define GUIDE_QRCODE_ALIGN_Y                            (62)
#define GUIDE_QRCODE_DESC_Y                             (204)
#define GUIDE_QRCODE_DESC_LINE_SPACE                    (8)
#define GUIDE_FINISH_QRCODE_ALIGN_Y                     (62)
#define GUIDE_FINISH_DESC_Y                             (212)
#define GUIDE_FINISH_DESC_LINE_SPACE                    (8)
#define GUIDE_MODE_HOME_MODE_TITLE_Y                    (62 + 4)

#define GUIDE_MODE_SWITCH_ICON_Y                        (88)
#define GUIDE_MODE_SWITCH_TITLE_Y                       (196)
#define GUIDE_MODE_SWITCH_DURATION_Y                    (241)
#define GUIDE_MODE_SWITCH_GUIDE_TITLE_Y                 (61)
#define GUIDE_MODE_SWITCH_GUIDE_TITLE_LINE_SPACE        (8)
#define GUIDE_MODE_SWITCH_GUIDE_ICON_Y                  (143)

#define GUIDE_PUMP_HOME_MODE_TITLE_Y                    (62 + 4)
#define GUIDE_PUMP_START_MODE_TITLE_Y                   (62 + 4)
#define GUIDE_PUMP_PAUSE_MODE_TITLE_Y                   (62 + 4)

#define GUIDE_PUMP_START_GUIDE_TITLE_Y                  (96)
#define GUIDE_PUMP_START_GUIDE_TITLE_LINE_SPACE         (8)
#define GUIDE_PUMP_START_GUIDE_ICON_Y                   (152)

#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_Y                  (96)
#define GUIDE_PUMP_PAUSE_GUIDE_TITLE_LINE_SPACE         (8)
#define GUIDE_PUMP_PAUSE_GUIDE_ICON_Y                   (152)

#define GUIDE_PUMP_FINISH_GUIDE_TITLE_Y                 (79)
#define GUIDE_PUMP_FINISH_GUIDE_TITLE_LINE_SPACE        (8)

#define GUIDE_SPEED_LABEL_Y                             (208)
#define GUIDE_SPEED_DONE_MODE_TITLE_Y                   (62 + 4)
#define GUIDE_SPEED_GUIDE_PRESS_X                       (116)
#define GUIDE_SPEED_GUIDE_PRESS_Y                       (146 + 2)
#define GUIDE_SPEED_GUIDE_PRESS_LINE_SPACE              (4)
#define GUIDE_SPEED_GUIDE_ADJUST_Y                      (199 + 2)
#define GUIDE_SPEED_GUIDE_ADJUST_LINE_SPACE             (8)
#define GUIDE_SPEED_GUIDE_ICON_X                        (181)
#define GUIDE_SPEED_GUIDE_ICON_Y                        (130)

#define GUIDE_LEVEL_SIDE_DIR_Y                          (208)
#define GUIDE_LEVEL_SIDE_DIR_LINE_SPACE                 (8)
#define GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_Y              (62 + 4)
#define GUIDE_LEVEL_SIDE_GUIDE_PRESS_Y                  (95 + 2)
#define GUIDE_LEVEL_SIDE_GUIDE_OR_Y                     (146 + 2)
#define GUIDE_LEVEL_SIDE_GUIDE_ADJUST_Y                 (202 + 2)
#define GUIDE_LEVEL_SIDE_GUIDE_TEXT_LINE_SPACE          (8)

#define GUIDE_BOTH_LEVEL_DIR_Y                          (206)
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_Y                   (60)
#define GUIDE_BOTH_LEVEL_OVER_LIMIT_LINE_SPACE          (8)
#define GUIDE_LEVEL_BOTH_DONE_MODE_TITLE_Y              (62 + 4)
#define GUIDE_LEVEL_BOTH_GUIDE_PRESS_X                  (49)
#define GUIDE_LEVEL_BOTH_GUIDE_PRESS_Y                  (146)
#define GUIDE_LEVEL_BOTH_GUIDE_PRESS_LINE_SPACE         (4)
#define GUIDE_LEVEL_BOTH_GUIDE_AND_X                    (225)
#define GUIDE_LEVEL_BOTH_GUIDE_AND_Y                    (146)
#define GUIDE_LEVEL_BOTH_GUIDE_AND_LINE_SPACE           (4)
#define GUIDE_LEVEL_BOTH_GUIDE_ADJUST_Y                 (191)
#define GUIDE_LEVEL_BOTH_GUIDE_ADJUST_LINE_SPACE        (8)
#define GUIDE_LEVEL_BOTH_GUIDE_LEFT_ICON_Y              (137)
#define GUIDE_LEVEL_BOTH_GUIDE_RIGHT_ICON_Y             (137)

#ifdef __cplusplus
}
#endif
#endif // __UI_LANG_ZH_LAYOUT_H__
