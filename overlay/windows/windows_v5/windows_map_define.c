/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" // IWYU pragma: keep
#include "windows_map_define.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define WINDOW_PRIO_DUMMY             WINDOW_PRIO_LOW
#define WINDOW_PRIO_SETTING           WINDOW_PRIO_LOW
#define WINDOW_PRIO_PUMP              WINDOW_PRIO_LEVEL2
#define WINDOW_PRIO_HOME              WINDOW_PRIO_LEVEL8
#define WINDOW_PRIO_GUIDE             WINDOW_PRIO_LEVEL9
#define WINDOW_PRIO_LOGO              WINDOW_PRIO_HIGH
#define WINDOW_PRIO_BATTERY           WINDOW_PRIO_HIGH
#define WINDOW_PRIO_POPUP             WINDOW_PRIO_HIGH
#define WINDOW_PRIO_AIRPLANE_MODE     WINDOW_PRIO_HIGH
#define WINDOW_PRIO_CHARGE            WINDOW_PRIO_HIGH
#define WINDOW_PRIO_OTA               WINDOW_PRIO_HIGH
#define WINDOW_PRIO_PRODUCT_TS        WINDOW_PRIO_TOP
#define WINDOW_PRIO_FACTORY_RESET     WINDOW_PRIO_TOP
#define WINDOW_PRIO_POWER_OFF         WINDOW_PRIO_TOP
#define WINDOW_PRIO_SYSTEM_ABNORMAL   WINDOW_PRIO_TOP
#define WINDOW_PRIO_THEME_GUIDE       WINDOW_PRIO_GUIDE

DECLARE_WM_HANDLE_OBJ(m_window_dummy);
DECLARE_WM_HANDLE_OBJ(m_window_setting);
DECLARE_WM_HANDLE_OBJ(m_window_pump);
DECLARE_WM_HANDLE_OBJ(m_window_home);
DECLARE_WM_HANDLE_OBJ(m_window_guide);
DECLARE_WM_HANDLE_OBJ(m_window_logo);
DECLARE_WM_HANDLE_OBJ(m_window_battery);
DECLARE_WM_HANDLE_OBJ(m_window_popup);
DECLARE_WM_HANDLE_OBJ(m_window_airplane_mode);
DECLARE_WM_HANDLE_OBJ(m_window_charge);
DECLARE_WM_HANDLE_OBJ(m_window_ota);
DECLARE_WM_HANDLE_OBJ(m_window_product_test);
DECLARE_WM_HANDLE_OBJ(m_window_factory_reset);
DECLARE_WM_HANDLE_OBJ(m_window_power_off);
DECLARE_WM_HANDLE_OBJ(m_window_system_abnormal);
DECLARE_WM_HANDLE_OBJ(m_window_theme_guide);

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const window_life_cycle_t m_ui_screen_null = {
    .on_create = NULL,
    .on_resume = NULL,
    .on_pause = NULL, 
    .on_destory = NULL,
};

/**
* 注意: 新增窗口处理集合, 请按window_id顺序排列，便于二分法查找.
*/
static const window_context_handle_t g_process_handle_list[] = {
    {ID_WINDOW_DUMMY, WINDOW_PRIO_DUMMY, PAGE_HANDLE_NAME(dummy), REFS_T(m_window_dummy)},
    // {ID_WINDOW_SETTINGS,            WINDOW_PRIO_SETTING,    PAGE_HANDLE_NAME(setting),  REFS_T(m_window_setting)},
    {ID_WINDOW_PUMP, WINDOW_PRIO_PUMP, PAGE_HANDLE_NAME(pump), REFS_T(m_window_pump)},
    {ID_WINDOW_HOME, WINDOW_PRIO_HOME, PAGE_HANDLE_NAME(home), REFS_T(m_window_home)},
    {ID_WINDOW_GUIDE,               WINDOW_PRIO_GUIDE,      PAGE_HANDLE_NAME(guide),    REFS_T(m_window_guide)},
    {ID_WINDOW_LOGO, WINDOW_PRIO_LOGO, PAGE_HANDLE_NAME(logo), REFS_T(m_window_logo)},
    {ID_WINDOW_BATTERY,             WINDOW_PRIO_BATTERY,    PAGE_HANDLE_NAME(battery ), REFS_T(m_window_battery)},
    {ID_WINDOW_POPUP_WINDOW,        WINDOW_PRIO_POPUP,      PAGE_HANDLE_NAME(popup_window),    REFS_T(m_window_popup)},
    {ID_WINDOW_AIRPLANE_MODE, WINDOW_PRIO_AIRPLANE_MODE, PAGE_HANDLE_NAME(airplane_mode), REFS_T(m_window_airplane_mode)},
    {ID_WINDOW_CHARGE, WINDOW_PRIO_CHARGE, PAGE_HANDLE_NAME(charge), REFS_T(m_window_charge)},
    {ID_WINDOW_OTA_UPGRADE,         WINDOW_PRIO_OTA,        PAGE_HANDLE_NAME(ota),      REFS_T(m_window_ota)},
    {ID_WINDOW_PRODUCT_TEST_MODE,  WINDOW_PRIO_PRODUCT_TS, PAGE_HANDLE_NAME(product_test), REFS_T(m_window_product_test)},
    {ID_WINDOW_FACTORY_RESET, WINDOW_PRIO_FACTORY_RESET, PAGE_HANDLE_NAME(factory_reset), REFS_T(m_window_factory_reset)},
    {ID_WINDOW_POWER_OFF, WINDOW_PRIO_POWER_OFF, PAGE_HANDLE_NAME(power_off), REFS_T(m_window_power_off)},
    {ID_WINDOW_SYSTEM_ABNORMAL, WINDOW_PRIO_SYSTEM_ABNORMAL, PAGE_HANDLE_NAME(system_abnormal), REFS_T(m_window_system_abnormal)},
    {ID_WINDOW_THEME_GUIDE, WINDOW_PRIO_THEME_GUIDE, PAGE_HANDLE_NAME(theme_guide), REFS_T(m_window_theme_guide)},
};

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
const char *to_window_name(uint8_t window_id) 
{
    switch (window_id) {
        case ID_WINDOW_DUMMY:
            return "dummy";
        case ID_WINDOW_SETTINGS:
            return "setting";
        case ID_WINDOW_PUMP:
            return "breast_pump";
        case ID_WINDOW_GUIDE:
            return "guide";
        case ID_WINDOW_HOME:
            return "home";
        case ID_WINDOW_LOGO:
            return "logo";
        case ID_WINDOW_BATTERY:
            return "battery";
        case ID_WINDOW_POPUP_WINDOW:
            return "popup_window";
        case ID_WINDOW_AIRPLANE_MODE:
            return "airplane_mode";
        case ID_WINDOW_CHARGE:
            return "charging";
        case ID_WINDOW_OTA_UPGRADE:
            return "ota_upgrade";
        case ID_WINDOW_PRODUCT_TEST_MODE:
            return "product_test";
        case ID_WINDOW_FACTORY_RESET:
            return "factory_reset";
        case ID_WINDOW_POWER_OFF:
            return "power_off";
        case ID_WINDOW_SYSTEM_ABNORMAL:
            return "system_abnormal";
        case ID_WINDOW_THEME_GUIDE:
            return "theme_guide";
        default:
            return "unknow";
    }
}

void get_window_state_hdr_list(window_context_handle_t **list, uint16_t *list_len)
{
    *list = (window_context_handle_t *)g_process_handle_list;
    *list_len = LENGTH_OF_ARRAY(g_process_handle_list);
}
