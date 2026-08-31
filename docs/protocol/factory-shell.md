# 产测 Shell 命令（#cmd）

产测上位机通过 BLE Factory 服务发送以 `#` 开头的文本命令，固件在 `overlay/ble/factory_service/ble_factory_shell.c` 解析并直接调用 motor/led/storage 等业务，用于产线调试与烧录。

- 若 Factory 通道数据以 `#` 开头，先走 `ble_factory_shell_handle()`，否则分发到 `DEVICE_BLE_FAC_MSG_CHANNEL` 走 comm 协议。
- 与三套二进制协议独立，是文本命令通道。

## 命令列表

| 命令 | 说明 |
|------|------|
| `#pump` | 泵控制 |
| `#pumpLvl` | 泵档位 |
| `#pumpSpd` | 泵速度 |
| `#pCtrl` | 泵控制参数 |
| `#vCtrl` | 阀控制 |
| `#algoSmp` | 算法采样开关 |
| `#algoSmpIntvl` | 算法采样间隔 |
| `#tripleWrite` | 三元组烧录（productID/deviceID/key） |
| `#utcTime` | UTC 校时 |
| `#customUpdate` | 自定义模式下发 |
| `#customClear` | 清除自定义模式 |
| `#sysRst` | 系统复位 |
| `#forbidSlp` | 禁止休眠 |
| `#chargeCurAdj` | 充电电流调节 |
| `#BackLightLevel` | 背光档位 |
| `#lcdBk` | LCD 背光 |
| `#GlobalBacklight` | 全局背光 |
| `#GlobalAdaptive` | 全局自适应调光开关 |
| `#LightAdaptive` | 光感自适应 |
| `#LsSmpCtrl` | 光感采样控制 |
| `#LsSmpIntvl` | 光感采样间隔 |
| `#NightLightSwitch` | 夜灯开关 |
| `#NightLightGear` | 夜灯档位 |
| `#NightLightLevel` | 夜灯亮度 |
| `#NightLightStartup` | 夜灯开机状态 |
| `#ledNightLight` | LED 夜灯 |
| `#imgTest` | 图片测试 |

## 说明

- 完整命令解析逻辑见 `overlay/ble/factory_service/ble_factory_shell.c`。
- 工厂通道同时承载 comm 协议（新协议产测）与 `#shell` 文本命令，按首字节是否 `#` 分流。
