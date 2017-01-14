#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include "base.h"
#include "program.h"
#include <openthread-message.h>
#include <openthread.h>
#include "coapServer.h"
#include "coapClient.h"
#include "uartCostumeHandler.h"

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include <hw_gpio.h>
#include <hw_timer2.h>

#include <stdlib.h>
#include <stdio.h>

#define HostSwap16(v)						\
(((v & 0x00ffU) << 8) & 0xff00) |			\
(((v & 0xff00U) >> 8) & 0x00ff)

//Begin PWM variable declaration
uint8_t pwmDutyCycle = 0;
uint8_t isIncrementing = 1;
uint8_t stepSize = 1;
uint8_t skipSteps = 100;
uint8_t skipCounter = 0;
//End PWM variable declaration

void responseHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo, ThreadError aResult) {

	uint16_t length, offset;
	char *buffer;

	//check status of message
	switch (aResult) {
	case kThreadError_Abort:
		uartCostumeWritet("> Response aborted");
		return;
	case kThreadError_None:
		uartCostumeWritet("> Response received");
		break;
	case kThreadError_ResponseTimeout:
		uartCostumeWritet("> TimeOut for a response");
		return;
	default:
		uartCostumeWritef("***Response message: Unknown error: %i", aResult)
		return;
	}

	//get location message
	length = otGetMessageLength(aMessage) - otGetMessageOffset(aMessage);
	offset = otGetMessageOffset(aMessage);

	//read message
	buffer = malloc(sizeof(char) * (length + 1));
	otReadMessage(aMessage, offset, buffer, length);

	//Add null at end of string, just to be sure
	buffer[length] = '\0';
	uartCostumeWritef("\tPayload: \"%s\"", buffer);
	free(buffer);

	(void) aContext;
	(void) aHeader;
	(void) aMessage;
	(void) aMessageInfo;
	(void) aResult;
}

void printList(otInstance *sInstance, const char *resource, const char *message) {
	otRouterInfo RouterInfo;
	otChildInfo ChildInfo;

	for (uint8_t i = 0;; i++) {
		if (otGetRouterInfo(sInstance, i, &RouterInfo) != kThreadError_None) {
			break;
		}

		if (RouterInfo.mAllocated) {
			char sAddress[31];
			otIp6Address address;

			//make Ip6Address
			sprintf(sAddress, "fdde:ad00:beef:0:0:ff:fe00:%04x", RouterInfo.mRloc16);
			SucceedOrPrint(otIp6AddressFromString(sAddress, &address), "Can not parse address");

			coapClientTransmit(sInstance, address, kCoapRequestGet, resource, message,
					&responseHandler);
			//sendMessage(sInstance, address);
		}
	}

	for (uint8_t i = 0;; i++) {
		if (otGetChildInfoByIndex(sInstance, i, &ChildInfo) != kThreadError_None) {
			return;
		}

		if (ChildInfo.mTimeout > 0) {
			char sAddress[31];
			otIp6Address address;

			//make Ip6Address
			sprintf(sAddress, "fdde:ad00:beef:0:0:ff:fe00:%04x", ChildInfo.mRloc16);
			SucceedOrPrint(otIp6AddressFromString(sAddress, &address), "Can not parse address");

			coapClientTransmit(sInstance, address, kCoapRequestGet, resource, message,
					&responseHandler);
			//sendMessage(sInstance, address);
		}
	}
}

uint16_t a = 1;
uint16_t b = 1;

void setup(otInstance *sInstance) {
	contextInfo *instanceInfo = malloc(sizeof(contextInfo));
	//contextInfo *descriptionInfo = malloc(sizeof(contextInfo));

	instanceInfo->info = sInstance;
	instanceInfo->next = 0;

	//descriptionInfo->info = "A standard, not installed, device with pwm test running";
	//descriptionInfo->next = instanceInfo;

	//setup and start openthread
	otSetUp(sInstance, 13, 0xface);
	uartCostumeWritet("ot setup done");

	coapServerStart(sInstance);
	coapServerCreateResource(sInstance, "enabled", coapServerEnabledRequest, instanceInfo);
	//coapServerCreateResource(sInstance, "enabled", coapServerEnabledRequest, instanceInfo);

	uartCostumeWritef("CoAP server port: %i", OTCOAP_PORT);

	//Initialize timer2 as PWM
	timer2_config cfg;
	hw_timer2_init(&cfg);
	hw_timer2_set_frequency(500);
	hw_timer2_set_pwm_duty_cycle(0, 0);
	hw_timer2_enable();

	//Initialize GPIO pin P4_0 as GPIO using PWM timer 2
	hw_gpio_set_pin_function(HW_GPIO_PORT_4, HW_GPIO_PIN_0, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PWM2);

	//prevent unsused variable error
	//(void)sInstance;
}

void loop(otInstance *sInstance) {
	//prevent unsused variable error
	(void) sInstance;

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
