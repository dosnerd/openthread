#define HostSwap16(v)						\
(((v & 0x00ffU) << 8) & 0xff00) |			\
(((v & 0xff00U) >> 8) & 0x00ff)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coapServer.h"
#include "base.h"
#include <openthread-message.h>

void coapServerTestRequestHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {

	uint16_t length, offset;
	char *buffer;
	//const char *buf = "Response back of: ";		//response message
	const char *text = "%04x => Response back of: ";
	char buf[strlen(text)];
	sprintf(buf, text, otGetRloc16(otStaticInstance(STATIC_GET)));//response message
	ThreadError err;

	coapServerPrintRequest(aHeader, "mytest");

	//print message
	length = otGetMessageLength(aMessage) - otGetMessageOffset(aMessage);
	offset = otGetMessageOffset(aMessage);

	buffer = malloc(sizeof(char) * (length + 1));
	otReadMessage(aMessage, offset, buffer, length);

	//be sure that string end with null character
	buffer[length] = '\0';
	uartCostumeWritef("\tPayload: %s: ", buffer);

	//create header
	otCoapHeader aCoapHeader;
	otCoapHeaderInit(&aCoapHeader, kCoapTypeAcknowledgment, kCoapResponseContent);
	otCoapHeaderSetMessageId(&aCoapHeader, otCoapHeaderGetMessageId(aHeader));
	otCoapHeaderSetPayloadMarker(&aCoapHeader);

	//create message
	otMessage returnMessage = otCoapNewMessage(otStaticInstance(STATIC_GET), &aCoapHeader);
	if (returnMessage == 0) {
		uartCostumeWritet("Can not create response message");
		otFreeMessage(aMessage);
		return;
	}

	//writing response
	otAppendMessage(returnMessage, buf, strlen(buf));
	otAppendMessage(returnMessage, buffer, strlen(buffer));
	free(buffer);

	//send response and clean up
	err = otCoapSendResponse(otStaticInstance(STATIC_GET), returnMessage, aMessageInfo);
	switch (err) {
	case kThreadError_None:
		break;
	case kThreadError_NoBufs:
		uartCostumeWritet("<RESPONSE>No buff");
		break;
	default:
		uartCostumeWritef("<RESPONSE>Unknown error: %i", err);
		break;
	}
	(void) aContext;
}

void coapServerStart(otInstance *sInstance) {
	otCoapResource *sCoapResource;
	otCoapRequestHandler mHandler;

	//start server
	SucceedOrPrint(otCoapServerSetPort(sInstance, OTCOAP_PORT), "Can not set port");
	SucceedOrPrint(otCoapServerStart(sInstance), "Can not start Coap server");

	//creating test resource
	sCoapResource = calloc(1, sizeof(otCoapResource));
	mHandler = coapServerTestRequestHandler;
	sCoapResource->mUriPath = "mytest";
	sCoapResource->mHandler = mHandler;
	sCoapResource->mNext = 0;
	sCoapResource->mContext = sInstance;

	(void) sCoapResource;
	(void) sInstance;
	//adding test resource
	SucceedOrPrint(otCoapServerAddResource(sInstance, sCoapResource), "Can not add test resource");
}

void coapServerPrintRequest(otCoapHeader *aHeader, const char *aUriPath) {
	otCoapCode coapCode = otCoapHeaderGetCode(aHeader);

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
	(void)aUriPath;
}
