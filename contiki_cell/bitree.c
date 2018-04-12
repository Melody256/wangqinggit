/**
 * <bitree.c>
 * @author: zhenhang<czhenhang@gmail.com>
 * @create_at:
 */
#include "bitree.h"

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
 int func_idx_bitree;

RECURSION_FUNCTION(bitree) {
    FUNCTION_INIT(bitree)
    FRAME_PRINTF("%d,",INPUTPARA(num));
    TASK_NODE(bitree) *node[2];
    if (INPUTPARA(lchild) != NULL && INPUTPARA(rchild) != NULL) {
        node[0] = INPUTPARA(lchild);
        node[1] = INPUTPARA(rchild);
        CALL_SUBFUNCTION_PREPARE(bitree, node, 2, CALL_MODE_CELL);
        CALL_SUBFUNCTION(node, 2);
    }
    FUNC_END();
}

int root_func_bitree() {
    FRAME_PRINTF("v_01");
    TASK_NODE(bitree) *pTaskNode;
    CALL_FUNCTION_PREPARE(bitree,pTaskNode,CALL_MODE_ARK);
    bitree_create(&pTaskNode, 0);//shushihua
    // if(pTaskNode == NULL) return -1;
    // pTaskNode->num = 100;
    CALL_FUNCTION(pTaskNode,CALL_BLOCKING);
    return 0;
}

int bitree_create(TASK_NODE(bitree)** ppBiTree, int depth)
{
    if(depth > 5 ){
        (*ppBiTree) = NULL;
        return 1;
    }else{
        (*ppBiTree) = (TASK_NODE(bitree)* )malloc(sizeof(TASK_NODE(bitree)));
        if(!*ppBiTree){
            return 0;
        }else{
            memset(*ppBiTree, 0, sizeof(TASK_NODE(bitree)));
            (*ppBiTree)->num = rand();
            bitree_create(&(*ppBiTree)->lchild, depth+1);
            bitree_create(&(*ppBiTree)->rchild, depth+1);
        }
    }
    return 1;
}

int end_func_bitree(){
    
}
