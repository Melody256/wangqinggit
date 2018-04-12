/*
 * recursion_parallel.c
 *
 *  Created on: 2014-10-22
 *      Author: wintice
 */

#include "contiki.h"
#include "contiki_cell.h"
#include "recursion_parallel.h"
#include "matrix.h"
#include "message.h"
#include "som.h"
#include "som_kohonenlayer_output.h"
#include "som_train.h"
#include "cvm.h"
#include "matrixcvm.h"
#include "linearSearch.h"
#include "SearchBST.h"

process_event_t event_recursion_calling;
process_event_t event_recursion_return;
process_event_t event_recursion_exit;
int shm_id_ark = 0;
char frame_func_return_flag[MAXNUM_CALL] =
{ 0 };
volatile struct str_ark_status* pt_ark_status = NULL;
int busy_flag = 0;
int g_FuncEnable = 0;
int shm_base[MAXNUM_FUNCTIONS] =
{ 0 };
int gSemIdFunc[MAXNUM_FUNCTIONS] =
{ 0 };
pthread_mutex_t frame_mutex[MAXNUM_FUNCTIONS];
int gShmIdFunc[MAXNUM_FUNCTIONS] =
{ 0 };
void * task_head_addr[MAXNUM_FUNCTIONS] =
{ (void *) 0 };
struct task_head_str* task_head_ark[MAXNUM_FUNCTIONS] =
{ (struct task_head_str*) 0 };
struct task_head_str* task_head_cell[MAXNUM_FUNCTIONS] =
{ (struct task_head_str*) 0 };
bool func_enable[MAXNUM_FUNCTIONS] =
{ 0 };
//void *(*recursion_functions[MAXNUM_FUNCTIONS])(void *) = { NULL };
//Ark functions list - if you add a new function, you have to add it as a new element to the bottom of this list.
FUNC_LIST(NULL, RECURSION_FUNCTION_NAME(message),
		RECURSION_FUNCTION_NAME(matrix), RECURSION_FUNCTION_NAME(som),
		RECURSION_FUNCTION_NAME(som_kohonenlayer_output),
		RECURSION_FUNCTION_NAME(som_train), RECURSION_FUNCTION_NAME(cvm),
		RECURSION_FUNCTION_NAME(matrixcvm),
		RECURSION_FUNCTION_NAME(linearSearch),
                RECURSION_FUNCTION_NAME(SearchBST));

char *g_strFunc_NameA[MAXNUM_FUNCTIONS] =
{ 0 };
char *g_strFunc_Name[MAXNUM_FUNCTIONS] =
{ 0 };
int recursion_funcitons_running[MAXNUM_CALL] =
{ 0 };

PROCESS(recursion_parallel_process, "recursion parallel process");
PROCESS_THREAD( recursion_parallel_process, ev, data)
{
	PROCESS_BEGIN()
		if (ev == PROCESS_EVENT_INIT)
		{
			int i, nFuncIdx;
			event_recursion_calling = process_alloc_event();
			event_recursion_return = process_alloc_event();
			event_recursion_exit = process_alloc_event();
			while ((shm_id_ark = shmget(SHM_KEY_ARK, 0, 0)) == -1)
				perror(" ark head shmget:");\
			pt_ark_status = (volatile struct str_ark_status*) shmat(shm_id_ark,
					(void*) 0, 0);
			pt_ark_status->num_available++;
			pt_ark_status->num_cells++;
			for (i = 1, nFuncIdx = 1; recursion_functions[i] != NULL; i++)
			{
				struct str_func_call_node* node_calling;
				node_calling = malloc(sizeof(struct str_func_call_node));
				node_calling->mode = CALL_MODE_INIT;
				node_calling->pt_node = NULL;
				node_calling->shmid = nFuncIdx;
				node_calling->thread_idx = i;
				if ((*(recursion_functions[i]))((void*) node_calling) != NULL)
					nFuncIdx++;
			}
			g_FuncEnable = nFuncIdx;
		}
	PROCESS_END()
}

//Get the ark function index by the function name.
int GetFuncListIdx(const char * strFuncName)
{
int i;
for (i = 1; g_strFunc_NameA[i] != NULL && i < MAXNUM_FUNCTIONS; i++)
{
	if (strcmp(strFuncName, g_strFunc_NameA[i]) == 0)
		return i;
}
return -1;
}

// BKDR Hash Function
int BKDRHash(char *str)
{
unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
unsigned int hash = 0;
while (*str)
{
	hash = hash * seed + (*str++);
}
return (hash & 0x7FFFFFFF);
}

int GetAvailArk()
{
if (g_nArkIdx == 2)
{
	return 1;
}
else
	return 2;
}
