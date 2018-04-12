/*
 * matrix.h
 *
 *  Created on: 2014-9-25
 *      Author: wintice
 */

#ifndef MATRIX_H_
#define MATRIX_H_
#include "process.h"
#include "recursion_parallel.h"

TASK_NODE(matrix)
{
	NODE_HEAD()
	int originrowc;
	int origincolc;
	int rowa;
	int cola;
	int colb;SHM_PARA a; //int[]
	SHM_PARA b; //int[]
	SHM_PARA c; //int[]
};

extern int func_idx_matrix;

RECURSION_FUNCTION(matrix);
int root_func_matrix();
int end_func_matrix();
#endif /* MATRIX_H_ */
