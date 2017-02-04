/*
 * coapClient.h
 *
 *  Created on: Dec 19, 2016
 *      Author: ubuntu
 */

#ifndef EXAMPLES_APPS_NCP_CLI_COAPCLIENT_H_
#define EXAMPLES_APPS_NCP_CLI_COAPCLIENT_H_

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include <openthread.h>
#include <openthread-coap.h>

const otIp6Address *coapClientNameTable(char *name, otIp6Address addr, uint8_t action);

void coapClientTransmit(otInstance *sInstance, otIp6Address to, otCoapCode aCode,
		const char *aUriPath, const void *message, uint16_t len, otCoapResponseHandler aHandler);
void coapClientTransmitNonConfirm(otInstance *sInstance, otIp6Address to, otCoapCode aCode,
		const char *aUriPath, const void *message, uint16_t len, otCoapResponseHandler aHandler);
void coapClientTransmitWithName(otInstance *sInstance, const char *name, otCoapCode aCode,
		char *aUriPath, void *message, uint16_t len, otCoapResponseHandler aHandler);
void broadcast(otInstance *sInstance, const char *resource, const void *message, uint16_t len);

void standardResponseHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo, ThreadError aResult);

#endif /* EXAMPLES_APPS_NCP_CLI_COAPCLIENT_H_ */
