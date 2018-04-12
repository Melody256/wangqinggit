/*
 * shm_customer.c
 *
 *  Created on: 2014-4-7
 *      Author: user
 */

#include "shm_com.h"
#include "sem_com.h"
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include "contiki.h"
#include "contiki_cell.h"
#include "recursion_parallel.h"
#include "matrix.h"
#include "cvm.h"
#include "async_server.h"

#include <stdio.h> /* For ////printf() */
#define SIGRMTREQ 32
extern char** contiki_argv;
//extern process_event_t serial_line_event_message;
int g_nArkIdx;
int nCellIdx;
int exitflag = 0;

/*---------------------------------------------------------------------------*/
PROCESS(contiki_cell_process, "contiki cell process");
AUTOSTART_PROCESSES(&contiki_cell_process, &recursion_parallel_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(contiki_cell_process, ev, data)
{
	PROCESS_BEGIN()
		;
		static pid_t father_pid;
		static pthread_t thread_func_call[MAXNUM_CALL] =
		{ 0 };
		nCellIdx = contiki_argv[0][0];
		printf("%d\n", nCellIdx);
		sscanf(contiki_argv[1], "%d", (int *) &father_pid);
		g_nArkIdx = contiki_argv[2][0];
		if (nCellIdx == 1)
		{
			FUNC_SWITCHER_cvm = 1;
		}
		if (FUNC_SWITCHER_cvm)
		{
			SetServer();
			init_func_cvm();
		}
		SmTableInit();
		static struct etimer timer;
		etimer_set(&timer, CLOCK_CONF_SECOND / 100);
		/*进程主体*/

		do
		{
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER); //  || ev == event_recursion_return|| ev == serial_line_event_message);
			etimer_reset(&timer);
#ifdef RECURSION_ENABLE
			static int i;
			static int j;
			for (i = 1; i < g_FuncEnable; i++)
			{
				if (busy_flag < MAXNUM_CALL && exitflag != 1)
				{
					static void* task_chain_addr_temp;
					static int shmid_calling;
					shmid_calling = 0;
					struct node_head_str *task_chain_addr = NULL;
					if (task_head_ark[i]->task_undone != 0)
					{
						task_chain_addr_temp = (void *) -1;
						while (sem_p(gSemIdFunc[i]) != 0)
						{
//								PROCESS_WAIT_EVENT_UNTIL(
//										ev == PROCESS_EVENT_TIMER);
//								etimer_reset(&timer);
						}
						if (task_head_ark[i]->task_undone != 0)
						{
							shmid_calling = task_head_ark[i]->stack_top.shmid;
							while ((task_chain_addr_temp = shmat(shmid_calling,
									(void*) 0, 0)) == (void*) -1)
							{
//									PROCESS_WAIT_EVENT_UNTIL(
//											ev == PROCESS_EVENT_TIMER);
//									etimer_reset(&timer);
							}
							task_chain_addr =
									(struct node_head_str *) task_chain_addr_temp;
							if (strcmp(task_chain_addr->func, g_strFunc_Name[i])
									== 0)
							{
								task_head_ark[i]->stack_top.shmid =
										task_chain_addr->right.shmid;
								task_head_ark[i]->task_undone--;
							}
							else
							{
								shmdt(task_chain_addr_temp);
								task_chain_addr_temp = (void *) -1;
							}
						}
						while (sem_v(gSemIdFunc[i]) != 0)
						{
//								PROCESS_WAIT_EVENT_UNTIL(
//										ev == PROCESS_EVENT_TIMER);
//								etimer_reset(&timer);
						}
						if (task_chain_addr_temp != (void *) -1)
						{
							struct str_func_call_node* node_calling;
							node_calling = malloc(
									sizeof(struct str_func_call_node));
							node_calling->mode = task_chain_addr->mode;
							node_calling->pt_node =
									(void*) task_chain_addr_temp;
							node_calling->shmid = shmid_calling;
							FRAME_PRINTF("Cell %d:ark call func %s\n", nCellIdx,
									task_chain_addr->func);
							for (j = 0; j < MAXNUM_CALL; j++)
							{
								if (thread_func_call[j] == 0)
								{
									node_calling->thread_idx = j;
									while (pthread_create(&thread_func_call[j],
									NULL,
											recursion_functions[GetFuncListIdx(
													task_chain_addr->func)],
											(void*) node_calling) != 0)
										;
									break;
								}
							}
							busy_flag++;
							if (busy_flag == MAXNUM_CALL)
							{
								pt_ark_status->num_available--;
							}
							//FRAME_PRINTF("Cell %d:Ark call done.shmid:%d\n",cell_idx, shmid_calling);
						}
					}
					if (task_head_cell[i]->task_undone
							!= 0&&busy_flag < MAXNUM_CALL)
					{
						pthread_mutex_lock(&(frame_mutex[i]));
						if (task_head_cell[i]->task_undone != 0)
						{
							FRAME_PRINTF("Cell %d:cell call need %d\n",
									nCellIdx, task_head_cell[i]->task_undone);
							task_head_cell[i]->task_undone--;
							task_chain_addr_temp =
									task_head_cell[i]->stack_top.addr;
							task_chain_addr =
									(struct node_head_str *) task_chain_addr_temp;
							task_head_cell[i]->stack_top.addr =
									task_chain_addr->right.addr;
							struct str_func_call_node* node_calling;
							node_calling = malloc(
									sizeof(struct str_func_call_node));
							node_calling->mode = task_chain_addr->mode;
							node_calling->pt_node =
									(void*) task_chain_addr_temp;
							node_calling->shmid = 0;
							for (j = 0; j < MAXNUM_CALL; j++)
							{
								if (thread_func_call[j] == 0)
								{
									node_calling->thread_idx = j;
									while (pthread_create(&thread_func_call[j],
									NULL,
											recursion_functions[GetFuncListIdx(
													task_chain_addr->func)],
											(void*) node_calling) != 0)
										;
									break;
								}
							}
							busy_flag++;
							if (busy_flag == MAXNUM_CALL)
							{
								pt_ark_status->num_available--;
							}
							//FRAME_PRINTF("Cell %d:cell call done.addr:%x\n",cell_idx, (unsigned int)task_chain_addr_temp);
						}
						pthread_mutex_unlock(&(frame_mutex[i]));
					}
				}
			}
			for (i = 0; i < MAXNUM_CALL; i++)
			{
				if (frame_func_return_flag[i] == 1)
				{
					frame_func_return_flag[i] = 0;
					pthread_join(thread_func_call[i], NULL);
					FRAME_PRINTF("Cell %d:func return\n", nCellIdx);
					busy_flag--;
					if (busy_flag == MAXNUM_CALL - 1)
					{
						pt_ark_status->num_available++;
					}
					thread_func_call[i] = 0;
				}
			}
			if (exitflag == 1 && busy_flag == 0)
				break;
#endif
//		else if(ev == serial_line_event_message){
//			static char msg_temp[1024];
//			memset(msg_temp,0,1024);
//			strcpy(msg_temp,(char *)data);
//			do {
//				PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
//				if(sem_p(semid)==0){
//					if(msg_temp[0]=='r'){
//						//sscanf((char *)(&(msg_temp[2])), "%d %d %s",&(shm_buff_inst->dest_cc),&(shm_buff_inst->dest), shm_buff_inst->buffer);
//						shm_buff_inst->src_cc = fathernum;
//						shm_buff_inst->src = instnum;
//						shm_buff_inst->cmd = CMD_REMOTE_REQUEST;
//						kill(father_pid,SIGINT);
//					}
//					else {
//						//sscanf((char *)(&(msg_temp[0])), "%d %s",&(shm_buff_inst->dest), shm_buff_inst->buffer);
//						shm_buff_inst->src_cc = fathernum;
//						shm_buff_inst->src = instnum;
//						shm_buff_inst->dest_cc = fathernum;
//						shm_buff_inst->cmd = CMD_NATIVE;
//					}
//					while(sem_v(semid)!=0){
//						etimer_reset(&timer);
//						PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
//					}
//					etimer_reset(&timer);
//					break;
//				}
//				etimer_reset(&timer);
//			}while (1);
//		}
		} while (1);
		static int i;
		for (i = 1; i < g_FuncEnable; i++)
		{
			while (shmdt(task_head_addr[i]) == -1)
			{
				PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
				etimer_reset(&timer);
			}
			sem_p(gSemIdFunc[i]);
			struct shmid_ds ds;
			shmctl(gShmIdFunc[i], IPC_STAT, &ds);
			if (ds.shm_nattch == 0)
			{
				union semun sem_union;
				semctl(gSemIdFunc[i], 0, IPC_RMID, sem_union);
			}
			sem_v(gSemIdFunc[i]);
			shmctl(gShmIdFunc[i], IPC_RMID, NULL);
			if (task_head_cell[i] != NULL)
			{
				free(task_head_cell[i]);
			}
			pthread_mutex_destroy(&frame_mutex[i]);
		}
		pt_ark_status->num_cells--;
		pt_ark_status->num_available--;
		while (shmdt((void *) pt_ark_status) == -1)
		{
			PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
			etimer_reset(&timer);
		}
		if (FUNC_SWITCHER_cvm)
		{
			end_func_cvm();
			CloseServer();
		}
		SmTableDel();
		FRAME_PRINTF("Cell %d exit!\n", nCellIdx)
		;
		exit(0);
	PROCESS_END()
;
}

