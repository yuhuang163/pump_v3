# 框架异步通道：广播与延迟任务

全系统跨模块协作依赖两条异步通道：`broadcast`（广播）与 `handler_post`（延迟任务投递）。二者复用 `multi_func_msg_data_t` 信封（48B union），最终都可在 WM 线程汇合。

## 1. 广播（broadcast_manager.c）

### 订阅/发送

```c
typedef struct broadcast_obj {
    uint32_t margic;
    const char *name;
    void *context;
    void (*receiver_cb)(broadcast_mask_id_t id, void *context, int32_t arg1,
                        int32_t arg2, broadcast_priv_data_t *pdata);
    broadcast_mask_id_t id;   // {code, code_v2} 双位掩码
    uint8_t run_on_wm_thd;    // 1=投 WM 线程执行
} broadcast_obj_t;
```

- 订阅 `register_broadcast_receiver`（加锁去重挂链表），注销 `unregister_...`。
- 发送 `send_broadcast`：持锁遍历接收者链表，位掩码 `(rcv.code & send.code) || (code_v2 & ...)` 命中则 `post_bcr_task` 打包（pdata ≤ 64B 拷贝 + CRC16）。
- 工作线程 `bcr_work_thd`（2 个）：收包 → 校验 margic/CRC → `receiver_cb` 异步执行。

### 关键事件码

| 广播 | 说明 |
|------|------|
| `ID_BROADCAST_SYS_STATE_CHANGE` | 系统状态（POWER_ON/OFF/STANDBY/BOOT_COMPLETED） |
| `ID_BROADCAST_PM_STATE_CHANGE` | 电源状态（SUSPEND/RESUME/WAKEUP） |
| `ID_BROADCAST_BREAST_PUMP_STATE_CHANGE` | 吸奶业务状态 |
| `ID_BROADCAST_BLE_STATE_CHANGE` | BLE 连接状态 |
| `ID_BROADCAST_OTA_STATE_CHANGE` | OTA 状态 |

## 2. 延迟任务（handler_post.c）

```c
typedef struct hdr_post_args {
    uint32_t margic;
    const char *name;
    int32_t arg1, arg2;
    void *user_data;
    uint32_t from_tid;
    hdr_post_callback_fptr cb;
    struct thread_context *thd;
    uint8_t exclusive_task;
    uint8_t run_on_wm_thd;
    uint32_t timestamp;
} hdr_post_args_t;
```

- 用法宏 `POST_TASK_HANDLER_DELAYED(...)`：先 `remove_task_handler(name)` 同名去重，再 `post_task_handler()`。
- `delay < 15ms` 直接异步派发；否则分配 `hdr_post_node_t`，计算时间补偿后头插链表，单定时器驱动「到点队列」。
- 定时器回调遍历链表递减 `arrival_time`，到期 `hdr_dispatch_task` 派发并释放节点。
- 工作线程 `task_exec_thread`（2 个）：优先空闲线程，全忙按「已运行时长」排序投最忙者。

## 3. 线程忙闲软判定

`thread_common.c`：`set_thread_running` / `get_thread_running`，用 `is_running` 时间戳表示线程是否正在执行任务（0=阻塞收消息）。广播/投递调度用它做负载均衡，重启用它判断队列空闲。注意 WM 线程内部直接裸写 `thd->is_running`，其他线程走加锁封装。

## 4. WM 线程汇合点

`window_dispatch_thread` 同时注册进 handler_post 池和广播池，`MSG_TYPE_HANDLER_POST_EVENT` / `MSG_TYPE_BROADCAST_EVENT` 都在 WM 线程内执行。所有标记 `run_on_wm_thd` 的回调共享 WM 线程锁域，回调内不能长阻塞，也不得再抢 `window_switch_lock`。

## 5. 注意事项

- 广播 pdata 上限 64B，超出被 `__SEND_BROADCAST` 静默丢弃。
- 同步广播（sync=1）`WAIT_TASK_EXEC_FINISH` 轮询最多约 4.5s，回调内再发同步广播可能自锁。
- handler_post 同名删除用前缀 `strncmp`，`"pressure_test"` 会误删 `"pressure_test:continue"`。
- `user_data` 指针是借用，生命周期由调用方负责。
- `is_handler_post_queue_busy` 跳过 `HDR_SKIP_TASK_SET`（reboot/工厂/电机 play/misc/pm suspend 等）。
