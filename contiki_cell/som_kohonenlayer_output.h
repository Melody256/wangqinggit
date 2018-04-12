/*
 * som_kohonenlayer_output.h
 *
 *  Created on: 2014-11-24
 *      Author: wintice
 */

#ifndef SOM_KOHONENLAYER_OUTPUT_H_
#define SOM_KOHONENLAYER_OUTPUT_H_

#include "recursion_parallel.h"
typedef char BOOL;
typedef double REAL;

TASK_NODE(som_kohonenlayer_output)
{
	NODE_HEAD()
	LAYER InputLayer; /* - 输入层                                   */
	LAYER KohonenLayer; /* - Kohonen层                                */
};
int func_idx_som_kohonenlayer_output;
RECURSION_FUNCTION(som_kohonenlayer_output);
int root_func_som_kohonenlayer_output();
int end_func_som_kohonenlayer_output();
#endif /* SOM_KOHONENLAYER_OUTPUT_H_ */
