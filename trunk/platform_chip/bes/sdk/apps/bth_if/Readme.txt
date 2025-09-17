1. 设计目的
目前手表/耳机产品需要经常访问BTH核上的内容，然后再返回对应的执行结果给调用的核。
对应手表产品，蓝牙UI一般在系统核上实现，蓝牙stack一般在BTH核上实现；而对于耳机产品，蓝牙UI和蓝牙stack一般都在BTH核实现。
需要设计一套通用的蓝牙接口访问框架，不区分UI和stack是否在同一个CPU核上或者在同一个进程上，应用程序调用对应接口就能得到返回的信息。

2. 设计方法
BTH interface模块采用client-server设计模式，可以实现跨CPU核使用和在同一个CPU核使用，不同模块按user注册不同的接口函数。
/apps/bth_if/
├─bt_if_client/                         # client端实现
├─bt_if_server/                         # server端实现
├─tests/                                # 功能测试代码
├─bth_if_core.h                         # C/S通讯数据结构定义
├─bth_if_cmd_def.h                      # UI/APP调用接口数据结构定义(用户根据需要添加定义)
├─bth_if_evt_def.h                      # UI/APP回调返回数据结构定义(用户根据需要重新定义)
├─bth_if_rpc_comm.c                     # 跨核访问的实现
├─bth_if_inner_comm.c                   # 同核同进程访问的实现(TODO)
├─bth_if_ipc_comm.c                     # 同核跨进程访问的实现(TODO)
└─Readme.txt                            # 使用说明

bt_if_client/: client端，抽象出应用程序（Application）需要的接口和回调信息。

bt_if_server/: server端，抽象出协议栈（Stack）需要提供的接口和返回信息。目前BT/BLE协议栈解耦已经完成，
可以直接访问bes_bt_me/bes_bt_a2dp/bes_bt_hfp/bes_bt_avrcp函数指针调用对应的方法。

bth_if/: client和server端都会使用的公共代码，数据结构参考app_bth_if_ipc_cmd_msg_info_t定义。

3. 使用简单说明

#1. 添加对应模块的接口函数
app_bth_if_user_t数据类型中添加模块的user命名，例如：APP_BTH_IF_USER_BT。

bt_if_server/路径下添加模块的接口函数，例如：APP_BTH_IF_USER_BT接口函数如下：
static app_bth_if_ipc_msg_t bth_btif_ipc_handle_table = {
    .open           = app_bth_bt_if_enable,
    .close          = app_bth_bt_if_disable,
    .restart        = app_bth_bt_if_restart,
    .ctrl           = app_bth_bt_if_ctrl,
};
void app_bth_bt_if_init(void)
{
    bth_if_thread_server_register(APP_BTH_IF_USER_BT, &bth_btif_ipc_handle_table);
}

#2. 添加应用程序（Application）需要访问的接口和回调函数
需要把应用程序需要的回调函数接口指针传给server端。例如：APP_BTH_IF_USER_BT应用接口函数如下：

static bt_callbacks_t bt_state_report_if_cbs = {
    .size                           = sizeof(bt_callbacks_t),
    .adapter_state_changed_cb       = NULL,
    .adapter_properties_cb          = NULL,
    .remote_device_properties_cb    = NULL,
    .device_found_cb                = NULL,
    .discovery_state_changed_cb     = NULL,
    .pin_request_cb                 = NULL,
    .ssp_request_cb                 = NULL,
    .bond_state_changed_cb          = NULL,
    .acl_state_changed_cb           = NULL,
};

int32_t app_bth_module_bt_if_offline_open(uint8_t comm_type)
{
    TRACE(2, "[%s] callback_if_ptr=0x%p", __func__, &bt_state_report_if_cbs);
    bth_if_thread_client_open(APP_BTH_IF_USER_BT, &bt_state_report_if_cbs, comm_type);
    return 0;
}

#3. 添加私用命令的数据类型和定义
bth_if_cmd_def.h文件中app_bth_if_comm_subcmd_t添加私用命令的定义。
bth_if_cmd_def.h文件中定义私用命令的数据结构。例如：
typedef struct {
    bd_addr_t   bt_addr;
} __attribute__((aligned(1))) app_bt_create_conn_cmd_t;

然后调用bth_if_thread_client_ctrl()接口发送对应的数据到server端。

#4. 返回执行结果到client端
bth_if_cmd_def.h文件中app_bth_if_comm_result_rsp_t添加返回结果的命令定义。
bth_if_cmd_def.h文件中定义返回结果的数据结构。例如：
typedef struct {
    bd_addr_t   bt_addr;
    int32_t     rssi_value;
    uint8_t     device_name[64];
} __attribute__((aligned(1))) app_bt_discover_result_evt_t;

然后调用bth_if_send_result_to_app()接口发送执行的结果到client端。

