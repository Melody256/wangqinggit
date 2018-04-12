

#include "MaxSubSum.h"

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

#define LEFT 1
#define RIGHT 2

int func_idx_MaxSubSum;

TASK_NODE(MaxSubSum) {
	NODE_HEAD()
	SHM_PARA Array;
	SHM_PARA MaxLeftSum;
	SHM_PARA MaxRightSum;
	int switcher;    //用来指定函数结点的返回值应该填在MaxLeftSum还是MaxRightSum中
	int Left;
	int Right;
	int FinalAnswer;
};

RECURSION_FUNTION(MaxSubSum){
	FUNCTION_INIT(MaxSubSum)
	if(INPUTPARA(Left) == INPUTPARA(Right)){
		int *pArray=smload(INPUTPARA(Array));
		if(pArray[INPUTPARA(Left)] > 0)
			return pArray[INPUTPARA(Left)];
		else return 0;
	}else{
		int Center = (INPUTPARA(Left)+INPUTPARA(Right))/2;
		int *pArray=smload(INPUTPARA(Array));
		IF_FIRST_CALL(){			
			TASK_NODE(MaxSubSum) *node[2];
			CALL_SUBFUNCTION_PREPARE(MaxSubSum, node, 2, CALL_MODE_ARK);
			node[0]->Array = INPUTPARA(Array);
			node[1]->Array = INPUTPARA(Array);
			node[0]->Left = INPUTPARA(Left);
			node[1]->Left = Center+1;
			node[0]->Right = Center;
			node[1]->Right = INPUTPARA(Right);
			node[0]->switcher = LEFT;
			node[1]->switcher = RIGHT;
			CALL_SUBFUNCTION(node,2);
		}
		IF_WAITING_RETURN(){
			int MaxLeftBorderSum=0,MaxRightBorderSum=0;
			int LeftBorderSum=0,RightBoderSum=0;
			for(int i=Center;i >= INPUTPARA(Left);i--)
			{
				LeftBorderSum += pArray[i];
				if(LeftBorderSum > MaxLeftBorderSum)
					MaxLeftBorderSum = LeftBorderSum;
			}
			for(int i=Center+1;i <= INPUTPARA(Right);i++)
			{
				RightBorderSum+=pArray[i];
				if(RightBorderSum > MaxRightBorderSum)
					MaxRightBorderSum = RightBorderSum;
			}
			
			
			//先计算本次执行的结果值，本函数结点中的MaxLeftSum和MaxRightSum已经存放了由上一个结点返回的值
			SHMINPUTPARA(pMaxLeftSum,int,INPUTPARA(MaxLeftSum))
			SHMINPUTPARA(pMaxRightSum,int,INPUTPARA(MaxRightSum))
			int answer = Max3(*pMaxLeftSum,*pMaxRightSum,MaxLeftBorderSum+MaxRightBorderSum);
			//这一步用于返回结果,返回结果不能修改当前函数结点，而应该修改父结点的值
			TASK_NODE(MaxSubSum) *pToParent = smload(INPUTPARA(node_head).parent.uNodeIdx.shmid);
			if(INPUTPARA(switcher) == LEFT){
				int *pParentMaxLeftSum = smload(pToParent->MaxLeftSum);	
				*pParentMaxLeftSum = answer;
				smunload(pToParent->MaxLeftSum);
			}else if(INPUTPARA(switcher) == RIGHT){
				int *pParentMaxRightSum = smload(pToParent->MaxRightSum);		
				*pParentMaxRightSum = answer;
				smunload(pToParent->MaxRightSum);
			}else if(INPUTPARA(switcher) == FIRST){  //对于第一个结点,计算结果直接存在自身结点中
				INPUTPARA(FinalAnswer) = answer;
			}			
			smunload(INPUTPARA(node_head).parent.uNodeIdx.shmid);
			NODE_FINISH();
		}
	}
	NODE_FINISH();
	FUNC_END();	
}


int Max3(int a,int b,int c)
{
	return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

int root_func_MaxSubSum()
{
	//在共享内存区创建随机数组
	SHM_PARA Array;
	int length;
	printf("please input the length of Array which you want to sort: ");
	scanf("%d",&length);
	Array.nShmId = smalloc(IPC_PRIVATE,
			sizeof(int) * length);
	Array.offset = 0;
	if (Array.nShmId == -1) {
		printf("error:can't create the Array space");
		return -1;
	}	
	pArray = (int*) smload(Array);
	srand((int) time(0));
	for (int i = 0; i < length; i++) 
		pArray[i] = rand() % 10;
	
    //分配函数结点并填充剩余参数
	TASK_NODE(MaxSubSum) *pTaskNode;
	CALL_FUNCTION_PREPARE(MaxSubSum,pTaskNode,CALL_MODE_ARK);
	if(pTaskNode == NULL) return -1;	
	pTaskNode->Array = Array;
	pTaskNode->Left = 0;
	pTaskNode->Right = length-1;

	//执行该函数结点的调用，待调用返回后输出最终排序结果
	CALL_FUNCTION(pTaskNode,CALL_BLOCKING);

	smunload(ArrayNeedSort);
	smfree(ArrayNeedSort);
	return 0;
}