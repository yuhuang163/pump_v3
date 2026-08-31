# 传感器 / 算法

V3Pro 才具备气压传感、算法采集、夜灯能力；IMU/力传感板级 `IS_SUPPORT_DEVICE_IMU/FSENSOR=0` 关闭（driver 空壳）。实际激活链路两条：气压 → 佩戴检测/漏气补偿；光感 → 自适应调光/夜灯。

## 1. 统一调度（sensor_manager.c）

- 单线程 `sensor_thd` + 10ms 定时器，遍历任务表按 `polling_period` 节流、按 `report_type` 分发客户端。
- 任务注册做周期补偿：`polling_period -= SENSOR_THREAD_POLLING_PERIOD/3`（约扣 3.3ms）。
- 客户端分发做步进抽稀：`polling_cnt - old_cnt >= step` 才回调。
- 共享缓冲 `sensor_wrapper_data_t` 单缓冲复用，客户端须同步消费（锁内回调不可阻塞）。

## 2. 传感器周期

| 传感器 | 周期 | 触发 | 数据落点 |
|--------|------|------|----------|
| IMU | 20ms / 200ms(FIFO) | 轮询/中断 | `imu_data` |
| 力传感 | 160ms | ioctl | `force_raw_data` |
| 气压 | 20ms（动态） | ioctl | `air_pressure_data[2]` |
| 光感 | 100ms（动态） | DEVICE_READ | `light_sensor_data` |

气压芯片（按硬件版本选型）：V1 用 WF100D（I2C 0x6D，量程 100KPA，异常 10 次判不可用）；V2-V4 用 LIMS（I2C 0x6C，量程相反符号，读后预启动下次转换）。光感走 GPADC oneshot。

## 3. 算法框架（acq → core → post_proc）

- `root_algo_acq_proc.c`：采集采样（10 点/包 → 服务器上报），开关 + 动态改采样率。
- `root_algo_core.c`：`root_algo_init` 注册依赖回调 → `algo_fusion_lib_init`；结果回调 `algo_out_result_cb` 缓存 + 变化埋点 + `algo_manager_dispatch_to_clients`。
- `root_algo_post_proc.c`：`algo_fusion_data_cb` 组装气压 + 电机状态 → `algo_fusion_lib_input`。

`library/root_algo` 独立线程 `algo_fusion_work_thread` 收队后调 `V3WearDetectionMain` + `GetWearDetectionResult` + `GetCompensationStatus`，回 core。

## 4. 佩戴检测（wear_detection v3）

- 50Hz、3s 窗口（WIN_SIZE=150）、双通道，一阶 RC 低通（EXPRESSION 4Hz / 其它 3Hz）。
- 流程：模式/档位变化复位 → 取目标曲线 → 预处理 → 原始泵段最小 → 峰谷/波形分析 → 判定（先候选再抗抖：0→1 需 6 次，1→0 需 3 次）。
- 补偿 PI：kP=18/kI=3 + anti-windup，输出 ±3000，只在 GENERATE/HOLD 输出 `iPwmOffset`。
- 环形缓冲 `ring_buffer/aglo_ring_buffer.c` 为通用库，当前未被算法路径消费（佩戴检测用自带静态窗口）。

## 5. 自适应调光与夜灯

- 光感采样 → `adaptive_dimming`（misc 200ms 轮询：旋钮屏蔽 → 批均值 → EMA 滤波 → 滞回 + 去抖）→ 30ms slew 线程落背光/夜灯。
- 夜灯 BSP：`pwm_table[101]` 非线性表，`hwtimer` 35ms/步爬升，末段半阶插值；目标 duty 钳 [2,98]。
- 背光 ALS 按键/屏分开，按键有软缓升，屏有首包保护（首包未确认不落地防开机压暗）。

## 6. 数据链路

```text
气压 I2C → 20ms 轮询 → algo core → fusion 线程 → 佩戴/补偿结果
 → 分发：电机补偿 / 服务器漏气上报 / UI 弹窗 / 埋点

光感 GPADC → 100ms 轮询 → adaptive_dimming → misc 200ms/30ms → 背光/夜灯 PWM
```

## 7. 注意事项

- 传感器共享缓冲锁内回调不可阻塞。
- 气压 I2C 连续异常（WF100D 10 次）永久置不可用，需重新 open/init。
- 采样间隔动态调整走「移除重建任务」，改前须停采集。
- `V3WearDetectionMain` 的 `do_work==0` 只复位右通道（疑似 bug）。
- 目标曲线表 `p1_curve` 全占位（0.0f，TODO）。
- `v3_pressure_compensation_algo.c` 在 fusion 中被注释禁用，当前补偿走 wear_detection 内部 PI。
- `register_sensor_hook` 空实现且未被调用，sensor 模块自包含。
