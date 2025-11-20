#include <stdio.h>
#include "string.h"
#include "lpcmd.h"
#include "hal_dsi.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "plat_types.h"
#include "gra_sem.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

#ifdef __NuttX__
#include <arch/board/board.h>
#include <debug.h>
#include <nuttx/power/pm.h>
#include <nuttx/arch.h>
#else
#include "lcdc_fb.h"
#endif
#include "lcdc_drv.h"
#include "lcdc.h"

/* Debug option */
//#define GRAPHIC_DRV_DEBUG
//#define GRAPHIC_DRV_VERBOSE

#ifdef  GRAPHIC_DRV_VERBOSE
#ifndef GRAPHIC_DRV_DEBUG
#define GRAPHIC_DRV_DEBUG
#endif
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#define LCDC_REGINFO(...)  TR_INFO( TR_MOD(TEST), __VA_ARGS__)
#else
#define LCDC_INFO(...)
#define LCDC_REGINFO(...)
#endif

#ifdef  GRAPHIC_DRV_DEBUG
#define LCDC_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define LCDC_REP(...)
#endif

#define LCDC_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

typedef struct lpcmd_cmds_s
{
  uint8_t cmd;
  uint8_t count;
  uint8_t optype;
  union {
      uint8_t* pbuf;
      uint8_t params[LPCMDS_MAX_PARAMS_NUM];
  };
} lpcmd_cmds_t;

struct lpcmd_group_s {
    int count;
    int flag;
    sem_t sem;
    lpcmd_cmds_t cmds[LPCMDS_MAX_CMDS_NUM];
};

struct lpcmd_s {
    sem_t* lock;
    volatile int groups;
    struct lpcmd_group_s gcmds[LPCMDS_MAX_GROUP_NUM];
};

static sem_t g_sem;

static int g_initalize = 0;

static struct lpcmd_s lpcmds = {
    .lock = &g_sem,
    .groups = 0,
};

static struct lpcmd_s* lpcmds_getdev(void)
{
    if (!g_initalize) {
        memset(&lpcmds.gcmds, 0, sizeof(lpcmds.gcmds));

        gra_sem_init(lpcmds.lock, 0, 1);

        for (int i = 0; i < LPCMDS_MAX_GROUP_NUM; i++) {
            gra_sem_init(&lpcmds.gcmds[i].sem, 0, 0);
        }

        g_initalize = 1;
    }

    return &lpcmds;
}

/**
 * @brief lpcmds_register_cmds - register cmds to cache, will send at framedone
 * irq
 *
 * @param cmds reference struct lpcmd_node_s *
 * @param nums count
 * @return int
 */
int lpcmds_register_cmds(void* cmds, int nums, int flag)
{
    struct lpcmd_node_s* pcmds = cmds;
    int i;
    struct lpcmd_s* priv = lpcmds_getdev();
    int ret = 0;
    LCDC_ERR("lpcmds_register_cmds in cmd=%p,nums=%d\n",cmds,nums);

    if (!cmds || nums <= 0) {
        return -1;
    }

    /* lock */
    gra_sem_wait(priv->lock);
    /* too many params, support max num LPCMDS_MAX_PARAMS_NUM, default 4  */

    if (pcmds->count > LPCMDS_MAX_PARAMS_NUM) {
        LCDC_ERR("lpcmd mode register lpcmds failed, cmds too many params.\n");
        goto _exit;
    }

    /* busy */

    if (priv->groups >= LPCMDS_MAX_GROUP_NUM) {
        LCDC_ERR("lpcmd mode register failed, no memory to store new cmds.\n");
        goto _exit;
    }

    i = priv->groups;
    priv->gcmds[i].flag = flag;

    if (flag & LPCMDS_BLOCK) {
        gra_sem_reset(&priv->gcmds[i].sem, 0);
    }

    priv->groups++;
    for (int j = 0; j< nums; j++) {

        priv->gcmds[i].cmds[j].cmd = pcmds[j].cmd;
        priv->gcmds[i].cmds[j].count = pcmds[j].count;
        priv->gcmds[i].cmds[j].optype = pcmds[j].optype;
        if (pcmds[j].optype == LPCMD_TYPE_WRITE) {
            for (int k = 0; k < pcmds[j].count; k++) {
                priv->gcmds[i].cmds[j].params[k] = pcmds[j].params[k];
            }
        } else {
            if (flag & LPCMDS_NONBLOCK) {
                priv->gcmds[i].cmds[j].pbuf = NULL;
            } else {
                priv->gcmds[i].cmds[j].pbuf = pcmds[j].params;
            }
        }
    }

    priv->gcmds[i].count = nums;
    /* unlock */
    gra_sem_post(priv->lock);
    if (flag & LPCMDS_BLOCK) {
        if (in_isr()) {
            if (lcdc_lock()) {
                lpcmds_flush();
                lcdc_unlock();
            }
        }
        else {
            if (flag & LPCMDS_FLUSH) {
                if (lcdc_lock()) {
                    lpcmds_flush();
                    lcdc_unlock();
                }
            }
            ret = gra_sem_timedwait(&priv->gcmds[i].sem, 200);
            if (OK != ret) {
                LCDC_ERR("[lpcmd] register cmd wait send timeout, "
                "no te signal happend, clear has registered cmds ret = %d\n", ret);
            }
        }
    }
    return ret;

_exit:
    /* unlock */
    gra_sem_post(priv->lock);

    return -1;
}
extern void lcdc_exit_lowpower();

/**
 * @brief lpcmds_flush - send all cmds at cache
 *
 * @return int - success 0; failed -1;
 */

int lpcmds_flush(void)
{
    struct lcdc_config_t *config = NULL;
    struct lpcmd_s* priv = lpcmds_getdev();
    struct lpcmd_group_s* gcmds;
#ifdef CONFIG_LCDC_DSI
    enum HAL_CHIP_METAL_ID_T fixed_metal_id = HAL_CHIP_METAL_ID_0;
#if defined(CHIP_BEST2003)
    fixed_metal_id= HAL_CHIP_METAL_ID_7;
#elif defined(CHIP_BEST1502X)
    fixed_metal_id= HAL_CHIP_METAL_ID_2;
#elif defined(CHIP_BEST1600)
    fixed_metal_id= HAL_CHIP_METAL_ID_2;
#endif
#endif

    int ret = 0;
    int i;
    static int count = 0;

    if (lcdc_get_config(&config) != LCDC_OK)
        return -1;

    /* lock */
    if (in_isr()) {
        gra_sem_get_value(priv->lock, &ret);
        if (ret <= 0)
        {
            LCDC_INFO("in_isr %d\n",ret);
            return -1;
        }
    } else {
        ret = gra_sem_trywait(priv->lock);
        if (ret != OK)
        {
            LCDC_INFO("trywait %d\n",ret);
            return -1;
        }
    }

    if (priv->groups > 0)
        lcdc_exit_lowpower();

    for (i = 0; i < priv->groups; i++) {
        gcmds = &priv->gcmds[i];
#ifdef CONFIG_LCDC_DSI
        if (fixed_metal_id > HAL_CHIP_METAL_ID_0 && hal_get_chip_metal_id() < fixed_metal_id) {
            /* will cause error pin set hight */
            hal_dsi_reset(config->bus_port0, BOARD_LCDC_WIDTH);
            if (config->bus == LCDC_BUS_DUAL_DSI)
                hal_dsi_reset(config->bus_port1, BOARD_LCDC_WIDTH);
        }
#endif
        for (int j = 0; j < gcmds->count; j++) {

            switch (gcmds->cmds[j].optype) {
            case LPCMD_TYPE_WRITE:
                lcdc_send_cmd_list(config->bus_port0, gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                                      gcmds->cmds[j].params);
                if (config->bus == LCDC_BUS_DUAL_DSI) {
                    lcdc_send_cmd_list(config->bus_port1, gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                                          gcmds->cmds[j].params);
                }

                LCDC_INFO("count = %d, cmd write cmd = 0x%02x, count = %d, "
                          "param = 0x%02x, 0x%02x, 0x%02x, 0x%02x\n", count,
                          gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                          gcmds->cmds[j].params[0], gcmds->cmds[j].params[1],
                          gcmds->cmds[j].params[2], gcmds->cmds[j].params[3]);
                break;
            case LPCMD_TYPE_READ:
                if (!gcmds->cmds[j].pbuf) {
                    LCDC_ERR("cmd read cmd = 0x%02x, count = %d, "
                            "read buffer is NULL.\n",
                            gcmds->cmds[j].cmd, gcmds->cmds[j].count);
                    break;
                }

                lcdc_read_cmd(config->bus_port0, gcmds->cmds[j].cmd, gcmds->cmds[j].pbuf,
                                 gcmds->cmds[j].count);

                LCDC_INFO("count = %d, cmd read cmd = 0x%02x, count = %d, "
                          "param = 0x%02x, 0x%02x\n", count,
                          gcmds->cmds[j].cmd, gcmds->cmds[j].count,
                          gcmds->cmds[j].pbuf[0], gcmds->cmds[j].pbuf[1]);
                break;
            case LPCMD_TYPE_DELAY:
                LCDC_INFO("count = %d, cmd Delay = 0x%02x ms\n", count,
                         gcmds->cmds[j].cmd);
                if (in_isr())
                    hal_sys_timer_delay(MS_TO_TICKS(gcmds->cmds[j].cmd));
                else
                    osDelay(gcmds->cmds[j].cmd);
                break;
            default:
                LCDC_ERR("cmd optype error, type = %d, "
                          "cmd = 0x%02x, count = %d, "
                          "param = 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
                          gcmds->cmds[j].optype, gcmds->cmds[j].cmd,
                          gcmds->cmds[j].count, gcmds->cmds[j].params[0],
                          gcmds->cmds[j].params[1], gcmds->cmds[j].params[2],
                          gcmds->cmds[j].params[3]);
                break;
            }
            count++;
        }

        gcmds->count = 0;

        if (gcmds->flag & LPCMDS_BLOCK) {
            /* notify wait tasks */
            gra_sem_post(&gcmds->sem);
        }
    }

    priv->groups = 0;

    /* unlock */
    if (!in_isr())
        gra_sem_post(priv->lock);
    return 0;
}

/**
 * @brief lpcmds_clear_buffer - clear lpcmds buffer
 *
 */
void lpcmds_clear_buffer(void)
{
    struct lpcmd_s* priv = lpcmds_getdev();

    gra_sem_wait(priv->lock);

    priv->groups = 0;

    gra_sem_post(priv->lock);
}
