# Tuya IoTOS Embeded Demo WiFi & BLE smart mirror

[English](./README.md) | [中文](./README_zh.md)

## 简介 


本 Demo 基于涂鸦智能云平台、涂鸦智能APP、IoTOS Embeded WiFi &Ble SDK，使用涂鸦WiFi/WiFi+BLE系列模组组建一个智能镜demo，具有以下功能：
+ 彩屏显示时间日期星期天气和电量
+ 触摸按键本地控制，设置镜子补光灯颜色及亮度和屏幕的暗灭
+ 支持手机远程APP控制
+ 支持人体感应自动亮灯


## 快速上手

### 编译与烧录
+ 下载[Tuya IoTOS Embeded WiFi & BLE sdk](https://github.com/tuya/tuya-iotos-embeded-sdk-wifi-ble-bk7231n) 

+ 下载Demo至SDK目录的apps目录下 

  ```bash
  $ cd apps
  $ git clone https://github.com/Tuya-Community/tuya-iotos-embeded-demo-wifi-ble-smart-mirror
  ```
  
+ 在SDK根目录下执行以下命令开始编译：

  ```bash
  sh build_app.sh apps/bk7231n_mirror_demo bk7231n_mirror_demo 1.0.0 
  ```

+ 固件烧录授权相关信息请参考：[Wi-Fi + BLE 系列模组烧录授权](https://developer.tuya.com/cn/docs/iot/device-development/burn-and-authorization/burn-and-authorize-wifi-ble-modules/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd) 



### 文件介绍
```
├── src	
|    ├── mirror_driver
|    |    ├── tuya_mirror_pwm.c             //PWM驱动相关文件
|    |    ├── tuya_mirror_key.c             //触摸按键相关代码文件
|    |    └── tuya_mirror_screen.c            //显示屏相关代码文件
|    ├── mirror_soc                   //tuya SDK soc层接口相关文件
|    ├── tuya_device.c             //应用层入口文件
|    ├── tuya_app.c            //主要应用层
|    ├── svc_weather_service.c            //天气服务组件(暂不对外开放)
|    └── tuya_mirror_control.c             //设备功能逻辑
| 
├── include				//头文件目录
|    ├── mirror_driver
|    |    ├── tuya_mirror_pwm.h      
|    |    ├── tuya_mirror_key.h   
|    |    └── tuya_mirror_screen.h         
|    ├── mirror_soc
|    ├── tuya_device.h
|    ├── tuya_app.h
|    ├── svc_weather_service.h
|    └── tuya_mirror_control.h
|
└── output              //编译产物
```

注意：本demo涉及到的天气预报功能组件暂时不对外开放，实际会缺少svc_weather_service文件。在mirror_control.h中定义了宏WEATHER_FUNCTION，用于控制是否编译天气相关代码。

<br>

### 应用入口
入口文件：tuya_device.c

重要函数：device_init()

+ 调用 tuya_iot_wf_soc_dev_init_param() 接口进行SDK初始化，配置了工作模式、配网模式，同时注册了各种回调函数并存入了固件key和PID。
+ 调用 tuya_iot_reg_get_wf_nw_stat_cb() 接口注册设备网络状态回调函数。
+ 调用应用层初始化函数 app_mirror_init()

<br>

### dp点相关

+ 下发dp点数据流：dev_obj_dp_cb() -> deal_dp_proc()
+ 上报dp点接口: dev_report_dp_json_async()

|函数名 | OPERATE_RET dev_report_dp_json_async(IN CONST CHAR_T *dev_id,IN CONST TY_OBJ_DP_S *dp_data,IN CONST UINT_T cnt)|
|	---|---|
|    devid | 设备id（若为网关、MCU、SOC类设备则devid = NULL;若为子设备，则devid = sub-device_id)|
|    dp_data | dp结构体数组名|
|    cnt |dp结构体数组的元素个数|
|    Return    |  OPRT_OK: 成功  Other: 失败 |

### I/O 列表

|Screen|PIR|KEY|Light|
| --- | --- | --- | --- |
|`screen_RX` SOC_TX1|`IO` P6|`on/off` P22|`WARM` P24|
|||`set` P20|`COLD` P26|
|||`up` P14||
|||`down` P16||

<br>



## 相关文档

涂鸦Demo中心：https://developer.tuya.com/demo


<br>


## 技术支持

您可以通过以下方法获得涂鸦的支持:

- 开发者中心：https://developer.tuya.com
- 帮助中心: https://support.tuya.com/help
- 技术支持工单中心: [https://service.console.tuya.com](https://service.console.tuya.com/) 


<br>


