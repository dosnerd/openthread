#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include "base.h"
#include "program.h"
#include <openthread-message.h>
#include <openthread.h>
#include "network/coapServer.h"
#include "network/coapClient.h"
#include "uartCostumeHandler.h"

#include <openthread-ip6.h>

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include <hw_gpio.h>
#include <hw_timer2.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

//Begin PWM variable declaration
uint8_t pwmDutyCycle = 0;
uint8_t isIncrementing = 1;
uint8_t stepSize = 1;
uint8_t skipSteps = 100;
uint8_t skipCounter = 0;
//End PWM variable declaration

void setup(otInstance *sInstance) {
	contextInfo *instanceInfo = malloc(sizeof(contextInfo));
	contextInfo *descriptionInfo = malloc(sizeof(contextInfo));

	//fill in some info for some coap responses
	instanceInfo->info = sInstance;
	instanceInfo->next = 0;

	descriptionInfo->info = "A standard, not installed, device with pwm test running";
	descriptionInfo->next = instanceInfo;

	otEnableMulticastPromiscuousMode(sInstance);

	//setup and start openthread
	otSetUp(sInstance, 13, 0xface);
	uartCostumeWritet("ot setup done");

	//start coap server
	coapServerStart(sInstance, "AVANS_DEMOTICA");

	//add resources to coap server
	coapServerCreateResource(sInstance, "enabled", coapServerEnabledRequest, instanceInfo);
	coapServerCreateResource(sInstance, "description", coapServerDescriptionRequest,
			descriptionInfo);
	coapServerCreateResource(sInstance, "button", coapServerButtonRequest, instanceInfo);

	uartCostumeWritef("CoAP server port: %i", OTCOAP_PORT);

	//Initialize timer2 as PWM
	timer2_config cfg;
	hw_timer2_init(&cfg);
	hw_timer2_set_frequency(500);
	hw_timer2_set_pwm_duty_cycle(0, 0);
	hw_timer2_enable();

	//Initialize GPIO pin P4_0 as GPIO using PWM timer 2
	hw_gpio_set_pin_function(HW_GPIO_PORT_4, HW_GPIO_PIN_0, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PWM2);

	//initialize push button
	hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_6, HW_GPIO_MODE_INPUT_PULLUP,
			HW_GPIO_FUNC_GPIO);

	//prevent unsused variable error
	(void) sInstance;
}

uint16_t a = 1;
uint16_t b = 1;
void loop(otInstance *sInstance) {
	//prevent unsused variable error
	(void) sInstance;
	static otIp6Address currentIP = { { { 0 } } };
//	const otIp6Address *address;

//broadcast name when IP change
	if (otGetUnicastAddresses(sInstance)->mValid) {
		if (!otIsIp6AddressEqual(&currentIP, &(otGetUnicastAddresses(sInstance)->mAddress))) {
			currentIP = otGetUnicastAddresses(sInstance)->mAddress;
			broadcast(sInstance, "__UPDATEIP__", coapServerName(0, READ),
					strlen(coapServerName(0, READ)));
		}
	}

	//some dirty code that runs every few secondes
	if (a > 32755) {
		a = 0;
		b++;

		if (b > 5) {
			b = 0;
//			for (const otNetifMulticastAddress *addr = otGetMulticastAddresses(sInstance); addr;
//					addr = addr->mNext) {
//
//				coapClientTransmitNonConfirm(sInstance, addr->mAddress, kCoapRequestPost, "enabled",
//						0, 1, 0);
//			}
		}
	} else {
		a++;
	}

	//Start of DirtyPWM loop
	if (skipCounter >= skipSteps) {
		skipCounter = 0;
		if (isIncrementing) {

			if (pwmDutyCycle <= (255 - stepSize)) {
				pwmDutyCycle += stepSize;
				hw_timer2_set_pwm_duty_cycle(0, pwmDutyCycle);
			} else {
				isIncrementing = 0;
			}
		} else {
			if (pwmDutyCycle >= stepSize) {
				pwmDutyCycle -= stepSize;
				hw_timer2_set_pwm_duty_cycle(0, pwmDutyCycle);
			} else {
				isIncrementing = 1;
			}
		}
	} else {
		skipCounter++;
	}
	//End of DirtyPWM loop
}
