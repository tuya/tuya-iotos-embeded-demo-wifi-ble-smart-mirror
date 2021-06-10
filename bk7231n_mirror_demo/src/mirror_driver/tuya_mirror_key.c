/*
 * @file name: tuya_mirror_key.c
 * @Descripttion: 
 * @Author: zgw
 * @email: wuls@tuya.com
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-03-03 10:44:56
 * @LastEditors: zgw
 * @LastEditTime: 2021-06-09 17:06:12
 */
#include "tuya_mirror_key.h"
#include "tuya_gpio.h"
#include "tuya_key.h"

#define KEY_SWITCH_PIN            TY_GPIOA_22 
#define KEY_SET_PIN               TY_GPIOA_20
#define KEY_DOWN_PIN              TY_GPIOA_16
#define KEY_UP_PIN                TY_GPIOA_14

/**
 * Initialize keys IO port
 * @param  none
 * @return none
 */
void app_key_init(void)
{
    tuya_gpio_inout_set(KEY_SWITCH_PIN, TRUE);
    tuya_gpio_inout_set(KEY_SET_PIN, TRUE);
    tuya_gpio_inout_set(KEY_UP_PIN, TRUE);
    tuya_gpio_inout_set(KEY_DOWN_PIN, TRUE);
}

/**
 * Scan key IO level to generate key value
 * @param  trg trigger value,when the key is pressed,the trg will be assigned to the corresponding key value
 *              and the value will only appear once in the entire key trigger cycle
 * @param  cont Count value,when the key is pressed,the cont will be assigned to the corresponding key value
 *              and the value will always appear when the entire key is not released
 * @return none
 */
void app_key_scan(unsigned char *trg,unsigned char *cont)
{
    unsigned char read_data;
    
    if(KEY_RELEAS_LEVEL) {
        read_data = 0x0F;
    }else {
        read_data = 0x00; 
    }

    read_data = (tuya_gpio_read(KEY_SWITCH_PIN)<<3)|(tuya_gpio_read(KEY_SET_PIN)<<2)|(tuya_gpio_read(KEY_UP_PIN)<<1)|(tuya_gpio_read(KEY_DOWN_PIN));
    *trg = (read_data & (read_data ^ (*cont)));
    *cont = read_data;
}
