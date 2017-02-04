/*
 * coapClient.c
 *
 *  Created on: Dec 19, 2016
 *      Author: ubuntu
 */
#include "network/coapClient.h"

#include <string.h>

#include "base.h"
#include <stdlib.h>
#include <stdio.h>
#include <openthread-message.h>
#include "network/coapServer.h"

const otIp6Address *coapClientNameTable(char *name, otIp6Address addr, uint8_t action) {
	static uint8_t size = 0;
	static char** nameTable = 0;
	static otIp6Address *ipTable = 0;
	uint8_t i;

	switch (action) {
	case READ:
		for (i = 0; i < size; ++i) {
			if (strcmp(nameTable[i], name) == 0) {
				if (memcmp(&ipTable[i], &((otIp6Address ) { { { 0 } } }), sizeof(otIp6Address))) {
					return ipTable + i;
				}
			}
		}
		break;
	case WRITE:
		for (i = 0; i < size; ++i) {
			if (strcmp(nameTable[i], name) == 0) {
				uartCostumeWritet("Updated IP table");
				ipTable[i] = addr;
				return 0;
			}
		}
		break;
	case TRACK:
		if (nameTable) {
			nameTable = realloc(nameTable, sizeof(char *) * ++size);
			ipTable = realloc(ipTable, sizeof(otIp6Address) * size);
		} else {
			nameTable = malloc(sizeof(char *) * ++size);
			ipTable = malloc(sizeof(otIp6Address) * size);
		}

		ipTable[size - 1] = (otIp6Address ) { { { 0 } } };
		nameTable[size - 1] = malloc((strlen(name) + 1) * sizeof(char));
		strcpy(nameTable[size - 1], name);

		break;
	default:
		break;
	}

	return 0;
}

void coapClientTransmit(otInstance *sInstance, otIp6Address to, otCoapCode aCode,
		const char *aUriPath, const void *message, uint16_t len, otCoapResponseHandler aHandler) {
	int i;
	otIp6Address sock;
	otMessageInfo aMessageInfo;
	ThreadError err;

	//Creating header and fill it partially in
	otCoapHeader aCoapHeader;
	otCoapHeaderInit(&aCoapHeader, kCoapTypeConfirmable, aCode);
	otCoapHeaderSetMessageId(&aCoapHeader, rand());
	SucceedOrPrint(otCoapHeaderAppendUriPathOptions(&aCoapHeader, aUriPath),
			"Can not add Uri Path to option")
	otCoapHeaderSetPayloadMarker(&aCoapHeader);

	//create and write message
	otMessage aMessage = otCoapNewMessage(sInstance, &aCoapHeader);
	otAppendMessage(aMessage, message, len);

	//set address to 0, so it will be filled in by a lower layer
	for (i = 0; i < OT_IP6_ADDRESS_SIZE; i++) {
		sock.mFields.m8[i] = 0;
	}

	//create message info (0 that needs to be filled in automatically)
	aMessageInfo.mPeerPort = OTCOAP_PORT;
	aMessageInfo.mPeerAddr = to;
	aMessageInfo.mSockPort = 0;
	aMessageInfo.mSockAddr = sock;
	aMessageInfo.mHopLimit = 10;
	aMessageInfo.mInterfaceId = 0;
	aMessageInfo.mLinkInfo = 0;

	//sending request and clean up
	err = otCoapSendRequest(sInstance, aMessage, &aMessageInfo, aHandler, sInstance);
	switch (err) {
	case kThreadError_None:
		break;
	case kThreadError_NoBufs:
		uartCostumeWritet("<REQUEST>No buff\n");
		break;
	default:
		uartCostumeWritef("<REQUEST>Unknown error: %i", err)
		;
		break;
	}

	(void) aMessage;
}

void standardResponseHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
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

void coapClientTransmitNonConfirm(otInstance *sInstance, otIp6Address to, otCoapCode aCode,
		const char *aUriPath, const void *message, uint16_t len, otCoapResponseHandler aHandler) {
	int i;
	otIp6Address sock;
	otMessageInfo aMessageInfo;
	ThreadError err;

	//Creating header and fill it partially in
	otCoapHeader aCoapHeader;
	otCoapHeaderInit(&aCoapHeader, kCoapTypeNonConfirmable, aCode);
	otCoapHeaderSetMessageId(&aCoapHeader, 0); //id will be set now somewhere else
	if (aHandler != 0) {
		otCoapHeaderGenerateToken(&aCoapHeader, 8);
	}
	SucceedOrPrint(otCoapHeaderAppendUriPathOptions(&aCoapHeader, aUriPath),
			"Can not add Uri Path to option")
	otCoapHeaderSetPayloadMarker(&aCoapHeader);

	//create and write message
	otMessage aMessage = otCoapNewMessage(sInstance, &aCoapHeader);
	otAppendMessage(aMessage, message, len);

	//set address to 0, so it will be filled in by a lower layer
	for (i = 0; i < OT_IP6_ADDRESS_SIZE; i++) {
		sock.mFields.m8[i] = 0;
	}

	//create message info (0 that needs to be filled in automatically)
	aMessageInfo.mPeerPort = OTCOAP_PORT;
	aMessageInfo.mPeerAddr = to;
	aMessageInfo.mSockPort = 0;
	aMessageInfo.mSockAddr = sock;
	aMessageInfo.mHopLimit = 10;
	aMessageInfo.mInterfaceId = 0;
	aMessageInfo.mLinkInfo = 0;

	//sending request and clean up
	err = otCoapSendRequest(sInstance, aMessage, &aMessageInfo, aHandler, 0);
	switch (err) {
	case kThreadError_None:
		break;
	case kThreadError_NoBufs:
		uartCostumeWritet("<REQUEST>No buff\n");
		break;
	default:
		uartCostumeWritef("<REQUEST>Unknown error: %i", err)
		;
		break;
	}

	(void) aMessage;
}

void broadcast(otInstance *sInstance, const char *resource, const void *message, uint16_t len) {
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
			coapClientTransmitNonConfirm(sInstance, address, kCoapRequestPost, resource, message,
					len, 0);
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
			coapClientTransmitNonConfirm(sInstance, address, kCoapRequestPost, resource, message,
					len, 0);
		}
	}
}
