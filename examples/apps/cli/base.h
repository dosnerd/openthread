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

#define SucceedOrPrint(FUNC, ERROR) 	\
		if (FUNC != kThreadError_None)	\
		{								\
			 uartCostumeWritet(ERROR);	\
		}

typedef struct contextInfo {
	void *info;
	struct contextInfo *next;
} contextInfo;

void otSetUp(otInstance *sInstance, uint8_t channel, otPanId panId);
uint8_t otAvansState(uint8_t state);

#endif /* EXAMPLES_APPS_CLI_BASE_H_ */
