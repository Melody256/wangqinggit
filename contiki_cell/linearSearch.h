/*
    linaerSearch.h
    Created on: 2018-3-20
        Author: Qing
*/

#ifndef LINEARSEARCH_H_
#define LINEARSEARCH_H_ 
#include "process.h"
#include "recursion_parallel.h"

TASK_NODE(linearSearch){
    NODE_HEAD()
    int left;
    int right;
    SHM_PARA a; // int[]
    SHM_PARA k; // int
    SHM_PARA ans; // int           
};

extern int func_idx_linearSearch;

RECURSION_FUNCTION(linearSearch);
int root_func_linearSearch();
int end_func_linearSearch();

#endif /* LINEARSEARCH_H_ */
