/*
 * som.c
 *
 *  Created on: 2014-11-24
 *      Author: wintice
 */

#include <time.h>
#include "contiki_cell.h"
#include "recursion_parallel.h"
#include "som.h"
#include "som_train.h"
#include "som_kohonenlayer_output.h"

int func_idx_som;

static REAL Data[2][10] =
{
		{ 0.5512, 0.5123, 0.5087, 0.5001, 0.6012, 0.5298, 0.5000, 0.4965,
				0.5103, 0.5003 },
		{ 0.4488, 0.4877, 0.4913, 0.4999, 0.3988, 0.4702, 0.5000, 0.5035,
				0.4897, 0.4997 } };

RECURSION_FUNCTION(som)
{
	FUNCTION_INIT(som)
	/**user viraiable should be declared here.**/
		static const double Alpha_init = 0.1;
		static const double Sigma_init = 4.5;
		InitializeRandoms();
		if (INPUTPARA(depth) >= 1)
		{
			IF_FIRST_CALL()
			{
				//LastStep = INPUTPARA(depth)-1
				TASK_NODE(som) *node[1];
				CALL_SUBFUNCTION_PREPARE(som, node, 1, CALL_MODE_ARK);
				node[0]->origin_depth = INPUTPARA(origin_depth);
				node[0]->depth = INPUTPARA(depth) - 1;
				node[0]->net = INPUTPARA(net);
				node[0]->data = INPUTPARA(data);
				CALL_SUBFUNCTION(node, 1);
			}
			IF_WAITING_RETURN()
			{
				printf("Steps:%d/%d\n", INPUTPARA(depth),
						INPUTPARA(origin_depth));
				SHMINPUTPARA(whole_net, NET, INPUTPARA(net));
				whole_net->Alpha =
						Alpha_init
								* exp(
										-((REAL)INPUTPARA(depth))/INPUTPARA(origin_depth));
				whole_net->Sigma =
						Sigma_init
								* exp(
										-((REAL)INPUTPARA(depth))/INPUTPARA(origin_depth));
				SetInput(whole_net, Data, (int) RandomEqualREAL(0, 10));
				TASK_NODE(som_kohonenlayer_output) *node[1];
				CALL_SUBFUNCTION_PREPARE(som_kohonenlayer_output, node, 1,
						CALL_MODE_ARK);
				node[0]->InputLayer = whole_net->InputLayer;
				node[0]->KohonenLayer = whole_net->KohonenLayer;
				FRAME_PRINTF("call som kohonen layer output:%d\n",
						whole_net->KohonenLayer.Units);
				CALL_SUBFUNCTION(node, 1);
				DEL_SHM_PARA(whole_net, INPUTPARA(net));
			}
		}
		else
		{
			IF_FIRST_CALL()
			{
				printf("Steps:%d/%d\n", INPUTPARA(depth),
						INPUTPARA(origin_depth));
				SHMINPUTPARA(whole_net, NET, INPUTPARA(net));
				whole_net->Alpha = Alpha_init;
				whole_net->Sigma = Sigma_init;
				SetInput(whole_net, Data, (int) RandomEqualREAL(0, 10));
				static TASK_NODE(som_kohonenlayer_output) *node[1];
				CALL_SUBFUNCTION_PREPARE(som_kohonenlayer_output, node, 1,
						CALL_MODE_ARK);
				node[0]->InputLayer = whole_net->InputLayer;
				node[0]->KohonenLayer = whole_net->KohonenLayer;
				FRAME_PRINTF("call som kohonen layer output:%d\n",
						whole_net->KohonenLayer.Units);
				CALL_SUBFUNCTION(node, 1);
				DEL_SHM_PARA(whole_net, INPUTPARA(net));
			}
		}
		IF_WAITING_RETURN()
		{

			SHMINPUTPARA(whole_net, NET, INPUTPARA(net));
			FindWinner(whole_net);
			static TASK_NODE(som_train) *node[1];
			CALL_SUBFUNCTION_PREPARE(som_train, node, 1, CALL_MODE_ARK);
			node[0]->InputLayer = whole_net->InputLayer;
			node[0]->KohonenLayer = whole_net->KohonenLayer;
			node[0]->Winner = whole_net->Winner;
			node[0]->Alpha = whole_net->Alpha;
			node[0]->Sigma = whole_net->Sigma;
			node[0]->col = whole_net->col;
			FRAME_PRINTF("call som train:%d\n", whole_net->KohonenLayer.Units);
			CALL_SUBFUNCTION(node, 1);
			DEL_SHM_PARA(whole_net, INPUTPARA(net));
		}
		IF_WAITING_RETURN()
		{
			FRAME_PRINTF("Steps:%d end.\n", INPUTPARA(depth));
			NODE_FINISH()
			;
		}
		FUNC_END();
}

/******************************************************************************
 随机数值生成函数
 ******************************************************************************/

void InitializeRandoms()
{
	srand((unsigned int) time(0));  //为rand()函数设置生成伪随机数列的种子
}

REAL RandomEqualREAL(REAL Low, REAL High)
{
	return ((REAL) rand() / RAND_MAX) * (High - Low) + Low;
}

//REAL RandomNormalREAL(REAL Mu, REAL Sigma)
//{
//	REAL x,fx;
//
//	do {
//		x = RandomEqualREAL(Mu-3*Sigma, Mu+3*Sigma);
//		fx = (1 / (sqrt(2*PI)*Sigma)) * exp(-sqr(x-Mu) / (2*sqr(Sigma)));
//	}while (fx < RandomEqualREAL(0, 1));
//	return x;
//}

/******************************************************************************
 设置SOM输入和输出函数
 ******************************************************************************/

void SetInput(NET* Net, REAL Data[][10], int k)
{
	int i;
	double* output_inputlayer = (double *) smload(Net->InputLayer.Output);
	for (i = 0; i < Net->InputLayer.Units; i++)
	{
		output_inputlayer[i] = Data[i][k];    //输入层的输出直接等于样本输入值
	}
	smunload(Net->InputLayer.Output);
}

/******************************************************************************
 寻找获胜节点
 ******************************************************************************/
void FindWinner(NET* Net)
{
	int i;
	double* output_kohonenlayer = (double*) smload(Net->KohonenLayer.Output);
	double MinOut;
	MinOut = 5000; //MAX_REAL只是设置的一个初始比较值，会立即被下一个比它小的数值替代，设置的合理大即可
	for (i = 0; i < Net->KohonenLayer.Units; i++)
	{
		if (output_kohonenlayer[i] < MinOut)
		{
			Net->Winner = i;
			MinOut = output_kohonenlayer[i];
		}
	}
	smunload(Net->KohonenLayer.Output);
}

int root_func_som()
{
	return 0;
}
