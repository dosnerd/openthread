/*
 * RgbLed.h
 *
 *  Created on: Jan 19, 2017
 *      Author: ubuntu
 */

#ifndef RGBLED_H_
#define RGBLED_H_

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include <hw_gpio.h>
#include <hw_timer2.h>


#define READ 0,0
#define WRITE 1

void rgbInit(HW_GPIO_PORT PortR, HW_GPIO_PIN PinR, HW_GPIO_PORT PortG, HW_GPIO_PIN PinG, HW_GPIO_PORT PortB, HW_GPIO_PIN PinB);
void setRgb(uint8_t Rvalue, uint8_t Gvalue, uint8_t Bvalue);
uint8_t RState(uint8_t write, uint8_t value);
uint8_t GState(uint8_t write, uint8_t value);
uint8_t BState(uint8_t write, uint8_t value);

#endif /* RGBLED_H_ */
