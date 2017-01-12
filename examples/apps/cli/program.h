/*
 * program.h
 *
 *  Created on: Dec 15, 2016
 *      Author: ubuntu
 */

#ifndef EXAMPLES_APPS_CLI_INCLUDES_PROGRAM_H_
#define EXAMPLES_APPS_CLI_INCLUDES_PROGRAM_H_

#include <openthread.h>

//temp
void printList(otInstance *sInstance, const char *resource, const char *message);

void setup(otInstance *);
void loop(otInstance *);


#endif /* EXAMPLES_APPS_CLI_INCLUDES_PROGRAM_H_ */
