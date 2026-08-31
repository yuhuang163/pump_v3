# 平台适配 / 构建系统

## 1. 构建链路

```text
GNUmakefile（构建入口，默认 V3Pro）
 ├─ 解析 T / CHIP / 产品ID / UI版本 / 语言 / 硬件版本
 ├─ 收集 EXTRA_SRCS（os_base + drivers + root_source + overlay + library）
 ├─ 编译为 libextra.a（arm-none-eabi-gcc）
 └─ 递归调 SDK: make -C trunk/platform_chip/bes/sdk O=… T=ble_only_lt all
      └─ config/ble_only_lt/target.mk + common.mk
           ├─ 链接 + tail_section.lds.S 分区段
           ├─ objcopy -O binary → fill_sec_base.pl
           └─ 拷贝 elf/bin/map/lst 回 BIN_DIR
```

- `CHIP ?= best1306p`（Cortex-M33，`-mcpu=cortex-m33`）。
- SDK 目标仅两个：`ble_only_lt`（固件）、`ota_boot_v2`（bootloader）。
- 产物：`project/bin/firmware/*.bin`、`project/bin/bootloader/ota_boot_v2.bin`。

## 2. 配置解析

- `ROOT_PRODUCT_ID`：V3=`0x08`、V3Pro=`0x09`；根 GNUmakefile 默认 V3Pro，`configs/V3_2710IMC_GNUmakefile` 默认 V3。
- `ROOT_UI_VERSION`（V2~V7）：V3 系列 `0x02~0x07`，V3Pro 系列 `0x12~0x17`。
- `PRODUCT_LANGUAGE ?= en|zh`：编译期 SKU，一次一种语言（`font_v3/lang/<lang>`）。
- `V3_HARDWARE_VERSION`（V1~V4）：驱动芯片选型（气压 WF100D/LIMS、触摸 hyn/sc09b、LED tm1650/htr3316、LCD qspi/uart）。
- overlay 版本化目录（UI/windows/font/resources）按版本只编选中目录。

## 3. 产品配置差异（IMC vs IMP）

| 维度 | V3_2710IMC（新） | V3_2710IMP（旧） |
|------|------------------|------------------|
| 硬件版本 | V1~V4，默认 V4(DV4) | V1(DV1)，DV_DEVICE_ENABLE 开关 |
| Flash | 8MB（FLASH_SIZE=0x800000） | 4MB |
| 图片 | 外部 flash + JPEG/BAJ | 内部 flash 预留 |
| 语言/UI 版本化 | 有 | 无（旧 BES_SDK 模式） |
| WERROR | 1 | 0 |

## 4. 平台适配层（trunk/platform_chip/bes/os_base）

CMSIS-RTOS2 抽象：

| 文件 | 抽象 |
|------|------|
| `cmsis_os_thread.c` | `run_new_thread_wrapper`（u_heap 分配栈，osThreadNew） |
| `cmsis_os_memery.c` | SRAM u_heap 160KB + PSRAM 512KB（multi_heap） |
| `cmsis_os_mutex.c` | 可递归/优先级继承互斥锁 + 挂起检测链表 |
| `cmsis_os_queue.c` / `semaphore.c` | osMessageQueue / osSemaphore 薄封装 |
| `cmsis_rtos_timer.c` | 定时器双实现（CMSIS osTimer / SDK hwtimer） |
| `cmsis_os_base.c` | `get_timestamp` = TICKS_TO_MS(hal_sys_timer_get) |
| `cmsis_misc.c` | 平台钩子核心实现 |

平台钩子落点（`cmsis_os_api.h` 声明 → `cmsis_misc.c` 实现）：

- `platform_prepare_init`（u_heap_init）、`platform_post_init`（空）。
- `platform_get/set_utc_time`（PMU RTC）、`platform_system_reboot`、`platform_get_sys_reboot_reason`。
- `platform_get_tid`（线程号 %1000）。

看门狗由 SDK `app_wdt_open(5)`（5s）管理，应用层无看门狗代码。

## 5. 分区布局（project_partitions.h + tail_section.lds.S）

链接脚本 `tail_section.lds.S` 定义 `.lute_picture_resources / .littlefs / .factory_data / .reserved / .factory`（NOLOAD，放 Flash 尾部），大小由 GNUmakefile 传入宏决定。`os_base/cmsis_misc.c` 的 `platform_flash_read/write/erase` 按分区名分发：内部分区走 norflash_api，外部分区走 spi_norflash。

## 6. 分析脚本

- `map_section_sizes.py`：按文件/库成员汇总 .text/.rodata/.data/.bss。
- `ram_usage_analyzer.py`：RAM 段统计（.data/.bss/.ram*，排除 psram）。
- `psram_usage_analyzer.py`：`.psram*` 段统计。
- `compile_commands.json`：`build_helper.sh --non-interactive bear` 生成（bear -- make）。

## 7. 产物归档命名（build_helper.sh）

```
Lute_<V3|V3Pro>_<YYYYMMDD>_<D|P|T|C><major.minor>_H<major.minor>_<githash>_<md5折叠4B>_<APP|BOOT|OTA|RES>.bin
```

## 8. 注意事项

- 根 GNUmakefile 与 configs/ 是不同步的（根更新，configs 旧快照）。
- `project_partitions.h` 有笔误：`IMG_INT_DATA_SECTOR_NUM` 用了未定义 `IMG_INT_DATA_BLOCK_SIZE`（应为 `IMG_INT_DATA_SIZE`）。
- 线程栈全部从堆分配，SRAM 堆仅 160KB，栈深度配比见 `project_config.h`。
- `u_malloc_psram` 在 `PSRAM_ENABLE=0` 时返回 NULL（不加判断）。
- OTA 压缩 lzma 256KB 分块；`NEW_IMAGE_FLASH_OFFSET=0x400000` 防覆盖 XIP 代码。
- simulator 目录为空，PC 仿真未实装。
