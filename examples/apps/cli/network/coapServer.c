#define HostSwap16(v)						\
(((v & 0x00ffU) << 8) & 0xff00) |			\
(((v & 0xff00U) >> 8) & 0x00ff)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include <hw_gpio.h>

#include "network/coapServer.h"
#include "network/coapClient.h"
#include <openthread-message.h>
#include "base.h"

#include "uartCostumeHandler.h"

void coapServerSendResponse(otInstance *sInstance, otCoapHeader *aHeader,
		const otMessageInfo *aMessageInfo, const void *message, uint16_t len) {
	otCoapHeader aCoapHeader;
	ThreadError err;
	uint8_t tokenLenght = otCoapHeaderGetTokenLength(aHeader);
	const uint8_t *aToken = tokenLenght > 0 ? otCoapHeaderGetToken(aHeader) : 0;

	//create header
	if (otCoapHeaderGetType(aHeader) == kCoapTypeNonConfirmable) {
		if (tokenLenght == 0) {
			uartCostumeWritet("! No token for response message");
			return;
		}

		otCoapHeaderInit(&aCoapHeader, kCoapTypeNonConfirmable, kCoapResponseContent);
		otCoapHeaderSetMessageId(&aCoapHeader, rand());
		otCoapHeaderSetToken(&aCoapHeader, aToken, tokenLenght);
	} else {
		otCoapHeaderInit(&aCoapHeader, kCoapTypeAcknowledgment, kCoapResponseContent);
		otCoapHeaderSetMessageId(&aCoapHeader, otCoapHeaderGetMessageId(aHeader));
	}
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

char *coapServerName(char *name, uint8_t action) {
	static char *serverName = 0;

	if (action == WRITE) {
		if (serverName) {
			free(serverName);
		}

		serverName = malloc((strlen(name) + 1) * sizeof(char));
		strcpy(serverName, name);
	}

	return serverName;
}

void coapServerStart(otInstance *sInstance, char *name) {
	contextInfo *info = malloc(sizeof(contextInfo));
	coapServerName(name, WRITE);

	info->info = sInstance;
	info->next = 0;

	//start server
	SucceedOrPrint(otCoapServerSetPort(sInstance, OTCOAP_PORT), "Can not set port");
	SucceedOrPrint(otCoapServerStart(sInstance), "Can not start Coap server");

	//add test resource
	coapServerCreateResource(sInstance, "__RESPONSEIP__", coapServerResponseNameHandler, info);
	coapServerCreateResource(sInstance, "__UPDATEIP__", coapServerNameRequest, info);
}

otCoapResource *coapServerCreateResource(otInstance *sInstance, const char *uri,
		otCoapRequestHandler mHandler, contextInfo *mContextInfo) {
	otCoapResource *sCoapResource;

	//fill in resource data
	sCoapResource = calloc(1, sizeof(otCoapResource)); //resource may not be removed after method, to dynamic allocation
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

void coapServerResponseNameHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {

	coapServerPrintRequest(aHeader, "::Name Response::");

	char *buffer;
	otCoapCode code = otCoapHeaderGetCode(aHeader);
	uint16_t offset, length;

	if (code == kCoapRequestPost) {
		//get name from message
		offset = otGetMessageOffset(aMessage);
		length = otGetMessageLength(aMessage) - offset;
		buffer = malloc(sizeof(char) * (length + 1));

		otReadMessage(aMessage, offset, buffer, length);
		buffer[length] = 0;	//be sure that string ends with 0

		//info user though uart
		OTAPI const otIp6Address *tmp = &(aMessageInfo->mPeerAddr);
		uartCostumeWritef("New device registered/changed: %s: %x:%x:%x:%x:%x:%x:%x:%x\r\n", buffer,
				HostSwap16(tmp->mFields.m16[0]), HostSwap16(tmp->mFields.m16[1]),
				HostSwap16(tmp->mFields.m16[2]), HostSwap16(tmp->mFields.m16[3]),
				HostSwap16(tmp->mFields.m16[4]), HostSwap16(tmp->mFields.m16[5]),
				HostSwap16(tmp->mFields.m16[6]), HostSwap16(tmp->mFields.m16[7]));

		//adding to name table
		coapClientNameTable(buffer, aMessageInfo->mPeerAddr, WRITE);

		free(buffer);
	}

	(void) aMessageInfo;
	(void) aContext;
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

void coapServerNameRequest(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {
	coapServerPrintRequest(aHeader, "::Name Request::");

	contextInfo *sInfo = aContext;
	otInstance *sInstance = sInfo->info;
	char *buffer;
	otCoapCode code = otCoapHeaderGetCode(aHeader);
	uint16_t offset, length;

	if (code == kCoapRequestPost) {
		//get name from message
		offset = otGetMessageOffset(aMessage);
		length = otGetMessageLength(aMessage) - offset;
		buffer = malloc(sizeof(char) * (length + 1));

		otReadMessage(aMessage, offset, buffer, length);
		buffer[length] = 0;	//be sure that string ends with 0

		//info user though uart
		OTAPI const otIp6Address *tmp = &(aMessageInfo->mPeerAddr);
		uartCostumeWritef("New device registered/changed: %s: %x:%x:%x:%x:%x:%x:%x:%x\r\n", buffer,
				HostSwap16(tmp->mFields.m16[0]), HostSwap16(tmp->mFields.m16[1]),
				HostSwap16(tmp->mFields.m16[2]), HostSwap16(tmp->mFields.m16[3]),
				HostSwap16(tmp->mFields.m16[4]), HostSwap16(tmp->mFields.m16[5]),
				HostSwap16(tmp->mFields.m16[6]), HostSwap16(tmp->mFields.m16[7]));

		//adding to name table
		coapClientNameTable(buffer, aMessageInfo->mPeerAddr, WRITE);

		free(buffer);

		coapClientTransmitNonConfirm(sInstance, aMessageInfo->mPeerAddr, kCoapRequestPost,
				"__RESPONSEIP__", coapServerName(0, READ), strlen(coapServerName(0, READ)), 0);
	}

	(void) aMessageInfo;
	(void) aContext;
}

void coapServerButtonRequest(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo) {
	coapServerPrintRequest(aHeader, "button");

	contextInfo *sInfo = aContext;
	otInstance *sInstance = sInfo->info;
	bool status = !hw_gpio_get_pin_status(HW_GPIO_PORT_1, HW_GPIO_PIN_6); //get low active state of push button

	//send response
	coapServerSendResponse(sInstance, aHeader, aMessageInfo, &status, 1);

	(void) aMessage;
}

void coapServerPrintRequest(otCoapHeader *aHeader, const char *aUriPath) {
	otCoapCode coapCode = otCoapHeaderGetCode(aHeader);
	uint8_t token = 0;
	if (otCoapHeaderGetTokenLength(aHeader) > 0) {
		token = *otCoapHeaderGetToken(aHeader);
	}

	//inform, through uart, the type of request and the uri-path
	switch (coapCode) {
	case kCoapRequestGet:
		uartCostumeWritef("> GET: /%s, token: %i", aUriPath, token)
		break;
	case kCoapRequestPost:
		uartCostumeWritef("> POST: /%s, token: %i", aUriPath, token)
		;
		break;
	case kCoapRequestDelete:
		uartCostumeWritef("> DELETE: /%s, token: %i", aUriPath, token)
		break;
	case kCoapRequestPut:
		uartCostumeWritef("> PUT: /%s, token: %i", aUriPath, token)
		break;
	case kCoapCodeEmpty:
		uartCostumeWritef("> EMPTY: /%s, token: %i", aUriPath, token)
		break;
	default:
		uartCostumeWritef("***UNKOWN: /%s, token: %i", aUriPath, token)
		break;
	}
	(void) aUriPath;
}
