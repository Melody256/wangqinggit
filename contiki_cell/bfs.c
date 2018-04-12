/**
 * <bfs.c>
 * @author: zhenhang<czhenhang@gmail.com>
 * @create_at:
 */
#include "bfs.h"

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
 int func_idx_bfs;

RECURSION_FUNCTION(bfs) {
    FUNCTION_INIT(bfs)
    IF_FIRST_CALL() {
        FRAME_PRINTF("%d,devide\n!!", nCellIdx);

        int i,j;
        TASK_NODE(bfs) *node[1];
        int *matrix = INPUTPARA(matrix);
        int cur_node = INPUTPARA(cur_node);
        int *gn = INPUTPARA(graph_node);
        int *visited_node = INPUTPARA(visited_node);
        printf("%d ",gn[cur_node]);
        visited_node[cur_node] = 1;
        for ( i = 0; i < GRAPH_SIZE; i++)
        {
            if( matrix[cur_node*(GRAPH_SIZE)+i]  == 1 && visited_node[i] != 1) {
                CALL_SUBFUNCTION_PREPARE(bfs, node, 1, CALL_MODE_CELL);
                node[0]->graph_node = gn;
                node[0]->matrix = matrix;
                node[0]->visited_node = visited_node;
                node[0]->cur_node = i;
                CALL_SUBFUNCTION(node, 1);
            }
        }
    }
    IF_WAITING_RETURN()
    {
        FRAME_PRINTF("%d,second call\n!!", nCellIdx);
        NODE_FINISH();
    }
}//
}

int root_func_bfs() {
    int i,j;
    int gn[GRAPH_SIZE] = {0,1,2,3,4,5,6};
    int mt[GRAPH_SIZE][GRAPH_SIZE] =  {{0,1,1,1,0,0,0},{1,0,1,1,1,0,0},{1,1,0,0,0,0,0},{1,1,0,0,0,1,0},{1,0,0,0,0,0,0},{0,0,0,1,0,0,1},{0,0,0,0,0,1,0}};
    TASK_NODE(bfs) *pTaskNode;
    CALL_FUNCTION_PREPARE(bfs,pTaskNode,CALL_MODE_ARK);
    pTaskNode->matrix = malloc(sizeof(int) * GRAPH_SIZE * GRAPH_SIZE);
    pTaskNode->visited_node = malloc(sizeof(int) * GRAPH_SIZE);
    // pMatrix = (int*) smload(pTaskNode->matrix);
    for ( i = 0; i < GRAPH_SIZE; i++)
    {
        for ( j = 0; j < GRAPH_SIZE; j++)
        {
            pTaskNode->matrix[i*(GRAPH_SIZE)+j] = mt[i][j];
        }
        pTaskNode->visited_node[i] = -1;
        pTaskNode->graph_node[i] = gn[i];
    }
   if(pTaskNode == NULL) return -1;
    CALL_FUNCTION(pTaskNode,CALL_BLOCKING);
    return 0;
}

int end_func_bfs(){
    
}