/*
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @LastEditors: zgw
 * @file name: app_mirror.c
 * @Description: light init process include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2020-12-21 11:30:03
 * @LastEditTime: 2021-06-03 12:07:04
 */

#include "app_mirror.h"
#include "mirror_control.h"
#include "uni_time_queue.h"
#include "sys_timer.h"
#include "tuya_iot_wifi_api.h"
#include "FreeRTOS.h"
#include "tuya_hal_thread.h"
#include "uni_thread.h"
#include "queue.h"
#include "tuya_mirror_screen.h"
#include "soc_flash.h"

#define TASKDELAY_SEC         1000

extern MIRROR_CTRL_DATA_T mirror_ctrl_data;

STATIC VOID sensor_data_get_thread(PVOID_T pArg);
STATIC VOID sensor_data_deal_thread(PVOID_T pArg);
STATIC VOID key_scan_thread(PVOID_T pArg);
STATIC VOID diplay_send_thread(PVOID_T pArg);

OPERATE_RET app_mirror_init(IN APP_MIRROR_MODE mode)
{
    OPERATE_RET op_ret = OPRT_OK;

    if(APP_MIRROR_NORMAL == mode) {
        
        UCHAR_T read_buff[SAVE_DATA_LEN] = {0};
        uiSocFlashRead(APP_DATA_SAVE,APP_DATA_SAVE_OFFSET,SAVE_DATA_LEN,read_buff);
        mirror_data_load(read_buff);

        UCHAR_T i = 0;
        for(i = 0;i < SAVE_DATA_LEN;i++){
            PR_NOTICE("------- readbuff = %d -----",read_buff[i]);
        }

        mirror_device_init();

        tuya_hal_thread_create(NULL, "thread_data_get", 512*8, TRD_PRIO_4, sensor_data_get_thread, NULL);

        tuya_hal_thread_create(NULL, "thread_data_deal", 512*4, TRD_PRIO_4, sensor_data_deal_thread, NULL);

        tuya_hal_thread_create(NULL, "key_scan_thread", 512*4, TRD_PRIO_3, key_scan_thread, NULL);

        tuya_hal_thread_create(NULL, "diplay_send_thread", 512*4, TRD_PRIO_3, diplay_send_thread, NULL);

    }else {
        //not factory test mode
    }

    return op_ret;
}

STATIC VOID sensor_data_get_thread(PVOID_T pArg)
{   
    while(1) {

        mirror_data_get_handle();

        tuya_hal_system_sleep(TASKDELAY_SEC/2);
    }
}

STATIC VOID diplay_send_thread(PVOID_T pArg)
{     
    while(1) {

        tuya_hal_system_sleep(TASKDELAY_SEC/2);

        mirror_display_poll();
    }
}

STATIC VOID key_scan_thread(PVOID_T pArg)
{   
    while(1) {

        mirror_key_poll();
       
        tuya_hal_system_sleep(25);       
    }
}

STATIC VOID sensor_data_deal_thread(PVOID_T pArg)
{   
    while(1) {
        
        tuya_hal_system_sleep(TASKDELAY_SEC/2);

        if(mirror_ctrl_data.Wifi_state == connecting)
        {
            mirror_wifi_light_handle();
        }else {

            mirror_ctrl_handle();
        }
    }
}


VOID app_report_all_dp_status(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    GW_WIFI_NW_STAT_E wifi_state = 0xFF;

    op_ret = get_wf_gw_nw_status(&wifi_state);
    if (OPRT_OK != op_ret) {
        PR_ERR("get wifi state err");
        return;
    }
    if (wifi_state <= STAT_AP_STA_DISC || wifi_state == STAT_STA_DISC) {
        return;
    }
    
    INT_T dp_cnt = 0;
    dp_cnt = 7;

    if(!mirror_ctrl_data.Wifi_state) {
        return;
    } 

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    dp_arr[0].dpid = DPID_SWITCH;
    dp_arr[0].type = PROP_BOOL;
    dp_arr[0].time_stamp = 0;
    dp_arr[0].value.dp_value = mirror_ctrl_data.Mirror_switch;

    dp_arr[1].dpid = DPID_SWITCH_LED;
    dp_arr[1].type = PROP_BOOL;
    dp_arr[1].time_stamp = 0;
    dp_arr[1].value.dp_value = mirror_ctrl_data.Light_switch;

    dp_arr[2].dpid = DPID_LIGHT_MODE;
    dp_arr[2].type = PROP_ENUM;
    dp_arr[2].time_stamp = 0;
    dp_arr[2].value.dp_value = mirror_ctrl_data.Light_mode;

    dp_arr[3].dpid = DPID_LIGHT_VALUE;
    dp_arr[3].type = PROP_VALUE;
    dp_arr[3].time_stamp = 0;
    dp_arr[3].value.dp_value = mirror_ctrl_data.Light_value;
    
    dp_arr[4].dpid = DPID_BATTERY_STATUS;
    dp_arr[4].type = PROP_ENUM;
    dp_arr[4].time_stamp = 0;
    dp_arr[4].value.dp_value = mirror_ctrl_data.Battery_remain;

    dp_arr[5].dpid = DPID_PIR_MODE;
    dp_arr[5].type = PROP_BOOL;
    dp_arr[5].time_stamp = 0;
    dp_arr[5].value.dp_value = mirror_ctrl_data.PIR_switch;

    dp_arr[6].dpid = DPID_PIR_STATE;
    dp_arr[6].type = PROP_ENUM;
    dp_arr[6].time_stamp = 0;
    dp_arr[6].value.dp_value = mirror_ctrl_data.PIR_state;

    op_ret = dev_report_dp_json_async(NULL,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return;
}


VOID deal_dp_proc(IN CONST TY_OBJ_DP_S *root)
{
    UCHAR_T dpid;

    dpid = root->dpid;
    PR_DEBUG("dpid:%d",dpid);
    
    switch (dpid) {

    case DPID_SWITCH:

        mirror_ctrl_data.Mirror_switch = root->value.dp_bool;
        mirror_ctrl_data.Screen_switch = root->value.dp_bool;
        mirror_ctrl_data.Light_switch = root->value.dp_bool;
        if(mirror_ctrl_data.Mirror_switch == TRUE) {
            screen_bright_set(1);
        }else{
            screen_bright_set(0);
        }
        break;
        
    case DPID_SWITCH_LED:

        if(mirror_ctrl_data.Mirror_switch == TRUE) {
            mirror_ctrl_data.Light_switch = root->value.dp_bool;
        }
        break;

    case DPID_LIGHT_MODE:

        mirror_ctrl_data.Light_mode = root->value.dp_enum;

        if(TRUE == mirror_ctrl_data.Light_switch) {
                
            mirror_pwm_set(mirror_ctrl_data.Light_mode,mirror_ctrl_data.Light_value); 
        }

        break;

    case DPID_LIGHT_VALUE:

        mirror_ctrl_data.Light_value = root->value.dp_value;
        mirror_pwm_set(mirror_ctrl_data.Light_mode,mirror_ctrl_data.Light_value); 
        break;

    case DPID_PIR_MODE:

        mirror_ctrl_data.PIR_switch = root->value.dp_bool;
        if(mirror_ctrl_data.PIR_switch == TRUE) {
            mirror_ctrl_data.PIR_state = first_init;
        }
        break;

    default:
        break;
    }
    
    app_report_all_dp_status();

    return;

}