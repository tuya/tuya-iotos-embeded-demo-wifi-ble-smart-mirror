/*
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @LastEditors: zgw
 * @file name: tuya_app.h
 * @Description: light init process include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2020-12-21 11:30:03
 * @LastEditTime: 2021-05-27 11:41:39
 */

#ifndef __TUYA_APP_H__
#define __TUYA_APP_H__

#include "uni_log.h"
#include "tuya_cloud_error_code.h"
#include "tuya_cloud_com_defs.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/***********************************************************
*************************variable define********************
***********************************************************/
typedef enum{
    APP_MIRROR_NORMAL,       //normal mode
    APP_MIRROR_PRODTEST      //prodact test mode
}APP_MIRROR_MODE;

#define DPID_SWITCH                      1
#define DPID_SWITCH_LED                  2
#define DPID_LIGHT_MODE                  4
#define DPID_LIGHT_VALUE                 5
#define DPID_BATTERY_STATUS              101
#define DPID_PIR_MODE                    103
#define DPID_PIR_STATE                   105
/********************************************************************************
 * FUNCTION:       app_mirror_init
 * DESCRIPTION:    application initialization
 * INPUT:          mode:application mode
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
OPERATE_RET app_mirror_init(IN APP_MIRROR_MODE mode);

/********************************************************************************
 * FUNCTION:       deal_dp_proc
 * DESCRIPTION:    deal the data sented by app
 * INPUT:          root:app issued data structure
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         none
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID deal_dp_proc(IN CONST TY_OBJ_DP_S *root);

/*********************************************************************************
 * FUNCTION:       app_report_all_dp_status
 * DESCRIPTION:    report all dp date
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         dp_cnt needs to be modified when adding or deleting the dp function
 * HISTORY:        2021-01-12
 *******************************************************************************/
VOID app_report_all_dp_status(VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __TUYA_APP_H__ */