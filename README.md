# Tuya IoTOS Embeded Demo WiFi & BLE smart mirror

[English](./README.md) | [中文](./README_zh.md)

## Introduction 


This Demo is based on Toodle Smart Cloud Platform, Toodle Smart APP, IoTOS Embeded WiFi &Ble SDK, using Tuya WiFi/WiFi+BLE series modules to build a smart mirror demo with the following functions.
+ Color screen display time date Sunday gas and power
+ Touch button local control, set mirror fill light color and brightness and screen darkness and extinction
+ Support cell phone remote APP control
+ Support auto light up by human body sensor


## Quick start

### Compile and burn
+ Download [Tuya IoTOS Embeded WiFi & BLE sdk](https://github.com/tuya/tuya-iotos-embeded-sdk-wifi-ble-bk7231n) 

+ Download the demo to the apps directory of the SDK 

  ```bash
  $ cd apps
  $ git clone https://github.com/Tuya-Community/tuya-iotos-embeded-demo-wifi-ble-smart-mirror
  ```
  
+ Execute the following command in the SDK root directory to start compiling.

  ```bash
  sh build_app.sh apps/bk7231n_mirror_demo bk7231n_mirror_demo 1.0.0 
  ```

+ Firmware burn-in license information please refer to: [Wi-Fi + BLE series module burn-in license](https://developer.tuya.com/cn/docs/iot/device-development/burn-and-authorization/burn-and-authorize-wifi-ble-modules/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd) 



### File description

Translated with www.DeepL.com/Translator (free version)
```
├── src	
|    ├── mirror_driver
|    |    ├── tuya_mirror_pwm.c             //PWM driver related files
|    |    ├── tuya_mirror_key.c             //Touch button related code file
|    |    └── tuya_mirror_screen.c            //Display-related code files
|    ├── mirror_soc                   //tuya SDK soc layer interface related files
|    ├── tuya_device.c             //Application layer entry file
|    ├── app_mirror.c            //Main application layer
|    ├── svc_weather_service.c            //Weather service component (not open to the public at this time)
|    └── mirror_control.c             //Key-related logic
| 
├── include				//Header file directory
|    ├── mirror_driver
|    |    ├── tuya_mirror_pwm.h      
|    |    ├── tuya_mirror_key.h   
|    |    └── tuya_mirror_screen.h         
|    ├── mirror_soc
|    ├── tuya_device.h
|    ├── app_mirror.h
|    ├── svc_weather_service.h
|    └── mirror_control.h
|
└── output              //Compilation products
```

Note: The weather forecast function component involved in this demo is temporarily closed to the public, and the actual svc_weather_service file will be missing. The macro WEATHER_FUNCTION is defined in mirror_control.h to control whether weather-related code is compiled.

<br>

### Application entry
Entry file: tuya_device.c

Important functions: device_init()

+ Call tuya_iot_wf_soc_dev_init_param() interface to initialize the SDK, configure the operating mode, the mating mode, and register various callback functions and store the firmware key and PID.
+ Calling the tuya_iot_reg_get_wf_nw_stat_cb() interface to register the device network status callback functions.
+ Call the application layer initialization function app_mirror_init()

<br>

### dp point related

+ Send down dp point data stream: dev_obj_dp_cb() -> deal_dp_proc()
+ Report dp point interface: dev_report_dp_json_async()

| function name | OPERATE_RET dev_report_dp_json_async(IN CONST CHAR_T *dev_id,IN CONST TY_OBJ_DP_S *dp_data,IN CONST UINT_T cnt)|
| ---|--|
| devid | device id (if it is a gateway, MCU, SOC class device then devid = NULL; if it is a sub-device, then devid = sub-device_id)|
| dp_data | dp structure array name|
| cnt | number of elements of the dp structure array|
| return | OPRT_OK: Success Other: Failure |

### I/O List

Translated with www.DeepL.com/Translator (free version)

|Screen|PIR|KEY|Light|
| --- | --- | --- | --- |
|`screen_RX` SOC_TX1|`IO` P6|`on/off` P22|`WARM` P24|
|||`set` P20|`COLD` P26|
|||`up` P14||
|||`down` P16||

<br>



## Related Documents

tuya Demo Center: https://developer.tuya.com/demo


<br>


## Technical Support

You can get support for Tuya by using the following methods:

- Developer Center: https://developer.tuya.com
- Help Center: https://support.tuya.com/help
- Technical Support Work Order Center: [https://service.console.tuya.com](https://service.console.tuya.com/) 


<br>


