/*
 * som_train.c
 *
 *  Created on: 2014-11-23
 *      Author: wintice
 */
int func_idx_som_train;

#include "contiki_cell.h"
#include "recursion_parallel.h"
#include <sys/sem.h>
#include "som.h"
#include "som_train.h"
#include "shm_com.h"
#include "som_kohonenlayer_output.h"

RECURSION_FUNCTION(som_train)
{
	FUNCTION_INIT(som_train);
	/**user viraiable should be declared here.**/
	int limit = 100;
	int avg_subunits = 0;
	int fix_subunits = 0;
	char my_mode;
	FRAME_PRINTF("Cell %d:som train called:units %d\n!!", nCellIdx,
			INPUTPARA(KohonenLayer).Units);
	if (INPUTPARA(KohonenLayer).Units > limit)
	{
		IF_FIRST_CALL()
		{
			FRAME_PRINTF("Cell %d:devide\n!!", nCellIdx);
			if (INPUTPARA(KohonenLayer).Units < 200)
			{
				my_mode = CALL_MODE_CELL;
			}
			else
				my_mode = CALL_MODE_ARK;
			TASK_NODE(som_train) *node[9];
			CALL_SUBFUNCTION_PREPARE(som_train, node, 9, my_mode);
			avg_subunits = INPUTPARA(KohonenLayer).Units / 9;
			fix_subunits = INPUTPARA(KohonenLayer).Units % 9;
			int i;
			for (i = 0; i < 9; i++)
			{
				node[i]->InputLayer = INPUTPARA(InputLayer);
				node[i]->KohonenLayer = INPUTPARA(KohonenLayer);
				node[i]->Winner = INPUTPARA(Winner);
				node[i]->Alpha = INPUTPARA(Alpha);
				node[i]->Sigma = INPUTPARA(Sigma);
				node[i]->col = INPUTPARA(col);
				node[i]->KohonenLayer.Units = avg_subunits;
				if (i == 8)
					node[i]->KohonenLayer.Units += fix_subunits;
				node[i]->KohonenLayer.Output.offset = i * avg_subunits;
				node[i]->KohonenLayer.Weight.offset = i * avg_subunits
						* INPUTPARA(InputLayer).Units;
			}
			CALL_SUBFUNCTION(node, 9);FRAME_PRINTF("Cell %d:devide end\n!!",
					nCellIdx);
		}
		IF_WAITING_RETURN()
		{
			FRAME_PRINTF("Cell %d:second call\n!!", nCellIdx);
			NODE_FINISH()
			;
		}
	}
	else
	{
		FRAME_PRINTF("Cell %d: Start som train:units %d\n!!", nCellIdx,
				INPUTPARA(KohonenLayer).Units);
		int i, j;
		double Out, Weight, Lambda, Distance;
		SHMINPUTPARA(input, double, INPUTPARA(InputLayer).Output);
		SHMINPUTPARA(weight, double, INPUTPARA(KohonenLayer).Weight);
		for (i = 0; i < INPUTPARA(KohonenLayer).Units; i++)
		{
			for (j = 0; j < INPUTPARA(InputLayer).Units; j++)
			{
				Out = input[j];
				Weight = weight[i * INPUTPARA(InputLayer).Units + j];
				int iRow, iCol, jRow, jCol;
				iRow = (i + INPUTPARA(KohonenLayer).Output.offset)
						/ INPUTPARA(col);
				jRow = INPUTPARA(Winner) / INPUTPARA(col); //求出神经元i和获胜神经元在SOM拓扑中的行位置
				iCol = (i + INPUTPARA(KohonenLayer).Output.offset)
						% INPUTPARA(col);
				jCol = INPUTPARA(Winner) % INPUTPARA(col); //求出神经元i和获胜神经元在SOM拓扑中的列位置
				Distance = sqrt(sqr(iRow-jRow) + sqr(iCol - jCol));
				Lambda = exp(-sqr(Distance) / (2*sqr(INPUTPARA(Sigma)))); //返回的邻域符合高斯函数，参见书本定义
				weight[i * INPUTPARA(InputLayer).Units + j] += INPUTPARA(Alpha)
						* Lambda * (Out - Weight);
			}
		}
		DEL_SHM_PARA(input, INPUTPARA(InputLayer).Output);
		DEL_SHM_PARA(weight, INPUTPARA(KohonenLayer).Weight);FRAME_PRINTF(
				"Cell %d:som train end:units %d\n!!", nCellIdx,
				INPUTPARA(KohonenLayer).Units);
		NODE_FINISH()
		;
	}
	FUNC_END();
}

int root_func_som_train()
{
	return 0;
}

