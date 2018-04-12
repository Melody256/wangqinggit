/*
 * message.h
 *
 *  Created on: 2014-10-31
 *      Author: wintice
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_
#include "recursion_parallel.h"
#define MESSAGE_LENGTH 512

#define CMD_CLOSE 0x01
#define CMD_NATIVE 0x02
#define CMD_REMOTE 0x03
#define CMD_BROADCAST 0x04
#define CMD_REMOTE_REQUEST 0x05
#define CMD_INSTSUMUPT 0X06
#define CMD_TASK 0x07

#define DEST_BROARCAST 0

TASK_NODE(message)
{
	struct node_head_str node_head;
	int cell_idx_from;
	int cell_idx_dest;
	int rec_counter;
	int idx_cmd;SHM_PARA msg;
};

struct message_buffer
{
	char message[MESSAGE_LENGTH];
};

extern int func_idx_message;

void *recursion_func_message(void *);
int CallFunctionByMsg(char *msg);
int root_func_message();
int end_func_message();

#endif /* MESSAGE_H_ */
