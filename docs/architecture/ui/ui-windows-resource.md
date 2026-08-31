# UI / 窗口 / 资源管理

三层分工：Window Manager（逻辑）→ UI Manager Hook（LVGL 宿主）→ 版本化 UI 层（控件树门面）。资源由 Resource Manager 提供解码数据。

## 1. 三层职责

```text
Window Manager (trunk/root_source/windows + overlay/windows)
  业务/生命周期/按键状态机：窗口对象、历史栈、优先级、按键分发
UI Manager Hook (overlay/framework_hooks/ui)
  LVGL 线程 + 动画定时器 + Base/Overlay 双层绘制 + PM/快照
版本化 UI 层 (overlay/ui/v3_ui_vX)
  LVGL 控件树门面：ui_display_<page>_prepare/process/remove
Resource Manager (trunk/root_source/resource + overlay/resources)
  资源包校验/地址映射/主题加载
```

## 2. 关键文件

| 层 | 文件 |
|----|------|
| WM 核心 | `trunk/root_source/windows/windows_manager.c`、`windows_state.c` |
| UI 核心 | `trunk/root_source/ui/ui_manager.c`、`overlay/framework_hooks/ui/ui_manager_hook.c` |
| 资源核心 | `trunk/root_source/resource/resource_manager.c`、`img_res.c`、`resource_img_define.h` |
| 窗口注册 | `overlay/windows/windows_v6/windows_map_define.c/.h` |
| UI 页面 | `overlay/ui/v3_ui_v7/*/xxx_ui.c`、`ui_page_define.h` |
| 资源 ID | `overlay/resources/v3_res_v7/resource_id_define.h`、`resource_info.h` |

## 3. 窗口路由

`windows_map_define.c` 的 `g_process_handle_list[]`（要求 window_id 升序）：

```c
{ window_id, z_order, PAGE_HANDLE_NAME(xxx), REFS_T(m_window_xxx) }
// PAGE_HANDLE_NAME(name) = enter_new_window_<name>_screen
```

流程：`dispatch_new_window_args` → 校验优先级（target_z_order >= 当前才允许，否则后台 Blocked）→ `MSG_TYPE_WIN_DISPATCH` 到 WM 线程 → `window_switch` 二分查窗口对象 → `enter_new_window_xxx_screen`（分配 proc、注册回调）→ 生命周期 `CREATE/RESUME/PAUSE/DESTORY`。

历史栈 `push/pop_window_history`；弹窗 overlay 不销毁 base。

## 4. UI 线程与 LVGL

- LVGL 跑在自建 `ui_hdr_thd` 线程（`RT_PRIO_LVGL_HANDLER_THREAD`，16KB 栈），主循环 `lvgl_display_handler` 调 `lv_timer_handler()`。
- 动画：`ui_animation_play_init` → UI 线程 `display_prepare_cb` + `lv_timer_create(change_images_cb, period)`；每帧先 base 动画再 overlay 动画。
- `progress_cb(DRAW_AFTER)` 即页面刷新入口，桥接文件（如 `home_ui_handler.c`）在此调 `ui_display_xxx_process`。
- PM suspend/resume → `lv_timer_pause/resume`。
- 触摸走 LVGL indev；实体按键走 key_input 模块（与 LVGL indev 无关）。

## 5. 资源管理

- 包格式：魔数 `IMG_RES_MAGIC = 0x52534547`（"GESR"），头含 magic/CRC16/MD5/version/res_count，校验链 magic → CRC16 → MD5 → 各 obj CRC16。
- Base/Custom 双主题：BASE（8MB 起）+ CUSTOM（+8MB）；运行期 `STORE_ID_SYS_UI_STYLE` 决定，当前 `IS_SUPPORT_THEME_SWITCH(0)` 固定 BASE。
- 内部 flash 直接映射；外部 flash（`V3_IMG_RES_STORAGE_TYPE==EXTERNAL`）用 PSRAM 分配 + `platform_flash_read`。
- 资源 ID：uint16_t，BASE 范围 [0..10000]，按功能分组。

## 6. 版本化路径

编译期决定（include path 只加选中版本）：`ROOT_PRODUCT_ID`（V3Pro/V3）→ `ROOT_DEVICE_NAME`；`ROOT_UI_VERSION`（V2~V7）→ `ROOT_RES_PATH`；`V3_UX_VERSION` → windows 版本目录。运行时同产品三通道（base/custom × 内部/外部 flash × 版本路径）解耦。

## 7. 注意事项

- 弹窗关闭后 base 的 resume 被 `#if 0` 掉，不显式 resume。
- 低优先级 dispatch 被静默 Blocked。
- `lv_timer_handler` 返回值 >10000 被钳到 200（防死循环假象）。
- 动画 timer period 下限 30ms。
- 资源头固定大小 C 侧 32KB 与打包脚本（py 默认 64KB）须一致。
- LVGL 未初始化时 `dispatch_new_window` 轮询等 100×10ms，超时返回 -99。
