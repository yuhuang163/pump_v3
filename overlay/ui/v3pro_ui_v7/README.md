# V7 版 UI 框架说明

`overlay/ui/v3pro_ui_v7` 是吸奶器平台基于 **LVGL** 的第七代 UI 实现层，负责将业务窗口（`overlay/windows/windows_v6`）与底层显示调度（`ui_manager`）连接起来，完成各屏幕的绘制、刷新与资源管理。

> **V6 → V7 主要变化：** 新增力度/速度独立调节弹窗、长按交互组件、泵奶多阶段超时/即将完成提示、韵律确认弹窗、进度弧/流光通用组件等 16 个模块。详见 [13. V7 新增内容](#13-v7-新增内容)。

---

## 1. 框架定位

```
┌─────────────────────────────────────────────────────────────┐
│  Window Manager (windows_v6)                               │
│  窗口生命周期 / 按键 / 业务状态机                              │
└──────────────────────────┬──────────────────────────────────┘
                           │ ui_play_info_t (prepare/process/remove)
                           ▼
┌─────────────────────────────────────────────────────────────┐
│  UI Manager Hook (overlay/framework_hooks/ui)                │
│  LVGL 线程 / 动画定时器 / Base + Overlay 双层绘制              │
└──────────────────────────┬──────────────────────────────────┘
                           │ ui_display_xxx_prepare/process/remove
                           ▼
┌─────────────────────────────────────────────────────────────┐
│  v3pro_ui_v7（本目录）                                           │
│  页面 UI / 弹窗 / 图片解码 / 波形 / 快照                        │
└──────────────────────────┬──────────────────────────────────┘
                           │ ui_img_resource / resource_manager
                           ▼
┌─────────────────────────────────────────────────────────────┐
│  Resource Manager + LVGL                                     │
└─────────────────────────────────────────────────────────────┘
```

**职责边界：**

| 层级 | 职责 |
|------|------|
| Window Manager | 决定显示哪个窗口、何时进入/退出、业务数据读写 |
| UI Manager | 在独立 LVGL 线程中调度帧刷新，管理 Base 动画与 Overlay 叠加层 |
| v3pro_ui_v7 | 创建/更新/销毁 LVGL 控件树，加载主题资源，渲染具体界面 |

---

## 2. 目录结构

```
v3pro_ui_v7/
├── ui_page_define.h          # 统一头文件入口，聚合所有页面与公共模块
├── ui_img_define.h           # 图片数据结构定义
│
├── home/                     # 主界面（力度/模式/标签）
│   ├── home_ui.c             # 页面门面：Base/Custom 主题分发
│   ├── base/                 # 默认主题实现（按子模块拆分）
│   └── custom/               # 个性化主题实现
│
├── pump_home/                # 泵奶中界面
├── pump_finish/              # 泵奶完成提示
├── pump_finish_timeout/      # [V7] 泵奶完成超时提示
├── pump_will_finish/         # [V7] 泵奶即将完成提醒
├── home_idle/                # 屏保
├── home_night_light/         # 夜灯调节
├── battery/                  # 电量显示组件
├── low_power/                # 低电量界面
├── charge/                   # 充电界面
├── charge_adapter/           # 充电异常界面
├── power_off_charge/         # 关机充电界面
├── power_off/                # 关机界面
├── power_off_hold/           # [V7] 关机长按保持动画
├── power_on_hold/            # [V7] 开机长按保持动画
├── ble/                      # 蓝牙状态图标组件
├── guide/                    # 新手引导
├── theme_guide/              # 主题选择引导
├── logo/                     # 开机 Logo
├── ota/                      # OTA 升级界面
├── airplane_mode/            # 飞行模式
├── factory_reset/            # 恢复出厂
├── system_abnormal/          # 系统异常（存储/通信/温度等）
├── product_tests/            # 产测界面
│
│   # === V7 新增调节弹窗（独立于主页/泵奶页的力度速度调节 Overlay） ===
├── home_level_adjust/        # [V7] 主页力度调节弹窗
├── home_speed_adjust/        # [V7] 主页速度/频率调节弹窗
├── home_both_level_adjust/   # [V7] 主页双边力度同时调节弹窗
├── pump_level_adjust/        # [V7] 泵奶页力度调节弹窗
├── pump_speed_adjust/        # [V7] 泵奶页速度/频率调节弹窗
├── pump_both_level_adjust/   # [V7] 泵奶页双边力度同时调节弹窗
│
│   # === V7 新增泵奶交互模块 ===
├── pump_pause_timeout/       # [V7] 泵奶暂停超时提示
├── pump_stop_hold/           # [V7] 泵奶停止长按保持动画
│
│   # === V7 新增弹窗 ===
├── rhythm_confirm/           # [V7] 韵律模式切换确认弹窗
├── settings_success/         # [V7] 设置操作成功反馈提示
│
│   # === V7 新增通用组件 ===
├── progress_arc/             # [V7] 进度弧通用组件（环形进度条）
├── shine/                    # [V7] 流光/闪耀动画效果组件
│
├── page_popup/               # 页面内嵌弹窗（依附于当前页面 root）
├── top_popup/                # 顶层弹窗（独立 Overlay 窗口）
│
├── resource/                 # 图片资源加载封装
├── decode/                   # BAJ / PNG / JPG 解码
├── snapshot/                 # LVGL 快照缓存
└── wave/                     # 泵奶模式波形动画组件
```

---

## 3. 核心设计模式

### 3.1 页面三件套（Prepare / Process / Remove）

每个全屏页面对外暴露统一的生命周期接口，命名规范为：

```c
void ui_display_<page>_prepare(ui_manager_context_t *thiz);
void ui_display_<page>_remove(ui_manager_context_t *thiz);
int  ui_display_<page>_process(ui_manager_context_t *thiz);
```

| 阶段 | 调用时机 | 典型操作 |
|------|----------|----------|
| `prepare` | 页面首次显示前（UI 线程） | 分配上下文、创建 LVGL 根节点、初始化子组件、读取业务初始状态 |
| `process` | 每帧刷新（`ANI_TIMER_DRAW_AFTER`） | 同步业务数据、增量更新控件、触发子模块刷新 |
| `remove` | 页面退出时 | 释放解码缓存、删除 LVGL 对象、释放堆内存 |

Window Handler 通过 `ui_play_info_t` 将上述接口注册到 UI Manager：

```c
ui_play_info_t play_info = {
    .tag = "home_default_screen",
    .fps = HOME_ANIMATION_FREQ,
    .progress_cb = home_animation_progress_cb,
    .display_prepare_cb = home_ui_display_prepare,   // → ui_display_home_prepare
    .display_remove_cb  = home_ui_display_remove,    // → ui_display_home_remove
    .user_data = proc,  // 窗口上下文，如 ui_proc_home_t *
};
ui->hook->ui_animation_play_init(ui, play_info);
```

在 `progress_cb` 的 `ANI_TIMER_DRAW_AFTER` 分支中调用 `ui_display_xxx_process()`。

### 3.2 Base / Custom 双主题分发

支持 `RES_THEME_TYPE_BASE`（默认）与 `RES_THEME_TYPE_CUSTOM`（个性化）两套视觉资源。带主题的页面在门面文件（如 `home_ui.c`）中按当前主题类型动态绑定实现：

```c
int res_theme_type = thiz->priv.res->get_sys_theme_type();
if (res_theme_type == RES_THEME_TYPE_CUSTOM) {
    prepare = home_custom_ui_prepare;
    s_ops.remove  = home_custom_ui_remove;
    s_ops.process = home_custom_ui_process;
} else {
    prepare = home_base_ui_prepare;
    s_ops.remove  = home_base_ui_remove;
    s_ops.process = home_base_ui_process;
}
```

**适用页面：** `home`、`pump_home`、`pump_finish`、`guide`、`charge_adapter`、`home_idle`、`home_night_light`、`battery`、`low_power`、`ble` 等。

**复杂页面的子模块拆分（以 home 为例）：**

```
home/base/
├── home_base_ui.c             # 根节点、prepare/remove/process 总控
├── home_base_page_ui.c        # 多页轮播与切页过渡
├── home_base_home_page_ui.c   # 主页内容
├── home_base_animation.c      # 主页动画
└── home_base_ui_internal.h    # 页面私有数据结构与内部 API
```

### 3.3 子模块组件（Module）

部分可复用 UI 片段以独立模块形式存在，提供 `init / deinit / update` 接口，由父页面在 `prepare` 和 `process` 中调用。典型代表为 `ble_ui`：

```c
ble_ui_init(thiz);                                          // prepare 阶段
ble_ui_update(thiz, root, ble, x, y, &sync);               // 每帧 process
ble_ui_deinit(thiz);                                        // remove 阶段
```

V7 新增的通用组件（`progress_arc`、`shine`）也遵循此模式，可被多个页面复用。

---

## 4. 弹窗体系

V7 将弹窗分为两层，对应不同的显示通道与使用场景。

### 4.1 页面内嵌弹窗（`page_popup/`）

- **挂载方式：** 作为当前页面 `root` 的子节点
- **入口 API：** `ui_popup_update()` / `ui_popup_remove()`
- **适用场景：** 泵奶暂停/完成提示、操作结果、飞行模式提示、主题重选等
- **驱动方式：** 在页面 `process` 中根据 `proc->popup_ui_type` 调用

```c
ui_popup_update(thiz, proc->popup_ui_type, home_info->root, &home_info->popup_ui_index);
```

支持的 `popup_ui_id_e` 类型（节选）：

| 类型 | 说明 |
|------|------|
| `ID_POPUP_UI_PUMP_PAUSE_*` | 泵奶暂停相关 |
| `ID_POPUP_UI_PUMP_FINISH_*` | 泵奶完成相关 |
| `ID_POPUP_UI_OP_SUCCESS/FAILED` | 操作结果 |
| `ID_POPUP_UI_AIRPLANE_MODE_PROMPT` | 飞行模式提示 |
| `ID_POPUP_UI_THEME_RESELECT` | 主题重选 |
| `ID_POPUP_UI_RHYTHM_CONFIRM` | [V7] 韵律切换确认 |
| `ID_POPUP_UI_SETTINGS_SUCCESS` | [V7] 设置成功反馈 |

### 4.2 顶层弹窗（`top_popup/`）

- **挂载方式：** 通过 UI Manager 的 **Overlay** 通道独立绘制，不依附页面 root
- **入口 API：** `popup_window_ui_prepare()` / `popup_window_ui_process()` / `popup_window_ui_remove()`
- **适用场景：** 蓝牙连接状态、设备绑定、倒计时关机、低电量、系统/温度异常等全屏遮罩弹窗
- **驱动方式：** Window Popup Handler 调用 `ui_animation_overlay_init`

```c
ui->hook->ui_animation_overlay_init(ui, play_info);  // overlay 通道
// play_info.display_prepare_cb → popup_window_ui_prepare
// progress_cb ANI_TIMER_DRAW_AFTER → popup_window_ui_process
```

支持的 `popup_ui_id_e` 类型（节选）：

| 类型 | 说明 |
|------|------|
| `ID_POPUP_UI_BT_CONNECT/DISCONNECT/CONNECTING` | 蓝牙状态 |
| `ID_POPUP_UI_DEVICE_BIND/UNBIND` | 设备绑定 |
| `ID_POPUP_UI_TRAVEL_LOCK_COUNTDOWN/LOW_POWER/WILL_POWER_OFF` | 电源相关 |
| `ID_POPUP_UI_PUMP_PAUSE_SETTLE` | 暂停结算 |
| `ID_POPUP_UI_SYS_ERROR/TEMP_ERROR` | 系统/温度异常 |
| `ID_POPUP_UI_PUMP_PAUSE_TIMEOUT` | [V7] 泵奶暂停超时提示 |
| `ID_POPUP_UI_PUMP_FINISH_TIMEOUT` | [V7] 泵奶完成超时提示 |
| `ID_POPUP_UI_PUMP_WILL_FINISH` | [V7] 泵奶即将完成提醒 |

### 4.3 V7 新增：独立调节弹窗

V7 将力度调节、速度调节从页面内嵌弹窗中独立出来，作为专门的 Overlay 子模块（`home_level_adjust/`、`home_speed_adjust/`、`pump_level_adjust/` 等），支持更精细的动画与交互（如长按连续调节、双边同步调节）。它们通过 `ui_animation_overlay_init` 走独立 Overlay 通道，与页面生命周期间解耦。

### 4.4 Base 与 Overlay 绘制顺序

UI Manager Hook 在同一 LVGL 定时器回调中依次执行：

1. **Base 动画**（`play_info`）— 当前全屏页面
2. **Overlay 动画**（`overlay_play_info`）— 顶层弹窗与调节弹窗，保证叠加在最上层

---

## 5. 资源与图片解码

### 5.1 资源加载（`resource/ui_img_resource.c`）

封装 Resource Manager 的加载接口，页面通过 `res_img_info_t` 持有已加载的压缩资源：

| API | 说明 |
|-----|------|
| `ui_img_resource_init()` | 加载基础主题资源 |
| `ui_img_resource_init_with_theme()` | 按当前用户主题加载 |
| `ui_theme_img_resource_init()` | 指定主题类型加载 |
| `ui_img_resource_deinit()` | 卸载并释放 |

### 5.2 图片解码（`decode/`）

将压缩格式解码为 LVGL 可用的 `lv_draw_buf`，再设置到 `lv_image`：

| 模块 | 格式 | 主要 API |
|------|------|----------|
| `ui_img_baj_decode` | BAJ（平台主力格式） | `ui_img_baj_set_src()`、`ui_img_baj_theme_set_src()` |
| `ui_img_png_decode` | PNG | 用于弹窗对比背景等 |
| `ui_img_jpg_decode` | JPG | 部分大图资源 |

解码失败时 BAJ 模块会回退到 LVGL memfs 路径加载。换图前须调用 `ui_img_baj_release_decoded_buf()` 释放旧缓存。

### 5.3 辅助绘制工具

`ui_img_resource.c` 还提供弧形文字排版等工具函数：

- `ui_text_place_in_circle()` — 沿圆弧放置文字
- `ui_place_dot_on_arc()` — 在弧线上放置指示点

---

## 6. 公共组件

### 6.1 波形组件（`wave/ui_wave`）

用于主界面/泵奶界面展示泵奶模式韵律波形，支持：

- 波形类型：`STIMULATION`（刺激）、`LACTATION`（泌乳）、`MIX`（混合）
- 可配置颜色、振幅、周期、阴影
- 滚动动画（默认 20fps）

### 6.2 快照组件（`snapshot/ui_snapshot`）

预渲染并缓存常用背景图（如 `UI_SNAP_SHOT_COMMON_BG`），减少每帧重复解码开销。在页面切换前通过 `ui_snap_shot_perpare()` 准备。

### 6.3 进度弧组件（`progress_arc/`）[V7]

通用环形进度条组件，支持：

- 可配置起始角度、扫掠角度、半径、线宽
- 渐变色支持
- 平滑动画过渡
- 用于充电进度、泵奶进度等场景

### 6.4 流光组件（`shine/`）[V7]

闪耀/流光动画效果组件，用于：

- 开机/关机动画增强
- 按钮高亮流光效果
- 状态切换过渡动画

---

## 7. 页面清单

| 目录 | 对外接口 | 说明 |
|------|----------|------|
| `logo/` | `ui_display_logo_*` | 开机 Logo |
| `guide/` | `ui_display_guide_*` | 新手引导 |
| `theme_guide/` | `ui_display_theme_guide_*` | 主题选择 |
| `home/` | `ui_display_home_*` | 主界面 |
| `pump_home/` | `ui_display_pump_home_screen_*` | 泵奶中 |
| `pump_finish/` | `ui_display_pump_complete_tip_*` | 泵奶完成 |
| `pump_finish_timeout/` | `ui_display_pump_finish_timeout_*` | [V7] 泵奶完成超时提示 |
| `pump_will_finish/` | `ui_display_pump_will_finish_*` | [V7] 泵奶即将完成提醒 |
| `home_idle/` | `ui_display_home_screen_saver_*` | 屏保 |
| `home_night_light/` | `ui_display_home_night_light_adjust_*` | 夜灯调节 |
| `battery/` | `battery_ui_*` | 电量组件（被其他页面引用） |
| `low_power/` | `ui_display_battery_low_power_*` | 低电量全屏 |
| `charge/` | `ui_display_charge_screen_*` | 充电中 |
| `charge_adapter/` | `ui_display_charge_adapter_screen_*` | 充电异常 |
| `power_off_charge/` | `ui_display_power_off_charge_screen_*` | 关机充电 |
| `power_off/` | `ui_display_power_off_*` | 关机 |
| `power_off_hold/` | `ui_display_power_off_hold_*` | [V7] 关机长按保持动画 |
| `power_on_hold/` | `ui_display_power_on_hold_*` | [V7] 开机长按保持动画 |
| `ota/` | `ui_display_ota_*` | OTA 升级 |
| `airplane_mode/` | `ui_display_airplane_mode_*` | 飞行模式 |
| `factory_reset/` | `ui_display_factory_reset_*` / `ui_display_factory_reset_confirm_*` | 恢复出厂 |
| `system_abnormal/` | `ui_display_system_abnormal_*` | 系统异常 |
| `product_tests/` | `ui_display_product_tests_home_*` 等 | 产测 |
| `ble/` | `ble_ui_init/update/deinit` | 蓝牙图标组件 |
| `home_level_adjust/` | `ui_display_home_level_adjust_*` | [V7] 主页力度调节弹窗 |
| `home_speed_adjust/` | `ui_display_home_speed_adjust_*` | [V7] 主页速度调节弹窗 |
| `home_both_level_adjust/` | `ui_display_home_both_level_adjust_*` | [V7] 主页双边力度调节弹窗 |
| `pump_level_adjust/` | `ui_display_pump_level_adjust_*` | [V7] 泵奶力度调节弹窗 |
| `pump_speed_adjust/` | `ui_display_pump_speed_adjust_*` | [V7] 泵奶速度调节弹窗 |
| `pump_both_level_adjust/` | `ui_display_pump_both_level_adjust_*` | [V7] 泵奶双边力度调节弹窗 |
| `pump_pause_timeout/` | `ui_display_pump_pause_timeout_*` | [V7] 泵奶暂停超时提示 |
| `pump_stop_hold/` | `ui_display_pump_stop_hold_*` | [V7] 泵奶停止长按保持动画 |
| `rhythm_confirm/` | `ui_display_rhythm_confirm_*` | [V7] 韵律切换确认弹窗 |
| `settings_success/` | `ui_display_settings_success_*` | [V7] 设置成功反馈提示 |
| `progress_arc/` | `progress_arc_*` | [V7] 环形进度条通用组件 |
| `shine/` | `shine_*` | [V7] 流光/闪耀动画组件 |

所有页面头文件通过 `ui_page_define.h` 统一聚合，业务侧只需 `#include "ui_page_define.h"` 即可访问全部 UI API。

---

## 8. V7 新增交互模式

### 8.1 长按交互（Hold）

V7 新增了多处长按保持交互：

| 模块 | 场景 | 行为 |
|------|------|------|
| `power_off_hold/` | 关机确认 | 长按电源键，环形进度动画倒计时后关机 |
| `power_on_hold/` | 开机确认 | 长按电源键，环形进度动画倒计时后开机 |
| `pump_stop_hold/` | 停止泵奶 | 长按停止键，进度动画倒计时后停止泵奶 |

这些模块依赖 `progress_arc/` 组件渲染倒计时进度。

### 8.2 力度/速度独立调节

V7 将 V6 中内嵌在页面内的力度/速度调节逻辑拆分为独立 Overlay 模块：

- **主页：** `home_level_adjust/`（力度）、`home_speed_adjust/`（速度）、`home_both_level_adjust/`（双边同步）
- **泵奶页：** `pump_level_adjust/`（力度）、`pump_speed_adjust/`（速度）、`pump_both_level_adjust/`（双边同步）

每个调节弹窗均通过独立 Overlay 通道显示，支持退出保护（短按 power 键退出，非 power 键不退出）等 V7 新增策略。

### 8.3 泵奶多阶段超时提醒

V7 新增了泵奶流程中的多阶段超时/完成提醒：

| 模块 | 触发时机 | 行为 |
|------|----------|------|
| `pump_will_finish/` | 泵奶即将结束前 N 秒 | 弹出即将完成提示，用户可选择提前结束或继续 |
| `pump_finish_timeout/` | 泵奶完成后超时 | 若用户未操作，弹出超时提示引导用户退出 |
| `pump_pause_timeout/` | 泵奶暂停后超时 | 暂停超过设定时间后提示用户恢复或结束 |

### 8.4 韵律确认弹窗（`rhythm_confirm/`）

泵奶过程中韵律子模式自动切换时，弹出确认弹窗供用户确认或取消。V7 优化了策略：非确认退出时首次弹出标志不清零，避免重复弹出。

---

## 9. 与 Window Manager 的协作流程

以主界面为例，完整调用链如下：

```
1. Window 进入 HOME
      ↓
2. start_home_ui_animation(proc)
      ↓
3. ui->hook->ui_animation_play_init(play_info)    [消息投递到 LVGL 线程]
      ↓
4. display_prepare_cb → ui_display_home_prepare()
      ├── 分配 home_info
      ├── 创建 LVGL root
      ├── ble_ui_init()
      └── 读取泵奶模式等初始状态
      ↓
5. 每帧 ANI_TIMER_DRAW_AFTER → ui_display_home_process()
      ├── 同步力度/模式业务数据
      ├── 更新子模块 UI
      └── ui_popup_update()（如有内嵌弹窗）
      ↓
6. 窗口退出 → ui_animation_play_deinit
      ↓
7. display_remove_cb → ui_display_home_remove()
      ├── ble_ui_deinit()
      ├── ui_popup_remove()
      └── 释放 home_info
```

弹窗窗口（`ID_WINDOW_POPUP_WINDOW`）走 Overlay 通道，由 `popup_windows/*_popup_handle.c` 中的 Handler 驱动 `top_popup` 模块。

V7 新增的独立调节弹窗（力度/速度）同样走 Overlay 通道，但拥有独立的 play_info 和生命周期，不与主页面 root 绑定。

---

## 10. 新增页面开发指南

### 步骤 1：创建目录与三件套

```
my_page/
├── my_page_ui.c          # 门面（主题分发 + 对外 API）
├── my_page_ui.h          # 头文件声明
├── base/
│   ├── my_page_base_ui.c
│   └── my_page_base_ui_internal.h
└── custom/               # 若有个性化主题
    ├── my_page_custom_ui.c
    └── my_page_custom_ui_internal.h
```

### 步骤 2：实现生命周期函数

```c
// my_page_ui.h
void ui_display_my_page_prepare(ui_manager_context_t *thiz);
void ui_display_my_page_remove(ui_manager_context_t *thiz);
int  ui_display_my_page_process(ui_manager_context_t *thiz);
```

### 步骤 3：注册到 `ui_page_define.h`

```c
#include "my_page/my_page_ui.h" // IWYU pragma: keep
```

### 步骤 4：在 Window Handler 中对接

参考 `overlay/windows/windows_v6/home/home_ui_handler.c`：

- 实现 `display_prepare_cb` / `display_remove_cb`
- 在 `progress_cb` 的 `ANI_TIMER_DRAW_AFTER` 中调用 `process`
- 通过 `ui_play_info_t.user_data` 传入窗口上下文

### 步骤 5：资源与图片

1. 在 Resource Manager 中注册资源 ID
2. 页面内通过 `ui_img_resource_init_with_theme()` 加载
3. 使用 `ui_img_baj_theme_set_src()` 解码并绑定到 `lv_image`

---

## 11. 调试与测试开关

部分页面提供编译期测试宏，可在对应 `.c` 文件中启用以单独调试某一界面：

| 宏 | 位置 | 作用 |
|----|------|------|
| `IS_SUPPORT_HOME_UI_TEST` | `home_ui.c` | 跳过正常 home，直接显示指定测试页 |
| `IS_SUPPORT_LOGO_UI_TEST` | `logo_ui.c` | Logo 测试模式 |

本地调试时可在 `process` 中临时强制 `proc->popup_ui_type` 以验证弹窗样式。

---

## 12. 与 V6 UI 的主要差异

| 维度 | v3pro_ui_v6 | v3pro_ui_v7 |
|------|----------|----------|
| 页面数量 | ~28 个目录 | ~42 个目录 |
| 力度/速度调节 | 内嵌在页面中 | 独立 Overlay 模块（6 个） |
| 长按交互 | 无独立组件 | `power_off_hold` / `power_on_hold` / `pump_stop_hold` |
| 泵奶超时提醒 | 无 | `pump_finish_timeout` / `pump_pause_timeout` / `pump_will_finish` |
| 韵律确认 | page_popup 内嵌 | 独立 `rhythm_confirm` 模块，策略优化 |
| 通用组件 | `wave`、`snapshot`、`ble` | 新增 `progress_arc`（环形进度）、`shine`（流光） |
| 设置反馈 | page_popup 通用弹窗 | 独立 `settings_success` 模块 |
| Window Manager | windows_v5 | windows_v6 |
| 弹窗退出策略 | 统一策略 | 区分 power 键/非 power 键退出 |

---

## 13. V7 新增内容

以下 16 个模块为 V7 相对于 V6 的新增：

| 模块 | 类型 | 说明 |
|------|------|------|
| `home_level_adjust` | Overlay 弹窗 | 主页力度调节，支持独立 Overlay 通道 |
| `home_speed_adjust` | Overlay 弹窗 | 主页速度/频率调节 |
| `home_both_level_adjust` | Overlay 弹窗 | 主页双边力度同步调节 |
| `pump_level_adjust` | Overlay 弹窗 | 泵奶页力度调节，支持短按 power 退出 |
| `pump_speed_adjust` | Overlay 弹窗 | 泵奶页速度/频率调节 |
| `pump_both_level_adjust` | Overlay 弹窗 | 泵奶页双边力度同步调节 |
| `power_off_hold` | 全屏 Overlay | 关机长按保持，环形进度倒计时 |
| `power_on_hold` | 全屏 Overlay | 开机长按保持，环形进度倒计时 |
| `pump_stop_hold` | Overlay 动画 | 泵奶停止长按保持动画 |
| `pump_finish_timeout` | 顶层弹窗 | 泵奶完成后超时未操作提示 |
| `pump_pause_timeout` | 顶层弹窗 | 泵奶暂停后超时提示 |
| `pump_will_finish` | 顶层弹窗 | 泵奶即将完成前提醒 |
| `rhythm_confirm` | Overlay 弹窗 | 韵律子模式切换确认，退出策略优化 |
| `settings_success` | 页面弹窗 | 设置操作成功后的独立反馈提示 |
| `progress_arc` | 通用组件 | 环形进度条，可配置颜色/角度/动画 |
| `shine` | 通用组件 | 流光/闪耀动画效果 |

---

## 14. 依赖关系

```
v3pro_ui_v7
  ├── ui_manager_api.h          # UI 调度上下文
  ├── resource_manager_api.h    # 主题/图片资源
  ├── root_message.h            # popup_ui_id_e 等公共枚举
  ├── lvgl.h                      # LVGL 控件
  └── window_screen_*.h         # 各窗口业务上下文（overlay/windows/windows_v6）
```

编译时通过 overlay 构建系统将 `v3pro_ui_v7` 及其子目录加入 include path，与 `windows_v6` 配对使用。

---

## 15. 注意事项

1. **线程安全：** 所有 LVGL 操作必须在 UI 线程（`lv_lock`/`lv_unlock` 保护范围内）执行，`prepare`/`remove` 由 UI Manager Hook 保证。
2. **内存管理：** 页面上下文使用 `u_malloz`/`u_free` 分配；LVGL 对象使用 `LVGL_OBJ_DELETE` 宏销毁；图片解码缓存须在 `remove` 或换图前显式释放。
3. **增量刷新：** `process` 中应通过 `force_update` 等标志避免每帧全量重建控件，仅更新变化部分。
4. **弹窗通道选择：** 依附当前页面的提示用 `page_popup`；需要盖住整个屏幕、由独立 Popup Window 驱动的用 `top_popup` + Overlay 通道；独立调节弹窗走独立 Overlay play_info。
5. **按键退出策略：** V7 对调节弹窗区分 power 键和非 power 键退出行为——短按 power 键退出弹窗，非 power 键不退出。
6. **FPS 监控：** 页面 `process` 末尾可统计并打印 FPS（参考 `home_base_ui_process`），用于性能调优。
7. **Overlay 层级：** 多个 Overlay 同时存在时（如调节弹窗 + 顶层弹窗），注意 `overlay_play_info` 的栈式管理，避免层级混乱。
