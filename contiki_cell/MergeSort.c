

#include "MergeSort.h"

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

int func_idx_MergeSort;

TASK_NODE(MergeSort) {
	NODE_HEAD()  
	SHM_PARA ArrayNeedSort;  //需要进行排序的数组
	int Left;                //当前正在排序的部分的左边界
	int Right;               //当前正在排序的部分的右边界
};

RECURSION_FUNTION(MergeSort){
	FUNCTION_INIT(MergeSort)
	if(INPUTPARA(Left) < INPUTPARA(Right)){
		int Center = (INPUTPARA(Left)+INPUTPARA(Right))/2;
		IF_FIRST_CALL(){
			TASK_NODE(MergeSort) *node[2];
			CALL_SUBFUNCTION_PREPARE(MergeSort, node, 2, CALL_MODE_ARK);
			node[0]->ArrayNeedSort = INPUTPARA(ArrayNeedSort);
			node[1]->ArrayNeedSort = INPUTPARA(ArrayNeedSort);
			node[0]->Left = INPUTPARA(Left);
			node[1]->Left = Center+1;
			node[0]->Right = Center;
			node[1]->Right = INPUTPARA(Right);
			CALL_SUBFUNCTION(node,2);
		}
		IF_WAITING_RETURN(){
			int *pArrayNeedSort = smload(INPUTPARA(ArrayNeedSort));
			Merge(pArrayNeedSort,INPUTPARA(Left),Center+1,INPUTPARA(Right));
			smunload(INPUTPARA(ArrayNeedSort));
			NODE_FINISH();
		}
	}
	NODE_FINISH();
	FUNC_END();	
}

void Merge(int ArrayNeedSort[], int Lpos,int Rpos,int RightEnd)
{
	int LeftEnd,NumElements,TmpPos;
	LeftEnd = Rpos-1;
	TmpPos = Lpos;
	NumElements = RightEnd-Lpos+1;
	int *TmpArray = (int *)malloc(NumElements*sizeof(int));
	
	while(Lpos <= LeftEnd && Rpos <= RightEnd)
		if(ArrayNeedSort[Lpos] <= ArrayNeedSort[Rpos])
			TmpArray[TmpPos++] = ArrayNeedSort[Lpos++];
		else
			TmpArray[TmpPos++] = ArrayNeedSort[Rpos++];
	while(Lpos <= LeftEnd)
		TmpArray[TmpPos++] = ArrayNeedSort[Lpos++];
	while(Rpos <= RightEnd)
		TmpArray[TmpPos++] = ArrayNeedSort[Rpos++];		
	for(int i=0;i < NumElements;i++,RightEnd--)
		ArrayNeedSort[RightEnd] = TmpArray[RightEnd];
	free(TmpArray);
}

int root_func_MergeSort()
{
	//在共享内存区创建随机待排序数组
	SHM_PARA ArrayNeedSort;
	int length;
	printf("please input the length of Array which you want to sort: ");
	scanf("%d",&length);
	ArrayNeedSort.nShmId = smalloc(IPC_PRIVATE,
			sizeof(int) * length);
	ArrayNeedSort.offset = 0;
	if (ArrayNeedSort.nShmId == -1) {
		printf("error:can't create the Array space");
		return -1;
	}	
	pArrayNeedSort = (int*) smload(ArrayNeedSort);
	srand((int) time(0));
	for (int i = 0; i < length; i++) 
		pArrayNeedSort[i] = rand() % 10;
	
    //分配函数结点并填充剩余参数
	TASK_NODE(MergeSort) *pTaskNode;
	CALL_FUNCTION_PREPARE(MergeSort,pTaskNode,CALL_MODE_ARK);
	if(pTaskNode == NULL) return -1;
	pTaskNode->ArrayNeedSort = ArrayNeedSort;
	pTaskNode->Left = 0;
	pTaskNode->Right = length-1;
	
	//执行该函数结点的调用，待调用返回后输出最终排序结果
	CALL_FUNCTION(pTaskNode,CALL_BLOCKING);
	for(int i = 0;i<length;i++) 
		printf("%d ",pArrayNeedSort[i]);
	printf("\n");

	smunload(ArrayNeedSort);
	smfree(ArrayNeedSort);
	return 0;
}