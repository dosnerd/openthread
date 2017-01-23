/*
 * RgbLed.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: ubuntu
 */

#include "RgbLed.h"

void rgbInit(HW_GPIO_PORT PortR, HW_GPIO_PIN PinR, HW_GPIO_PORT PortG, HW_GPIO_PIN PinG, HW_GPIO_PORT PortB, HW_GPIO_PIN PinB){
        //Initialize timer2 as PWM
        timer2_config cfg;
        hw_timer2_init(&cfg);
        hw_timer2_set_frequency(500);
        hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM_2, 0);
        hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM_3, 0);
        hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM_4, 0);
        hw_timer2_enable();

        hw_gpio_set_pin_function(PortR, PinR, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PWM2);
        hw_gpio_set_pin_function(PortG, PinG, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PWM3);
        hw_gpio_set_pin_function(PortB, PinB, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PWM4);
}

void setRgb(uint8_t Rvalue, uint8_t Gvalue, uint8_t Bvalue){
        hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM_2, RState(WRITE,Rvalue));
        hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM_3, GState(WRITE,Gvalue));
        hw_timer2_set_pwm_duty_cycle(HW_TIMER2_PWM_4, BState(WRITE,Bvalue));
}

uint8_t RState(uint8_t write, uint8_t value){
        static uint8_t Rvalue = 0;

        if(write){
                Rvalue = value;
        }

        return Rvalue;
}

uint8_t GState(uint8_t write, uint8_t value){
        static uint8_t Gvalue = 0;

        if(write){
                Gvalue = value;
        }

        return Gvalue;
}

uint8_t BState(uint8_t write, uint8_t value){
        static uint8_t Bvalue = 0;

        if(write){
                Bvalue = value;
        }

        return Bvalue;
}
