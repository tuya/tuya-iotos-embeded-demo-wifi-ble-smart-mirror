# Tuya IoTOS Embedded Wi-Fi & Bluetooth LE Smart Mirror

[English](./README.md) | [中文](./README_zh.md)

## Overview


In this demo, we will show you how to prototype a smart mirror and make it IoT-enabled. Based on the [Tuya IoT Platform](https://iot.tuya.com/), we use Tuya's Wi-Fi and Bluetooth LE combo module, SDK, and the Tuya Smart app to connect the mirror to the cloud. The smart mirror features:
+ Color display of time and date, weather, and battery level
+ Touch switch, adjustable light color and brightness, and display on/off control
+ Remote control with a mobile app
+ PIR sensor-based auto light on/off


## Get started

### Compile and flash
+ Download [Tuya IoTOS Embedded Wi-Fi & Bluetooth LE SDK](https://github.com/tuya/tuya-iotos-embeded-sdk-wifi-ble-bk7231n).

+ Clone this demo to the `app` folder in the downloaded SDK.

   ```bash
   $ cd apps
   $ git clone https://github.com/Tuya-Community/tuya-iotos-embeded-demo-wifi-ble-smart-mirror
   ```

+ Execute the following command in the SDK root directory and start to build firmware.

   ```bash
   sh build_app.sh apps/bk7231n_mirror_demo bk7231n_mirror_demo 1.0.0 
   ```

+ For more information about flashing and authorization, see [Flash Firmware to and Authorize WB Series Modules](https://developer.tuya.com/en/docs/iot/device-development/burn-and-authorization/burn-and-authorize-wifi-ble-modules/burn-and-authorize-wb-series-modules?id=Ka78f4pttsytd).



### File introduction
```
├── src  
|    ├── mirror_driver
|    |    ├── tuya_mirror_pwm.c             // PWM driver
|    |    ├── tuya_mirror_key.c             // Touch key code
|    |    └── tuya_mirror_screen.c            // Display code
|    ├── mirror_soc                   // Interfaces of SoC layer
|    ├── tuya_device.c             // Entry file of the application layer
|    ├── tuya_app.c            // Main application layer
|    ├── svc_weather_service.c            // Weather service components (not public-facing now)
|    └── tuya_mirror_control.c             // Device control logic
|
├── include 			  // Header directory
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
└── output              // Production
```

> **Note**: The weather service components are not public-facing currently, so the `svc_weather_service` file will not be included in your actual development project. The macro `WEATHER_FUNCTION` defined in `mirror_control.h` can control whether to compile code of weather services.



### Entry to application

Entry file: `tuya_device.c`

Function: `device_init()`

+ Call `tuya_iot_wf_soc_dev_init_param()` for SDK initialization to configure working mode and pairing mode, register callbacks, and save the firmware key and PID.
+ Call `tuya_iot_reg_get_wf_nw_stat_cb()` to register callback of device network status.
+ Call the initialization function `app_mirror_init()` in the application layer.



### Data point (DP)

+ Send DP data: `dev_obj_dp_cb() -> deal_dp_proc()`
+ Report DP data: `dev_report_dp_json_async()`

| Function name | OPERATE_RET dev_report_dp_json_async(IN CONST CHAR_T *dev_id,IN CONST TY_OBJ_DP_S *dp_data,IN CONST UINT_T cnt) |
|---|---|
| devid | For gateways and devices built with the MCU or SoC, the `devid` is NULL. For sub-devices, the `devid` is `sub-device_id`. |
| dp_data | The name of DP struct array |
| cnt | The number of elements in the DP struct array |
| Return | `OPRT_OK`: success. Other values: failure. |

### Pin configuration

| Screen | PIR | Key | Light |
| --- | --- | --- | --- |
| `screen_RX` SOC_TX1 | `I/O` P6 | `On/Off` P22 | `Warm` P24 |
|  |  | `Set` P20 | `Cool` P26 |
|  |  | `Up` P14 |
|  |  | `Down` P16 |


## Reference

[Tuya Project Hub](https://developer.tuya.com/demo)


## Technical support

You can get support from Tuya with the following methods:

+ [Service & Support](https://service.console.tuya.com)
+ [Tuya IoT Developer Platform](https://developer.tuya.com/en/)
+ [Help Center](https://support.tuya.com/en/help)

