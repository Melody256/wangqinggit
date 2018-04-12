#include "QuickSort.h"

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

int func_idx_QuickSort;

TASK_NODE(QuickSort) {
	NODE_HEAD()
	SHM_PARA ArrayNeedSort;
	int low;
	int high;
};


RECURSION_FUNTION(QuickSort){
	FUNCTION_INIT(QuickSort)
	if(INPUTPARA(low) < INPUTPARA(high)){
		int privot;
		IF_FIRST_CALL(){
			TASK_NODE(QuickSort) *node[2];
			CALL_SUBFUNCTION_PREPARE(QuickSort, node, 2, CALL_MODE_ARK);
			int *pArrayNeedSort = smload(INPUTPARA(ArrayNeedSort));
			privot=partition(pArrayNeedSort,INPUTPARA(low),INPUTPARA(high));
			node[0]->ArrayNeedSort = INPUTPARA(ArrayNeedSort);
			node[1]->ArrayNeedSort = INPUTPARA(ArrayNeedSort);
			node[0]->low = INPUTPARA(low);
			node[1]->low = privot+1;
			node[0]->high = privot-1;
			node[1]->Right = INPUTPARA(high);
			CALL_SUBFUNCTION(node,2);
		}
	}
	NODE_FINISH();
	FUNC_END();	
}


int partition(int *ArrayNeedSort,int low,int high)
{
	int privotkey;
	privotkey=ArrayNeedSort[low];
	while(low<high){
		while(low<high && ArrayNeedSort[high]>=privotkey)
			high--;
		swap(ArrayNeedSort,low,high);
		while(low<high && ArrayNeedSort[low]<=privotkey)
			low++;
		swap(ArrayNeedSort,low,high);		
	}
	return low;
}

void swap(int *array,int low,int high)
{
	int tmp=array[low];
	array[low]=array[high];
	array[high]=tmp;	
}

int root_func_QuickSort()
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
	TASK_NODE(QuickSort) *pTaskNode;
	CALL_FUNCTION_PREPARE(QuickSort,pTaskNode,CALL_MODE_ARK);
	if(pTaskNode == NULL) return -1;
	pTaskNode->ArrayNeedSort = ArrayNeedSort;
	pTaskNode->low = 0;
	pTaskNode->high = length-1;
	
	//执行该函数结点的调用，待调用返回后输出最终排序结果
	CALL_FUNCTION(pTaskNode,CALL_BLOCKING);
	for(int i = 0;i<length;i++) 
		printf("%d ",pArrayNeedSort[i]);
	printf("\n");

	smunload(ArrayNeedSort);
	smfree(ArrayNeedSort);
	return 0;
}
