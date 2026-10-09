/*
 * Gray 码双沿旋钮驱动。
 * 仅在 IS_SUPPORT_KNOB_GRAY_DECODE=1 时编译，由 bsp_knob.c 转发调用。
 *
 * 满 KNOB_EDGES_PER_DETENT 个有效边沿上报一步；非法跳变丢弃。
 * 工程打开 GPIO_BOTH_EDGE_IRQ 时用硬件双边沿，否则按当前电平重挂反沿。
 */
#include "bsp_config.h" // IWYU pragma: keep
#include "bsp_common.h" // IWYU pragma: keep
#include "bsp_internal.h" // IWYU pragma: keep
#include "bsp_key_internal.h" // IWYU pragma: keep
#include "bsp_knob_internal.h" // IWYU pragma: keep

#if IS_SUPPORT_DEVICE_KNOB && IS_SUPPORT_KNOB_GRAY_DECODE

/*
* DEFINES
*****************************************************************************************
*/
#ifndef KNOB_DIR_INVERT
#define KNOB_DIR_INVERT                     0   /// 1: 再对调左右（默认已按旧驱动方向对齐）
#endif

#ifndef KNOB_EDGES_PER_DETENT
#define KNOB_EDGES_PER_DETENT               4   /// 一格对应的有效边沿数（常见 4，半周期器 2）
#endif

#ifndef KNOB_MIN_EDGE_INTERVAL_MS
#define KNOB_MIN_EDGE_INTERVAL_MS           0   /// 合法边沿最小间隔（ms），0=关闭；可试 1
#endif

#ifndef IS_SUPPORT_KNOB_LOG
#define IS_SUPPORT_KNOB_LOG                 0
#endif

/*
 * Gray 转移表：index = (prev_ab << 2) | curr_ab，ab = (A<<1)|B
 * +1 / -1 为一步半拍；非法跳变（毛刺）为 0。
 */
static const int8_t s_enc_table[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
};

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static uint16_t s_knob_left_count = 0;
static uint16_t s_knob_right_count = 0;
static uint8_t s_prev_ab = 0;
static int8_t s_acc = 0;
static uint32_t s_last_edge_time = 0;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint8_t knob_read_ab(void)
{
    uint8_t a = (uint8_t)hal_gpio_pin_get_val(KNOB_A_PIN);
    uint8_t b = (uint8_t)hal_gpio_pin_get_val(KNOB_B_PIN);
    return (uint8_t)((a << 1) | b);
}

static void knob_emit_step(int8_t dir)
{
    /* Gray 表 +1 与旧驱动顺时针相反，默认取反后与旧驱动对齐 */
    bool cw = (dir < 0);
#if KNOB_DIR_INVERT
    cw = !cw;
#endif
    if (cw) {
        s_knob_left_count++;
        bsp_irq_msg_send(INT_NUM_KNOB_RIGHT);
    } else {
        s_knob_right_count++;
        bsp_irq_msg_send(INT_NUM_KNOB_LEFT);
    }
}

#ifndef GPIO_BOTH_EDGE_IRQ
static void knob_arm_pin_irq(enum HAL_GPIO_PIN_T pin, HAL_GPIO_PIN_IRQ_HANDLER handler)
{
    uint32_t level = hal_gpio_pin_get_val(pin);
    struct HAL_GPIO_IRQ_CFG_T cfg = {
        true,
        1,
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
        level ? HAL_GPIO_IRQ_POLARITY_LOW_FALLING : HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        handler
    };
    hal_gpio_setup_irq(pin, &cfg);
}
#endif

static void knob_irq_callback(enum HAL_GPIO_PIN_T pin)
{
    uint8_t curr_ab;
    int8_t step;
    uint32_t now;

    (void)pin;

    curr_ab = knob_read_ab();
    now = get_timestamp();

#if KNOB_MIN_EDGE_INTERVAL_MS > 0
    if ((now - s_last_edge_time) < (uint32_t)KNOB_MIN_EDGE_INTERVAL_MS) {
#ifndef GPIO_BOTH_EDGE_IRQ
        knob_arm_pin_irq(KNOB_A_PIN, knob_irq_callback);
        knob_arm_pin_irq(KNOB_B_PIN, knob_irq_callback);
#endif
        return;
    }
#endif

    step = s_enc_table[(s_prev_ab << 2) | curr_ab];
#if IS_SUPPORT_KNOB_LOG
    BSP_LOG_PASS("knob gray prev:%u curr:%u step:%d acc:%d\r\n",
                 s_prev_ab, curr_ab, step, s_acc);
#endif

    if (step != 0) {
        s_last_edge_time = now;
        s_prev_ab = curr_ab;
        /* 换向时丢掉上一方向余量。停在格点后常残留 1～3 个边沿，
         * 不丢弃的话，反向第一格要先抵消余量，表现为要再转一格才上报。 */
        if ((s_acc > 0 && step < 0) || (s_acc < 0 && step > 0)) {
            s_acc = 0;
        }
        s_acc = (int8_t)(s_acc + step);

        if (s_acc >= KNOB_EDGES_PER_DETENT) {
            s_acc = 0;
            knob_emit_step(1);
        } else if (s_acc <= -KNOB_EDGES_PER_DETENT) {
            s_acc = 0;
            knob_emit_step(-1);
        }
    } else if (curr_ab == s_prev_ab) {
        /* 同态重复（电平回读抖动），忽略 */
    } else {
        /* 非法跳变：同步到当前电平，清空半拍累计，避免被毛刺带偏 */
#if IS_SUPPORT_KNOB_LOG
        BSP_LOG_PASS("knob gray illegal %u->%u, reset acc\r\n", s_prev_ab, curr_ab);
#endif
        s_prev_ab = curr_ab;
        s_acc = 0;
    }

#ifndef GPIO_BOTH_EDGE_IRQ
    /* 软双边沿：按当前电平重新挂下一沿，A/B 都重装以免漏边 */
    knob_arm_pin_irq(KNOB_A_PIN, knob_irq_callback);
    knob_arm_pin_irq(KNOB_B_PIN, knob_irq_callback);
#endif
}

static void knob_irq_control(bool enable)
{
    struct HAL_GPIO_IRQ_CFG_T cfg = {
        enable,
        1,
        HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
#ifdef GPIO_BOTH_EDGE_IRQ
        HAL_GPIO_IRQ_POLARITY_EDGE_BOTH,
#else
        HAL_GPIO_IRQ_POLARITY_LOW_FALLING,
#endif
        knob_irq_callback
    };

    if (!enable) {
        cfg.irq_handler = NULL;
        hal_gpio_setup_irq(KNOB_A_PIN, &cfg);
        hal_gpio_setup_irq(KNOB_B_PIN, &cfg);
        return;
    }

#ifdef GPIO_BOTH_EDGE_IRQ
    hal_gpio_setup_irq(KNOB_A_PIN, &cfg);
    hal_gpio_setup_irq(KNOB_B_PIN, &cfg);
#else
    knob_arm_pin_irq(KNOB_A_PIN, knob_irq_callback);
    knob_arm_pin_irq(KNOB_B_PIN, knob_irq_callback);
#endif

    LOG_PBUS("knob gray irq: %d, ab:%u", enable, knob_read_ab());
}

static void knob_gpio_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_map = {
        (enum HAL_IOMUX_PIN_T)KNOB_A_PIN,
        HAL_IOMUX_FUNC_AS_GPIO,
        HAL_IOMUX_PIN_VOLTAGE_VIO,
        HAL_IOMUX_PIN_PULLUP_ENABLE,
    };
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(KNOB_A_PIN, HAL_GPIO_DIR_IN, 1);

    gpio_map.pin = (enum HAL_IOMUX_PIN_T)KNOB_B_PIN;
    hal_iomux_init(&gpio_map, 1);
    hal_gpio_pin_set_dir(KNOB_B_PIN, HAL_GPIO_DIR_IN, 1);

    s_prev_ab = knob_read_ab();
    s_acc = 0;
    s_last_edge_time = get_timestamp();
    LOG_PBUS("knob gray init, ab:%u", s_prev_ab);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void bsp_knob_gray_init(void)
{
    knob_gpio_init();
    knob_irq_control(true);
}

uint16_t bsp_knob_gray_get_count(uint8_t index)
{
    if (index == 0) {
        return s_knob_left_count;
    }
    return s_knob_right_count;
}

#endif /* IS_SUPPORT_DEVICE_KNOB && IS_SUPPORT_KNOB_GRAY_DECODE */
