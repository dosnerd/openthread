/*
 * uartCostumeHandler.c
 *
 *  Created on: 12 jan. 2017
 *      Author: Acer
 */
#ifdef OPENTHREAD_CONFIG_FILE
#include OPENTHREAD_CONFIG_FILE
#else
#include <openthread-config.h>
#endif

#include <bsp_defaults.h>
#include <bsp_definitions.h>
#include "uartCostumeHandler.h"
#include <hw_uart.h>
#include <string.h>

#include <platform/uart.h>

//temp
#include "program.h"
#include "base.h"
#include "coapClient.h"

#define MAX_ARGS 32

//based of cli.cpp
struct Command {
	const char *mName;                         ///< A pointer to the command string.
	void (*mCommand)(int argc, char *argv[], otInstance *sInstance);  ///< A function pointer to process the command.
};

//const struct Command sCommands[];
const struct Command sCommands[] = {
		{ "echo", &ProcessEcho },
		{ "broadcast", &ProcessBroadcast },
		{"GET", &ProcessGET },
		{"POST", &ProcessPOST },
		{"PUT", &ProcessPUT },
		{"DELETE", &ProcessDELETE },
};

char *uartCostumeGetInputBuffer() {
	static char buffer[UART_INPUT_BUFFER];
	return buffer;
}
uint16_t *uartCostumeGetInputBufferLength() {
	static uint16_t length = 0;
	return &length;
}

//based of cli.cpp
void uartCostumeProcessLine(char *buffer, uint16_t bufferLen, otInstance *sInstance) {
	char *argv[MAX_ARGS];
	int argc = 0;
	char *cmd;

	for (; *buffer == ' '; buffer++, bufferLen--)
		;

	for (cmd = buffer + 1; cmd < buffer + bufferLen; ++cmd) {
		if (argc >= MAX_ARGS) {
			uartCostumeWritef("Error: too many args (max %d)\r\n", MAX_ARGS);
		}

		if (*cmd == ' ' || *cmd == '\r' || *cmd == '\n') {
			*cmd = '\0';
		}

		if (*(cmd - 1) == '\0' && *cmd != ' ') {
			argv[argc++] = cmd;
		}
	}

	cmd = buffer;

	for (unsigned int i = 0; i < sizeof(sCommands) / sizeof(sCommands[0]); i++) {
		if (strcmp(cmd, sCommands[i].mName) == 0) {
			(*sCommands[i].mCommand)(argc, argv, sInstance);
			uartCostumeWritet("Done");
			return;
		}
	}
}

//based on cli_uart.cpp
void uartCostumeProcess(otInstance *sInstance) {
	char aBuf;
	char *buffer = uartCostumeGetInputBuffer();
	uint16_t *length = uartCostumeGetInputBufferLength();

	// Wait until received data are available
	if (!hw_uart_read_buf_empty(HW_UART1)) {
		// Read element from the receive FIFO
		aBuf = UBA(HW_UART1)->UART2_RBR_THR_DLL_REG;
		otPlatUartReceived((uint8_t *) &aBuf, 1);

		switch (aBuf) {
		case '\n':
		case '\r':
			if (*length > 0) {
				buffer[*length] = '\0';
				uartCostumeProcessLine(buffer, *length, sInstance);
				*length = 0;
			}
			break;
		case '\b':
		case 127:
			//remove character
			if (*length > 0) {
				buffer[(*length)--] = aBuf;
			}
			break;
		default:
			//add character to buffer if possible
			if (*length < UART_INPUT_BUFFER) {
				buffer[(*length)++] = aBuf;
			} else {
				*length = 0;
			}
			break;
		}
	}
}

void uartCostumeWrite(const char *buf, uint16_t len) {
	(void) buf;
	(void) len;
	hw_uart_write_buffer(HW_UART1, buf, len);
}

void uartCostumeWritet(const char *text) {
	(void) text;
	hw_uart_write_buffer(HW_UART1, text, strlen(text));
	hw_uart_write_buffer(HW_UART1, "\r\n", 2);
}

void ProcessEcho(int argc, char *argv[], otInstance *sInstance) {
	uartCostumeWritet("ECHO: ");
	for (int i = 0; i < argc; ++i) {
		uartCostumeWritet(argv[i]);
	}
	(void)sInstance;
}

void ProcessBroadcast(int argc, char *argv[], otInstance *sInstance) {
	if (argc > 1) {
		char *buffer = malloc(strlen(argv[1]));
		strcpy(buffer, argv[1]);
		for (int i = 2; i < argc; ++i) {
			buffer = realloc(buffer, strlen(buffer) + strlen(argv[i]) + 1);

			strcat(buffer, " \0");
			strcat(buffer, argv[i]);
		}
		uartCostumeWritef("broadcast: %s", buffer)
		printList(sInstance, argv[0], buffer);
		free(buffer);
	} else {
		printList(sInstance, "mytest", "PING");
	}
}

void ProcessSend(int argc, char *argv[], otInstance *sInstance, otCoapCode code){
	otIp6Address address;
		char *buffer;

		if (argc > 2) {
			//make Ip6Address
			if (otIp6AddressFromString(argv[0], &address) != kThreadError_None) {
				uartCostumeWritet("Can not parse address");
				return;
			}

			buffer = malloc(strlen(argv[2]));
			strcpy(buffer, argv[2]);
			for (int i = 3; i < argc; ++i) {
				buffer = realloc(buffer, strlen(buffer) + strlen(argv[i]) + 1);

				strcat(buffer, " \0");
				strcat(buffer, argv[i]);
			}
			uartCostumeWritef("send to %s: %s",argv[0], buffer)

			coapClientTransmit(sInstance, address, code, argv[1], buffer, &responseHandler);
			free(buffer);
		} else {
			uartCostumeWritet("Not enough arguments. Use <addr> <resource> message...");
		}
}

void ProcessGET(int argc, char *argv[], otInstance *sInstance) {
	ProcessSend(argc, argv, sInstance, kCoapRequestGet);
}
void ProcessPOST(int argc, char *argv[], otInstance *sInstance) {
	ProcessSend(argc, argv, sInstance, kCoapRequestPost);
}
void ProcessPUT(int argc, char *argv[], otInstance *sInstance) {
	ProcessSend(argc, argv, sInstance, kCoapRequestPut);
}
void ProcessDELETE(int argc, char *argv[], otInstance *sInstance) {
	ProcessSend(argc, argv, sInstance, kCoapRequestDelete);
}
