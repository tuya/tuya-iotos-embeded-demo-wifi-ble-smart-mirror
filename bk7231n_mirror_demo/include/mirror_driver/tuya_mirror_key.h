/*
 * @file name: tuya_mirror_key.h
 * @Descripttion: 
 * @Author: zgw
 * @email: wuls@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-03-03 10:43:50
 * @LastEditors: zgw
 * @LastEditTime: 2021-06-09 17:09:32
 */
#ifndef __TUYA_MIRROR_KEY_H__
#define __TUYA_MIRROR_KEY_H__

#include "uni_log.h"
#include "tuya_cloud_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define KEY_RELEAS_LEVEL                   (1)      //1:high level  0:low level


#if KEY_RELEAS_LEVEL
#define KEY_CODE_RELEASE                   0x0F                     
#define KEY_CODE_SWITCH                    0x07
#define KEY_CODE_SET_LIGHT_COLOR           0x0B
#define KEY_CODE_UP                        0x0D
#define KEY_CODE_DOWN                      0x0E
#else
#define KEY_CODE_RELEASE                   0x00                     
#define KEY_CODE_SWITCH                    0x08
#define KEY_CODE_SET_LIGHT_COLOR           0x04
#define KEY_CODE_UP                        0x02
#define KEY_CODE_DOWN                      0x01
#endif


/**
 * Initialize keys IO port
 * @param  none
 * @return none
 */
void app_key_init(void);

/**
 * Scan key IO level to generate key value
 * @param  trg trigger value,when the key is pressed,the trg will be assigned to the corresponding key value
 *              and the value will only appear once in the entire key trigger cycle
 * @param  cont Count value,when the key is pressed,the cont will be assigned to the corresponding key value
 *              and the value will always appear when the entire key is not released
 * @return none
 */
void app_key_scan(unsigned char *trg,unsigned char *cont);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __APP_KEY_H__ */