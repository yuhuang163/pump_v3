# overlay/font

字库与语言包资源。图片资源对应 `overlay/resources/packaging_tools/`，语言包对应这里的 `packaging_tools/`。

```
overlay/font/
├── packaging_tools/          # 语言包打包（文案 + 字库 bin + 图片文字）
│   ├── pack.sh
│   └── pack_lang.py
├── tools/                    # 旧版单字体 LVGL 分包 / littlefs 镜像
├── font_v3/
│   ├── common/               # 56/69 数字字库，编进固件，不进语言包
│   └── lang/<zh|en>/
│       ├── bin/              # LVGL .bin 字库
│       └── font_img/         # 提示图 png / txt
└── bin/                      # 打包产物
    └── font_v3/<v3|v3pro>/<zh|en>/
```

## 打包语言包

```bash
cd overlay/font/packaging_tools
./pack.sh                 # 按 GNUmakefile 产品族，默认中文
./pack.sh --menu
./pack.sh v3pro zh
./pack.sh --ui v3 -l zh
```

产物（烧录目录只保留一份）：

- `overlay/font/bin/font_v3/<v3|v3pro>/<lang>/<PREFIX>_<日期>_Vxx.xx_<MD5>_LFS.bin`（1.5MB littlefs）
- `overlay/font/bin/font_v3/<v3|v3pro>/<lang>/<PREFIX>_<日期>_Vxx.xx_<MD5>_OTA.bin`（OTA 整包容器）
- 烧 LFS 到外部 Flash `0xE00000`，长度 `0x180000`

需要 `python3 -m pip install --user littlefs-python`（也可用 `./preview.sh --install`）。

## 预览语言包

```bash
cd overlay/font/packaging_tools
./preview.sh --install                 # 安装 littlefs-python
./preview.sh --open                    # 预览 bin/ 下最新 *_LFS.bin
./preview.sh --open ../bin/font_v3/v3pro/zh/V3PRO_*_LFS.bin
```

会列出 FDS key / CRC，并在同目录生成 `*.preview.html`（圆屏上的文案坐标、字库、图片文字）。

宏 ↔ ID 对照表（按页，不依赖镜像）：打开

- `overlay/ui/v3pro_ui_v7/lang/ui_lang_id_map.html`
- 步骤说明：`docs/architecture/ui/lang-edit.md`

由 `python3 overlay/ui/v3_ui_v7/lang/tools/gen_ui_lang_tables.py` 生成。

