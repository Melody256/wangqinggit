/*
 * som_train.h
 *
 *  Created on: 2014-11-23
 *      Author: wintice
 */

#ifndef SOM_TRAIN_H_
#define SOM_TRAIN_H_

#include "som.h"

TASK_NODE(som_train)
{
	NODE_HEAD()
	LAYER InputLayer; /* - 输入层                                   */
	LAYER KohonenLayer; /* - Kohonen层                                */
	int col;
	int Winner; /*上一次训练的获胜节点                       */
	REAL Alpha; /* -Kohonen层的学习率                       */
	REAL Sigma; /* -求获胜节点邻域时使用的参数 */
};

extern int func_idx_som_train;
RECURSION_FUNCTION (som_train);
int root_func_som_train();
int end_func_som_train();
#endif /* SOM_TRAIN_H_ */
