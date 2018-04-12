/*
    SearchBST.h
    Created on: 2018-4-10
        Author: Qing
*/

#ifndef SEARCHBST_H_
#define SEARCHBST_H_ 
#include "process.h"
#include "recursion_parallel.h"

TASK_NODE(SearchBST){
    NODE_HEAD()
    int left;
    int right;
    SHM_PARA data;
    SHM_PARA lchild;
    SHM_PARA rchild;  //int[]           
};

extern int func_idx_SearchBST;

RECURSION_FUNCTION(SearchBST);
int root_func_SearchBST();
int end_func_SearchBST();

#endif /* SEARCHBST_H_ */
