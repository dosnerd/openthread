/*
 * base.h
 *
 *  Created on: Dec 15, 2016
 *      Author: ubuntu
 */

#ifndef EXAMPLES_APPS_CLI_BASE_H_
#define EXAMPLES_APPS_CLI_BASE_H_

#include <openthread.h>
#include <platform/logging.h>
#include <cli/cli-uart.h>

#if OPENTHREAD_ENABLE_DEFAULT_LOGGING
#define cliPrint(...)				\
		otPlatLog(0, 0, __VA_ARGS__);
#else
#define cliPrint(...)
#endif

#define STATIC_GET 0
#define SucceedOrPrint(FUNC, ERROR) 	\
		if (FUNC != kThreadError_None)	\
		{								\
			cliPrint(ERROR)				\
		}

//void cliPrint(const char *, ...);
void otSetUp(otInstance *sInstance, uint8_t channel, otPanId panId);
otInstance *otStaticInstance(otInstance *instance);

#endif /* EXAMPLES_APPS_CLI_BASE_H_ */
