/*
 * message.c
 *
 *  Created on: 2014-10-31
 *      Author: wintice
 */

#include "recursion_parallel.h"
#include "contiki_cell.h"
#include "shm_com.h"
#include "message.h"

extern int exitflag;

int func_idx_message;

RECURSION_FUNCTION(message)
{
	FUNCTION_INIT(message)
//static		int limit = 500;
		SHMINPUTPARA(msg, char, INPUTPARA(msg));
		if (INPUTPARA(cell_idx_dest)
				== nCellIdx|| INPUTPARA(cell_idx_dest)==DEST_BROARCAST)
		{
			switch (INPUTPARA(idx_cmd))
			{
			case CMD_NATIVE:
				;
				printf("Got a local message\n");
				printf("%s", msg);
				break;
			case CMD_CLOSE:
				exitflag = 1;
				break;
			case CMD_TASK:
				printf("Got a task call:%s\n", msg);
				printf("%s\n", msg);
				CallFunctionByMsg(msg);
				break;
			}
			INPUTPARA(rec_counter)--;
		}
		DEL_SHM_PARA(msg, INPUTPARA(msg));
		if (INPUTPARA(rec_counter) != 0)
		{
			static TASK_NODE(message) *node_temp;
			CALL_FUNCTION_PREPARE(message,node_temp,CALL_MODE_ARK);
			if(node_temp != NULL)
			{
				node_temp->cell_idx_dest = INPUTPARA(cell_idx_dest);
				node_temp->cell_idx_from = INPUTPARA(cell_idx_from);
				node_temp->idx_cmd = INPUTPARA(idx_cmd);
				node_temp->msg = INPUTPARA(msg);
				node_temp->rec_counter = INPUTPARA(rec_counter);
				CALL_FUNCTION(node_temp,CALL_BLOCKING);
			}
		}
		NODE_FINISH()
		;
		END_NODE();
}

int CallFunctionByMsg(char *msg)
{
	int nFuncIdx;
	if ((nFuncIdx = GetFuncListIdx(msg)) == -1)
	{
		printf("Func doesn't exist");
		return -1;
	}
	else
	{
		printf("Func list-idx is:%d\n", nFuncIdx);
		struct str_func_call_node *pNodeCalling;
		pNodeCalling = malloc(sizeof(struct str_func_call_node));
		pNodeCalling->mode = CALL_MODE_ROOT;
		pNodeCalling->pt_node = NULL;
		pNodeCalling->shmid = 0;
		(*(recursion_functions[nFuncIdx]))((void*) pNodeCalling);
		return 0;
	}
}

int root_func_message()
{
	return 0;
}
