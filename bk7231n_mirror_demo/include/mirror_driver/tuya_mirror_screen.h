/*
 * @file name: tuya_mirror_screen.h
 * @Descripttion: 
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-05-13 17:33:05
 * @LastEditors: zgw
 * @LastEditTime: 2021-06-09 18:03:00
 */
#ifndef __TUYA_MIRROR_SCREEN_H__
#define __TUYA_MIRROR_SCREEN_H__

#include "uni_log.h"
#include "tuya_cloud_error_code.h"

typedef enum {
    CON,
    TEMP,
}DISPLAY_TYPE;

typedef enum {
    sun,
    rain,
    cloud,
}CONDTION_TYPE;

typedef enum {
    high_percent,
    medium_percent,
    low_percent,
    charging,
}BATTERY_STATE;

typedef struct {
    INT_T sec;     /* seconds [0-59] */
    INT_T min;     /* minutes [0-59] */
    INT_T hour;    /* hours [0-23] */
    INT_T mday;    /* day of the month [1-31] */
    INT_T mon;     /* month [0-11] */
    INT_T year;    /* year. The number of years since 1900 */
    INT_T wday;    /* day of the week [0-6] 0-Sunday...6-Saturday */
}DIS_TIME_T;

extern UINT8_T *Cmd_buff[12];

/**
 * Initialization screen display,enable serial port and send display instructions
 * @param  none
 * @return none
 */
VOID screen_init(VOID);

/**
 * Display hours and minutes
 * @param  hours 24-hour clock, 0~23 hours
 * @param  mins 0~59 mins
 * @return none
 */
VOID screen_display_time(INT_T hours, INT_T mins);

/**
 * Display weeks
 * @param  weeks 0~6:Sunday to Saturday
 * @return none
 */
VOID screen_display_week(INT_T weeks);

/**
 * Display years
 * @param  years Example: If it is 2020 year, enter 2020
 * @return none
 */
VOID screen_display_year(INT_T year);

/**
 * Display weather and temperature
 * @param  condtion Weather code
 * @param  temp Temperature value
 * @param  display_type Decide whether to display temperature or weather
 * @return none
 */
VOID screen_display_weather(CONDTION_TYPE condtion, INT_T temp, DISPLAY_TYPE display_type);

/**
 * Display battery icon
 * @param  state Remaining battery power state
 * @return none
 */
VOID screen_display_battery(BATTERY_STATE state);

/**
 * Display Month and day
 * @param  month 0~11 January to December
 * @param  day 1~31
 * @return none
 */
VOID screen_display_day(INT_T month, INT_T day);

/**
 * Set the screen brightness and darkness
 * @param  on_off Decide whether to set the screen off or on
 * @return none
 */
VOID screen_bright_set(UINT8_T on_off);

/**
 * Used to pull down the serial port in advance when the module restarts,avoid the abnormal situation of a white screen on the screen
 * @param  none
 * @return none
 */
VOID screen_power_off(VOID);
#endif
