/*
 * matrixcvm.c
 *
 *  Created on: 2014-10-21
 *      Author: wintice
 */

#include "matrixcvm.h"

#include "contiki_cell.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <time.h>

#include "sem_com.h"
#include "shm_com.h"

int func_idx_matrixcvm;

RECURSION_FUNCTION(matrixcvm)
{
	FUNCTION_INIT(matrixcvm)
	/**user viraiable should be declared here.**/
		int limitsky = 4000;
		int limitark = 1000;
		int limitcell = 1;
		char my_mode1, my_mode2;
		if (INPUTPARA(rowa) > limitcell)
		{
			IF_FIRST_CALL()
			{
				FRAME_PRINTF("%d,devide\n!!", nCellIdx);
				if (INPUTPARA(rowa) < limitark || INPUTPARA(colb) < limitark)
				{
					my_mode1 = my_mode2 = CALL_MODE_CELL;
				}
				else if (INPUTPARA(rowa) < limitsky
						|| INPUTPARA(colb) < limitsky)
				{
					my_mode1 = my_mode2 = CALL_MODE_ARK;
				}
				else
				{
					my_mode1 = CALL_MODE_ARK;
					my_mode2 = CALL_MODE_SKY;
				}
				TASK_NODE(matrixcvm) *node[INPUTPARA(originrowc)];
				CALL_SUBFUNCTION_PREPARE(matrixcvm, node, (INPUTPARA(originrowc)/2), my_mode1);
				CALL_SUBFUNCTION_PREPARE(matrixcvm, &(node[(INPUTPARA(originrowc)/2)]), (INPUTPARA(originrowc)-INPUTPARA(originrowc)/2), my_mode2);
				int i;
				for (i = 0; i < INPUTPARA(originrowc); i++)
				{
					node[i]->origincolc = INPUTPARA(origincolc);
					node[i]->originrowc = INPUTPARA(originrowc);
					node[i]->cola = INPUTPARA(cola);
					node[i]->rowa = 1;
					node[i]->colb = INPUTPARA(colb);
					node[i]->a = INPUTPARA(a);
					node[i]->b = INPUTPARA(b);
					node[i]->c = INPUTPARA(c);
					node[i]->offseta = INPUTPARA(offseta)
							+ (i * INPUTPARA(cola));
					node[i]->offsetb = INPUTPARA(offsetb);
					//node[i]->offsetc = INPUTPARA(offsetc)
					//		+ (((i + 1) > 2 ? 1 : 0) * rowa1
					//				* INPUTPARA(origincolc)
					//				+ ((i + 1) % 2 == 1 ? 0 : colb1));
                    node[i]->c.uSize = INPUTPARA(colb)*sizeof(int);
                    node[i]->c.offset = node[i]->offseta*sizeof(int); 
				}
				CALL_SUBFUNCTION(node, INPUTPARA(originrowc));
                FRAME_PRINTF("%d,first call\n!!",
						nCellIdx);
			}
			IF_WAITING_RETURN()
			{
				if (INPUTPARA(origincolc) == INPUTPARA(colb))
				{
					FRAME_PRINTF("%d,matrixcvm end\n!!", nCellIdx);
					//cvmload(&INPUTPARA(c));
					//cvmsave(&INPUTPARA(c));
					//cvmunload(&INPUTPARA(c));
				}
				NODE_FINISH()
				;
			}
		}
		else
		{
			FRAME_PRINTF("%d,it can run!\n!!", nCellIdx);
			int* matrixa =
					&(((int *) cvmload(&INPUTPARA(a)))[INPUTPARA(offseta)]);
			int* matrixb =
					&(((int *) cvmload(&INPUTPARA(b)))[INPUTPARA(offsetb)]);
			int* matrixc =
					(int *) cvmload(&INPUTPARA(c));
			int i, j, k;
			int element_temp;
			element_temp = matrixa[0];
			for (i = 0; i < INPUTPARA(rowa); i++)
			{
				for (j = 0; j < INPUTPARA(cola); j++)
				{
					element_temp = matrixa[input_para->cola * i + j];
					for (k = 0; k < INPUTPARA(colb); k++)
					{
						matrixc[i * INPUTPARA(origincolc) + k] += element_temp
								* matrixb[j * INPUTPARA(origincolc) + k];
					}
				}
			}
            cvmsave(&INPUTPARA(c));
			cvmunload(&INPUTPARA(a));
			cvmunload(&INPUTPARA(b));
			cvmunload(&INPUTPARA(c));
			NODE_FINISH()
			;FRAME_PRINTF("%d,run over!\n!!", nCellIdx);
		}
		FUNC_END();
}

int root_func_matrixcvm()
{
	int nDimension = 0;
	CVM_PARA cvmMatrixa, cvmMatrixb, cvmMatrixc;
	int* pMatrixa, *pMatrixb, *pMatrixc;
	cvmMatrixa.Ark_Idx = cvmMatrixb.Ark_Idx = cvmMatrixc.Ark_Idx = g_nArkIdx;
	printf("please input the dimension of the matrixs:\n");
	scanf("%d", &nDimension);
	cvmMatrixa.uSize = cvmMatrixb.uSize = cvmMatrixc.uSize = nDimension
			* nDimension * sizeof(int);
	/*在共享内存区创造随机矩阵*/
	cvmalloc(&cvmMatrixa);
	cvmalloc(&cvmMatrixb);
	cvmalloc(&cvmMatrixc);
	pMatrixa = (int*) cvmload(&cvmMatrixa);
	pMatrixb = (int*) cvmload(&cvmMatrixb);
	pMatrixc = (int*) cvmload(&cvmMatrixc);

	srand((int) time(0));
	int i = 0, j = 0, k = 0;
	for (i = 0; i < nDimension * nDimension; i++)
	{
		pMatrixa[i] = rand() % 10;
		//printf("%d,",);
	}
//	printf("\n");
	for (i = 0; i < nDimension * nDimension; i++)
	{
		pMatrixb[i] = rand() % 10;
		//printf("%d,",);
	}
//	printf("\n");
	for (i = 0; i < nDimension * nDimension; i++)
	{
		pMatrixc[i] = 0;
	}
	/****创建函数调用节点***/
	TASK_NODE(matrixcvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(matrixcvm, pTaskNode, CALL_MODE_SKY);
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->a = cvmMatrixa;
	pTaskNode->b = cvmMatrixb;
	pTaskNode->c = cvmMatrixc;
	pTaskNode->offseta = pTaskNode->offsetb = pTaskNode->offsetc = 0;
	pTaskNode->cola = pTaskNode->colb = pTaskNode->rowa =
			pTaskNode->originrowc = pTaskNode->origincolc = nDimension;
	int matrixc;
	char cCorrectFlag = 1;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	for (i = 0; i < nDimension; i++)
	{
		for (j = 0; j < nDimension; j++)
		{
			matrixc = 0;
			for (k = 0; k < nDimension; k++)
			{
				matrixc += pMatrixa[i * nDimension + k]
						* pMatrixb[k * nDimension + j];
			}
			if (matrixc != pMatrixc[i * nDimension + j])
				cCorrectFlag = 0; //printf("Not correct Answer!::%d,%d\n",matrixc,pMatrixc[i*nDimension+j]);
			//else
			//printf("Right!\n");
		}
	}
	if (cCorrectFlag == 1)
		printf("Correct!");
	else
		printf("Not correct!");
//	for(i = 0;i<nDimension;i++) {
//		for(j = 0;j<nDimension;j++) {
//			printf("%d,",pMatrixc[i*nDimension+j]);
//		}
//		printf("\n");
//	}
	cvmunload(&cvmMatrixa);
	cvmunload(&cvmMatrixb);
	cvmunload(&cvmMatrixc);
	cvmfree(&cvmMatrixa);
	cvmfree(&cvmMatrixb);
	cvmfree(&cvmMatrixc);
	return 0;
}
