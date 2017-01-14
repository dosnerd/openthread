/*
 * program.h
 *
 *  Created on: Dec 15, 2016
 *      Author: ubuntu
 */

#ifndef EXAMPLES_APPS_CLI_COAP_SERVER_H_
#define EXAMPLES_APPS_CLI_COAP_SERVER_H_

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include <openthread.h>
#include <openthread-coap.h>
#include "base.h"

#define OTCOAP_PORT OT_DEFAULT_COAP_PORT

void coapServerStart(otInstance *sInstance);
void coapServerSendResponse(otInstance *sInstance, otCoapHeader *aHeader,
		const otMessageInfo *aMessageInfo, const void *message, uint16_t len);
otCoapResource *coapServerCreateResource(otInstance *sInstance, const char *uri,
		otCoapRequestHandler mHandler, contextInfo *mContextInfo);
void coapServerRemoveResource(otInstance *sInstance, otCoapResource *sCoapResource);

void coapServerTestRequestHandler(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo);
void coapServerEnabledRequest(void *aContext, otCoapHeader *aHeader, otMessage aMessage,
		const otMessageInfo *aMessageInfo);

void coapServerPrintRequest(otCoapHeader *aHeader, const char *aUriPath);

#endif /* EXAMPLES_APPS_CLI_INCLUDES_PROGRAM_H_ */
