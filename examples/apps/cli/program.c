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

#include <stdlib.h>
#include <stdio.h>

#define HostSwap16(v)						\
(((v & 0x00ffU) << 8) & 0xff00) |			\
(((v & 0xff00U) >> 8) & 0x00ff)

void responseHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo, ThreadError aResult) {

	uint16_t length, offset;
	char *buffer;

	//check status of message
	switch (aResult) {
	case kThreadError_Abort:
		cliPrint("> Response aborted")
		return;
	case kThreadError_None:
		cliPrint("> Response received")
		break;
	case kThreadError_ResponseTimeout:
		cliPrint("> TimeOut for a response")
		return;
	default:
		cliPrint("***Response message: Unknown error: %i", aResult)
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
	cliPrint("\tPayload: \"%s\"", buffer);
	free(buffer);

	(void) aContext;
	(void) aHeader;
	(void) aMessage;
	(void) aMessageInfo;
	(void) aResult;
}



void printList(otInstance *sInstance) {
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

			coapClientTransmit(address, kCoapRequestGet, "mytest", "Koekjes", &responseHandler);
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
			otIp6AddressFromString(sAddress, &address);

			coapClientTransmit(address, kCoapRequestGet, "mytest", "Koekjes", &responseHandler);
			//sendMessage(sInstance, address);
		}
	}
}

uint16_t a = 1;
uint16_t b = 1;

void setup(otInstance *sInstance) {
	//setup and start openthread
	otSetUp(sInstance, 13, 0xface);
	cliPrint("ot setup done");

	coapServerStart(sInstance);

	cliPrint("CoAP server port: %i", OTCOAP_PORT);

	//prevent unsused variable error
	//(void)sInstance;
}

void loop(otInstance *sInstance) {
	//prevent unsused variable error
	(void) sInstance;
	if (a > 32755) {
		a = 0;
		b++;

		if (b > 10) {
			b = 0;

			printList(sInstance);

		}
	} else {
		a++;
	}
}
