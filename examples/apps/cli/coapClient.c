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


void coapClientTransmit(otIp6Address to, otCoapCode aCode, const char *aUriPath, const char *message, otCoapResponseHandler aHandler) {
	int i;
	otCoapOption contentType;
	otIp6Address sock;
	otMessageInfo aMessageInfo;
	ThreadError err;

	//Creating header and fill it partially in
	otCoapHeader aCoapHeader;
	otCoapHeaderInit(&aCoapHeader, kCoapTypeConfirmable, aCode);
	otCoapHeaderSetMessageId(&aCoapHeader, rand());
	SucceedOrPrint(otCoapHeaderAppendUriPathOptions(&aCoapHeader, aUriPath),
			"Can not add Uri Path to option")

	//create content-type option
	contentType.mNumber = kCoapOptionContentFormat;
	contentType.mLength = 2;
	contentType.mValue = 0;

	//adding content-type and set payload marker
	otCoapHeaderAppendOption(&aCoapHeader, &contentType);
	otCoapHeaderSetPayloadMarker(&aCoapHeader);

	//create and write message
	otMessage aMessage = otCoapNewMessage(otStaticInstance(STATIC_GET), &aCoapHeader);
	otAppendMessage(aMessage, message, strlen(message));

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
	//otMessageInfo * const pMessageInfo = &aMessageInfo;

	//sending request and clean up
	err = otCoapSendRequest(otStaticInstance(STATIC_GET), aMessage, &aMessageInfo, aHandler, 0);
	switch (err) {
	case kThreadError_None:
		break;
	case kThreadError_NoBufs:
		cliPrint("No buff")
		break;
	default:
		cliPrint("Unknown error: %i", err)
		break;
	}

	(void) aMessage;
}
