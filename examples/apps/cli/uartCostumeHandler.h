/*
 * uartCostumeHandler.h
 *
 *  Created on: 12 jan. 2017
 *      Author: Acer
 */

#ifndef EXAMPLES_APPS_CLI_UARTCOSTUMEHANDLER_H_
#define EXAMPLES_APPS_CLI_UARTCOSTUMEHANDLER_H_

#include <openthread.h>
#include <openthread-coap.h>
#include <stdlib.h>
#include <stdio.h>

#define UART_INPUT_BUFFER 512


#if OPENTHREAD_ENABLE_DEFAULT_LOGGING
#define uartCostumeWritef(...)	\
		otPlatLog(0, 0, __VA_ARGS__);
#else
#define uartCostumeWritef(...)
#endif

char *uartCostumeGetInputBuffer();
uint16_t *uartCostumeGetInputBufferLength();
void uartCostumeProcess(otInstance *sInstance);
void uartCostumeWrite(const char *buf, uint16_t len);
void uartCostumeWritet(const char *text);

void ProcessBroadcast(int argc, char *argv[], otInstance *sInstance);
void ProcessEcho(int argc, char *argv[], otInstance *sInstance);

void ProcessSend(int argc, char *argv[], otInstance *sInstance, otCoapCode code);
void ProcessGET(int argc, char *argv[], otInstance *sInstance);
void ProcessPOST(int argc, char *argv[], otInstance *sInstance);
void ProcessPUT(int argc, char *argv[], otInstance *sInstance);
void ProcessDELETE(int argc, char *argv[], otInstance *sInstance);

#endif /* EXAMPLES_APPS_CLI_UARTCOSTUMEHANDLER_H_ */