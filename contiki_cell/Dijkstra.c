#include "Dijkstra.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>

#include "contiki_cell.h"
#include "recursion_parallel.h"
#include "sem_com.h"
#include "shm_com.h"

#define N 6
#define INFINITE 99999

typedef int flag;
int func_idx_Dijkstra;

TASK_NODE(Dijkstra) {
	NODE_HEAD()
	int Dist[N-1];
	int min;
	int visiting;
	flag if_root;
};

RECURSION_FUNTION(Dijkstra){
	FUNCTION_INIT(Dijkstra)
	if(if_root){  //root
		IF_FIRST_CALL(){
			int *finish=malloc((N-1)*sizeof(int));
			int size=0;
			for(int i=0;i<N-1;i++){
				if(i == min-1) finish=1;
				else finish=0,size++;
			}
			int *pToNode=malloc(size*sizeof(TASK_NODE(Dijkstra)*));
			CALL_SUBFUNCTION_PREPARE(Dijkstra, pToNode, size, CALL_MODE_ARK);
			for(int i=0,j=0;i<n-1;i++){
				if(finish[i]==0){
					pToNode[j]->min=INPUTPARA(min);
					pToNode[j]->visiting=i;
					j++;
				}
			}
			CALL_SUBFUNCTION();
		}
		IF_WAITING_RETURN(){
				int *pToNode=malloc(size*sizeof(TASK_NODE(Dijkstra)*));
				CALL_SUBFUNCTION_PREPARE(Dijkstra, pToNode, size, CALL_MODE_ARK);
				for(int i=0,j=0;i<n-1;i++){
					if(finish[i]==0){
						pToNode[j]->min=INPUTPARA(min);
						pToNode[j]->visiting=i;
						j++;
					}
				}
				CALL_SUBFUNCTION();		
		}
	}else{
		//deal relax and return the result
	}

	free(pToNode)
	free(finish);
	NODE_FINISH();
	FUNC_END();	
}


int root_func_Dijkstra()
{
	TASK_NODE(Dijkstra) *pTaskNode;
	CALL_FUNCTION_PREPARE(Dijkstra,pTaskNode,CALL_MODE_ARK);
	if(pTaskNode == NULL) return -1;
	
	int graph[N][N]={{INFINITE,INFINITE,10,INFINITE,30,100},
	{INFINITE,INFINITE,5,INFINITE,INFINITE,INFINITE},
	{INFINITE,INFINITE,INFINITE,50,INFINITE,INFINITE},
	{INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,10},
	{INFINITE,INFINITE,INFINITE,20,INFINITE,60},
	{INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE}};
	
	pTaskNode->min=0;
	for(int i=1;i<N;++i){
		pTaskNode->Dist[i-1]=graph[0][i];
		if(graph[0][i] < pTaskNode->Dist[min])
			pTaskNode->min=i;
	}
	pTaskNode->visiting=min;
	pTaskNode->if_root=true;
	
	CALL_FUNCTION(pTaskNode,CALL_BLOCKING);
	return 0;
}
