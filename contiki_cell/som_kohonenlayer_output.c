/*
 * som_kohonenlayer_output.c
 *
 *  Created on: 2014-11-24
 *      Author: wintice
 */

#include <stdlib.h>
#include "contiki_cell.h"
#include "recursion_parallel.h"
#include "som.h"
#include "som_kohonenlayer_output.h"
int func_idx_som_kohonenlayer_output;

RECURSION_FUNCTION(som_kohonenlayer_output)
{
	FUNCTION_INIT(som_kohonenlayer_output);
	/**user viraiable should be declared here.**/
	int limit = 100;
	int avg_subunits = 0;
	int fix_subunits = 0;
	char my_mode;
	if (INPUTPARA(KohonenLayer).Units > limit)
	{
		IF_FIRST_CALL()
		{
			FRAME_PRINTF("%d,devide\n!!", nCellIdx);
			if (INPUTPARA(KohonenLayer).Units < 200)
			{
				my_mode = CALL_MODE_CELL;
			}
			else
				my_mode = CALL_MODE_ARK;
			TASK_NODE(som_kohonenlayer_output) *node[9];
			CALL_SUBFUNCTION_PREPARE(som_kohonenlayer_output, node, 9, my_mode);
			avg_subunits = INPUTPARA(KohonenLayer).Units / 9;
			fix_subunits = INPUTPARA(KohonenLayer).Units % 9;
			int i;
			for (i = 0; i < 9; i++)
			{
				node[i]->InputLayer = INPUTPARA(InputLayer);
				node[i]->KohonenLayer = INPUTPARA(KohonenLayer);
				node[i]->KohonenLayer.Units = avg_subunits;
				if (i == 8)
					node[i]->KohonenLayer.Units += fix_subunits;
				node[i]->KohonenLayer.Output.offset = i * avg_subunits;
				node[i]->KohonenLayer.Weight.offset = i * avg_subunits
						* INPUTPARA(InputLayer).Units;
			}
			CALL_SUBFUNCTION(node, 9);FRAME_PRINTF("%d,devide end\n!!",
					nCellIdx);
		}
		IF_WAITING_RETURN()
		{
			FRAME_PRINTF("%d,second call\n!!", nCellIdx);
			NODE_FINISH()
			;
		}
	}
	else
	{
		FRAME_PRINTF(
				"Cell %d: start doing the kohonen layer output:units: %d\n!!",
				nCellIdx, INPUTPARA(KohonenLayer).Units);
		int i, j;
		double Sum;
		SHMINPUTPARA(input, double, INPUTPARA(InputLayer).Output);
		SHMINPUTPARA(output, double, INPUTPARA(KohonenLayer).Output);
		SHMINPUTPARA(weight, double, INPUTPARA(KohonenLayer).Weight);
		for (i = 0; i < INPUTPARA(KohonenLayer).Units; i++)
		{
			Sum = 0;
			for (j = 0; j < INPUTPARA(InputLayer).Units; j++)
			{
				Sum += sqr(input[j]-weight[i*INPUTPARA(InputLayer).Units+j]);
			}
			output[i] = sqrt(Sum);
		}
		DEL_SHM_PARA(input, INPUTPARA(InputLayer).Output);
		DEL_SHM_PARA(output, INPUTPARA(KohonenLayer).Output);
		DEL_SHM_PARA(weight, INPUTPARA(KohonenLayer).Weight);FRAME_PRINTF(
				"Cell %d: the kohonen layer output done:units: %d\n!!",
				nCellIdx, INPUTPARA(KohonenLayer).Units);
		NODE_FINISH()
		;
	}
	FUNC_END();
}

int root_func_som_kohonenlayer_output()
{
	return 0;
}
