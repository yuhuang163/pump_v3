# 其余管理器：LED / 音频 / 输入 / 异常 / 飞行模式 / 产测

## 1. LED / 背光 / 夜灯（led/ + drivers/led/）

- 6 颗状态灯（软 PWM）+ 按键/屏背光 + 夜灯爬升 PWM + 全局自适应（ALS）。
- 每颗 `led_status_t`：mode（ONOFF/BLINK/LEVEL/BREATH），blink 含 period/status/times/finish_status。
- `led_control.c` 软 PWM 由 hwtimer 中断翻转 GPIO；`led_device_thread` 10ms 轮询跑 blink 状态机。
- 夜灯 `night_light.c`：`pwm_table[101]` 非线性表，35ms 爬升 + 末段半阶插值，target 钳 [2,98]。
- 全局自适应 `set_global_adaptive_dimming_switch` 写 Flash，同步 key/ui/night_light 三路 ALS。
- 状态快照 `led_state_info_t` 仅在变化时分发（去重）。

## 2. 音频（audio/）

- `audio_manager.c` 0 字节空文件，`drivers/audio/bsp_audio.c` 全返回 0/-1。**整链路空壳**，未接线。
- 接口：`audio_sound_prop_t`（tag/filename/cb/sound_id）、`play_audio_sound / set_audio_filepath / force_play_music`。

## 3. 输入（key_input/ + drivers/key/ + external/scan_key/）

- 按键扫描状态机（scan_key_base.c）：`START → DOWN（去抖）→ DOWN_AGAIN/DOWN_LONG(2s~10s) → UP`，双击靠 interval tick。
- 事件：`KEY_EVENT_DOWN/UP/CLICK/DOUBLE/LONG_2S..LONG_10S`，keycode `KEY_CODE_1..12`。
- 客户端注册 `key_evt_client_node_t`，按 shield 等级（NONE..FORBIDDEN）+ keycode/keytype mask 过滤分发。
- 软复位 `SYS_SOFT_RESET_CODE = KEY_CODE_8`。
- 电源键与旋钮键互斥去抖 100ms。

## 4. 异常检测（abnormal/）

三类异常 + 弹窗仲裁 + 核心锁定：

- `BATTERY_TEMP_HIGH`（ERROR）：5s 轮询，去抖 4×5s，滞回 60/45°C。
- `STORAGE_FAILURE`（FATAL）：flash 失败计数超阈值（30）触发，`is_core_locked`。
- `COMMUNICATION_ERROR`（ERROR）：气压/充电/触摸键/LED/LCD 就绪状态。

仲裁：同优先级替换、高抢占低；异常激活时禁止新任务/App 通信。

## 5. 飞行模式（airplane_mode/）

- `enter/exit`，状态持久化 `STORE_ID_SYS_AIRPLANE_MODE_STATE`。
- 旅行锁键 → 5s 倒计时弹窗 → 进入；OTA 中禁止进入。
- 飞行模式下屏蔽 IMU 唤醒，监测吸奶状态防误退出。

## 6. 自检/产测（tests/ + overlay/product_tests/）

测试类型：FACTORY / AGING_TEST / SUCTION_TEST / SELF_HARDWARE / STRESS / COMMON / LCD_COLOR。

- 工厂表（FAC_TST_READ_SW_VER / MOTOR_CTR / AGING_TEST / SUCTION_TEST / ENTER / EXIT）、硬件自检表（STORAGE / CRC_CHECK）。
- 专用产测线程 200ms 轮询路由到 `factory_tests_thread_handler` / `hw_self_tests_thread_handler`。

## 7. 通用工具（utils/）

`gernal_utils.c`（crc16/crc32/hex/bytes 转换）、`log_utils.c`、`string_utils.c`、`time_utils.c`（2000 年 epoch 基）。

## 8. 外部库（external/）

| 库 | 用途 | 状态 |
|----|------|------|
| littlefs | FDS 文件系统 | 活跃（bsp_database.c） |
| scan_key | 按键扫描状态机 | 活跃 |
| md5 | 资源完整性校验 | 活跃（img_res.c） |
| cJSON | JSON 解析 | 未引用（dead） |
| quicklz | LZ 压缩 | 未引用（dead） |
| crypto / base64 | 加解密 | 经 SDK 间接使用 |
