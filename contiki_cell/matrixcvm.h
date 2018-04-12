/*
 * matrixcvm.h
 *
 *  Created on: 2014-9-25
 *      Author: wintice
 */

#ifndef MATRIXCVM_H_
#define MATRIXCVM_H_
#include "process.h"
#include "recursion_parallel.h"
#include "cvm_api.h"

TASK_NODE(matrixcvm)
{
	NODE_HEAD()
	int originrowc;
	int origincolc;
	int rowa;
	int cola;
	int colb;
	int offseta;
	int offsetb;
	int offsetc;CVM_PARA a; //int[]
	CVM_PARA b; //int[]
	CVM_PARA c; //int[]
};

extern int func_idx_matrixcvm;

RECURSION_FUNCTION(matrixcvm);
int root_func_matrixcvm();
int end_func_matrixcvm();
#endif /* MATRIXCVM_H_ */
