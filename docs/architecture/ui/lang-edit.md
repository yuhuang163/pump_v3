# 怎么加文案 / 控件坐标

只改 `overlay/ui/<v3|v3pro>_ui_v7/lang/<zh|en>/` 里的源宏。  
**不要手改** `ui_str_id.h`、`ui_widget_id.h`、`ui_layout_id.h`、`ui_lang_bind.h`、`ui_lang_default.c`。

对照（按页）：`overlay/ui/<ui>_ui_v7/lang/ui_lang_id_map.html`  
圆屏预览：`overlay/font/packaging_tools/preview.sh --open`

三种东西不要混：

| 你要改的 | 写在哪 | 运行时 |
|----------|--------|--------|
| 显示文字 | `strings.h` 的 `FOO_TEXT` / `UI_STR_*` | `ui_str(UI_STR_ID_*)` |
| 某个控件的 x/y/行距/最大宽 | `layout.h` 的 `FOO_Y`、`FOO_X`、`FOO_LINE_SPACE`、`FOO_MAX_W` | `ui_widget(UI_W_FOO)` |
| 两个控件之间的间隙 | `layout.h` 的 `FOO_GAP` / `FOO_SHIFT_Y` | `ui_layout_i16(UI_LAY_*)` |

`FOO` 是控件 stem。例如 `HOME_MAIN_HINT_Y` 和 `HOME_MAIN_HINT_TEXT` 共用 stem `HOME_MAIN_HINT`。

## 命名

- 前缀决定页面：`HOME_`、`PUMP_`、`GUIDE_` …（见 `gen_ui_lang_tables.py` 的 `UI_PAGE_PREFIX_RULES`）
- 文案宏：`STEM_TEXT` 或 `UI_STR_...`
- 坐标宏：`STEM_Y`（几乎都要）、可选 `STEM_X`、`STEM_LINE_SPACE`、`STEM_MAX_W`
- 名字带 `ICON` → 控件类型 ICON，否则 TEXT
- **en 和 zh 宏名必须完全一样**，只改引号里的字或数字
- **新宏追加在文件末尾**。插到中间会让后面 ID 全部错位，旧 LFS/OTA 包会乱

## 四种改法

**1. 只改已有文案**  
改 `en/strings.h` 和 `zh/strings.h` 同一宏。不用跑 gen。要进外部包再 `./pack.sh`。

**2. 只改已有控件坐标**  
改两种语言 `layout.h` 里已有的 `STEM_Y` 等。不用跑 gen。要进外部包再 pack。

**3. 新增一句文案**

```c
/* 两种语言 strings.h 末尾，宏名相同 */
#define PUMP_NEW_TIP_TEXT   "日文或中文..."
```

页面里写 `PUMP_NEW_TIP_TEXT`。然后跑 gen，再 pack。

**4. 新增一个控件（可带文案）**

```c
/* layout.h 末尾，两种语言都加 */
#define PUMP_NEW_TIP_Y            (208)
#define PUMP_NEW_TIP_LINE_SPACE   (8)

/* 若还要文案：strings.h 末尾 */
#define PUMP_NEW_TIP_TEXT         "..."
```

页面用 `PUMP_NEW_TIP_Y` / `PUMP_NEW_TIP_TEXT`，或 `ui_label_apply(label, UI_W_PUMP_NEW_TIP, UI_STR_ID_PUMP_NEW_TIP_TEXT)`。然后：

```bash
python3 overlay/ui/v3_ui_v7/lang/tools/gen_ui_lang_tables.py
cd overlay/font/packaging_tools
./pack.sh --ui v3pro -l zh
./preview.sh --open
```

固件要用新 ID 时才需要重新编译；只改已有宏的字符串/数字，只重烧语言包即可。
