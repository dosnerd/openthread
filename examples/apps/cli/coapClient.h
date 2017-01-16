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

void coapClientTransmit(otInstance *sInstance, otIp6Address to, otCoapCode aCode,
		const char *aUriPath, const void *message, uint16_t len, otCoapResponseHandler aHandler);

#endif /* EXAMPLES_APPS_NCP_CLI_COAPCLIENT_H_ */
