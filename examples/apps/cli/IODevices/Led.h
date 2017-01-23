/*
 * Led.h
 *
 *  Created on: Jan 18, 2017
 *      Author: ubuntu
 */

#ifndef LED_H_
#define LED_H_

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include <hw_gpio.h>

void init(HW_GPIO_PORT Port, HW_GPIO_PIN Pin);

#endif /* LED_H_ */
