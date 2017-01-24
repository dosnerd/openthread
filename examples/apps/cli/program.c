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
#include "IODevices/RgbLed.h"
#include <hw_tempsens.h>
#include <hw_gpadc.h>

#include <stdlib.h>
#include <stdio.h>

#include <string.h>

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

void broadcast(otInstance *sInstance, const char *resource, const char *message) {
	otRouterInfo RouterInfo;
	otChildInfo ChildInfo;

	//find and send to all routers
	for (uint8_t i = 0;; i++) {
		if (otGetRouterInfo(sInstance, i, &RouterInfo) != kThreadError_None) {
			break;
		}

		//if router exists
		if (RouterInfo.mAllocated) {
			char sAddress[31];
			otIp6Address address;

			//make Ip6Address
			sprintf(sAddress, "fdde:ad00:beef:0:0:ff:fe00:%04x", RouterInfo.mRloc16);
			SucceedOrPrint(otIp6AddressFromString(sAddress, &address), "Can not parse address");

			//transmit request
			coapClientTransmit(sInstance, address, kCoapRequestGet, resource, message,
					strlen(message), &responseHandler);
		}
	}

	//find and send to all childs
	for (uint8_t i = 0;; i++) {
		if (otGetChildInfoByIndex(sInstance, i, &ChildInfo) != kThreadError_None) {
			return;
		}

		//check if child exists
		if (ChildInfo.mTimeout > 0) {
			char sAddress[31];
			otIp6Address address;

			//make Ip6Address
			sprintf(sAddress, "fdde:ad00:beef:0:0:ff:fe00:%04x", ChildInfo.mRloc16);
			SucceedOrPrint(otIp6AddressFromString(sAddress, &address), "Can not parse address");

			//transmit request
			coapClientTransmit(sInstance, address, kCoapRequestGet, resource, message,
					strlen(message), &responseHandler);
		}
	}
}

void setup(otInstance *sInstance) {
	contextInfo *instanceInfo = malloc(sizeof(contextInfo));
	contextInfo *descriptionInfo = malloc(sizeof(contextInfo));

	//fill in some info for some coap responses
	instanceInfo->info = sInstance;
	instanceInfo->next = 0;

	descriptionInfo->info = "A standard, not installed, device with pwm test running";
	descriptionInfo->next = instanceInfo;

	//setup and start openthread
	otSetUp(sInstance, 13, 0xface);
	uartCostumeWritet("ot setup done");

	//start coap server
	coapServerStart(sInstance);

	//add resources to coap server
	coapServerCreateResource(sInstance, "enabled", coapServerEnabledRequest, instanceInfo);
	coapServerCreateResource(sInstance, "description", coapServerDescriptionRequest,
			descriptionInfo);
	coapServerCreateResource(sInstance, "button", coapServerButtonRequest, instanceInfo);
	coapServerCreateResource(sInstance, "rgbled", coapServerRgbLedRequest, instanceInfo);
	coapServerCreateResource(sInstance, "temperature", coapServerTemperatureRequest, instanceInfo);
	coapServerCreateResource(sInstance, "sensorread", coapServerSensorReadRequest, instanceInfo);

	uartCostumeWritef("CoAP server port: %i", OTCOAP_PORT);

	//initialize general purpose ADC
	hw_gpadc_init(NULL);
	hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE_SINGLE_ENDED);
	hw_gpadc_enable();

	//initialize pin P0_6 as analog input
	hw_gpio_set_pin_function(HW_GPIO_PORT_0, HW_GPIO_PIN_7, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_ADC);

	//initialize temperature sensor
	hw_tempsens_enable();

	//initialize RGB led on P3_0..P3_2
	rgbInit(HW_GPIO_PORT_3, HW_GPIO_PIN_0, HW_GPIO_PORT_3, HW_GPIO_PIN_1, HW_GPIO_PORT_3, HW_GPIO_PIN_2);

	//initialize push button
	hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_6, HW_GPIO_MODE_INPUT_PULLUP,
			HW_GPIO_FUNC_GPIO);


	//prevent unsused variable error
	(void)sInstance;
}

uint16_t a = 1;
uint16_t b = 1;
void loop(otInstance *sInstance) {
	//prevent unsused variable error
	(void) sInstance;
	otIp6Address address;
	static uint8_t state = 1;

	//some dirty code that runs every few secondes
	if (a > 32755) {
		a = 0;
		b++;

		if (b > 1) {
			b = 0;

			//check if push button pressed
			if (!hw_gpio_get_pin_status(HW_GPIO_PORT_1, HW_GPIO_PIN_6)) {

				//try to convert address
				if (otIp6AddressFromString(uartCostumeGetInputBuffer(), &address)
						!= kThreadError_None) {
					uartCostumeWritet("\nInvalid address given");
					return;
				}

				//transmit request for disable/enable device
				coapClientTransmit(sInstance, address, kCoapRequestPut, "enabled", &state,
						1, responseHandler);
				state = !state;
				uartCostumeWritet("Request transmitted");
			}

		}
	} else {
		a++;
	}
}
