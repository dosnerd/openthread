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

void coapClientTransmit(otIp6Address, otCoapCode, const char *, const char *, otCoapResponseHandler);

#endif /* EXAMPLES_APPS_NCP_CLI_COAPCLIENT_H_ */
