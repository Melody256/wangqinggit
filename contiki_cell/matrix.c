/*
 * matrix.c
 *
 *  Created on: 2014-10-21
 *      Author: wintice
 */

#include "matrix.h"

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

int func_idx_matrix;

RECURSION_FUNCTION(matrix)
{
	FUNCTION_INIT(matrix)
	/**user viraiable should be declared here.**/
		int limit = 100;
		char my_mode;
		if (INPUTPARA(rowa) > limit || INPUTPARA(colb) > limit)
		{
			IF_FIRST_CALL()
			{
				FRAME_PRINTF("%d,devide!!\n", nCellIdx);
				if (INPUTPARA(rowa) < 200 || INPUTPARA(colb) < 200)
				{
					my_mode = CALL_MODE_CELL;
				}
				else
					my_mode = CALL_MODE_ARK;
				TASK_NODE(matrix) *node[4];
				CALL_SUBFUNCTION_PREPARE(matrix, node, 4, my_mode);
				FRAME_PRINTF("got nodes!!\n");
				int rowa1;
				rowa1 = INPUTPARA(rowa) / 2;
				int rowa2;
				rowa2 = INPUTPARA(rowa) - rowa1;
				int colb1;
				colb1 = INPUTPARA(colb) / 2;
				int colb2;
				colb2 = INPUTPARA(colb) - colb1;
				int i;
				for (i = 0; i < 4; i++)
				{
					node[i]->a = INPUTPARA(a);
					node[i]->b = INPUTPARA(b);
					node[i]->c = INPUTPARA(c);
					node[i]->a.offset = INPUTPARA(a).offset
							+ (((i + 1) > 2 ? 1 : 0) * rowa1 * INPUTPARA(cola))
									* sizeof(int);
					node[i]->b.offset = INPUTPARA(b).offset
							+ (((i + 1) % 2 == 1 ? 0 : colb1)) * sizeof(int);
					node[i]->c.offset = INPUTPARA(c).offset
							+ (((i + 1) > 2 ? 1 : 0) * rowa1
									* INPUTPARA(origincolc)
									+ ((i + 1) % 2 == 1 ? 0 : colb1))
									* sizeof(int);
					node[i]->origincolc = INPUTPARA(origincolc);
					node[i]->originrowc = INPUTPARA(originrowc);
					node[i]->cola = INPUTPARA(cola);
					node[i]->rowa = ((i + 1) > 2 ? rowa2 : rowa1);
					node[i]->colb = ((i + 1) % 2 == 1 ? colb1 : colb2);
				}
				CALL_SUBFUNCTION(node, 4);FRAME_PRINTF("%d,first call\n!!",
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
			FRAME_PRINTF("%d,it can run!\n!!", nCellIdx);
			int* matrixa = (int *) smload(INPUTPARA(a));
			int* matrixb = (int *) smload(INPUTPARA(b));
			int* matrixc = (int *) smload(INPUTPARA(c));
			int i, j, k;
			int element_temp;
			element_temp = matrixa[0];
			for (i = 0; i < INPUTPARA(rowa); i++)
			{
				for (j = 0; j < input_para->cola; j++)
				{
					element_temp = matrixa[input_para->cola * i + j];
					for (k = 0; k < INPUTPARA(colb); k++)
					{
						matrixc[i * INPUTPARA(origincolc) + k] += element_temp
								* matrixb[j * INPUTPARA(origincolc) + k];
					}
				}
			}
			smunload(INPUTPARA(a));
			smunload(INPUTPARA(b));
			smunload(INPUTPARA(c));
			NODE_FINISH()
			;FRAME_PRINTF("%d,run over!\n!!", nCellIdx);
		}
		FUNC_END();
}

int root_func_matrix()
{
	int nDimension = 0;
	SHM_PARA smMatrixa, smMatrixb, smMatrixc;
	int* pMatrixa, *pMatrixb, *pMatrixc;

	printf("please input the dimension of the matrixs:\n");
	scanf("%d", &nDimension);
	/*在共享内存区创造随机矩阵*/
	smMatrixa.nShmId = smalloc(IPC_PRIVATE,
			sizeof(int) * nDimension * nDimension);
	smMatrixb.nShmId = smalloc(IPC_PRIVATE,
			sizeof(int) * nDimension * nDimension);
	smMatrixc.nShmId = smalloc(IPC_PRIVATE,
			sizeof(int) * nDimension * nDimension);
	smMatrixa.offset = 0;
	smMatrixb.offset = 0;
	smMatrixc.offset = 0;
	if (smMatrixa.nShmId == -1 || smMatrixb.nShmId == -1
			|| smMatrixc.nShmId == -1)
	{
		printf("error:can't create the matrix space");
		return -1;
	}
	pMatrixa = (int*) smload(smMatrixa);
	pMatrixb = (int*) smload(smMatrixb);
	pMatrixc = (int*) smload(smMatrixc);

	srand((int) time(0));
	int i = 0, j = 0;
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
	smunload(smMatrixa);
	smunload(smMatrixb);
	/****创建函数调用节点***/
	TASK_NODE(matrix) *pTaskNode;
	CALL_FUNCTION_PREPARE(matrix, pTaskNode, CALL_MODE_ARK);
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->a = smMatrixa;
	pTaskNode->b = smMatrixb;
	pTaskNode->c = smMatrixc;
	pTaskNode->cola = pTaskNode->colb = pTaskNode->rowa =
			pTaskNode->originrowc = pTaskNode->origincolc = nDimension;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
//	for(i = 0;i<nDimension;i++) {
//		for(j = 0;j<nDimension;j++) {
//			printf("%d,",pMatrixc[i*nDimension+j]);
//		}
//		printf("\n");
//	}
	smunload(smMatrixc);
	smfree(smMatrixa);
	smfree(smMatrixb);
	smfree(smMatrixc);
	return 0;
}
