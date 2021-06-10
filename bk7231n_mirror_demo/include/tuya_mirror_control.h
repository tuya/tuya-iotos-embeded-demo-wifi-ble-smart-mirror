/*
 * @file name: tuya_mirror_control.h
 * @Descripttion: 
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-03-01 10:44:58
 * @LastEditors: zgw
 * @LastEditTime: 2021-06-07 15:18:01
 */
#ifndef __TUYA_MIRROR_CONTROL_H__
#define __TUYA_MIRROR_CONTROL_H__

#include "uni_log.h"
#include "tuya_cloud_error_code.h"
#include "tuya_mirror_screen.h"


#define WEATHER_FUNCTION                             0

#define DISTANCE_THRESHOLD                           10
#define LIGHT_INTENSITY_THRESHOLD                    25
#define SIT_REMIND_THRESHOLD                         6


#define APP_DATA_SAVE                                20
#define APP_DATA_SAVE_OFFSET                         10

#define SAVE_DATA_LEN                                15

#define LIGHT_MODE_ADDR                              0
#define LIGHT_VALUE_H_ADDR                           1
#define LIGHT_VALUE_L_ADDR                           2
#define TIME_SEC_ADDR                                3
#define TIME_MIN_ADDR                                4
#define TIME_HOUR_ADDR                               5
#define DAY_ADDR                                     6
#define MONTH_ADDR                                   7
#define YEAR_BYTE_1_ADDR                             8
#define YEAR_BYTE_2_ADDR                             9
#define YEAR_BYTE_3_ADDR                             10
#define YEAR_BYTE_4_ADDR                             11
#define WEEK_DAY_ADDR                                12
#define CONDTION_ADDR                                13
#define TEMP_ADDR                                    14



typedef enum {
    white,
    medium,
    warm
}LIGHT_MODE;

typedef enum {
    none,
    trigger,
    first_init
}PIR_STATE;

typedef enum {
    time_out,
    connecting,
    connected
}WIFI_STATE;

typedef struct {
    UCHAR_T sec;     /* seconds [0-59] */
    UCHAR_T min;     /* minutes [0-59] */
    UCHAR_T hour;    /* hours [0-23] */
    UCHAR_T mday;    /* day of the month [1-31] */
    UCHAR_T mon;     /* month [0-11] */
    INT_T year;    /* year. The number of years since 1900 */
    UCHAR_T wday;    /* day of the week [0-6] 0-Sunday...6-Saturday */
}MIRROR_TIME_T;

typedef struct {
    CONDTION_TYPE condtion_type;
    INT_T temp;
}MIRROR_WEATHER_T;


typedef struct {
    BOOL_T Mirror_switch;
    BOOL_T Screen_switch;
    BOOL_T Light_switch;
    BOOL_T Light_switch_old;
    LIGHT_MODE Light_mode;
    SHORT_T Light_value;
    BOOL_T PIR_switch;
    PIR_STATE PIR_state;
    BATTERY_STATE Battery_remain;
    MIRROR_TIME_T Mirror_time;
    MIRROR_WEATHER_T Mirror_weather;
    UCHAR_T Wifi_state;
}MIRROR_CTRL_DATA_T;

/********************************************************************************
 * FUNCTION:       mirror_device_init
 * DESCRIPTION:    device initialization
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_device_init(VOID);

/********************************************************************************
 * FUNCTION:       mirror_data_get_handle
 * DESCRIPTION:    Get ADC sensor data
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_data_get_handle(VOID);

/********************************************************************************
 * FUNCTION:       mirror_ctrl_handle
 * DESCRIPTION:    sensor data deal handle
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_ctrl_handle(VOID);

/********************************************************************************
 * FUNCTION:       mirror_ctrl_all_off
 * DESCRIPTION:    Close all control components
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_ctrl_all_off(VOID);

/********************************************************************************
 * FUNCTION:       mirror_wifi_light_handle
 * DESCRIPTION:    Control the lights in the distribution network
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_wifi_light_handle(VOID);

/********************************************************************************
 * FUNCTION:       mirror_key_poll
 * DESCRIPTION:    Key scan polling function
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_key_poll(VOID);

/********************************************************************************
 * FUNCTION:       mirror_display_poll
 * DESCRIPTION:    Screen display polling function
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID mirror_display_poll(VOID);

VOID mirror_data_save(UCHAR_T *data);
VOID mirror_data_load(UCHAR_T *data);

#endif