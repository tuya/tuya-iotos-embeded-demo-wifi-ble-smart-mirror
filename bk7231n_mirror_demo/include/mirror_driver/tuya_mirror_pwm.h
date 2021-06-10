/*
 * @file name: tuya_mirror_pwm.h
 * @Descripttion: 
 * @Author: zgw
 * @email: liang.zhang@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-03-03 17:19:36
 * @LastEditors: zgw
 * @LastEditTime: 2021-06-09 19:20:56
 */

#ifndef __TUYA_MIRROR_PWM_H__
#define __TUYA_MIRROR_PWM_H__

#include "uni_log.h"
#include "tuya_cloud_error_code.h"


/**
 * @brief pwm init data structure
 * pwm send data structure
 */
typedef struct 
{
    USHORT_T usFreq;            ///< PWM Frequency
    USHORT_T usDuty;            ///< PWM Init duty
    UCHAR_T ucList[2];          ///< GPIO List 
    UCHAR_T ucChannelNum;       ///< GPIO List length
    BOOL_T bPolarity;           ///< PWM output polarity
    BOOL_T bCCTFlag;           ///< CCT drive mode flag
}USER_PWM_INIT_T;

typedef struct 
{
    USHORT_T duty_cold;         ///< R value,rang from 0 to 1000
    USHORT_T duty_warm;        ///< W value,rang from 0 to 1000
}USER_PWM_DUTY_T;

/**
 * Initialize the two-way PWM used to drive the light board
 * @param  none
 * @return Function Operation Result  OPRT_OK is ok other is fail 
 */
OPERATE_RET mirror_pwm_init(VOID);

/**
 * Set the output channel and duty cycle of PWM for the color and brightness of the lamp
 * @param  color The color of the light
 * @param  duty Duty cycle 0~1000
 * @return Function Operation Result  OPRT_OK is ok other is fail 
 */
OPERATE_RET mirror_pwm_set(UCHAR_T color, USHORT_T duty);

/**
 * Turn off the pwm output of all channels
 * @param  none
 * @return Function Operation Result  OPRT_OK is ok other is fail 
 */
OPERATE_RET mirror_pwm_off(VOID);

#endif