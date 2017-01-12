/*
 * base.h
 *
 *  Created on: Dec 15, 2016
 *      Author: ubuntu
 */

#ifndef EXAMPLES_APPS_CLI_BASE_H_
#define EXAMPLES_APPS_CLI_BASE_H_

#include <openthread.h>
#include "uartCostumeHandler.h"
#include <platform/logging.h>
#include <cli/cli-uart.h>

#define STATIC_GET 0
#define SucceedOrPrint(FUNC, ERROR) 	\
		if (FUNC != kThreadError_None)	\
		{								\
			 uartCostumeWritet(ERROR);	\
		}

//void cliPrint(const char *, ...);
void otSetUp(otInstance *sInstance, uint8_t channel, otPanId panId);
otInstance *otStaticInstance(otInstance *instance);

#endif /* EXAMPLES_APPS_CLI_BASE_H_ */
