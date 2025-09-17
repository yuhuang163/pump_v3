1.设计目的
本层为API接口类服务层，对用户屏蔽了底层的CS架构，UI/APP可直接调用本层提供的API访问BT的功能并获取相关的结果。

2.设计方法
/apps/bth_if/bt_api_watch/
├─bt_api_def.h                                   # user调用API及获取结果所需定义的数据结构
├─bt_api_eshell_test.cpp                         # eshell测试
├─bt_callback_observer.cpp                       # 各个callback观察者的派生类获取结果的实现
├─bt_callback_observer.h                         # 观察者抽象类及各个callback观察者派生类
├─bt_user_api_callback.cpp                       # 从Client端获取API执行结果的实现
├─bt_user_api_callback.h                         # 提供访问API执行结果的接口
├─bt_user_api.cpp                                # API接口实现和结果返回实现
├─bt_user_api.h                                  # 提供API接口的类
├─Makefile                                       # Makefile文件
└─Readme.txt                                     # 使用说明

3.简单使用说明
a.以访问单例类成员函数的方式调用BtUserApi类中的Enable（）函数；
例：BtUserApi::GetIstance().Enable();

b.注册要调用API的观察者；（观察者被通知后会获得API的执行结果，如果通过d步骤主动获取结果可以跳过此步）；
例：auto DisconnectObr = std::make_shared<BtAclDisconnectReportObserver>();
    BtUserApiCallback::GetIstance().AddObr(DisconnectObr);

c.以访问单例类成员函数的方式调用BtUserApi类中提供的接口；
例：BtUserApi::GetIstance().BtDisconnectDevice((const char *)conn_info.bt_addr.addr);

d.以访问单例类成员函数的方式调用BtUserApiCallback类中提供的接口获取对应API执行结果；
例：BtUserApiCallback::GetIstance().GetBtUserApiCallback(BTH_IF_CORE_COMM_RESULT_BT_ACL_DISCONN_EVT);