# OTA 升级协议

OTA 协议由固件侧 `trunk/root_source/ota/`（编排）+ SDK 侧 `trunk/platform_chip/bes/sdk/services/ota/bes_ota/src/ota_control.cpp`（落 flash）共同实现，承载在新版 comm 协议（0x5CC5）的 OTA 服务（`COMM_PROTOCOL_OTA_SERVICE = 9`）上，走 Remote BLE channel。

## 升级对象

```c
typedef enum {
    MAIN_FIRMWARE,       // 主固件
    IMAGE_RESOURCE,      // 图片资源
    TOUCH_KEY_FIRMWARE,  // 触摸按键固件
} ota_object_e;
```

## 状态机

```c
typedef enum {
    OTA_NONE, OTA_START, OTA_DOWNLOAD, OTA_SUCCESS, OTA_FAIL, OTA_END,
} ota_upgrade_state_e;
```

`OTA_BLOCK_SIZE = 1024`。数据流：BLE 数据包 → `ota_write_image_data` → 组 `ota_image_info_t` 入队 → `ota_work_thd` 逐块调 hook write cb → SDK 写 flash。

## SDK 侧命令 / 应答

| 命令 | 说明 |
|------|------|
| `OTA_COMMAND_START` | 开始升级（携带 `crc32OfImage` 整镜像 CRC32，初值 0xffffffff） |
| `OTA_COMMAND_SEGMENT_VERIFY` | 分段校验（`crc32OfSegment`，1% 段 CRC） |
| `OTA_COMMAND_RESUME_VERIFY` | 断点续传校验（携带 breakPoint + randomCode） |
| `OTA_COMMAND_IMAGE_APPLY` | 应用镜像 |
| `OTA_COMMAND_GET_VERSION` | 查询版本 |
| `OTA_COMMAND_GET_OTA_RESULT` | 查询升级结果 |
| `OTA_COMMAND_CONFIG_OTA` | 配置 OTA |
| `OTA_COMMAND_SIDE_SELECTION` | 侧选择 |

对应 `OTA_RSP_*` 应答：START / SEGMENT_VERIFY / RESUME_VERIFY / IMAGE_APPLY / VERSION / RESULT / CONFIG / SIDE_SELECTION / SET_USER / ROLE_SWITCH 等。

## 断点续传

- `OTA_COMMAND_RESUME_VERIFY` 带 `breakPoint + randomCode`，校验通过回 `OTA_RSP_RESUME_VERIFY`。
- `ota_control_env.resume_at_breakpoint` 与 `breakPoint` 决定是否跳过已收字节；`resume_at_breakpoint == false` 时 `breakPoint` 清零。

## 校验

- 整镜像 CRC32（`CRC32_OF_IMAGE`，初值 0xffffffff）；分段 CRC32（1% 段）。
- 镜像尾部 512B 内找 `CRC32_OF_IMAGE=0x` 关键字做 sanity。
- 镜像合法 magic：`NORMAL_BOOT` / `COPY_NEW_IMAGE` 决定 boot 切换；`app_update_magic_number_of_app_image` 改 magic。

## 压缩 OTA 包

`build_compressed_ota.sh` 将 bin 按 256KB 分块，逐块 lzma -9 压缩，拼 `ff ff ff ff` magic + CRC 占位 + 每块大小头。`OTA_BIN_COMPRESSED=1` + `NEW_IMAGE_FLASH_OFFSET=0x400000`（8MB 盘）确保 OTA 写入区不覆盖正在 XIP 运行的代码。

## 注意事项

- BLE 断连即中止 OTA（`ota_hook.c`）。
- `OTA_EVENT_RESUME_OTA` 在 `RESUME_VERIFY` 后上报，避免 stale progress。
- 图片资源 OTA 进度存 `img_res_ota_info_t`（`PERSIST_KEY_IMG_RES_OTA_INFO`）。
- flash 写缓冲 `OTA_CONTROL_NORFLASH_BUFFER_LEN = FLASH_SECTOR_SIZE*2`；`PUYA_FLASH_ERASE_LIMIT` 时限制擦写次数（每擦一次最多 4+1 次写）。
