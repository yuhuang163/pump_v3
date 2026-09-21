# 窗口层业务实现（overlay/windows/windows_v6）

窗口层是 UI 三层结构中的**业务决策层**：决定"当前显示哪个 UI"，处理按键/广播/触摸事件与窗口跳转。真正的 LVGL 绘制在 `overlay/ui/` 页面层，二者通过 `xxx_ui_handler.c` 桥接。

## 1. 三层桥接模式

每个窗口固定三文件：

| 文件 | 职责 |
|------|------|
| `window_screen_xxx.c` | 注册 `window_life_cycle_t` 静态壳 |
| `window_screen_xxx_handle.c` | 窗口入口 `enter_new_window_xxx_screen`、`entry/exit_event_cb`、按键/广播/电池回调、跳转策略 |
| `xxx_ui_handler.c` | UI 动画桥接：`start_xxx_ui_animation` → `ui_animation_play_init` → `ui_display_xxx_prepare/process/remove` |

### 1.1 入口统一模板

`PAGE_HANDLE_NAME(name)` 宏展开为 `enter_new_window_<name>_screen(wm, window_id)`，各窗口几乎一致：

1. `u_malloz(sizeof(ui_proc_xxx_t))` 分配 proc（首成员 `window_context_t w_ctx`）；
2. `SET_WINDOW_CONTEXT(proc)` + `margic = UI_WINDOW_MARGIC_CODE`（守卫宏校验）；
3. `find_module(ID_MODULE_UI/KEY_INPUT/BATTERY/BLE/PUMP/...)` 取模块句柄；
4. 配置 `w_ctx`（new_window_id / evt_cb_entry / evt_cb_exit / not_push_history / is_popup_window / user_data / async_evt_handler_cb）；
5. `wm->dispatch_new_window(w_ctx)` 交给窗口管理器。

### 1.2 生命周期

- `entry_event_cb`：`ON_CREATE`（一次性初始化：背光/Wakelock/读参数/起动画）、`ON_RESUME`（`ADD_KEYEVENT_CB`/`ADD_BATTERY_STATE_CB`、`wm_ensure_ble_adv_started()`、`window_run_state=RUNNING`）。
- `exit_event_cb`：`ON_PAUSE`（停子 UI/动画）、`ON_DESTORY`（`REMOVE_ALL_HANDER_TASK()` 清延迟任务、反注册回调、`release_wakelock`、`u_free(proc)`）。
- 所有回调首行 `window_run_state != RUNNING 则 ignore`。

### 1.3 UI 动画桥接协议

```c
ui_play_info_t play_info = {
    .tag = "...", .fps = XX,
    .progress_cb = xxx_animation_progress_cb,
    .display_prepare_cb = xxx_ui_display_prepare,  // → ui_display_xxx_prepare(ui)
    .display_remove_cb  = xxx_ui_display_remove,   // → ui_display_xxx_remove(ui)
    .user_data = proc,
};
ui->hook->ui_animation_play_init(ui, play_info);
```

`progress_cb` 按状态分派：`ANI_TIMER_DRAW_AFTER` → `ui_display_xxx_process`（逐帧绘制 + 业务 tick）；`ANI_POPUP_DISPLAY_FINISH` → `POST_WM_TASK_HANDLER_NO_DELAY` 投递异步完成回调（超时跳转/退出策略）。

### 1.4 完整调用链（Home 启动泵奶）

```
home POWER 单击 → on_home_ui_keyevent_hdr
 → home_start_pump_with_sub_ui(proc, sub_ui)
 → dispatch ID_WINDOW_PUMP (extra.arg1 = sub_ui_type)
   → enter_new_window_pump_screen：on_create 读 work mode
     → arg1==SETTINGS_SUCCESS ? 先 Success 页 : switch_pump_status_ui
     → WRITE_PUMP_WORK_STATUS(START) + pump_seal_check_start
     → show_pump_home_ui → ui_animation_play_init → ui_display_pump_home_*_prepare/process
泵奶完成广播 ID_BROADCAST_BREAST_PUMP_STATE_CHANGE(COMPLETED)
 → dispatch ID_WINDOW_HOME(arg1 = HOME_SUB_UI_TYPE_PUMP_RESULT)
   → home on_create → show_home_pump_result_ui → 3s 超时回主页动画
```

## 2. 窗口注册表与优先级

优先级分级（`windows_map_define.c`）：`LOW < LEVEL2 < LEVEL8 < LEVEL9(=GUIDE) < HIGH < TOP`。

| 窗口 | ID | 优先级 | 入口 |
|------|----|--------|------|
| dummy | `ID_WINDOW_DUMMY` | LOW | 休眠占位/唤醒分流 |
| power_on_guide | `ID_WINDOW_POWER_ON_GUIDE` | LOW | 长按开机引导 |
| pump | `ID_WINDOW_PUMP` | LEVEL2 | 泵奶运行 |
| home | `ID_WINDOW_HOME` | LEVEL8 | 主界面（导航根） |
| guide | `ID_WINDOW_GUIDE` | LEVEL9 | 新手引导 |
| theme_guide | `ID_WINDOW_THEME_GUIDE` | LEVEL9 | 主题选择 |
| logo | `ID_WINDOW_LOGO` | HIGH | 开机 LOGO |
| battery | `ID_WINDOW_BATTERY` | HIGH | 低电 |
| popup_window | `ID_WINDOW_POPUP_WINDOW` | HIGH | 通用弹窗 |
| airplane_mode | `ID_WINDOW_AIRPLANE_MODE` | HIGH | 飞行模式 |
| charge | `ID_WINDOW_CHARGE` | HIGH | 充电 |
| ota | `ID_WINDOW_OTA_UPGRADE` | HIGH | 升级 |
| product_test | `ID_WINDOW_PRODUCT_TEST_MODE` | TOP | 产测 |
| factory_reset | `ID_WINDOW_FACTORY_RESET` | TOP | 恢复出厂 |
| power_off | `ID_WINDOW_POWER_OFF` | TOP | 关机 |
| system_abnormal | `ID_WINDOW_SYSTEM_ABNORMAL` | TOP | 系统异常 |
| setting | （注释） | — | 无独立设置页 |

优先级生效规则：后台分派时 `target_z_order < current_z_order` 则拒绝切换；TOP 级窗口可抢占普通窗口（HOME 被 PUMP 抢占时进 on_pause）。

## 3. 逐窗口业务逻辑

### 3.1 Dummy（休眠占位）
- 待机入口：`pm_suspend_prepare_enter` 分派 `ID_WINDOW_DUMMY`，作为前台占位，允许任意界面进入休眠。
- 广播 `ID_BROADCAST_PM_STATE_CHANGE`：`SYS_RESUME` 时按 `wakeup_reason` 分流（tests 模式非 NONE → PRODUCT_TEST，否则 HOME）。
- 进窗不绘制开机引导、不关机；POWER LONG_10S → `platform_system_reboot()`。
- WM 特例：dummy 允许在 `on_suspend_gotoing` 时切换；不压历史栈。

### 3.1.1 Power On Guide（开机引导）
- 目录 `overlay/windows/windows_v6/power_on_guide/`，ID 为 `ID_WINDOW_POWER_ON_GUIDE`。
- 短按未满足开机时长、关机充电页短按等走本窗口，不再占用 dummy。
- `IS_SUPPORT_POWER_ON_GUIDE_UI`：适配器不匹配一律进 `POWER_OFF(CHARGE)`（关机充电/异常页），含 `power_hook` 的引导、短按抬键、长按 2s 确认开机、detect 期间仍按住、reboot；不进 `ID_WINDOW_CHARGE`。否则长按开机引导（POWER DOWN/UP 控制进度）。
  - 引导结束（长按完成 / 静默超时）：V3Pro 充电中 → `POWER_OFF(CHARGE)`；适配器不匹配 → `POWER_OFF(CHARGE)`；未充电长按完成 → LOGO，静默超时 → 关机。
  - V3：长按完成 → LOGO；静默超时 → 关机（适配器不匹配进 `POWER_OFF(CHARGE)`）。

### 3.2 Logo（开机 LOGO）
- on_create：背光/自适应亮度、`start_logo_ui_animation`、延迟 500ms 做触摸键固件版本检查。
- on_pause：停动画 + 开广播。流向靠 Dummy→Logo→Guide/Battery/Home 顺序。

### 3.3 Theme Guide（主题选择）
- 入口：系统异常退出且 `get_sys_theme_type()==NONE` 才进。
- KNOB 移动选择（SIMPLE/IP）；POWER 短按确认当前主题并写 BASE/CUSTOM，随后进 `ID_WINDOW_HOME`（`HOME_SUB_UI_TYPE_PUMP_MODE_UNSELECT`），按键灯全灭。START 按键本窗口屏蔽。V7 无主题重选弹窗，不再等 `THEME_RESELECT`。

### 3.4 Guide（新手引导）
- 约 24 个子页（QRCODE/INTRO/泵奶步骤/模式/力度/速度/完成）。on_create 读 `is_window_guidance_deployed`。
- 完成：`set_window_guidance_deployed_flag(true)` → 出口分流。

### 3.5 Home（主界面，最复杂）
- 按键码：`ALL_KEY_CODE_NO_COMBINE | BOTH_CONTROL | TRAVEL_LOCK | FACTORY_RESET`；on_resume 置 key shield level 1。
- on_create 按 `extra.arg1`（home_sub_ui_type_e）直接进对应子页。
- 按键分流：FACTORY_RESET LONG5S → 恢复出厂确认；TRAVEL_LOCK LONG2S → 飞行模式；POWER/START LONG2S → 关机确认；POWER 单击 → 启动泵奶/模式确认；旋钮 → 模式调节；L/R → 单边力度；BOTH → 双边力度；FREQUENCY → 速度。
- 广播：PM_STATE_CHANGE（idle → 屏保/idle release）；电池回调：charge error → CHARGE、低电关机 → BATTERY。
- 屏保三方案（HOME_IDLE_SCHEME）：电源退出 / 任意键退出 / 任意键动作。
- 子 UI：HOME_UI_TYPE_{NONE/HOME/HOME_IDLE/PUMP_RESULT/SETTINGS_SUCCESS/NIGHT_LIGHT_ADJUST/POWER_OFF_HOLD/LEVEL_ADJUST/BOTH_LEVEL_ADJUST/SPEED_ADJUST}。

### 3.6 Pump（泵奶运行）
- 广播 `ID_BROADCAST_BREAST_PUMP_STATE_CHANGE`：COMPLETED → HOME(PUMP_RESULT)；TIMEOUT → 结算；RUNNING_30M_TIP → 暂停+提示；WILL_FINISH → 即将完成。
- 电池回调：charge error → 充电异常结算；低电 → POWER_OFF(LOW_POWER)；FORBID_START → 禁启动。
- 子 UI（PUMP_UI_TYPE_*）：HOME / RHYTHM_CONFIRM / SETTINGS_SUCCESS / STOP_HOLD / POWER_OFF_HOLD / LEVEL/BOTH_LEVEL/SPEED_ADJUST / FINISH_TIMEOUT / FINISH_CHARGE_ERROR / PAUSE_TIMEOUT / WILL_FINISH / LOW_POWER / SEAL_LEAK / FIT_NOT_DETECTED。
- 气密检测（V3Pro）：检测中（Checking seal...）不漏/轻微 → Seal check passed；大漏在本泵奶窗口未展示过则进 SEAL_LEAK，已展示过则补偿中（Air leak, suction adjusting...）。检测外大漏仅窗口内首次跳 SEAL_LEAK。佩戴检测超时（`WORN_OVT`）窗口内未展示过则进 FIT_NOT_DETECTED 并暂停；已展示过时检测中出补偿中、检测外不处理。`IS_SUPPORT_PUMP_FIT_NOT_DETECTED_ONCE` 控制进入泵奶窗口至退出仅展示一次未佩戴页。SEAL_LEAK 仅响应 power/start/旋钮；非末页短按 power/start 切下一页，末页启动泵奶并退出。FIT_NOT_DETECTED 短按 power/start 恢复泵奶。

### 3.7 Battery（低电）
- on_create：vbus+error → CHARGE；否则低电页 + 自动关机调度（EMPTY 立即，否则超时）。
- 出口：charge error→CHARGE；开始充电→退出；飞行→AIRPLANE；引导未完成→GUIDE；否则 HOME。

### 3.8 Airplane Mode（飞行模式）
- 长按解锁（LONG_2S），5s 进度弧预跑 1s；超时 `AIRPLANE_MODE_TIMEOUT_TIME` → POWER_OFF(AUTO)；`exit` → 分流。

### 3.9 Charge（充电）
- 永久 Wakelock；正常 → 充电显示完自动退出；适配器不匹配 → 充电异常页 + 1s 后停/断 BLE。
- LONG3S → 退出充电策略；LONG10S → reboot。
- V3Pro 适配器不匹配：POWER 单击 → DUMMY 开机引导。

### 3.10 OTA（升级）
- 广播 `ID_BROADCAST_OTA_STATE_CHANGE`：START→下载中 / SUCCESS→完成 / FAIL→错误。POWER/START 单击（非下载中）→ HOME。

### 3.11 Power Off（关机）
- 永久 Wakelock、关背光/夜灯、断/停 BLE、按 `extra.arg1` 埋点（MANUAL/LOW_POWER/AUTO/ABNORMAL）。充电态 LONG3S → 退出充电、LONG10S → reboot。
- V3Pro 关机充电 / 适配器不匹配：POWER 单击 → 开机引导。
- `PM_POWER_OFF_TYPE_CHARGE` + `CHARGE_STATE_ERROR`：停留在关机窗不适配 UI（复用 charge adapter 页），拔出或状态恢复非 ERROR 才关机。V3 未开 `IS_SUPPORT_POWER_OFF_WHEN_CHARGING`，因此不走关机充电页，只走该停留判定。

### 3.12 System Abnormal（系统异常）
- `ADD_ABNORMAL_STATE_CB` 订阅；`map_abnormal_type_to_ui_type` 映射 BATTERY_TEMP_HIGH/STORAGE_FAILURE/COMMUNICATION_ERROR；异常清除 → 退出。
- 出口优先级：factory 未部署→PRODUCT_TEST → 主题 NONE→THEME_GUIDE → charge→CHARGE → 低电→BATTERY → 飞行→AIRPLANE → 引导→GUIDE → HOME。
- `IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD`：LONG10S → 重启。

### 3.13 Product Test（产测）
- 广播 `ID_BROADCAST_FACTORY_TEST_CMD`：LCD 颜色 / 老化测试。on_resume 延迟 100ms `tests_manager_set_enable(true)`。子 handler：home/lcd_color/aging/common。

### 3.14 Factory Reset（恢复出厂）
- `extra.arg1==0` 静默重置；否则确认页。POWER DOWN→确认、UP→取消；非 POWER 单击 → 回 home 子页。

## 4. 子 UI 类型映射

**home_sub_ui_type_e（9 种）**：NONE / PUMP_RESULT(结算页) / PUMP_MODE_UNSELECT / PUMP_MODE_ADJUST / PUMP_RHYTHM_ADJUST / LEVEL_ADJUST(单边,arg2 左右) / BOTH_LEVEL_ADJUST / SPEED_ADJUST / MODE_SET。

**pump_sub_ui_type_e（2 种）**：NONE(直接启动泵奶) / SETTINGS_SUCCESS(先成功页再泵奶主页)。

**PUMP_UI_TYPE ↔ handler 文件**：RHYTHM_CONFIRM→pump_rhythm_confirm、SETTINGS_SUCCESS→pump_settings_success、STOP_HOLD→pump_stop_hold、POWER_OFF_HOLD→pump_power_off_hold、LEVEL/BOTH/SPEED_ADJUST→pump_*_adjust、FINISH_TIMEOUT→pump_finish_timeout、FINISH_CHARGE_ERROR→pump_finish_charge_error、PAUSE_TIMEOUT→pump_pause_timeout、WILL_FINISH→pump_will_finish、LOW_POWER→pump_low_power、SEAL_LEAK→pump_seal_leak、FIT_NOT_DETECTED→pump_fit_not_detected、HOME→pumping。

## 5. 通用退出策略（几乎所有窗口共用）

```
适配器不匹配(CHARGE_STATE_ERROR+VBUS) → CHARGE
  → 低电 → BATTERY
  → 飞行模式 → AIRPLANE_MODE
  → 引导未完成 → GUIDE
  → 否则 → HOME
```

（system_abnormal 前插 PRODUCT_TEST / THEME_GUIDE。）

## 6. 注意事项

- **永久 Wakelock**：CHARGE / POWER_OFF / SYSTEM_ABNORMAL 三窗口 `acquire_wakelock(name, -1)` 只在 destroy 释放。
- **`not_push_history=1`** 的窗口（DUMMY/HOME/POPUP/THEME_GUIDE）不进历史栈，保证 HOME 是导航根。
- **异步投递惯例**：广播回调只拷贝数据 + `POST_WM_TASK_HANDLER_NO_DELAY` 转发到 wm 线程，不在持锁上下文直接调窗口 API。
- **overlay 弹窗优先级**（`popup_overlay_priority_get`，越大越高）：旅行锁倒计时 40 ＞ 低电/即将关机 30 ＞ 蓝牙连断/绑定 10。已显示 overlay 时新请求走 `WINDOW_ASYNC_EVT_POPUP_UPDATE`：更高优先级或同级不同类型可原地抢占并重置计时；更低优先级或同类型忽略。旅行锁长按倒计时不被蓝牙打断。
- **模式切换确认持久化**：`pump_ui_persist_t.rhythm_confirm_shown` 按上电周期记忆，同时上报 `RHYTHM_SWITCHED_REPORT` 供 App 同步。
- **产测隔离**：PRODUCT_TEST 窗口 `tests_manager_set_enable(true/false)`；Dummy 唤醒分流依据 `tests_manager_get_current_mode()!=TST_NONE`。
- **按键码覆盖策略**：HOME/PUMP/PRODUCT_TEST 用全键+组合键；弹窗用高屏蔽；CHARGE 仅 POWER、BATTERY 仅 POWER/START。
- **长按语义**：LONG2S=关机确认、LONG5S=恢复出厂、LONG10S=强制重启。
- **IS_SUPPORT_* 配置化差异**：`IS_SUPPORT_GUIDE_WINDOW` / `IS_SUPPORT_THEME_GUIDE_WINDOW` / `IS_SUPPORT_POWER_ON_GUIDE_UI` / `IS_SUPPORT_SYS_ABNORMAL_RESTART_HOLD` / `IS_SUPPORT_POWER_OFF_WHEN_CHARGING` 决定窗口出现与否。
