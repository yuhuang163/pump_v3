/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "hal_pwm.h" // IWYU pragma: keep
#include "hal_timer.h" // IWYU pragma: keep
#include "hwtimer_list.h" // IWYU pragma: keep
#include "bsp_common.h"  // IWYU pragma: keep
#include "bsp_config.h"  // IWYU pragma: keep
#include "bsp_lcd_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#if IS_SUPPORT_ADAPTIVE_DIMMING
#define IS_SUPPORT_LCD_BACKLIGHT_CLIMB            (1)  /// 支持爬升
#else
#define IS_SUPPORT_LCD_BACKLIGHT_CLIMB            (0)  /// 支持爬升
#endif

#define LCD_BACKLIGHT_PERIOD                      1000
#define LCD_BACKLIGHT_TIME_PERIOD                 25
#define LCD_BACKLIGHT_LOG_DEBUG                   (1)

typedef struct {
    HWTIMER_ID time_id;
    uint8_t is_running;   /// 定时器状态，0：停止，1：运行
    uint8_t current_duty;   /// 当前占空比
    uint8_t target_duty;   /// 当前占空比
    uint8_t onoff;   /// 开关状态，0：关，1：开
    uint8_t duty_update_flag;    /// 占空比更新标志，0：不更新，1：更新    
    uint32_t pulse_width;   /// 当前脉冲宽度
} backlight_timer_cfg_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static backlight_timer_cfg_t backlight_info;
static const uint16_t lcd_backlight_pwm_table[101] = {
    0, 0, 2, 5, 9, 14, 21, 29, 39, 51,
    65, 80, 96, 115, 135, 157, 181, 207, 235, 265,
    296, 330, 366, 403, 443, 484, 528, 574, 621, 671,
    723, 777, 834, 892, 952, 1015, 1080, 1147, 1217, 1288,
    1362, 1438, 1516, 1597, 1680, 1765, 1852, 1942, 2034, 2128,
    2225, 2324, 2426, 2529, 2636, 2744, 2855, 2968, 3084, 3202,
    3323, 3446, 3572, 3700, 3830, 3963, 4098, 4236, 4376, 4519,
    4665, 4813, 4963, 5116, 5271, 5429, 5590, 5753, 5918, 6087,
    6257, 6431, 6607, 6785, 6967, 7150, 7337, 7526, 7717, 7912,
    8108, 8308, 8510, 8715, 8922, 9133, 9345, 9561, 9779, 10000,
    10000
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LCD_BACKLIGHT_CLIMB
static void lcd_backlight_gpio_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    if (backlight_info.duty_update_flag == 1) {
        backlight_info.duty_update_flag = 0;
        if (backlight_info.pulse_width == 0) {
            backlight_info.onoff = 0;
#if LCD_BACKLIGHT_LOG_DEBUG
            LOG_PBUS("lcd backlight off...");
#endif
        }
        hal_pwm_update_pulse_width(LCD_LCD_BL_PWM_ID, backlight_info.pulse_width, LCD_BACKLIGHT_PERIOD);
    }
}

static void bsp_lcd_backlight_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T gpio_irq_cfg = {
        enable,
        0, 
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
        lcd_backlight_gpio_irq_callback
    };
    hal_gpio_setup_irq(LCD_LCD_BL_PIN, &gpio_irq_cfg);
}

static void bsp_lcd_backlight_climb_start(void)
{
    int ret = 0;
    if (hwtimer_active(backlight_info.time_id) == 0) {
        ret = hwtimer_start(backlight_info.time_id, MS_TO_HWTICKS(LCD_BACKLIGHT_TIME_PERIOD));
        if (ret != E_HWTIMER_OK) {
            LOGE("lcd backlight time start fail ret=%d", ret);
            return;
        }
    }
    backlight_info.is_running = true;
}

static void bsp_lcd_backlight_climb_stop(void)
{
    int ret = hwtimer_stop(backlight_info.time_id);

    if (ret != E_HWTIMER_OK && ret != E_HWTIMER_IN_CALLBACK) {
        LOGE("lcd backlight time stop fail ret=%d", ret);
    }
    backlight_info.is_running = false; 
}

static void bsp_lcd_backlight_timer_handler(void *param)
{
    if (backlight_info.current_duty > backlight_info.target_duty && backlight_info.current_duty > 0) {
        backlight_info.current_duty--;
        backlight_info.pulse_width = lcd_backlight_pwm_table[backlight_info.current_duty];
        backlight_info.duty_update_flag = 1;
        hwtimer_start(backlight_info.time_id, MS_TO_HWTICKS(LCD_BACKLIGHT_TIME_PERIOD));
    } else if (backlight_info.current_duty < backlight_info.target_duty && backlight_info.current_duty < 100) {
        backlight_info.current_duty++;
        backlight_info.pulse_width = lcd_backlight_pwm_table[backlight_info.current_duty];
        backlight_info.duty_update_flag = 1;
        hwtimer_start(backlight_info.time_id, MS_TO_HWTICKS(LCD_BACKLIGHT_TIME_PERIOD));
    } else {
#if LCD_BACKLIGHT_LOG_DEBUG
        LOG_PBUS("time is over, current_duty: %d, target_duty: %d", 
            backlight_info.current_duty, backlight_info.target_duty);
#endif
    }
}

static int8_t bsp_lcd_backlight_timer_register(void)
{
    backlight_info.time_id = hwtimer_alloc(bsp_lcd_backlight_timer_handler, NULL);
    if (backlight_info.time_id == NULL) {
        LOGE("register lcd backlight timer failed");
        return -1;
    }
    backlight_info.is_running = false;
    return 0;
}

static void lcd_backlight_level_set(uint16_t level)
{    
    if (backlight_info.onoff == 0 && level > 0) {
        struct HAL_PWM_CFG_T pwm_cfg = {
            .freq = LCD_BACKLIGHT_PERIOD,
            .ratio = 2,
            .inv = false,
            .sleep_on = false,
        };
        backlight_info.onoff = 1;
        backlight_info.current_duty = 2; /// 2 才亮灯
        backlight_info.pulse_width = lcd_backlight_pwm_table[backlight_info.current_duty];
        hal_pwm_enable(LCD_LCD_BL_PWM_ID, &pwm_cfg);
        bsp_lcd_backlight_irq_control(true);
        LOG_PBUS("lcd backlight on");
    } 
    
    uint8_t target_duty = 0;
    if (level > LCD_BACKLIGHT_LEVEL_MAX) {
        target_duty = 100;
    } else {
        target_duty = ((level * 100) / LCD_BACKLIGHT_LEVEL_MAX);
    }
    if (backlight_info.target_duty != target_duty) {
        backlight_info.target_duty = target_duty;
        bsp_lcd_backlight_climb_start();
        LOG_PBUS("duty: %d, level: %d", backlight_info.target_duty, backlight_info.current_duty);
    }
}
#else
static void lcd_backlight_level_set(uint16_t level)
{
    if (level == 0) {
        backlight_info.onoff = 0;
        backlight_info.pulse_width = 0;
        backlight_info.current_duty = 0;
        hal_pwm_disable(LCD_LCD_BL_PWM_ID);
        return;
    } 

    if (backlight_info.onoff == 0) {
        struct HAL_PWM_CFG_T pwm_cfg = {
            .freq = LCD_BACKLIGHT_PERIOD,
            .ratio = 2,
            .inv = false,
            .sleep_on = false,
        };
        backlight_info.onoff = 1;
        backlight_info.current_duty = 2; /// 2 才亮灯
        hal_pwm_enable(LCD_LCD_BL_PWM_ID, &pwm_cfg);
        LOG_PBUS("lcd backlight on");
    }
    
    if (backlight_info.pulse_width != level) {
        backlight_info.pulse_width = level;
        hal_pwm_update_pulse_width(LCD_LCD_BL_PWM_ID, backlight_info.pulse_width, LCD_BACKLIGHT_PERIOD);
    }
    LOG_PBUS("duty: %d, level: %d", backlight_info.pulse_width, level);
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_lcd_backlight_init(void)
{
    LCD_LCD_BL_PWM_IMUX();
#if IS_SUPPORT_LCD_BACKLIGHT_CLIMB
    if (bsp_lcd_backlight_timer_register() != 0) {
        return;
    }
    backlight_info.current_duty = 0;
    backlight_info.target_duty = 0;
    backlight_info.onoff = 0;
#endif
}

void bsp_lcd_backlight_resume(void)
{
#if IS_SUPPORT_LCD_BACKLIGHT_CLIMB
    backlight_info.current_duty = 0;
    backlight_info.target_duty = 0;
#endif
    bsp_lcd_backlight_init();
}

void bsp_lcd_backlight_suspend(void)
{
#if IS_SUPPORT_LCD_BACKLIGHT_CLIMB
    backlight_info.current_duty = 0;
    backlight_info.target_duty = 0;
#endif
    hal_pwm_disable(LCD_LCD_BL_PWM_ID);
}

void bsp_lcd_backlight_control(uint16_t level)
{
    lcd_backlight_level_set(level);
}