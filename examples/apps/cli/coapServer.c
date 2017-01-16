#define HostSwap16(v)						\
(((v & 0x00ffU) << 8) & 0xff00) |			\
(((v & 0xff00U) >> 8) & 0x00ff)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include <hw_gpio.h>

#include "coapServer.h"
#include <openthread-message.h>
#include "base.h"

#include "uartCostumeHandler.h"

void coapServerSendResponse(otInstance *sInstance, otCoapHeader *aHeader,
		const otMessageInfo *aMessageInfo, const void *message, uint16_t len) {
	otCoapHeader aCoapHeader;
	ThreadError err;

	//create header
	otCoapHeaderInit(&aCoapHeader, kCoapTypeAcknowledgment, kCoapResponseContent);
	otCoapHeaderSetMessageId(&aCoapHeader, otCoapHeaderGetMessageId(aHeader));
	otCoapHeaderSetPayloadMarker(&aCoapHeader);

	//create message
	otMessage returnMessage = otCoapNewMessage(sInstance, &aCoapHeader);
	if (returnMessage == 0) {
		uartCostumeWritet("Can not create response message");
		return;
	}

	//writing response
	otAppendMessage(returnMessage, message, len);

	//send response and clean up
	err = otCoapSendResponse(sInstance, returnMessage, aMessageInfo);
	switch (err) {
	case kThreadError_None:
		break;
	case kThreadError_NoBufs:
		uartCostumeWritet("<RESPONSE>No buff");
		break;
	default:
		uartCostumeWritef("<RESPONSE>Unknown error: %i", err)
		break;
	}
}

void coapServerStart(otInstance *sInstance) {
	contextInfo *info = malloc(sizeof(contextInfo));
	info->info = sInstance;
	info->next = 0;

	//start server
	SucceedOrPrint(otCoapServerSetPort(sInstance, OTCOAP_PORT), "Can not set port");
	SucceedOrPrint(otCoapServerStart(sInstance), "Can not start Coap server");

	//add test resource
	coapServerCreateResource(sInstance, "mytest", coapServerTestRequestHandler, info);
}

otCoapResource *coapServerCreateResource(otInstance *sInstance, const char *uri,
		otCoapRequestHandler mHandler, contextInfo *mContextInfo) {
	otCoapResource *sCoapResource;

	//fill in resource data
	sCoapResource = calloc(1, sizeof(otCoapResource));//resource may not be removed after method, to dynamic allocation
	sCoapResource->mUriPath = uri;
	sCoapResource->mHandler = mHandler;
	sCoapResource->mNext = 0;
	sCoapResource->mContext = mContextInfo;

	//add resource
	if (otCoapServerAddResource(sInstance, sCoapResource) != kThreadError_None) {
		uartCostumeWritet("Resource not created");
		free(sCoapResource);
		return 0;
	} else {
		return sCoapResource;
	}
}

void coapServerRemoveResource(otInstance *sInstance, otCoapResource *sCoapResource) {
	//remove and free resource
	otCoapServerRemoveResource(sInstance, sCoapResource);
	free(sCoapResource);
}

void coapServerTestRequestHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {

	uint16_t length, offset;
	const char *text = "%04x => Response back of: %s";
	char *buf;
	char *buffer;
	contextInfo *sInstanceInfo = aContext;
	otInstance *sInstance = sInstanceInfo->info;

	//inform through uart that a request is received
	coapServerPrintRequest(aHeader, "mytest");

	//read message
	length = otGetMessageLength(aMessage) - otGetMessageOffset(aMessage);
	offset = otGetMessageOffset(aMessage);
	buffer = malloc(sizeof(char) * (length + 1));
	otReadMessage(aMessage, offset, buffer, length);

	//be sure that string end with null character
	buffer[length] = '\0';
	uartCostumeWritef("\tPayload: %s: ", buffer);

	//create response message
	buf = malloc(sizeof(char) * (length + strlen(text) + 1));
	sprintf(buf, text, otGetRloc16(sInstance), buffer);
	free(buffer);

	//send response
	coapServerSendResponse(sInstance, aHeader, aMessageInfo, buf, strlen(buf));
	free(buf);
}

void coapServerEnabledRequest(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {
	coapServerPrintRequest(aHeader, "enabled");

	uint8_t state = otAvansState(2);
	uint16_t offset;
	contextInfo *sInstanceInfo = aContext;
	otInstance *sInstance = sInstanceInfo->info;
	otCoapCode code = otCoapHeaderGetCode(aHeader);

	//set state with put request, return always current/new state
	if (code == kCoapRequestPut) {
		//reading message
		offset = otGetMessageOffset(aMessage);
		otReadMessage(aMessage, offset, &state, 1);

		//saving data
		state &= 1;
		otAvansState(state);
		uartCostumeWritef("New state: %i", state);
	}

	//send response
	coapServerSendResponse(sInstance, aHeader, aMessageInfo, &state, 1);
}

void coapServerDescriptionRequest(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {
	coapServerPrintRequest(aHeader, "description");

	contextInfo *sInfo = aContext;
	const char *description = sInfo->info;
	otInstance *sInstance = sInfo->next->info;
	otCoapCode code = otCoapHeaderGetCode(aHeader);

	//set state with put request, return always current/new state
	if (code == kCoapRequestGet) {
		coapServerSendResponse(sInstance, aHeader, aMessageInfo, description, strlen(description));
	} else {
		const char *m = "This method is not allowed here";
		coapServerSendResponse(sInstance, aHeader, aMessageInfo, m, strlen(m));
	}

	(void) aMessage;
}

void coapServerButtonRequest(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {
	coapServerPrintRequest(aHeader, "button");

	contextInfo *sInfo = aContext;
	otInstance *sInstance = sInfo->info;
	bool status = !hw_gpio_get_pin_status(HW_GPIO_PORT_1, HW_GPIO_PIN_6);//get low active state of push button

	//send response
	coapServerSendResponse(sInstance, aHeader, aMessageInfo, &status, 1);

	(void) aMessage;
}

void coapServerPrintRequest(otCoapHeader *aHeader, const char *aUriPath) {
	otCoapCode coapCode = otCoapHeaderGetCode(aHeader);

	//inform, through uart, the type of request and the uri-path
	switch (coapCode) {
	case kCoapRequestGet:
		uartCostumeWritef("> GET: /%s", aUriPath)
		break;
	case kCoapRequestPost:
		uartCostumeWritef("> POST: /%s", aUriPath)
		break;
	case kCoapRequestDelete:
		uartCostumeWritef("> DELETE: /%s", aUriPath)
		break;
	case kCoapRequestPut:
		uartCostumeWritef("> PUT: /%s", aUriPath)
		break;
	case kCoapCodeEmpty:
		uartCostumeWritef("> EMPTY: /%s", aUriPath)
		break;
	default:
		uartCostumeWritef("***UNKOWN: /%s", aUriPath)
		break;
	}
	(void) aUriPath;
}
