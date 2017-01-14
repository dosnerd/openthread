/*
 * base.c
 *
 *  Created on: Dec 15, 2016
 *      Author: ubuntu
 */

#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include <openthread.h>
#include "base.h"


/*void cliPrint(const char *aFormat, ...)
{
        //print to cli if enabled
#if OPENTHREAD_ENABLE_DEFAULT_LOGGING
	va_list args;
	va_start(args, aFormat);
	otPlatLog(0, 0, aFormat, args);
	va_end(args);
#else
	(void)aLogLevel;
	(void)aFormat;
#endif // OPENTHREAD_ENABLE_DEFAULT_LOGGING
}*/

void otSetUp(otInstance *sInstance, uint8_t channel, otPanId panId){
        //setup openthread
        otSetChannel(sInstance, channel);
        otSetPanId(sInstance, panId);
        otInterfaceUp(sInstance);

        //start openthread
        otThreadStart(sInstance);
}

/*otInstance *otStaticInstance(otInstance *instance){
	static otInstance *sInstance;
	if (instance != 0){
		sInstance = instance;
	}

	return sInstance;
}*/
