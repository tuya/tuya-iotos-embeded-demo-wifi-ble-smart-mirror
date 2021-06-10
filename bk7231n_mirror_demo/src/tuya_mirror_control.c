/*
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @LastEditors: zgw
 * @file name: tuya_mirror_control.c
 * @Description: 
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2020-12-16 18:51:29
 * @LastEditTime: 2021-06-10 13:23:44
 */

#include "tuya_mirror_control.h"
#include "tuya_gpio.h"
#include "tuya_uart.h"
#include "BkDriverUart.h"
#include "tuya_mirror_pwm.h"
#include "sys_timer.h"
#include "tuya_mirror_key.h"
#include "uni_time.h"
#include "soc_adc.h"
#include "tuya_hal_thread.h"
#include "tuya_mirror_screen.h"
#include "cJSON.h"
#include "string.h"
#include "BkDriverGpio.h"

#if WEATHER_FUNCTION

#include "svc_weather_service.h"

#endif


/*********************Global structure**********************/
MIRROR_CTRL_DATA_T mirror_ctrl_data = {0};

/************************** Light **************************/
#define LIGHT_VALUE_DEFAULTS              (700)
#define WIFI_LED_PORT                     (8)

STATIC UCHAR_T flash_flag = 0;

/************************** Timer **************************/
STATIC TIMER_ID connect_state_timer;
STATIC TIMER_ID double_click_timer;
STATIC TIMER_ID off_timer;
STATIC TIMER_ID sec_timer;
STATIC TIMER_ID get_weather;
STATIC TIMER_ID get_date;

STATIC VOID connect_state_cb(UINT_T timerID, PVOID_T pTimerArg);
STATIC VOID double_click_timer_cb(UINT_T timerID, PVOID_T pTimerArg);
STATIC VOID off_timer_cb(UINT_T timerID, PVOID_T pTimerArg);
STATIC VOID sec_timer_cb(UINT_T timerID, PVOID_T pTimerArg);
STATIC VOID get_weather_timer_cb(UINT_T timerID, PVOID_T pTimerArg);
STATIC VOID get_date_timer_cb(UINT_T timerID, PVOID_T pTimerArg);

/************************** Keys ***************************/
UINT8_T key_buf = 0;
UINT8_T key_old = 0;
UINT8_T key_delay_cont = 0;

UINT8_T key_trg = 0x00;
UINT8_T key_cont = 0x00;

UINT8_T double_click_flag = 0;

/************************ screen ***************************/
#define SCREEN_POWER_PORT              (28)

/******************** Battery detection ********************/
#define CHARGING_FLAG_PORT              (7)

#define TEMP_ADC_DATA_LEN           (4)
tuya_adc_dev_t tuya_adc;

UINT8_T power_sample_cnt = 0;
float power_sample_sum = 0;
UINT8_T low_power_flag = 0;

UINT8_T charging_flag = 0;
STATIC VOID mirror_battery_detect(VOID);
STATIC VOID mirror_charging_detect(VOID);

/****************** Weather & date get *********************/
#if WEATHER_FUNCTION
UINT8_T *weather_data = "[\"w.temp\",\"w.conditionNum\"]";
STATIC VOID weather_get_cb(IN TY_WEATHER_STATUS_E result, IN CHAR_T *p_data, IN UINT_T p_nex_time_min);
#endif

STATIC VOID conditon_num_change(UCHAR_T *condition_num);
STATIC VOID mirror_date_get(VOID);

/************************** Pir ****************************/
#define PIR_IN_PORT                      (6)
STATIC VOID mirror_pir_detect(VOID);

/**************** Handle & poll functions ******************/
UINT8_T time_get_count = 0;
UINT8_T display_type_flag = 1;
VOID mirror_key_poll(VOID);
VOID light_switch_handle(VOID);
VOID pir_data_handle(VOID);


/***********************************************************
********************* initialize ***************************
***********************************************************/
VOID mirror_device_init(VOID)
{
    // Used to control the time acquisition interval
    sys_add_timer(connect_state_cb, NULL, &get_date);

    // Used to detect double-click button trigger
    sys_add_timer(double_click_timer_cb, NULL, &double_click_timer);

    // Used for automatic mode screen off timing
    sys_add_timer(off_timer_cb, NULL, &off_timer);

    // secend counting timer
    sys_add_timer(sec_timer_cb, NULL, &sec_timer);

    #if WEATHER_FUNCTION
    // Used to control the weather acquisition interval
    sys_add_timer(get_weather_timer_cb, NULL, &get_weather);
    sys_start_timer(get_weather, 1000*120, TIMER_CYCLE);
    #endif
    
    // Used to control the time acquisition interval
    sys_add_timer(get_date_timer_cb, NULL, &get_date);

    sys_start_timer(get_date, 1000*10, TIMER_CYCLE);
    sys_start_timer(sec_timer, 1000, TIMER_CYCLE);
    sys_start_timer(connect_state_timer, 1000*180, TIMER_ONCE);

    mirror_pwm_init();

    app_key_init();

    // Initialize io port that used to control the screen power; high level: power on , low level: power off
    tuya_gpio_inout_set(SCREEN_POWER_PORT, FALSE);
    tuya_gpio_write(SCREEN_POWER_PORT,FALSE);
    
    tuya_gpio_inout_set(WIFI_LED_PORT,FALSE);

    screen_init();

    tuya_gpio_inout_set(PIR_IN_PORT, TRUE);

    BkGpioInitialize(CHARGING_FLAG_PORT,INPUT_PULL_DOWN);
    
    // Initialize ADC 
    tuya_adc.priv.pData = Malloc(TEMP_ADC_DATA_LEN * sizeof(USHORT_T));
    memset(tuya_adc.priv.pData, 0, TEMP_ADC_DATA_LEN*sizeof(USHORT_T));
    tuya_adc.priv.data_buff_size = TEMP_ADC_DATA_LEN;
    
    #if WEATHER_FUNCTION
    // Register weather information callback function
    tuya_svc_weather_server_timer_get_asynchronization_init(weather_get_cb);
    
    tuya_svc_weather_server_timer_start_asynchronization(weather_data, 1000);
    #endif
}

/***********************************************************
**************** Control handle ****************************
***********************************************************/
VOID mirror_ctrl_handle(VOID)
{   
    MIRROR_CTRL_DATA_T *p;

    p = &mirror_ctrl_data;

    light_switch_handle();

    pir_data_handle();
}

/***********************************************************
**************** Data get handle  **************************
***********************************************************/
VOID mirror_data_get_handle(VOID)
{   
    if(mirror_ctrl_data.Wifi_state == connecting) {

        return;
    }

    time_get_count++;
    
    if(time_get_count == 10) { //20*500ms = 10s

        time_get_count = 0;
        
        // Every 10 seconds, write save data to flash
        UCHAR_T save_data_buff[SAVE_DATA_LEN] = {0};
        mirror_data_save(save_data_buff);
        opSocFlashWrite(APP_DATA_SAVE,APP_DATA_SAVE_OFFSET,save_data_buff,SAVE_DATA_LEN);
    }
    
    // Used ADC to measure battery voltage
    mirror_battery_detect();

    // Detect battery charging state
    mirror_charging_detect();

    // Detect pir IO port
    mirror_pir_detect();
}

/***********************************************************
**************** Wifi light handle *************************
***********************************************************/
VOID mirror_wifi_light_handle(VOID)
{   

    if(flash_flag) {

        tuya_gpio_write(WIFI_LED_PORT,FALSE);

        flash_flag = 0;
    }else {
        tuya_gpio_write(WIFI_LED_PORT,TRUE);

        flash_flag = 1;
    }
}

/***********************************************************
************** Screen display handle ***********************
***********************************************************/
VOID mirror_display_poll(VOID)
{
    MIRROR_CTRL_DATA_T *p;
    p = &mirror_ctrl_data;
    
    if(p->Wifi_state == connecting) {
        
        screen_bright_set(0);
        return;
    } 

    if(display_type_flag >= 41) {

        display_type_flag = 1;
    }else if(display_type_flag % 18 == 0) { //18*500ms

        if(WEATHER_FUNCTION) {

            screen_display_weather(p->Mirror_weather.condtion_type, p->Mirror_weather.temp, CON);
        }
    }else if(display_type_flag % 15 == 0) {

        if(WEATHER_FUNCTION) {
           
        screen_display_weather(p->Mirror_weather.condtion_type, p->Mirror_weather.temp, TEMP);
        }
    }else if(display_type_flag % 40 == 0) {

        screen_display_year(mirror_ctrl_data.Mirror_time.year);
    }else if(display_type_flag % 25 == 0) {

        screen_display_day(mirror_ctrl_data.Mirror_time.mon, mirror_ctrl_data.Mirror_time.mday);
    }else if(display_type_flag % 20 == 0) {

        screen_display_week(mirror_ctrl_data.Mirror_time.wday);
    }else if(display_type_flag % 5 == 0) {

        if(charging_flag) {

            screen_display_battery(charging);
        }else{

            screen_display_battery(p->Battery_remain);
        }
    }
    else {

        screen_display_time(p->Mirror_time.hour, p->Mirror_time.min);
    }

    display_type_flag++;
}


/***********************************************************
***************** Timer call back **************************
***********************************************************/
STATIC VOID connect_state_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    if(mirror_ctrl_data.Wifi_state != connected) {
        mirror_ctrl_data.Wifi_state = time_out;
        return;
    }
}

STATIC VOID double_click_timer_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    switch (double_click_flag)
    {
    case 1: //single click
        if(mirror_ctrl_data.Mirror_switch == TRUE) {

            mirror_ctrl_data.Mirror_switch = FALSE;
            mirror_ctrl_data.Light_switch = FALSE;
            mirror_ctrl_data.Screen_switch = FALSE;
            screen_bright_set(0);
        }else {

            mirror_ctrl_data.Mirror_switch = TRUE;
            mirror_ctrl_data.Light_switch = TRUE;
            mirror_ctrl_data.Screen_switch = TRUE;
            screen_bright_set(1);
        }
        app_report_all_dp_status();

        break;
    case 2: //double click
        if(mirror_ctrl_data.Mirror_switch == TRUE) {
            
            if(mirror_ctrl_data.Screen_switch == TRUE) {

                mirror_ctrl_data.Screen_switch = FALSE;
                screen_bright_set(0);
            }else {

                mirror_ctrl_data.Screen_switch = TRUE;
                screen_bright_set(1);
            }

        }

        break;
    default:
        break;
    }

    double_click_flag = 0;
}

STATIC VOID off_timer_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    mirror_ctrl_data.Light_switch = FALSE;
}

STATIC VOID sec_timer_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    mirror_ctrl_data.Mirror_time.sec++;
    if(mirror_ctrl_data.Mirror_time.sec >= 60) {
        mirror_ctrl_data.Mirror_time.sec = 0;
        mirror_ctrl_data.Mirror_time.min++;
        if(mirror_ctrl_data.Mirror_time.min >= 60) {
            mirror_ctrl_data.Mirror_time.min = 0;
            mirror_ctrl_data.Mirror_time.hour++;
            if(mirror_ctrl_data.Mirror_time.hour >= 24) {
                mirror_ctrl_data.Mirror_time.hour = 0;
            }
        }
    }
}

#if WEATHER_FUNCTION
STATIC VOID get_weather_timer_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    if(mirror_ctrl_data.Wifi_state != connecting) {

        tuya_svc_weather_server_timer_start_asynchronization(weather_data, 1000);
    }
}
#endif

STATIC VOID get_date_timer_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    mirror_date_get();
}


/***********************************************************
************************ Keys ******************************
***********************************************************/
STATIC VOID mirror_key_event(UINT8_T key_event)
{
    MIRROR_CTRL_DATA_T *p;

    p = &mirror_ctrl_data;

    if(key_event == KEY_CODE_SWITCH) {
        double_click_flag++;
        PR_NOTICE("-----------click_flag = %d-------------",double_click_flag);
        if(double_click_flag == 1) {

            sys_start_timer(double_click_timer, 300, TIMER_ONCE);
        }

    }else if(key_event == KEY_CODE_SET_LIGHT_COLOR) {

        if(p->Mirror_switch == FALSE) {

            return;
        }

        if(p->Light_switch == FALSE) {

            return;
        }

        p->Light_mode++;

        if(p->Light_mode > 2){

            p->Light_mode = 0;
        }

        if(p->Wifi_state != connecting){

            mirror_pwm_set(p->Light_mode,p->Light_value);
        }


        PR_NOTICE("-----------change light mode to %d-------------",p->Light_mode);
    }
    else if(key_event == KEY_CODE_UP) {

    }
    else if(key_event == KEY_CODE_DOWN) {

    }
}

VOID mirror_key_poll(VOID)
{
    MIRROR_CTRL_DATA_T *p;

    p = &mirror_ctrl_data;

    app_key_scan(&key_trg,&key_cont);
    if(key_trg != 0){
        PR_NOTICE("-----------key_trg = %d-----key_cont = %d-------------",key_trg,key_cont);
    }

    switch (key_cont)
    {
    case KEY_CODE_RELEASE:

        if(key_buf != 0) {

            mirror_key_event(key_buf);
        }

        key_buf = 0;
        key_old = KEY_CODE_RELEASE;

        break;
    case KEY_CODE_SWITCH:
        
        if(p->Wifi_state == connecting) {
            
            return;
        }

        vTaskDelay(10);
        app_key_scan(&key_trg,&key_cont);

        if(key_cont == KEY_CODE_SWITCH) {

            key_buf = KEY_CODE_SWITCH;
        }

        key_old = KEY_CODE_SWITCH;

        break;
    case KEY_CODE_SET_LIGHT_COLOR:
        if(key_buf == 0xff) {
            break;
        }

        if(key_old == KEY_CODE_SET_LIGHT_COLOR) {

            key_delay_cont++;
        }else{

            key_delay_cont = 0;
        }

        if(key_delay_cont >= 2) {

            key_buf = KEY_CODE_SET_LIGHT_COLOR;
        }

        if(key_delay_cont >= 200) {

            key_buf = 0xff;
            key_delay_cont = 0;

            screen_power_off();
            tuya_gpio_write(SCREEN_POWER_PORT,TRUE);
            
            tuya_iot_wf_gw_unactive();
            PR_NOTICE("-----------connect reset-------------");
        }

        key_old = KEY_CODE_SET_LIGHT_COLOR;

        break;
    case KEY_CODE_UP:

        if(p->Mirror_switch == FALSE) {

            key_buf = 0;

            return ;
        }

        if(p->Light_switch == FALSE) {

            key_buf = 0;

            return ;
        }

        if(key_old == KEY_CODE_UP) {

            key_delay_cont++;
        }else{

            key_delay_cont = 0;
        }

        if(key_delay_cont >= 2) {

            key_buf = KEY_CODE_UP;
        }

        if(key_delay_cont >= 40) {

            key_buf = 0;

            if(p->Light_value <= 995) {

                p->Light_value += 10;

                mirror_pwm_set(p->Light_mode,p->Light_value);
            }
        }
        
        key_old = KEY_CODE_UP;
        break;
    case KEY_CODE_DOWN:

        if(p->Mirror_switch == FALSE) {

            key_buf = 0;

            return ;
        }

        if(p->Light_switch == FALSE) {

            key_buf = 0;

            return ;
        }

        if(key_old == KEY_CODE_DOWN) {

            key_delay_cont++;
        }else{

            key_delay_cont = 0;
        }

        if(key_delay_cont >= 2) {

            key_buf = KEY_CODE_DOWN;
        }

        if(key_delay_cont >= 40) {

            key_buf = 0;

            if(p->Light_value>=205) {

                p->Light_value -= 10;

                mirror_pwm_set(p->Light_mode,p->Light_value);
            } 
        }
        
        key_old = KEY_CODE_DOWN;        
        break;          
    default:
        break;
    }
 
}

/***********************************************************
********************** Others ******************************
***********************************************************/
VOID light_switch_handle(VOID)
{
    MIRROR_CTRL_DATA_T *p;

    p = &mirror_ctrl_data;
    
    tuya_gpio_write(WIFI_LED_PORT,FALSE);

    if(p->Light_switch != TRUE) {

        mirror_pwm_off();
    }

    if((p->Light_switch_old != p->Light_switch)&&(p->Light_switch == TRUE)) {

        mirror_pwm_set(p->Light_mode,p->Light_value);
    }
    
    p->Light_switch_old = p->Light_switch;
}

VOID pir_data_handle(VOID)
{
    MIRROR_CTRL_DATA_T *p;

    p = &mirror_ctrl_data;

    if(p->PIR_state == trigger) {

        p->PIR_state = none;

        if((p->PIR_switch != TRUE)||(p->Mirror_switch != TRUE)) {

            return;
        }

        if(IsThisSysTimerRun(off_timer) == TRUE) {
            sys_stop_timer(off_timer);
            sys_start_timer(off_timer, 1000*600, TIMER_ONCE);
        }else {
            sys_start_timer(off_timer, 1000*600, TIMER_ONCE);
        }

        p->Light_switch = TRUE;

    }else if(p->PIR_state == first_init) {
        
        p->PIR_state = none;
        sys_start_timer(off_timer, 1000*600, TIMER_ONCE);
    }
}

STATIC VOID mirror_battery_detect(VOID)
{   
    MIRROR_CTRL_DATA_T *p;

    p = &mirror_ctrl_data;

    
    if(!charging_flag) {
        
        USHORT_T adc_value = 0;
        float adc_voltage = 0.0;

        tuya_hal_adc_init(&tuya_adc);
        tuya_hal_adc_value_get(TEMP_ADC_DATA_LEN, &adc_value);
        adc_voltage = 2.4*((float)adc_value/2048);

        tuya_hal_adc_finalize(&tuya_adc);

        power_sample_sum += adc_voltage;
        power_sample_cnt++;

        if(power_sample_cnt == 10) {
            adc_voltage = power_sample_sum/power_sample_cnt;
            power_sample_sum = 0;
            power_sample_cnt = 0;
            PR_NOTICE("-----------adc_voltage = %f-------------",adc_voltage);

            if(adc_voltage >= 2.2) {

                p->Battery_remain = high_percent; 
            }else if(adc_voltage >= 2.1) {

                p->Battery_remain = medium_percent; 
            }else {
                
                p->Battery_remain = low_percent; 
            }

        }else {
            return;
        }
    }

}

STATIC VOID mirror_date_get(VOID)
{
    if(mirror_ctrl_data.Wifi_state == connecting) {
        
        return;
    }

    POSIX_TM_S cur_time;
    if( uni_local_time_get(&cur_time) != OPRT_OK ) {
        PR_NOTICE("cant get local time");
    }

    mirror_ctrl_data.Mirror_time.sec = (UCHAR_T)cur_time.tm_sec;
    mirror_ctrl_data.Mirror_time.min = (UCHAR_T)cur_time.tm_min;
    mirror_ctrl_data.Mirror_time.hour = (UCHAR_T)cur_time.tm_hour;

    if(mirror_ctrl_data.Mirror_time.year != cur_time.tm_year) {

        mirror_ctrl_data.Mirror_time.year = (1900 + cur_time.tm_year);

    }

    if((mirror_ctrl_data.Mirror_time.mon != cur_time.tm_mon)||(mirror_ctrl_data.Mirror_time.mday != cur_time.tm_mday)) {

        mirror_ctrl_data.Mirror_time.mon= (UCHAR_T)cur_time.tm_mon;
        mirror_ctrl_data.Mirror_time.mday = (UCHAR_T)cur_time.tm_mday;

    }

    if(mirror_ctrl_data.Mirror_time.wday != cur_time.tm_wday) {
        mirror_ctrl_data.Mirror_time.wday = (UCHAR_T)cur_time.tm_wday;

    }
}

STATIC VOID mirror_pir_detect(VOID)
{    
    if(tuya_gpio_read(PIR_IN_PORT)) {
        PR_NOTICE("-----------SOMEONE HERE-------------");
        mirror_ctrl_data.PIR_state = trigger;
    }
}

STATIC VOID mirror_charging_detect(VOID)
{
    if(tuya_gpio_read(CHARGING_FLAG_PORT)) {
        charging_flag = 1;

    }else {

        charging_flag = 0;
    }
}

VOID mirror_ctrl_all_off(VOID)
{

}

#if WEATHER_FUNCTION
STATIC VOID weather_get_cb(IN TY_WEATHER_STATUS_E result, IN CHAR_T *p_data, IN UINT_T p_nex_time_min)
{
    cJSON *root = 0;
    cJSON *temp = 0;
    cJSON *condition = 0;

    root = cJSON_Parse(p_data);
    if(root) {

        temp = cJSON_GetObjectItem(root,"w.temp");
        mirror_ctrl_data.Mirror_weather.temp = temp->valueint;
        
        condition = cJSON_GetObjectItem(root,"w.conditionNum");
        conditon_num_change(condition->valuestring);
        PR_NOTICE("-------condition : %d-------",mirror_ctrl_data.Mirror_weather.condtion_type);
    }


    cJSON_Delete(root);
}
#endif

VOID mirror_data_save(UCHAR_T *data)
{
    MIRROR_CTRL_DATA_T *p;
    p = &mirror_ctrl_data;

    data[LIGHT_MODE_ADDR] = p->Light_mode;
    data[LIGHT_VALUE_H_ADDR] = (UCHAR_T)(p->Light_value >> 8);
    data[LIGHT_VALUE_L_ADDR] = (UCHAR_T)p->Light_value;
    data[TIME_SEC_ADDR] = p->Mirror_time.sec;
    data[TIME_MIN_ADDR] = p->Mirror_time.min;
    data[TIME_HOUR_ADDR] = p->Mirror_time.hour;
    data[DAY_ADDR] = p->Mirror_time.mday;
    data[MONTH_ADDR] = p->Mirror_time.mon;
    data[YEAR_BYTE_1_ADDR] = (UCHAR_T)(p->Mirror_time.year >> 24);
    data[YEAR_BYTE_2_ADDR] = (UCHAR_T)(p->Mirror_time.year >> 16);
    data[YEAR_BYTE_3_ADDR] = (UCHAR_T)(p->Mirror_time.year >> 8);
    data[YEAR_BYTE_4_ADDR] = (UCHAR_T)p->Mirror_time.year;
    data[WEEK_DAY_ADDR] = p->Mirror_time.wday;
    data[CONDTION_ADDR] = p->Mirror_weather.condtion_type;
    data[TEMP_ADDR] = p->Mirror_weather.temp;
}

VOID mirror_data_load(UCHAR_T *data)
{
    MIRROR_CTRL_DATA_T *p;
    p = &mirror_ctrl_data;

    p->Light_mode = data[LIGHT_MODE_ADDR];
    p->Light_value = (((USHORT_T)data[LIGHT_VALUE_H_ADDR])<<8) | ((USHORT_T)data[LIGHT_VALUE_L_ADDR]);
    p->Mirror_time.sec = data[TIME_SEC_ADDR];
    p->Mirror_time.min = data[TIME_MIN_ADDR];
    p->Mirror_time.hour = data[TIME_HOUR_ADDR];
    p->Mirror_time.mday = data[DAY_ADDR];
    p->Mirror_time.mon = data[MONTH_ADDR];
    p->Mirror_time.year = (((INT_T)data[YEAR_BYTE_1_ADDR])<<24) | (((INT_T)data[YEAR_BYTE_2_ADDR])<<16) | (((INT_T)data[YEAR_BYTE_3_ADDR])<<8) | ((INT_T)data[YEAR_BYTE_4_ADDR]);
    p->Mirror_time.wday = data[WEEK_DAY_ADDR];
    p->Mirror_weather.condtion_type = data[CONDTION_ADDR];
    p->Mirror_weather.temp = data[TEMP_ADDR];
}

STATIC VOID conditon_num_change(UCHAR_T *condition_num)
{
    // Because of this is a demo, not every condtions can be displayed on the screen, so all condtions was converted to "rain"、"sun" and "cloud".
    if((strcmp(condition_num,"101") == 0)||\
       (strcmp(condition_num,"107") == 0)||\
       (strcmp(condition_num,"111") == 0)||\
       (strcmp(condition_num,"112") == 0)||\
       (strcmp(condition_num,"113") == 0)||\
       (strcmp(condition_num,"118") == 0)||\
       (strcmp(condition_num,"122") == 0)||\
       (strcmp(condition_num,"123") == 0)||\
       (strcmp(condition_num,"134") == 0)||\
       (strcmp(condition_num,"139") == 0)||\
       (strcmp(condition_num,"141") == 0)||\
       (strcmp(condition_num,"143") == 0)||\
       (strcmp(condition_num,"144") == 0)||\
       (strcmp(condition_num,"145") == 0)) {
        
        mirror_ctrl_data.Mirror_weather.condtion_type = rain;
    }else if((strcmp(condition_num,"120") == 0)||\
             (strcmp(condition_num,"119") == 0)||\
             (strcmp(condition_num,"146") == 0)) {

        mirror_ctrl_data.Mirror_weather.condtion_type = sun;
    }else if((strcmp(condition_num,"129") == 0)||\
             (strcmp(condition_num,"142") == 0)) {

        mirror_ctrl_data.Mirror_weather.condtion_type = cloud;
    }
}
