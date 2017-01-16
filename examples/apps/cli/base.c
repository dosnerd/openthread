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

void otSetUp(otInstance *sInstance, uint8_t channel, otPanId panId){
        //setup openthread
        otSetChannel(sInstance, channel);
        otSetPanId(sInstance, panId);
        otInterfaceUp(sInstance);

        //start openthread
        otThreadStart(sInstance);
}

uint8_t otAvansState(uint8_t value){
	static uint8_t state = 1;
	if (value < 2) {
		state = value;
	}

	return state;
}
