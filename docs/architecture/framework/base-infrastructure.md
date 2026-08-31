# 框架基础设施（base / misc / hal）

## 1. base 目录

| 文件 | 职责 |
|------|------|
| `root_main.c` | 固件入口，启动 init_thd |
| `module_register.c` | 模块注册表 + 两段式初始化 |
| `device_register.c` | 外设总线 pbus，宿主所有驱动 |
| `broadcast_manager.c` | 广播 pub/sub |
| `handler_post.c` | 延迟任务投递引擎 |
| `hang_detect.c` | 卡死检测（**当前空壳**） |
| `thread_common.c` | 线程运行态软判定 |
| `time_system.c` | UTC 时间模块 |
| `reboot.c` | 优雅重启（等队列空闲，最多 5s） |
| `root_list.c` | 侵入式单链表（唯一链表实现） |
| `user_defined_thread.c` | 用户线程/压测入口 |

## 2. 通用消息信封（root_message.h）

```c
#define MULTI_FUNC_MSG_DATA_SIZE (48)
typedef struct multi_func_msg_data {
    uint32_t msg_type;
    union { uint8_t parms[48]; window_entry_extra_t extra;
            window_event_data_t window_evt_data; sensor_wrapper_data_t sensor_data;
            ble_event_type_t ble_evt_type; factory_test_entry_extra_t factory_extra;
            hdr_post_args_t hdr_post; ota_event_info_t ota_info;
            input_key_event_data_t key_event_data; bcr_pkg_struct_t bcr_pkg;
            product_test_entry_extra_t product_test_extra;
            data_bury_entry_extra_t data_bury_extra; } union_body;
} multi_func_msg_data_t;
```

广播包 `bcr_pkg_struct_t` 与延迟任务 `hdr_post_args_t` 共用此 union，通过 `msg_type` 决定 WM 线程分发（WIN_DISPATCH / HANDLER_POST / BROADCAST / INPUT_KEY 等）。

## 3. 线程与优先级（root_common_config.h）

集中管理所有线程优先级、栈深、消息队列深度。栈默认 1KB/单位，主线程 16KB，LVGL 渲染线程 16KB（最大）。优先级示例：`RT_PRIO_BRC_DISPATCH_THREAD = OS_PRIORITY_REALTIME`，`RT_PRIO_LVGL_HANDLER_THREAD = REALTIME6`。

## 4. 时间系统（time_system.c）

- 上电校时：`rtc_timestamp > PRODUCT_BUILD_TIMESTAMP && rtc_timestamp > 存储时间` 才保留 RTC 值，否则回写构建时间。
- `sync_utc_time()` 写存储并广播时区变化。UTC 时间经 `platform_get_utc_time`（PMU RTC）读取。

## 5. 杂项轮询（misc_manager.c）

轻量轮询调度器：一个 20ms 周期定时器把 `MISC_WORK_MSG_ID_TIMER` 塞进 misc 线程队列，遍历 `task_list` 按 `msg_id_mask` 过滤 + `polling_period` 节流执行。监听 PM suspend 广播停/起定时器。

## 6. HAL 抽象（root_source/hal/）

纯头文件，只定义外设的 ioctl opcode 枚举、状态/数据/回调类型，不含实现（实现在 `drivers/*/bsp_*.c`）。

| 文件 | 覆盖 |
|------|------|
| `hal_motor.h` | 泵电机 + 电磁阀 opcode、`motor_pump_para_t`、`PUMP_DUTY_SCALE_FACTOR=100` |
| `hal_panel.h` | LCD 面板 TLVS 消息格式（data ≤14B） |
| `hal_ble.h` | ADV/GATT/OTA 事件、4 服务、`ble_event_cb_func` |
| `hal_led.h` | LED 背光闪烁/呼吸/常亮 |
| `hal_battery.h` | 充电状态机、`CHARGE_EVENT_*` 回调 |
| `hal_pmu.h` | 休眠轮询、定时唤醒、`wakeup_type_e` |
| `hal_input.h` | 按键 MCU 触控/按键、按键固件 OTA |
| `hal_force_sensor.h` | 力传感器 ADC/压力等级/校准 |
| `hal_imu.h` | 六轴 raw/FIFO、校准 |
| `hal_air_pressure.h` | 气压传感器状态 |
| `hal_flash.h` / `hal_fatfs.h` / `hal_database.h` | flash / FS(POSIX 风格函数指针集) / 数据库 |
| `hal_audio.h` / `hal_power.h` / `hal_uarts.h` / `hal_light_sensor.h` | 音频 / 掉电 / UART / 光感 |

## 7. 日志（root_log_define.h + project_log_filter.h）

两级过滤：编译期 `US_LOG_COMPILE_LEVEL` + `IS_SUPPORT_LOG_<MOD>` 裁剪；运行期 `g_log_filter` 位掩码 + `g_log_level_set`。24 个日志模块（PM/PBUS/WM/UI/KEY/SENSOR/RES/MISC/PMP/OTA/BLE/MOTOR/DB/BCR/HDR/TIME/ALGO/PROTOCOL/AUDIO/AIR/STORE/HG/SRV/STEP/TEST/BURY），`project_log_filter.h` 提供 `LOG_FILTER_ALL` 等聚合。输出格式带 UTC 时间 + tick + tid。

## 8. 注意事项

- `hang_detect.c` 是空壳：`init_hang_detect_context` / `add_to_hang_detect_list` 均为空函数，锁卡死检测不生效（`MUTEX_INIT_HOLD` 链路断开）。
- `find_module` 静态缓存非线程安全（裸写无锁），register 完成后业务线程起来前多用，风险低。
- `platform_post_init` 当前为空实现。
- `root_list.c` 的 `__list_empty` 条件回调返回 NULL 会 LOGE「memery leak」并中断清理。
