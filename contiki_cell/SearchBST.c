/*
    SearchBST.c
    Created on: 2018-4-10
        Author: Qing
*/

#include "SearchBST.h"

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

int func_idx_SearchBST;

RECURSION_FUNCTION(SearchBST){
    FUNCTION_INIT(SearchBST)
    int limit = 5;
    int ave;
    int sortlength = INPUTPARA(right) - INPUTPARA(left) + 1;

    if(sortlength > limit){
        ave = (INPUTPARA(left) + INPUTPARA(right)) / 2;
        IF_FIRST_CALL(){
            TASK_NODE(SearchBST) *node[2];
            CALL_SUBFUNCTION_PREPARE(SearchBST, node, 2, CALL_MODE_ARK);
            node[0]->left = INPUTPARA(left);
            node[1]->left = ave + 1;
            node[0]->right = ave;
            node[1]->right = INPUTPARA(right);
            node[0]->data = INPUTPARA(data);
            node[1]->data = INPUTPARA(data);
            node[0]->lchild = INPUTPARA(lchild);
            node[1]->lchild = INPUTPARA(lchild);  
            node[0]->rchild = INPUTPARA(rchild);
            node[1]->rchild = INPUTPARA(rchild);            

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
        int* pdata = (int*) smload(INPUTPARA(data));
        int* plc = (int*) smload(INPUTPARA(lchild));
        int* prc = (int*) smload(INPUTPARA(rchild));
        int i;
        int current = 0;
        for(i = 0; i < sortlength; i++){
            current = i + INPUTPARA(left);  // index of the current node
            InsertBST(0, current, pdata, plc, prc);
        }

        smunload(INPUTPARA(data));
        smunload(INPUTPARA(lchild));
        smunload(INPUTPARA(rchild));
        NODE_FINISH();
        FRAME_PRINTF("%d, run over\n", nCellIdx);    
    }            
  
    FUNC_END();
}


int InsertBST(int start, int cur, int *pd, int *pl, int *pr){  
    if(start == cur)         
        return 1;  
    else if(pd[start] == pd[cur])  
        return 0;  
    else if(pd[start] > pd[cur]){
        if(pl[start] == -1){
            pl[start] = cur;
        }  
        start = pl[start];
        return InsertBST(start, cur, pd, pl, pr);
    }  
    else{  
        if(pr[start] == -1){
            pr[start] = cur;
        }
        start = pr[start];
        return InsertBST(start, cur, pd, pl, pr); 
    } 
}


int Search(int *pd, int *pl, int *pr, int cur, int k){       
    if(k < pd[cur] && pl[cur] != -1)  
        return Search(pd, pl, pr, pl[cur], k);  
    else if(k > pd[cur] && pr[cur] != -1)  
        return Search(pd, pl, pr, pr[cur], k); 
    else
        return pd[cur]; 
}  


int root_func_SearchBST(){
    int num = 0;    
    float innum;
    int Key;  // the will-be-searched integer
    int result = -1;  // search result
    SHM_PARA smData, smLchild, smRchild;  
    int* pData;
    int* pL;
    int* pR;
   
    printf("Please input the amount of nodes of the will-be-searched bitree:\n");
    scanf("%d", &num);
    
    smData.nShmId = smalloc(IPC_PRIVATE, sizeof(int) * num);
    smLchild.nShmId = smalloc(IPC_PRIVATE, sizeof(int) * num);
    smRchild.nShmId = smalloc(IPC_PRIVATE, sizeof(int) * num);
    smData.offset = 0;
    smLchild.offset = 0;
    smRchild.offset = 0;
    
    if(smData.nShmId == -1 || smLchild.nShmId == -1 || smRchild.nShmId == -1){
        printf("Error: can't create the array space!\n");
        return -1;
    }

    pData = (int*) smload(smData);
    pL = (int*) smload(smLchild);
    pR = (int*) smload(smRchild);
    
    srand((int) time(0));
    printf("The nodes are:\n");
    int i = 0;    
    for(i = 0; i < num; i++){
        pData[i] = rand() % 100;   
        printf("%d,", pData[i]);
        if((i + 1)%10 == 0) printf("\n");
        pL[i] = -1; 
        pR[i] = -1;
    }  // init and print the nodes


    printf("Please input the integer you want to search(between 0 and 99):\n");
    scanf("%f", &innum);      
    while(1){
        if(innum - ((int) innum) == 0){
            if(innum > 99 || innum < 0){
                printf("The integer must be between 0 and 99! Input again:\n");
                scanf("%f", &innum);
            }
            else{
                Key = ((int) innum);  // legal search keyword
                break;
            }     
        }    
        else {
            printf("Not an integer, input again:\n");     
            scanf("%f", &innum);
        }
    }
    printf("The key is %d.\n", Key);
    
    clock_t start, finish;
    float duration; 

    start = clock();
    TASK_NODE(SearchBST) *pTaskNode;
    CALL_FUNCTION_PREPARE(SearchBST, pTaskNode, CALL_MODE_ARK);
    if(pTaskNode == NULL) return -1;
    pTaskNode->left = 0;
    pTaskNode->right = num - 1;
    pTaskNode->data = smData;
    pTaskNode->lchild = smLchild;
    pTaskNode->rchild = smRchild;
    CALL_FUNCTION(pTaskNode, CALL_BLOCKING);

    result = Search(pData, pL, pR, 0, Key);
    if(result == Key){
        printf("Found! The result is:\n");
        printf("%d\n", result); 
    }
    else{
        printf("Not found... The last visited node is:\n");
        printf("%d\n", result);
    }

    finish = clock();
    duration = (float)(finish - start) / CLOCKS_PER_SEC * 1000;
    printf("本次查找范围： %d 个数据，查找用时： %f ms\n", num, duration);

    smunload(smData);
    smunload(smLchild);
    smunload(smRchild);

    smfree(smData);
    smfree(smLchild);
    smfree(smRchild);
    return 0;

}
