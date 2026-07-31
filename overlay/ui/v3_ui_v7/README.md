# V6 版 UI 框架说明

`overlay/ui/v3_ui_v6` 是吸奶器平台基于 **LVGL** 的第六代 UI 实现层，负责将业务窗口（`overlay/windows/windows_v5`）与底层显示调度（`ui_manager`）连接起来，完成各屏幕的绘制、刷新与资源管理。

---

## 1. 框架定位

```
┌─────────────────────────────────────────────────────────────┐
│  Window Manager (windows_v5)                               │
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
│  v3_ui_v6（本目录）                                           │
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
| v3_ui_v6 | 创建/更新/销毁 LVGL 控件树，加载主题资源，渲染具体界面 |

---

## 2. 目录结构

```
v3_ui_v6/
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
├── home_idle/                # 屏保
├── home_night_light/         # 夜灯调节
├── battery/                  # 电量显示组件
├── low_power/                # 低电量界面
├── charge/                   # 充电界面
├── charge_adapter/           # 充电异常界面
├── power_off_charge/         # 关机充电界面
├── power_off/                # 关机界面
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

---

## 4. 弹窗体系

V6 将弹窗分为两层，对应不同的显示通道与使用场景。

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

### 4.3 Base 与 Overlay 绘制顺序

UI Manager Hook 在同一 LVGL 定时器回调中依次执行：

1. **Base 动画**（`play_info`）— 当前全屏页面
2. **Overlay 动画**（`overlay_play_info`）— 顶层弹窗，保证叠加在最上层

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
| `home_idle/` | `ui_display_home_screen_saver_*` | 屏保 |
| `home_night_light/` | `ui_display_home_night_light_adjust_*` | 夜灯调节 |
| `battery/` | `battery_ui_*` | 电量组件（被其他页面引用） |
| `low_power/` | `ui_display_battery_low_power_*` | 低电量全屏 |
| `charge/` | `ui_display_charge_screen_*` | 充电中 |
| `charge_adapter/` | `ui_display_charge_adapter_screen_*` | 充电异常 |
| `power_off_charge/` | `ui_display_power_off_charge_screen_*` | 关机充电 |
| `power_off/` | `ui_display_power_off_*` | 关机 |
| `ota/` | `ui_display_ota_*` | OTA 升级 |
| `airplane_mode/` | `ui_display_airplane_mode_*` | 飞行模式 |
| `factory_reset/` | `ui_display_factory_reset_*` / `ui_display_factory_reset_confirm_*` | 恢复出厂 |
| `system_abnormal/` | `ui_display_system_abnormal_*` | 系统异常 |
| `product_tests/` | `ui_display_product_tests_home_*` 等 | 产测 |
| `ble/` | `ble_ui_init/update/deinit` | 蓝牙图标组件 |

所有页面头文件通过 `ui_page_define.h` 统一聚合，业务侧只需 `#include "ui_page_define.h"` 即可访问全部 UI API。

---

## 8. 与 Window Manager 的协作流程

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

---

## 9. 新增页面开发指南

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
#include "my_page_ui.h" // IWYU pragma: keepin
```

### 步骤 4：在 Window Handler 中对接

参考 `overlay/windows/windows_v5/home/home_ui_handler.c`：

- 实现 `display_prepare_cb` / `display_remove_cb`
- 在 `progress_cb` 的 `ANI_TIMER_DRAW_AFTER` 中调用 `process`
- 通过 `ui_play_info_t.user_data` 传入窗口上下文

### 步骤 5：资源与图片

1. 在 Resource Manager 中注册资源 ID
2. 页面内通过 `ui_img_resource_init_with_theme()` 加载
3. 使用 `ui_img_baj_theme_set_src()` 解码并绑定到 `lv_image`

---

## 10. 调试与测试开关

部分页面提供编译期测试宏，可在对应 `.c` 文件中启用以单独调试某一界面：

| 宏 | 位置 | 作用 |
|----|------|------|
| `IS_SUPPORT_HOME_UI_TEST` | `home_ui.c` | 跳过正常 home，直接显示指定测试页 |
| `IS_SUPPORT_LOGO_UI_TEST` | `logo_ui.c` | Logo 测试模式 |

本地调试时可在 `process` 中临时强制 `proc->popup_ui_type` 以验证弹窗样式。

---

## 11. 与 V5 UI 的主要差异

| 维度 | v3_ui_v5 | v3_ui_v6 |
|------|----------|----------|
| 页面组织 | 较扁平，部分功能合并 | 按业务拆分为独立目录，Base/Custom 分层 |
| 弹窗 | 集中在 `popup/` | 拆分为 `page_popup`（内嵌）与 `top_popup`（顶层） |
| 主界面 | 单一实现 | `home/base` 按力度/模式/标签拆分子模块 |
| 图片解码 | 较简单 | 独立 `decode/` 模块，支持 BAJ/PNG/JPG |
| 公共组件 | 较少 | `wave`、`snapshot`、`ble` 等可复用组件 |
| 泵奶界面 | `breast_pump_ui` | 独立 `pump_home` + `pump_finish` |

---

## 12. 依赖关系

```
v3_ui_v6
  ├── ui_manager_api.h          # UI 调度上下文
  ├── resource_manager_api.h    # 主题/图片资源
  ├── root_message.h            # popup_ui_id_e 等公共枚举
  ├── lvgl.h                      # LVGL 控件
  └── window_screen_*.h         # 各窗口业务上下文（overlay/windows）
```

编译时通过 overlay 构建系统将 `v3_ui_v6` 及其子目录加入 include path，与 `windows_v5` 配对使用。

---

## 13. 注意事项

1. **线程安全：** 所有 LVGL 操作必须在 UI 线程（`lv_lock`/`lv_unlock` 保护范围内）执行，`prepare`/`remove` 由 UI Manager Hook 保证。
2. **内存管理：** 页面上下文使用 `u_malloz`/`u_free` 分配；LVGL 对象使用 `LVGL_OBJ_DELETE` 宏销毁；图片解码缓存须在 `remove` 或换图前显式释放。
3. **增量刷新：** `process` 中应通过 `force_update` 等标志避免每帧全量重建控件，仅更新变化部分。
4. **弹窗通道选择：** 依附当前页面的提示用 `page_popup`；需要盖住整个屏幕、由独立 Popup Window 驱动的用 `top_popup` + Overlay 通道。
5. **FPS 监控：** 页面 `process` 末尾可统计并打印 FPS（参考 `home_base_ui_process`），用于性能调优。
