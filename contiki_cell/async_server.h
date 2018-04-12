/*
 * async_server.h
 *
 *  Created on: 2014-5-15
 *      Author: user
 */
#include <pthread.h>

#ifndef ASYNC_SERVER_H_
#define ASYNC_SERVER_H_

typedef struct
{
	int CMD;
	int size;
	char message[512];
} SKY_MESSAGE_BUFF;

int SetServer();
int CloseServer();

#endif /* ASYNC_SERVER_H_ */
