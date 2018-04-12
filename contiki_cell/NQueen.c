#include "NQueen.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>

#include "contiki_cell.h"
#include "recursion_parallel.h"
#include "sem_com.h"
#include "shm_com.h"
#define N 8  
#define INFINITY 9999

int func_idx_NQueen;

TASK_NODE(NQueen) {
	NODE_HEAD()
	int CurrentRow;
	SHMPARA Position; //当前行的列布局
	int Num; //表示该结点执行完后产生的符合的布局数目，对于根结点和第零行结点是结果参数
};



RECURSION_FUNTION(NQueen){
	FUNCTION_INIT(NQueen)
	if(INPUTPARA(CurrentRow) == INFINITY){ //根节点
		IF_FIRST_CALL(){
			TASK_NODE(NQueen) *node[N];
			CALL_SUBFUNCTION_PREPARE(NQueen, node, N, CALL_MODE_ARK);
			for(int i=0;i<N；i++){
				node[i]->CurrentRow = 0;
				node[i]->Position.shmid = smalloc(IPC_PRIVATE,N*sizeof(int));
				SHMINPUTPARA(pPosition,int,node[i]->Position)
				memset(pPosition,0,N*sizeof(int));
				pPosition[1]=i;    //分别给每种情况的棋盘做第一次布局				
			}
			CALL_SUBFUNCTION(node,N);
		}
		IF_WAITING_RETURN(){
			printf("The sulution of eight Queen Problem is %d.\n",INPUTPARA(Num));
			NODE_FINISH();
		}
	}else if(INPUTPARA(CurrentRow) == 0){  //第0行的结点,会依执行情况生成不同数目的新函数结点
		IF_FIRST_CALL(){
			SHMINPUTPARA(pPosition,int,node[i]->Position)
			for(int i=0;i<N;i++){
				int ok=1;			
				pPosition[1] = i;  //尝试放在第一行第i列
				if(pPosition[1] == pPosition[0] || 1-pPosition[1] == 0-pPosition[0] || 1+pPosition[1] == 0+pPosition[0]){//检查是否与第0行的放置冲突
					ok=0;break;
				}
				//如果该列放置没有冲突，生成以该布局为基础的新函数结点
			}
		}
		IF_WAITING_RETURN(){
			NODE_FINISH();
		}
	}else{  //其他结点
		NODE_FINISH();
	}
	NODE_FINISH();
	FUNC_END();	
}



int root_func_NQueen()
{
	TASK_NODE(NQueen) *pTaskNode;
	CALL_FUNCTION_PREPARE(NQueen,pTaskNode,CALL_MODE_ARK);
	if(pTaskNode == NULL) return -1;
	pTaskNode->CurrentRow=INFINITY;
	CALL_FUNCTION(pTaskNode,CALL_BLOCKING);

	return 0;
}