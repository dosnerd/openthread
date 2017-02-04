/*
 * NRP.h
 *
 *  Created on: 19 jan. 2017
 *      Author: Acer
 */

#ifndef EXAMPLES_APPS_CLI_NETWORK_NRP_H_
#define EXAMPLES_APPS_CLI_NETWORK_NRP_H_

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include "base.h"
#include <openthread.h>
#include <openthread-coap.h>

typedef struct coapRequest {
	otInstance *sInstance;
	const char *name;
	otCoapCode aCode;
	const char *aUriPath;
	const void *message;
	uint16_t len;
	otCoapResponseHandler aHandler;
};

typedef struct NRPrequest {
	coapRequest requestData;
	const char serverName;
};

otIp6Address nrpNrpList(const char* name, otIp6Address address, uint8_t mode);
void nrpSendOverCoap(coapRequest requestData, const char *serverName);
void nrpBroadcastRequest();

#endif /* EXAMPLES_APPS_CLI_NETWORK_NRP_H_ */
