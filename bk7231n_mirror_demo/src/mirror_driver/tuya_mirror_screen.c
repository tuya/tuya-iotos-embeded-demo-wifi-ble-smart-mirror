/*
 * @file name: tuya_mirror_screen
 * @Descripttion: 
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-05-13 17:33:53
 * @LastEditors: zgw
 * @LastEditTime: 2021-06-09 18:01:42
 */

#include "tuya_mirror_screen.h"
#include "tuya_uart.h"
#include "tuya_gpio.h"
#include "uni_time.h"
#include "sys_timer.h"
#include "stdio.h"
#include "mirror_control.h"
#include "BkDriverGpio.h"

tuya_uart_t *uart0;

#define SCREEN_TX_PORT                     (10)
#define SCREEN_RX_PORT                     (11)
extern MIRROR_CTRL_DATA_T mirror_ctrl_data;

UINT8_T *screen_cmd_buff[2] = {
    "CLR(15);\r\n",
    "CLR(15);FSIMG(2198064,0,140,218,99,0);FSIMG(2281900,170,5,60,60,1);FSIMG(2355160,280,190,87,45,0);FSIMG(2307160,5,85,52,51,0);\r\n",
};

UINT8_T *bright_cmd_buff[2] = {
    "BL(255);\r\n",  //full dack
    "BL(0);\r\n",   //full bright
};

UINT8_T *icon_buff[] = {
    /* 0 ~ 9 and ':' */
    "2289100","2290660","2292220","2293780","2295340","2296900","2298460","2300020","2301580","2303140","2305600",

    /* character: '年'、'月'、'日' (11~13) */
    "2312464","2314264","2316064",

    /* character: '周日' ~ '周六' (14~20) */
    "2349832","2317864","2323192","2328520","2333848","2339176","2344504",

    /* '℃' and white block (21~22)*/
    "2386480","2387560",

    /* icon of condtion : sun、rain、cloud (23~25)*/

    "2241228","2249420","2258520",

    /* character of condtion : sun、rain、cloud (26~28)*/
    "2267620","2272380","2277140",

    /* icon of battery : high、medium、low、charging (29~32)*/
    "2355160","2362990","2370820","2378650",
};

/**
 * Initialization screen display,enable serial port and send display instructions
 * @param  none
 * @return none
 */
VOID screen_init(VOID)
{

    uart0 = (tuya_uart_t *) tuya_driver_find(TUYA_DRV_UART, TUYA_UART0);
    
    if (NULL == uart0) {

        PR_ERR("tuya uart find failed");
        return;
    }

    TUYA_UART_8N1_CFG(uart0, 115200, 1024, 0);
    tuya_uart_init(uart0);
    
    tuya_hal_system_sleep(1000);
    screen_bright_set(0);
    tuya_hal_system_sleep(200);
    tuya_uart_write(uart0, screen_cmd_buff[0], strlen(screen_cmd_buff[0]));
    tuya_hal_system_sleep(200);
    tuya_uart_write(uart0, screen_cmd_buff[1], strlen(screen_cmd_buff[1]));
    tuya_hal_system_sleep(200);

    screen_display_time(mirror_ctrl_data.Mirror_time.hour, mirror_ctrl_data.Mirror_time.min);
    tuya_hal_system_sleep(200);
    screen_display_year(mirror_ctrl_data.Mirror_time.year);
    tuya_hal_system_sleep(200);
    screen_display_day(mirror_ctrl_data.Mirror_time.mon, mirror_ctrl_data.Mirror_time.mday);
    tuya_hal_system_sleep(200);
    screen_display_week(mirror_ctrl_data.Mirror_time.wday);
    tuya_hal_system_sleep(200);
    screen_display_weather(mirror_ctrl_data.Mirror_weather.condtion_type, mirror_ctrl_data.Mirror_weather.temp, CON);
    tuya_hal_system_sleep(200);
}

/**
 * Display hours and minutes
 * @param  hours 24-hour clock, 0~23 hours
 * @param  mins 0~59 mins
 * @return none
 */
VOID screen_display_time(INT_T hours, INT_T mins)
{
    if((hours < 0)||(hours > 23)) {

        return;
    }

    if((mins < 0)||(mins > 59)) {

        return;
    }

    uint8_t data_buff[160] = {0};
    
    snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,235,20,26,30,0);FSIMG(%s,262,20,26,30,0);FSIMG(2305600,289,20,26,30,1);FSIMG(%s,311,20,26,30,0);FSIMG(%s,337,20,26,30,0);\r\n",\
            icon_buff[hours/10],\
            icon_buff[hours%10],\
            icon_buff[mins/10],\
            icon_buff[mins%10]);
        
    tuya_uart_write(uart0, data_buff, strlen(data_buff));
}

/**
 * Display weeks
 * @param  weeks 0~6:Sunday to Saturday
 * @return none
 */
VOID screen_display_week(INT_T weeks)
{
    if((weeks < 0)||(weeks > 6)) {
        return;
    }

    uint8_t data_buff[40] = {0};
    
    snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,280,95,72,37,0);\r\n",icon_buff[weeks+14]);

    tuya_uart_write(uart0, data_buff, strlen(data_buff));
}

/**
 * Display years
 * @param  years Example: If it is 2020 year, enter 2020
 * @return none
 */
VOID screen_display_year(INT_T year)
{
    uint8_t data_buff[200] = {0};

    snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,85,80,26,30,0);FSIMG(%s,112,80,26,30,0);FSIMG(%s,139,80,26,30,0);FSIMG(%s,166,80,26,30,0);FSIMG(2312464,193,80,30,30,0);\r\n",\
            icon_buff[year/1000],\
            icon_buff[year/100%10],\
            icon_buff[year/10%10],\
            icon_buff[year%10]);

    tuya_uart_write(uart0, data_buff, strlen(data_buff));
}

/**
 * Display Month and day
 * @param  month 0~11 January to December
 * @param  day 1~31
 * @return none
 */
VOID screen_display_day(INT_T month, INT_T day)
{
    if((month < 0)||(month > 11)) {

        return;
    }

    if((day < 1)||(day > 31)) {

        return;
    }

    uint8_t data_buff[250] = {0};

    snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,65,115,26,30,0);FSIMG(%s,92,115,26,30,0);FSIMG(2314264,128,115,30,30,0);FSIMG(%s,163,115,26,30,0);FSIMG(%s,190,115,26,30,0);FSIMG(2316064,226,115,30,30,0);\r\n",\
            icon_buff[(month+1)/10],\
            icon_buff[(month+1)%10],\
            icon_buff[day/10],\
            icon_buff[day%10]);

    tuya_uart_write(uart0, data_buff, strlen(data_buff));
}

/**
 * Display weather and temperature
 * @param  condtion Weather code
 * @param  temp Temperature value
 * @param  display_type Decide whether to display temperature or weather
 * @return none
 */
VOID screen_display_weather(CONDTION_TYPE condtion, INT_T temp, DISPLAY_TYPE display_type)
{
    uint8_t data_buff[160] = {0};

    switch (display_type) {
    
    case CON:
        if(condtion == sun) { //the size of icon "sun" is different than other condtion icon.
            snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,3,0,64,64,0);FSIMG(%s,75,15,70,34,0);\r\n",icon_buff[condtion+23],icon_buff[condtion+26]);
            tuya_uart_write(uart0, data_buff, strlen(data_buff));
        }else {
            snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,3,0,70,65,0);FSIMG(%s,75,15,70,34,0);\r\n",icon_buff[condtion+23],icon_buff[condtion+26]);
            tuya_uart_write(uart0, data_buff, strlen(data_buff));            
        }
        break;
    case TEMP:
            snprintf(data_buff,sizeof(data_buff),"FSIMG(2387560,75,15,70,34,0);FSIMG(%s,75,19,26,30,0);FSIMG(%s,101,19,26,30,0);FSIMG(2386480,127,19,18,30,0);\r\n",\
                    icon_buff[temp/10],\
                    icon_buff[temp%10]);
            tuya_uart_write(uart0, data_buff, strlen(data_buff));    
        break;
    default:
        break;
    }
}

/**
 * Display battery icon
 * @param  state Remaining battery power state
 * @return none
 */
VOID screen_display_battery(BATTERY_STATE state)
{
    if((state < 0)||(state > 3)) {

        return;
    }

    uint8_t data_buff[40] = {0};

    snprintf(data_buff,sizeof(data_buff),"FSIMG(%s,280,190,87,45,0);\r\n",icon_buff[state+29]);

    tuya_uart_write(uart0, data_buff, strlen(data_buff));
}

/**
 * Set the screen brightness and darkness
 * @param  on_off Decide whether to set the screen off or on
 * @return none
 */
VOID screen_bright_set(UINT8_T on_off)
{
    tuya_uart_write(uart0, bright_cmd_buff[on_off], strlen(bright_cmd_buff[on_off]));
}

/**
 * Used to pull down the serial port in advance when the module restarts,avoid the abnormal situation of a white screen on the screen
 * @param  none
 * @return none
 */
VOID screen_power_off(VOID)
{
    tuya_uart_deinit(uart0);
    tuya_gpio_inout_set(SCREEN_TX_PORT, FALSE);
    tuya_gpio_write(SCREEN_TX_PORT,FALSE);
    tuya_gpio_inout_set(SCREEN_RX_PORT, FALSE);
    tuya_gpio_write(SCREEN_RX_PORT,FALSE);
}