程序运行逻辑如下：
1. 开始运行时，mainwindow实时响应用户操作，是主线程
2. 在mainwindow运行的同时，开启接收线程，每隔100ms接收每根电动缸的位置

主要文件及其作用
mainwindow：主窗口的配置及初始化
config：常用的全局变量，如平台的硬件参数，每根电动缸的位置信息，平台末端位姿
motorcontrol：平台与上位机UDP通信指令以及平台位置逆解和速度逆解
panel：除主界面mainwindow以外的其他界面的初始化和配置
platformcontrol：平台速度，位置控制模式函数


需要修改的部分
config.cpp：平台初始化时T0；(需要测量具体高度)

platformcontrol.cpp：
motorSpeedControl，位置限制，视电动缸行程决定
PlatFormPositionControl，位置限制，视电动缸行程决定

motorcontrol.cpp：
GetPulse，所有求出来的电动缸长度都是上下平台对应点的距离L，在转化为脉冲的时候，还需要L减去一个基础长度才
是电动缸的行程。此基础长度为平台复位时上下平台对应点的初始距离。
