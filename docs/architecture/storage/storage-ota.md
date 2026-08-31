# 存储抽象 / OTA 升级

## 1. 存储抽象（storage/ + overlay/storage/）

### 三套抽象

- KV（user_database，littlefs FDS）
- 分区（factory_flash）
- RAM 镜像 + CRC 脏同步

### 分区布局（root_storage_base.h）

`all_in_one / phy_init / ota_upgrade / userdata / littleFs / littleFsExt / factory_data / dev_info / img_int_data / img_ext_data`。

内部 flash（NorFlash）：FDS（littlefs）、FACTORY_DATA（0x2000）、IMG_INT_DATA（图片，IMC V3/V4 为 0 图片全走外 flash）。外部 flash（16MB）：BASE_IMG_RES（8MB）、THEME_IMG_RES（7MB）、FDS_EXT。

### 存储格式

`magic + 数据 + CRC16` 三段式。读校验失败静默重置并写回默认值。magic 家族 `0xAFFA5880~0xAFFA5899`（MAC_ADDR / FAC_FLAG / IMU_CALIB / FAC_DEVINFO / SYS_INFO / USER_BIND / PUMP_DATA_HEAD/BODY / UTC / REBOOT_REASON 等）。

dispatch 表按 `persist_store_id_e` 索引 put/get/del/put_ram/get_ram。

### 工厂校准数据（factory_persist_data.c）

- `factory_dev_info_t`：product_id / device_id / enctyption_key / sn + light_sensor_calib_tbl[5] + motor_type + battery_type。
- 吸力参数表：按 `motor_type`（SHIGUANG/LONGYI/default）选择编译期预设表拷贝（非 flash 读）。
- IMU 校准：`imu_cali_storage_t`（magic + imu_cali + crc）。

### 吸奶数据循环缓冲

`ROOT_BREAST_PUMP_DATA_BODY_NUM=70` 条，head/tail 索引，每条 magic+CRC；RAM→Flash 每 5s 同步，关机强制落盘。

## 2. OTA 升级（ota/ota_manager.c）

### 升级对象与状态

```c
ota_object_e: MAIN_FIRMWARE / IMAGE_RESOURCE / TOUCH_KEY_FIRMWARE
ota_upgrade_state_e: NONE / START / DOWNLOAD / SUCCESS / FAIL / END
```

`OTA_BLOCK_SIZE = 1024`。数据流：BLE 包 → `ota_write_image_data` → 组 `ota_image_info_t` 入队 → `ota_work_thd` 调 hook write cb → SDK 写 flash。

### SDK 断点续传与校验

- `RESUME_VERIFY`（breakPoint + randomCode）→ `RSP_RESUME_VERIFY`。
- 整镜像 CRC32（`CRC32_OF_IMAGE`，初值 0xffffffff）+ 分段 CRC32（1% 段）。
- 镜像 magic `NORMAL_BOOT` / `COPY_NEW_IMAGE` 决定 boot 切换。
- 压缩 OTA：lzma 256KB 分块 + `ff ff ff ff` magic。

协议细节见 [docs/protocol/ota-protocol.md](../../protocol/ota-protocol.md)。

## 3. 注意事项

- 读取校验失败会静默重置（丢数据）但打 ERROR 日志。
- `factory_reset`：user_database_reset + 时间同步 + 可选 reboot，flash 写用 `storage_rw_lock` 门闩。
- BLE 断连即中止 OTA。
- 图片资源 OTA 进度存 `img_res_ota_info_t`。
- `drivers/fatfs/bsp_fatfs.c` 基本 stub（真实逻辑在 `#if 0`）。
