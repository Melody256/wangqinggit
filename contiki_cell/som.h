/*
 * som.h
 *
 *  Created on: 2014-11-24
 *      Author: wintice
 */

#ifndef SOM_H_
#define SOM_H_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
typedef char BOOL;
typedef double REAL;

#define FALSE         0
#define TRUE          1
#define NOT           !
#define AND           &&
#define OR            ||

#define MIN_REAL      -HUGE_VAL
#define MAX_REAL      +HUGE_VAL
#define MIN(x,y)      ((x)<(y) ? (x) : (y))
#define MAX(x,y)      ((x)>(y) ? (x) : (y))

#define PI            (2*asin(1))
#define sqr(x)        ((x)*(x))
#define exp(x)        pow(2.718,(x))

#define INPUT_DIM             2   //定义输入的维数

typedef struct
{ /* 定义SOM中的层                                                       */
	int Units; /* 该层的单元数（Kohonen层对应于神经元数，输入层对应于输入维度）       */
	SHM_PARA Output; /* 定义第i个神经元的输出                                               */
	SHM_PARA Weight; /* 与第i个神经元相连的权值                                             */
} LAYER;

typedef struct
{ /* 定义SOM网络:                                */
	LAYER InputLayer; /* - 输入层                                   */
	LAYER KohonenLayer; /* - Kohonen层                                */
	int col;
	int Winner; /*上一次训练的获胜节点                       */
	REAL Alpha; /* -Kohonen层的学习率                       */
	REAL Sigma; /* -求获胜节点邻域时使用的参数 */
} NET;

TASK_NODE(som)
{
	NODE_HEAD()
	int origin_depth;
	int depth;
	SHM_PARA net;
	SHM_PARA data;
};

extern int func_idx_som;

void InitializeRandoms();
REAL RandomEqualREAL(REAL Low, REAL High);
void SetInput(NET* Net, REAL Data[][10], int k);
RECURSION_FUNCTION (som);
void FindWinner(NET* Net);
int root_func_som();
#endif /* SOM_H_ */
