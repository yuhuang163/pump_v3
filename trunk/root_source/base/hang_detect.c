/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"

#if IS_SUPPORT_HANG_DETECT

/*
* DEFINES
*****************************************************************************************
*/
#define HANG_DETECT_MAX_LOCKS              (48)
#define HANG_DETECT_MAX_WAITERS            (4)
#define HANG_DETECT_CYCLE_MAX_HOPS         (16)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct hang_waiter {
    os_thread_id tid;
    uint32_t wait_tick;
    uint8_t used;
} hang_waiter_t;

typedef struct hang_lock_item {
    os_mutex_id lock;
    const char *name;
    uint32_t max_hold_ms;
    uint32_t max_wait_ms;
    os_thread_id owner;
    uint32_t hold_tick;
    uint16_t nest;
    uint8_t used;
    uint8_t is_ui;
    hang_waiter_t waiters[HANG_DETECT_MAX_WAITERS];
} hang_lock_item_t;

static hang_lock_item_t s_locks[HANG_DETECT_MAX_LOCKS];
static hang_lock_item_t s_snap[HANG_DETECT_MAX_LOCKS];
static thread_context_t s_hang_thd;
static uint8_t s_inited;
static uint8_t s_fatal_handling;
static uint32_t s_start_tick;
static uint32_t s_last_dump_tick;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static bool is_ui_lock_name(const char *name)
{
    if (name == NULL) {
        return false;
    }
    if (strstr(name, "ui_") != NULL) {
        return true;
    }
    if (strstr(name, "window_switch") != NULL) {
        return true;
    }
    if (strstr(name, "history_stack") != NULL) {
        return true;
    }
    return false;
}

static const char *thread_name_of(os_thread_id tid)
{
    const char *name;

    if (tid == NULL) {
        return "-";
    }
    name = osThreadGetName(tid);
    return name ? name : "?";
}

static const char *thread_state_of(os_thread_id tid)
{
    if (tid == NULL) {
        return "-";
    }
    switch (os_thread_get_state_wrapper(tid)) {
        case OS_THREAD_RUNNING:
            return "RUN";
        case OS_THREAD_READY:
            return "RDY";
        case OS_THREAD_BLOCKED:
            return "BLK";
        case OS_THREAD_SUSPENDED:
            return "SUS";
        case OS_THREAD_DELETED:
            return "DEL";
        default:
            return "?";
    }
}

static hang_lock_item_t *find_lock_item(os_mutex_id lock)
{
    int i;

    if (lock == NULL) {
        return NULL;
    }
    for (i = 0; i < HANG_DETECT_MAX_LOCKS; i++) {
        if (s_locks[i].used && s_locks[i].lock == lock) {
            return &s_locks[i];
        }
    }
    return NULL;
}

static void waiter_add(hang_lock_item_t *item, os_thread_id tid, uint32_t now)
{
    int i;

    for (i = 0; i < HANG_DETECT_MAX_WAITERS; i++) {
        if (item->waiters[i].used && item->waiters[i].tid == tid) {
            return;
        }
    }
    for (i = 0; i < HANG_DETECT_MAX_WAITERS; i++) {
        if (!item->waiters[i].used) {
            item->waiters[i].used = 1;
            item->waiters[i].tid = tid;
            item->waiters[i].wait_tick = now;
            return;
        }
    }
}

static void waiter_del(hang_lock_item_t *item, os_thread_id tid)
{
    int i;

    for (i = 0; i < HANG_DETECT_MAX_WAITERS; i++) {
        if (item->waiters[i].used && item->waiters[i].tid == tid) {
            item->waiters[i].used = 0;
            item->waiters[i].tid = NULL;
            item->waiters[i].wait_tick = 0;
            return;
        }
    }
}

static hang_lock_item_t *find_waited_lock_in(hang_lock_item_t *table, os_thread_id tid)
{
    int i;
    int w;

    for (i = 0; i < HANG_DETECT_MAX_LOCKS; i++) {
        if (!table[i].used) {
            continue;
        }
        for (w = 0; w < HANG_DETECT_MAX_WAITERS; w++) {
            if (table[i].waiters[w].used && table[i].waiters[w].tid == tid) {
                return &table[i];
            }
        }
    }
    return NULL;
}

static void dump_lock_table(hang_lock_item_t *table, const char *reason)
{
    int i;
    int w;

    LOGE("[HANG] ===== %s =====", reason ? reason : "dump");
    for (i = 0; i < HANG_DETECT_MAX_LOCKS; i++) {
        hang_lock_item_t *item = &table[i];
        uint32_t hold_ms;

        if (!item->used) {
            continue;
        }
        if (item->owner == NULL && item->waiters[0].used == 0 &&
            item->waiters[1].used == 0 && item->waiters[2].used == 0 &&
            item->waiters[3].used == 0) {
            continue;
        }
        hold_ms = item->owner ? get_timestamp_ms(item->hold_tick) : 0;
        LOG_HG("%s lock=%s owner=%s(%s) nest=%u hold=%lums",
               item->is_ui ? "[UI]" : "    ",
               item->name ? item->name : "?",
               thread_name_of(item->owner),
               thread_state_of(item->owner),
               (unsigned)item->nest,
               (unsigned long)hold_ms);
        for (w = 0; w < HANG_DETECT_MAX_WAITERS; w++) {
            if (!item->waiters[w].used) {
                continue;
            }
            LOG_HG("      waiter=%s(%s) wait=%lums",
                   thread_name_of(item->waiters[w].tid),
                   thread_state_of(item->waiters[w].tid),
                   (unsigned long)get_timestamp_ms(item->waiters[w].wait_tick));
        }
    }
}

static bool should_dump(void)
{
    if (s_last_dump_tick == 0) {
        s_last_dump_tick = get_timestamp();
        return true;
    }
    if (get_timestamp_ms(s_last_dump_tick) >= (HANG_DETECT_DUMP_INTERVAL_S * 1000U)) {
        s_last_dump_tick = get_timestamp();
        return true;
    }
    return false;
}

static void handle_hang_fatal(const char *reason)
{
    if (s_fatal_handling) {
        return;
    }
    s_fatal_handling = 1;

    dump_lock_table(s_snap, reason);
    LOGE("[HANG] unrecoverable, reboot in %dms: %s",
         HANG_DETECT_REBOOT_DELAY_MS, reason ? reason : "hang");

    /*
     * 死锁后不要走 __system_reboot()：它会等 handler_post/broadcast 队列空闲，
     * 这些线程本身可能就是死锁参与者，会再次卡死。
     * 也不弹异常窗：UI/WM 锁经常就是死锁点。
     */
#if HANG_DETECT_REBOOT_ON_DEADLOCK
    msleep(HANG_DETECT_REBOOT_DELAY_MS);
    platform_system_reboot();
#else
    s_fatal_handling = 0;
#endif
}

static void report_hang(const char *reason)
{
    if (s_fatal_handling) {
        return;
    }
    if (!should_dump()) {
        return;
    }
    handle_hang_fatal(reason);
}

static bool walk_deadlock_cycle_in(hang_lock_item_t *table, os_thread_id start)
{
    os_thread_id tid = start;
    int hop;

    for (hop = 0; hop < HANG_DETECT_CYCLE_MAX_HOPS; hop++) {
        hang_lock_item_t *waited = find_waited_lock_in(table, tid);
        os_thread_id owner;

        if (waited == NULL || waited->owner == NULL) {
            return false;
        }
        owner = waited->owner;
        if (owner == start) {
            return hop > 0;
        }
        if (owner == tid) {
            return false;
        }
        tid = owner;
    }
    return false;
}

static void scan_locks(void)
{
    int i;
    int w;
    int ui_involved = 0;
    int timeout_hit = 0;
    int deadlock_hit = 0;
    uint32_t pri;

    /* 一次原子快照，避免扫描中途 UI/WM 解锁再加锁，把不同时刻拼成假死锁环 */
    pri = platform_int_lock_global();
    memcpy(s_snap, s_locks, sizeof(s_locks));
    platform_int_unlock_global(pri);

    for (i = 0; i < HANG_DETECT_MAX_LOCKS; i++) {
        hang_lock_item_t *item = &s_snap[i];

        if (!item->used) {
            continue;
        }

        if (item->owner != NULL && item->max_hold_ms > 0) {
            uint32_t hold_ms = get_timestamp_ms(item->hold_tick);
            if (hold_ms >= item->max_hold_ms) {
                timeout_hit = 1;
                if (item->is_ui) {
                    ui_involved = 1;
                }
                LOG_HG("hold timeout %s owner=%s hold=%lums max=%lu",
                       item->name ? item->name : "?",
                       thread_name_of(item->owner),
                       (unsigned long)hold_ms,
                       (unsigned long)item->max_hold_ms);
            }
        }

        for (w = 0; w < HANG_DETECT_MAX_WAITERS; w++) {
            uint32_t wait_ms;

            if (!item->waiters[w].used) {
                continue;
            }
            wait_ms = get_timestamp_ms(item->waiters[w].wait_tick);
            if (item->max_wait_ms > 0 && wait_ms >= item->max_wait_ms) {
                LOG_HG("wait long %s waiter=%s owner=%s wait=%lums (no reboot)",
                       item->name ? item->name : "?",
                       thread_name_of(item->waiters[w].tid),
                       thread_name_of(item->owner),
                       (unsigned long)wait_ms);
            }
            if (walk_deadlock_cycle_in(s_snap, item->waiters[w].tid)) {
                deadlock_hit = 1;
                if (item->is_ui) {
                    ui_involved = 1;
                }
            }
        }
    }

    if (deadlock_hit) {
        report_hang(ui_involved ? "UI deadlock cycle" : "deadlock cycle");
    } else if (timeout_hit) {
        report_hang(ui_involved ? "UI lock timeout" : "lock timeout");
    }
}

static void hang_detect_thread(const void *args)
{
    (void)args;
    while (1) {
        msleep(HANG_DETECT_POLL_PERIOD_MS);
        if (get_timestamp_ms(s_start_tick) < (HANG_DETECT_STARTUP_GRACE_S * 1000U)) {
            continue;
        }
        scan_locks();
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void add_to_hang_detect_list(os_mutex_id lock, const char *lock_name, int max_hold_time_s)
{
    int i;
    uint32_t pri;
    hang_lock_item_t *item = NULL;
    uint32_t hold_s;
    uint32_t wait_s;
    uint8_t is_ui;

    if (lock == NULL) {
        return;
    }

    is_ui = is_ui_lock_name(lock_name) ? 1 : 0;
    if (max_hold_time_s > 0) {
        hold_s = (uint32_t)max_hold_time_s;
    } else {
        hold_s = HANG_DETECT_TIMEOUT_S;
    }
    wait_s = hold_s;

    pri = platform_int_lock_global();
    if (find_lock_item(lock) != NULL) {
        platform_int_unlock_global(pri);
        return;
    }
    for (i = 0; i < HANG_DETECT_MAX_LOCKS; i++) {
        if (!s_locks[i].used) {
            item = &s_locks[i];
            break;
        }
    }
    if (item == NULL) {
        platform_int_unlock_global(pri);
        LOGE("[HANG] lock table full, drop %s", lock_name ? lock_name : "?");
        return;
    }
    memset(item, 0, sizeof(*item));
    item->lock = lock;
    item->name = lock_name;
    item->max_hold_ms = hold_s * 1000U;
    item->max_wait_ms = wait_s * 1000U;
    item->is_ui = is_ui;
    item->used = 1;
    platform_int_unlock_global(pri);
    LOG_HG("watch lock=%s hold=%lus wait=%lus ui=%u",
           lock_name ? lock_name : "?",
           (unsigned long)hold_s,
           (unsigned long)wait_s,
           (unsigned)is_ui);
}

void remove_from_hang_detect_list(os_mutex_id lock)
{
    hang_lock_item_t *item;
    uint32_t pri;

    pri = platform_int_lock_global();
    item = find_lock_item(lock);
    if (item) {
        memset(item, 0, sizeof(*item));
    }
    platform_int_unlock_global(pri);
}

os_status_e hang_detect_mutex_lock(os_mutex_id mutex_id, uint32_t timeout)
{
    hang_lock_item_t *item;
    os_thread_id self;
    uint32_t pri;
    uint32_t now;
    os_status_e status;

    if (mutex_id == NULL) {
        return osErrorParameter;
    }
    if (!s_inited || is_interrupt_handler_wrapper()) {
        return os_mutex_wait_wrapper(mutex_id, timeout);
    }

    self = os_thread_get_id_wrapper();
    now = get_timestamp();
    pri = platform_int_lock_global();
    item = find_lock_item(mutex_id);
    if (item && self && item->owner != self) {
        waiter_add(item, self, now);
    }
    platform_int_unlock_global(pri);

    status = os_mutex_wait_wrapper(mutex_id, timeout);

    pri = platform_int_lock_global();
    item = find_lock_item(mutex_id);
    if (item && self) {
        waiter_del(item, self);
        if (status == osOK) {
            if (item->owner == self) {
                item->nest++;
            } else {
                item->owner = self;
                item->hold_tick = get_timestamp();
                item->nest = 1;
            }
        }
    }
    platform_int_unlock_global(pri);
    return status;
}

os_status_e hang_detect_mutex_unlock(os_mutex_id mutex_id)
{
    os_status_e status;
    hang_lock_item_t *item;
    os_thread_id self;
    uint32_t pri;

    if (mutex_id == NULL) {
        return osErrorParameter;
    }
    status = os_mutex_release_wrapper(mutex_id);
    if (!s_inited || is_interrupt_handler_wrapper() || status != osOK) {
        return status;
    }

    self = os_thread_get_id_wrapper();
    pri = platform_int_lock_global();
    item = find_lock_item(mutex_id);
    if (item && (item->owner == self || item->owner == NULL)) {
        if (item->nest > 1) {
            item->nest--;
        } else {
            item->nest = 0;
            item->owner = NULL;
            item->hold_tick = 0;
        }
    }
    platform_int_unlock_global(pri);
    return status;
}

void init_hang_detect_context(void)
{
    if (s_inited) {
        return;
    }
    s_start_tick = get_timestamp();
    s_last_dump_tick = 0;
    s_inited = 1;
    RUN_NEW_THREAD(&s_hang_thd, "hang_detect_thd", hang_detect_thread,
                   RT_PRIO_HANG_DETECT_THREAD, HG_DETECT_THREAD_STACKSIZE);
    LOG_HG("hang detect started, poll=%dms prio=%d",
           HANG_DETECT_POLL_PERIOD_MS, RT_PRIO_HANG_DETECT_THREAD);
}

#endif /* IS_SUPPORT_HANG_DETECT */
