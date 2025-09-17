编译选项：make T=platform_tester COMPILE_TARGET=BASIC_TEST CHIP=best1502x UTILS_ESHELL_EN=1 ESHELL_ON_TRACE=1 UTILS_ESHELL_BT_TEST=1 DEBUG=1 DEBUG_PORT=2 APP_TRACE_RX_ENABLE=1 SLEEP_STATS_TRACE=1

1.	测试接口说明
1.btct_set_bt_sleep enable
	Btc侧睡眠接口，enable true进入睡眠，false 退出睡眠。
	HCI TX，RX如下：
	btct_set_bt_sleep 1
	TX:01 77 fc 08 01 01 d0 07 32 00 00 00 
	RX:04 0e 04 05 77 fc 00

	btct_set_bt_sleep 0
	01 77 fc 08 00 01 d0 07 32 00 00 00 
	04 0e 04 05 77 fc 00
2.btct_ble_adv_with_data_en interval
	Btc侧广播使能接口，使用了一组默认参数（具体数据参考btdrv_common_set_default_adv_data）， interval 广播间隔。
	HCI TX，RX如下：
	tx:01 72 fc 07 01 11 22 33 44 55 66 
	 rx:04 0e 04 05 72 fc 00 
	 tx:01 06 20 0f 00 00 00 00 00 00 00 00 00 00 00 00 00 07 00 
	 rx:04 0e 04 05 06 20 0c 
	 tx:01 08 20 20 18 02 01 1a 08 09 42 45 53 5f 42 4c 45 02 01 1a 08 09 42 45 53 5f 42 4c 45 00 00 00 00 00 00 00 
	 rx:04 0e 04 05 08 20 00 
	 tx:01 0a 20 01 01 
	 rx:04 0e 04 05 0a 20 0c
	
3.btct_ble_create_connect interval
	Btc侧创建连接接口，interval 连接间隔（成功连接后的周期性通讯时间）
	HCI TX，RX如下：
	TX: 01 72 fc 07 01 11 22 33 44 55 66 
	RX: 04 0e 04 05 72 fc 00 
	TX: 01 0d 20 19 50 00 30 00 00 00 12 34 56 78 90 ab 00 a0 00 a0 00 00 00 64 00 10 00 10 00 
	RX: 04 0f 04 00 05 0d 20 
	连接成功后：
	RX: 04 3e 13 01 00 00 00 00 00 12 34 56 78 90 ab a0 00 00 00 64 00 01

4.btct_ble_adv_en interval
	TX: 01 72 fc 07 01 12 34 56 78 90 ab 
	RX: 04 0e 04 05 72 fc 00 
	TX: 01 06 20 0f 20 00 20 00 00 00 00 00 00 00 00 00 00 07 00 
	RX: 04 0e 04 05 06 20 00 
	TX: 01 0a 20 01 01 
	RX: 04 0e 04 05 0a 20 00
	连接成功后：
	04 3e 13 01 00 01 00 01 00 11 22 33 44 55 66 a0 00 00 00 64 00 01
5. btct_enter_sniff conhandle sniff_interval attempt timeout
	After bt connection, enter sniff。根据上一条BT_CREATE_CONNECTION命令中evt返回的conhandle信息(0080)，填入此条enter sniff命令中的conhandle位置：
	TX: 01 03 08 0a 80 00 20 03 20 03 01 00 01 00 
	RX: 04 0f 04 00 05 03 08 
	RX: 04 14 06 00 80 00 02 20 03（slave那边也会上这条mode change的evt）

6.btct_create_connect
	Btc侧 连接接口
	HCI TX RX如下：
	TX: 01 72 fc 07 00 11 22 33 44 55 66 
	RX: 04 0e 04 05 72 fc 00 
	TX: 01 0f 08 02 05 00 
	RX: 04 0e 04 05 0f 08 00 
	TX: 01 05 04 0d 12 34 56 78 90 ab 18 cc 02 00 00 00 01 
	RX: 04 0f 04 00 05 05 04 
	RX: 04 03 0b 00 80 00 12 34 56 78 90 ab 01 00 
	RX: 04 1b 03 80 00 05
7.btct_pscan_bothscan_en interval window
	Btc侧 page scan接口，interval scan间隔，window sacn开窗
	HCI TX RX如下：
	TX: 01 72 fc 07 00 12 34 56 78 90 ab 
	RX: 04 0e 04 05 72 fc 00 
	TX: 01 0f 08 02 05 00 
	RX: 04 0e 04 05 0f 08 00 
	TX: 01 1c 0c 04 50 00 12 00 
	RX: 04 0e 04 05 1c 0c 00 
	TX: 01 1a 0c 01 03 
	RX: 04 0e 04 05 1a 0c 00 
	TX: 01 05 0c 03 02 00 02 
	RX: 04 0e 04 05 05 0c 00
	连接成功后：
	RX:04 07 ff 00 11 22 33 44 55 66 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	RX:04 03 0b 00 80 00 11 22 33 44 55 66 01 00 
	RX:04 1b 03 80 00 05

8.btct_pscan_interlaced_en interval window
9. btct_sys_sleep
	Sys侧进入睡眠接口，通过拉高PIN 15（可通过bt_common_test.c PIN_WAKEUP宏定义来更改）来唤醒，已配置PIN 13高电平输出（可通过bt_common_test.c PIN_TRIGGER宏定义来更改），可用杜邦线接PIN 13与PIN 15来唤醒。睡眠后uart rx失效，唤醒后才可继续使用eshell测试指令。
	睡眠LOG：
	[14:58:18.630] btct_sys_sleep
	[14:58:18.634] btct_gpio_irq_wakeup_set en:1
	唤醒LOG：
	[14:58:53.868] wakeup pin:15

10.btct_init
	Bt common test初始化接口，1.start bt 2.hci open
11.btct_deinit
	Bt common test退出接口，1.hci off 2.stop bt
	

2.	测试场景
1.	SYS侧睡眠+BTC sleep
	a)	btct_set_bt_sleep 1 -- 使能btc侧睡眠
	b)	btct_sys_sleep  --sys侧进入睡眠
2.	SYS侧睡眠+BTC bothscan
	a)	btct_pscan_bothscan_en 1280 18
	b)	btct_sys_sleep  --sys侧进入睡眠
3.	SYS侧睡眠+BTC bt connect
	a)	Slave：btct_pscan_bothscan_en 1280 18
	b)	Master：btct_create_connect
	c)	btct_sys_sleep  --sys侧进入睡眠
4.	SYS侧睡眠+BTC sniff mode
	a)	Slave：btct_pscan_bothscan_en 1280 18
	b)	Master：btct_create_connect
	c)	btct_enter_sniff 128 800 1 1
	d)	btct_sys_sleep  --sys侧进入睡眠
5.	SYS侧睡眠+BTC ble adv
	a)	btct_ble_adv_en 100
	b)	btct_sys_sleep  --sys侧进入睡眠
6.	SYS侧睡眠+BTC ble connected
	a)	Slave：btct_ble_adv_en 100
	b)	Master：btct_ble_create_connect 160
	c)	btct_sys_sleep  --sys侧进入睡眠

测试结束后可调用 btct_deinit 来stop bt

