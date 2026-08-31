# UI 页面层业务实现（overlay/ui/v3_ui_v7 / v3pro_ui_v7）

页面层是 UI 三层结构中的**渲染端**：用 LVGL 构建控件树，纯粹"读状态、画控件"，不做业务决策。`ui_proc_xxx_t`（业务上下文）由窗口层传入，页面层通过 `READ_*` 宏只读消费。

## 1. 三段式生命周期

每个页面一组三件套（facade），统一注册到 `ui_page_define.h`：

| 函数 | 线程 | 职责 |
|------|------|------|
| `ui_display_xxx_prepare` | UI 线程 | `u_malloz` 私有 `xxx_display_info_t`、`xxx_root_init()`（黑底非点击 root）、组件 init、读初始状态、`force_update=true` |
| `ui_display_xxx_process` | 每帧 | 从 `play_info.user_data` 取 proc，`READ_*` 读状态与 `shown_xxx` 缓存比对决定是否重绘，调子模块 update，帧尾 `force_update=false` |
| `ui_display_xxx_remove` | 退出 | 释放解码缓冲、`ui_shine_remove`、组件 deinit、`LVGL_OBJ_DELETE(root)`、`u_free(ctx)` |

关键机制：
- **增量刷新**：`shown_xxx` 缓存 + `force_update`（进入/切页/状态翻转必须置 true）。
- **`READ_*` 宏**：页面不直接碰 manager，用 `READ_PUMP_SETTING_INFO` / `READ_PUMP_WORK_MODE` / `READ_PUMP_MODE_STEP_INDEX` / `READ_PUMP_RESULT_INFO` 读进 proc 内联结构。
- **定时完成只报一次**：`completed` 标志保护 `progress_cb(ANI_POPUP_DISPLAY_FINISH)`。

## 2. 页面业务分组

### 2.1 home（首页）
- facade 按主题分派 `home_base_*` / `home_custom_*`。
- 页面模型：`HOME_PAGE_MAIN + MODE_0/1 + RHYTHM_0/1/2`（第 6 页 customize 由 `custom_mode_active` 门控）。
- MAIN 页：模式背景 PNG、模式标题、计时器（2 段 `MM:SS` / 3 段 `HH:MM:SS`）、shine 提示、速度图标 + 数值、Lx/Rx 档位。
- 模式页：260px 深色圆底、模式图标、标题（超长省略号 + 二分法自适应字号）、时长、chrome 圆点、shine 提示。
- 切换动画两段式插值（先滑移再 hero 缩放）。

### 2.2 pump_home（泵运行动画页）
- process：`READ_PUMP_MODE_STEP_INDEX` → time_index → `READ_PUMP_SETTING_INFO` → 子模式档位；`anim_play_state = (pump_status == PAUSE)`。
- 波浪动画是**顺序 BAJ 帧资源序列**（非 `ui_wave` 组件，`IS_SUPPORT_WAVE_OBJECT_ENABLE=0` 禁用）；暂停时波浪上移 46px、透明度 60%。

### 2.3 调节类 Overlay 弹窗（V7 新增）
六个同构：home/pump × level/speed/both_level adjust。
- 结构：root + progress_arc + value_label + dir_label + tip_img(shine)。
- 强调色随模式：stimulation `0xE2637E`、lactation `0x64A4F2`、rhythm `0x9C78E8`。
- 档位上限：STIMULATION → LEVEL_5，其它 → LEVEL_15。
- 圆弧 220ms 平滑插值；speed 用 5 段独立 lv_arc；pump_level_adjust 追加吸奶累计时长。
- V7 按键退出策略：短按电源键退出调节弹窗，非电源键不再退出（与 V6 不同）。

### 2.4 pump_finish 系列与倒计时页
- `pump_finish`：成功图标 + 双行静态文案，force_update 一次性绘制。
- `pump_finish_timeout` / `pump_pause_timeout` / `pump_stop_hold`：2s 倒计时 progress_arc（done 色 0xEF4C4E）+ POWER_KEY_ICON。
- `power_off_hold` / `power_on_hold`：按住显示 2s 进度弧，`holding` 翻转时 reset。
- `airplane_mode`：长按解锁页，5s 进度弧预跑 1s。

### 2.5 纯状态提示页（静态）
`rhythm_confirm`（QUANDARY 图标 + shine 提示）、`settings_success`（两 style）、`system_abnormal`（系统错误/电池高温两套文案）、`charge_adapter`、`pump_will_finish`、`home_idle`（屏保）。

### 2.6 转圈等待页（spin arc 复用）
`power_off` / `factory_reset` / `restart` 三页同构：50px 弧段 3ms/度匀速旋转 + 文案（power_off 文案随 `power_off_text_type_e` 切换）。

### 2.7 guide（新手引导，V7 大页面）
- 约 24 个子页（QRCODE/INTRO/泵奶各步骤/模式/力度/速度/FINISH）。
- 每子页 `xxx_build/update/remove` 三件套；`guide_page_switch` 先 `guide_page_content_reset`（逐个 remove 再重建）。
- `guide_base_qrcode_ui.c`：从 `get_factory_data()->dev_info.sn` 拼绑定 URL 渲染 LVGL qrcode。

### 2.8 theme_guide（主题选择）
两选项（Base minimal / Custom IP）：预览 PNG + select_ring + 标签，读 `selected_index`。Base 预览资源被注释为 0，当前仅 IP 主题预览生效。

### 2.9 ota
三态：下载中（进度弧 + 百分比 + 体积）/ 错误 / 完成。进度弧**匀速追赶动画**（90°/s，换包回退时硬跳）。

### 2.10 product_tests（产线测试）
- `product_home_ui.c`：白底，MAC 名 + LVGL 二维码（MAC 冒号分隔）+ 老化时间（仅 AGING 模式）；V3Pro 追加电量。
- `product_lcd_color_ui.c`：`color_id==0x06` 画 16 灰阶横条，否则全屏纯色。
- `aging_test_ui.c`：蓝/粉背景每 10s 交替，显示老化时间 + 状态。

### 2.11 logo
`logo_display_ui.c`：加载 BAJ 启动图居中，先建后隐 50ms 再显示（防闪屏）。

## 3. 弹窗体系

### 3.1 page_popup（页内嵌）
`ui_popup.c` + `ui_popup_component.c`，挂在页面 root 内，`lv_obj_move_foreground(root)` 盖在内容上。支持 PUMP_RUNING_30M_TIP / PUMP_REST_MODE_ADJUST_TIP。组件：mask + 文本 + 图标 + ring 进度弧 + 倒计时数字。

### 3.2 top_popup（Overlay 独立层）
`ui_popup_window.c`，root 建在 `lv_layer_top()`。支持 BT_CONNECT/DISCONNECT/CONNECTING、TRAVEL_LOCK_COUNTDOWN、LOW_POWER。走 `overlay_play_info.progress_cb` 回传。旅行锁/飞行模式进度弧预跑 1s 对齐 LONG_2S。

## 4. 组件类

| 组件 | 职责 | 关键点 |
|------|------|--------|
| progress_arc | 倒计时进度弧（直径 352/线宽 11） | done+remain 双弧，被倒计时页/ota/travel_lock 复用 |
| wave | 矢量波浪 | `IS_SUPPORT_WAVE_OBJECT_ENABLE=0` 禁用，pump_home 用 BAJ 帧序列 |
| snapshot | 预解码缓冲缓存（NATIVE 背景/ARGB 图标/shine mask） | **缓冲归 snapshot 所有，页面严禁销毁** |
| shine | 提示图/文字扫光（mask + 亮层 + 软边 alpha） | 默认 mask 宽 140，被多页复用 |
| decode | BAJ/PNG/JPG 解码 | 解码失败回退 LVGL memfs PNG |
| resource | 资源封装（memfs 装载） | logo 加载 BAJ 启动图 |
| ble | 全局 BLE 图标 | 断开灰闪、连上隐藏 |
| battery | 电池图标 | 仅 V3Pro 有实现，V3 空桩 |

## 5. V3 vs V3Pro 差异

1. **`pump_seal_leak/` 仅 V3Pro**（密封性检查页，4 子页 CHECK_FIT/CHECK_TUBING/CHECK_CUP/PAUSE），`ui_page_define.h` 差异即 V3Pro 多 `#include "pump_seal_leak_ui.h"`。
2. **电量显示**：V3Pro 的 `battery_ui.c` 完整实现（电池框+填充+充电图标+低电红/充电绿），home/pump_home process 里 `battery_ui_update`（5 参含 BLE 态）；V3 空桩。
3. **BLE 图标位置**：V3 左上 (0,16)；V3Pro 移到 (150,18) 与电池并排。
4. **product_tests 电量**：V3Pro 产线 home/aging 页额外显示电量百分比。
5. **shine 组件**：V3Pro `ui_shine_prepare_from_text` 多 `line_space` 参数。
6. **lang 文案/布局**：en/zh strings.h 与 layout.h 不同。

### Base/Custom 双主题分派
- facade（home/pump_finish/home_screen_saver/charge_adapter/guide）里 `get_sys_theme_type()`，CUSTOM → `xxx_custom_ui_*`，否则 base。
- **当前 `IS_SUPPORT_THEME_SWITCH=0`**，所有 custom 实现是空桩，base 是唯一活跃路径。theme_guide 提供主题选择 UI 但自定义主题页面未落地，属预留通道。

## 6. 注意事项

- 解码缓冲释放路径：`lv_draw_buf_t *` 先 `lv_image_set_src(obj, NULL)` 再 `lv_draw_buf_destroy`；BAJ 走 `ui_img_baj_release_decoded_buf`，不能混用。
- root 清 `CLICKABLE|SCROLLABLE`，防挡子控件。
- 倒计时显示下限 `show_num == 0 ? 1`（永不显示 0）。
- 圆弧角度：popup ring 100ms 步进 / spin `elapsed/3%360` / OTA 匀速追赶 / speed 五段独立。
- 测试宏（`IS_SUPPORT_HOME_UI_TEST`/`LOGO_UI_TEST`/`OTA_TEST` 等）发布需确认关闭。
- `IS_SUPPORT_xxx_COMP_BG` 全部为 0（调试残留像素对比特性）。
- guide 每切页重建整页容器（成本高但状态干净）。
- 低电量真实 UI 在 top_popup（`ID_POPUP_UI_LOW_POWER`），`low_power_ui.c` 是空桩。
