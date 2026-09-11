# 语言包 FDS 多文件布局

外部 Flash `FDS_EXT`（littlefs，1.5MB @ `0xE00000`）按对象存语言资源。每个字号、每个页面文案表、每个页面组件表各占一个整数 key，可单独改写；整包仍用同一容器一次性安装。

加载逻辑在 `overlay/ui/lang_pack/`（v3 / v3pro 共用）。产品差异只在各自 `overlay/ui/<ui>_ui_v7/lang/` 的文案、坐标和生成表。无 `RDY1` 时直接用固件 default 表，固件不内置 `lang_pack.bin`。

编译开关：`IS_SUPPORT_LANG_PACK`（`boards/project_support_overlay.h`，或 `make firmware IS_SUPPORT_LANG_PACK=0`）。关闭后不编加载器和语言包 OTA。

## 1. Key 空间

| 对象 | Key | 说明 |
|------|-----|------|
| 清单 | `LANG` `0x4C414E47` | 包头 + `lang_manifest_entry_t[]` |
| 提交 | `RDY1` `0x52445931` | 与清单字段绑定，开机只比对清单↔ready |
| 字库 | `0x464E5400 \| role` | 仅语言相关字号（中文 Noto 19/22/26/29/33） |
| 图片文字 | `0x46494D00 \| img_id` | `font_img` |
| 页面文案 | `0x50530000 \| page` | 该页全部字符串 + 坐标/对齐等 |
| 页面组件 | `0x50570000 \| page` | 该页 ICON/坐标标量，记录格式与文案相同 |

页面枚举见生成头 `ui_page_id.h`（`UI_PAGE_HOME` / `UI_PAGE_PUMP` / …）。

宏 ↔ ID 对照：`overlay/ui/<ui>_ui_v7/lang/ui_lang_id_map.html`（按页）。怎么加文案/坐标见 [`lang-edit.md`](lang-edit.md)。圆屏坐标预览：`overlay/font/packaging_tools/preview.sh --open`。

## 2. 统一记录

文案文件与组件文件都是 `lang_page_file_t` + `ui_lang_item_t[]`：

- `kind`：`TEXT` / `ICON` / `SCALAR`
- 坐标：`x/y/obj_align/text_align/line_space/max_w/opa/color/flags/font_role`
- 文案：`text_bytes` + UTF-8（组件项为 0）

## 3. 公共 Latin 字库

Aeonik 19/22/26/29/33/56/69 在 `overlay/font/font_v3/common/`，编进固件，不进语言包。  
英文包只有页面表 + `font_img`。中文包只带 Noto 五个字号。`UI_FONT_AEONIK*` 一律走公共 Aeonik。

## 4. 整包更新

`pack_lang.py` 产出：

1. **容器** `*_OTA.bin`：清单 + 各对象拼接（OTA `TYPE=0x04`）
2. **LFS 镜像**：已拆好的各 key + 精简 `LANG` 清单 + `RDY1`

OTA 仍把容器写到 `LANG`；`END` 后 `ui_lang_pack_install_container()` 拆到各 key，再把 `LANG` 收成清单并写 `RDY1`。下次开机 `probe` 按清单分别加载。烧录 `*_LFS.bin` 时 `LANG` 已是精简清单，`probe` 用「能否在 `manifest_size` 之后再读 1 字节」判断是否还要 explode，不要用 `payload_size` 比较（精简清单里 `payload_size` 仍含对象总长）。也可只改某一个 `FNT*` / `PS*` / `PW*` 文件（改完需同步清单 CRC 与 `RDY1`）。

Schema major = 2。旧 schema 1 单文件包不再兼容。
