/*
    linearSearch.c
    Created on: 2018-3-21
        Author: Qing
*/

#include "linearSearch.h"

#include "contiki_cell.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>

#include "sem_com.h"
#include "shm_com.h"

int func_idx_linearSearch;

RECURSION_FUNCTION(linearSearch){
    FUNCTION_INIT(linearSearch)
    int limit = 10;  // maximum group size
    int ave;
    int len = INPUTPARA(right) - INPUTPARA(left) + 1;

    if(len > limit){
        ave = (INPUTPARA(left)+INPUTPARA(right)) / 2;
        IF_FIRST_CALL(){
            TASK_NODE(linearSearch) *node[2];
            CALL_SUBFUNCTION_PREPARE(linearSearch, node, 2, CALL_MODE_ARK);
            node[0]->a = INPUTPARA(a);
            node[1]->a = INPUTPARA(a);
            node[0]->k = INPUTPARA(k);
            node[1]->k = INPUTPARA(k);
            node[0]->ans = INPUTPARA(ans);
            node[1]->ans = INPUTPARA(ans);  
            node[0]->left = INPUTPARA(left);
            node[1]->left = ave + 1;
            node[0]->right = ave;
            node[1]->right = INPUTPARA(right);
            CALL_SUBFUNCTION(node, 2);
            FRAME_PRINTF("%d, first call\n", nCellIdx);
        }
        IF_WAITING_RETURN(){
            FRAME_PRINTF("%d, second call\n", nCellIdx);
            NODE_FINISH();   
        }
    }
    else{
        FRAME_PRINTF("%d, it can run\n", nCellIdx);    
        int* pa = (int*) smload(INPUTPARA(a));
        int* pk = (int*) smload(INPUTPARA(k));
        int* pans = (int*) smload(INPUTPARA(ans));
        int i;
        int current = 0;
        for(i = 0; i < len; i++){
            current = i + INPUTPARA(left);  // index of the current element
            if(pa[current] == pk[0]){
                pans[0] = pa[current]; 
            }
        }

        smunload(INPUTPARA(a));
        smunload(INPUTPARA(a));
        smunload(INPUTPARA(ans));
        NODE_FINISH();
        FRAME_PRINTF("%d, run over\n", nCellIdx);    
    }            
  
    FUNC_END();
}


int root_func_linearSearch(){
    int Length = 0;  // length of the array
    float innum = -1;  // input number
    SHM_PARA smKey;  // keep the will-be-searched integer
    SHM_PARA smAnswer;  // search result
    SHM_PARA smArray;  
    int* pKey;
    int* pAns;
    int* pA;
   
    printf("Please input the length of the will-be-searched array:\n");
    scanf("%d", &Length);
    
    smKey.nShmId = smalloc(IPC_PRIVATE, sizeof(int) * 1); 
    smAnswer.nShmId = smalloc(IPC_PRIVATE, sizeof(int) * 1);
    smArray.nShmId = smalloc(IPC_PRIVATE, sizeof(int) * Length);
    smKey.offset = 0;
    smAnswer.offset = 0;
    smArray.offset = 0;
    if(smArray.nShmId == -1 || smAnswer.nShmId == -1 || smKey.nShmId == -1){
        printf("Error: can't create the array space!\n");
        return -1;
    }
    pKey = (int*) smload(smKey);
    pAns = (int*) smload(smAnswer);
    pA = (int*) smload(smArray);
    
    srand((int) time(0));
    printf("The array is:\n");
    int i = 0;    
    for(i = 0; i < Length; i++){
        pA[i] = rand() % 100;   
        printf("%d,", pA[i]);
        if((i + 1)%10 == 0) printf("\n");
    }  // print the array

    pAns[0] = -1;  // -1 means NOT FOUND
    smunload(smArray);

    printf("Please input the integer you want to search(between 0 and 99):\n");
    scanf("%f", &innum);      
    while(1){
        if(innum - ((int) innum) == 0){
            if(innum > 99 || innum < 0){
                printf("The integer must be between 0 and 99! Input again:\n");
                scanf("%f", &innum);
            }
            else{
                pKey[0] = ((int) innum);  // legal search keyword
                break;
            }     
        }    
        else {
            printf("Not an integer, input again:\n");     
            scanf("%f", &innum);
        }
    }
    printf("The key is %d.\n", pKey[0]);

    smunload(smKey);
    
    clock_t start, finish;
    float duration;

    start = clock();
    TASK_NODE(linearSearch) *pTaskNode;
    CALL_FUNCTION_PREPARE(linearSearch, pTaskNode, CALL_MODE_ARK);
    if(pTaskNode == NULL) return -1;
    pTaskNode->a = smArray;
    pTaskNode->k = smKey;
    pTaskNode->ans = smAnswer;
    pTaskNode->left = 0;
    pTaskNode->right = Length - 1;
    CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
 
    printf("The output is (-1 means NOT FOUND):\n");
    printf("%d\n", pAns[0]); 

    finish = clock();    
    duration = (float)(finish - start) / CLOCKS_PER_SEC * 1000;
    printf("本次查找范围： %d 个数据，查找用时： %f ms\n", Length, duration);    

    smunload(smAnswer);

    smfree(smKey);
    smfree(smAnswer);
    smfree(smArray);
    return 0;

}
