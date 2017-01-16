/*
 * coapClient.c
 *
 *  Created on: Dec 19, 2016
 *      Author: ubuntu
 */
#include <string.h>

#include "coapClient.h"
#include "base.h"
#include "coapServer.h"

#include <stdlib.h>
#include <stdio.h>
#include <openthread-message.h>

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
