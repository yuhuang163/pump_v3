# 启动流程与模块注册机制

## 1. 启动序列

入口 `trunk/root_source/base/root_main.c` 的 `root_main()`，由 SDK `bleonly_main.c:main()` 在 `CFG_CUSTOM_APP` 分支接管后调用：

```text
SDK bleonly_main.c:main()
 ├─ app_wdt_open(5) / hwtimer_init / hal_sys_timer_open / uart0 trace
 ├─ (ENABLE_FLASH1) spi_flash_open（外部 flash）
 ├─ mpu_cfg / pmu_open / analog_open / norflash_api_init / factory_section_open / nv_record_init
 ├─ ble_app_init() + bes_ble_gap_register_data_fill_handle(...)
 └─ #if CFG_CUSTOM_APP → root_main(0, NULL)

root_main()
 ├─ platform_prepare_init()   // u_heap_init + os_mutex_module_init + bootmode/boot_cause
 ├─ reset_log_filter(LOG_FILTER)
 ├─ RUN_NEW_THREAD("init_thd", RT_PRIO_MAIN_THREAD, 16KB)
 └─ os_kernel_start_wrapper()

init_thread()
 ├─ platform_post_init()          // 当前为空实现（扩展点）
 ├─ init_hang_detect_context()    // 空壳
 ├─ init_broadcast_context()      // 2 个 bcr_work_thd
 ├─ init_handler_post_context()   // 2 个 hdr_post_thd + 定时器
 ├─ register_module()             // ① 建所有模块 context 挂链表
 ├─ framework_extra_hook()        // ② overlay 逐个 register_*_hook()
 ├─ init_module()                 // ③ 两遍遍历：先 perpare_init 后 post_init
 ├─ init_user_log_filter()
 └─ run_user_thread_dispatch()    // 启动 WM 窗口调度
```

## 2. 模块注册（两段式）

`trunk/root_source/base/module_register.c`：

- 静态数组 `s_module_register[]` 按 `IS_SUPPORT_MODULE_*` 编译期裁剪，集中列出全部 `register_module_for_xxx_new()`。
- `register_module()` 遍历数组，每个注册函数 `MALLOC_FOR_PTR` 建 context、填公开函数指针，最后 `ADD_MODULE(...)`（即 `add_new_module`）构造 `module_node_t` 挂到全局 `s_module_obj_list`。
- `init_module()` 对链表**两次完整遍历**：第一遍 `perpare_init`，第二遍 `post_init`。保证 prepare 阶段可 `find_module()` 到所有 context（拿依赖），post 阶段用依赖。
- `find_module(id)` 线性扫描 + `static __cache[ID_MODULE_MAX]` 缓存。

```c
typedef struct module_node {
    const char *name;
    module_desc_id_e id;
    int (*perpare_init)(void *ptr);
    int (*post_init)(void *ptr);
    void *module_ptr;
    uint32_t mem_usage;
    list_head_t list;
} module_node_t;
```

## 3. 设备注册（外设总线 pbus）

`trunk/root_source/base/device_register.c` 的 `register_module_for_peripheral_bus_new`：

- 是所有外设驱动的宿主，把 `register_device_for_*()`（drivers 目录实现）依次执行，`rt_device_node_t` 挂到 `priv.device_obj_list`。
- `on_post_pbus_init`：FS remount → 逐个 `node->open(dev)` → 注册 UART 调试接收 → 注册 `ID_BROADCAST_SYS_STATE_CHANGE` 广播，收到 `SYSTEM_BROADCAST_BOOT_COMPLETED` 置 `is_init_ready`。
- 提供 `find_device(id)`（带缓存）、`suspend/resume`、`pmu_sleep_poll`、`add/del_period_wakeup_cb`。

```c
typedef struct rt_device_node {
    const char *name;
    device_desc_id_e id;
    device_status_e status;
    int (*open)(struct rt_device_node *dev);
    int (*close)(struct rt_device_node *dev);
    int (*read)(struct rt_device_node *dev, void *buf, uint32_t n);
    int (*write)(struct rt_device_node *dev, const void *buf, uint32_t n);
    int (*ioctl)(struct rt_device_node *dev, uint32_t op_code, void *args);
    int (*suspend)(struct rt_device_node *dev);
    int (*resume)(struct rt_device_node *dev);
} rt_device_node_t;
```

## 4. Overlay 钩子注入

`overlay/framework_hooks/framework_hook.c` 的 `framework_extra_hook()` 在 `register_module()` 后、`init_module()` 前执行，用 `find_module()` 拿各模块 context，逐个 `register_*_hook()`，把 storage/输入键/马达/泵奶/电源/飞行模式/资源/UI/算法/产测/埋点/服务器/异常/LED/OTA/BLE 共 16 类产品钩子挂进框架主干。

## 5. 模块注册顺序

`module_register.c` 的 `s_module_register[]` 顺序即初始化顺序（pbus 最靠前）：PBUS → PM → STORAGE → BLE → RES → TIME_SYSTEM → UI → WM → KEY_INPUT → SENSOR → OTA → MISC → BATTERY → BREAST_PUMP → TESTS → DATA_BURIAL → US_ALGO → MOTOR → AUDIO → AIRPLANE_MODE → PROTOCOL → SERVERS → SYS_ABNORMAL → LED。

## 6. 依赖约束

- `overlay -> trunk -> drivers -> platform_chip`，禁止倒置。
- `trunk/include` 被上层直接引用；实现仅在 `root_source/`。
- `drivers` 不依赖上层业务；`platform_chip` 仅被 `drivers` 与 `root_source` 适配层使用。
- 模块间不直接互调，统一 `find_module` 拿 context + 广播/投递解耦。
