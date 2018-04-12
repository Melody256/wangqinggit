/*
 * sky.c
 *
 *  Created on: 2015-4-6
 *      Author: wintice
 */

#include "contiki_cell.h"
#include <stdio.h>
#include "cvm.h"
#include <stddef.h>

int TaskMigrate(int shmid, size_t size)
{
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	SMPT tCvmPt;
	tCvmPt.nShmId = smalloc(0, sizeof(_CVMPT));
	tCvmPt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(tCvmPt);
	pCvmPt->tCache.nShmId = shmid;
	pCvmPt->uSize = size;
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->cCvmOp = SKY_CALL;
	pTaskNode->pCvmPt = tCvmPt;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	smunload(tCvmPt);
	smfree(tCvmPt);
	return 0;
}

int SkyFuncReturn(int shmid, int nArkIdx)
{
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	SMPT tCvmPt;
	tCvmPt.nShmId = smalloc(0, sizeof(_CVMPT));
	tCvmPt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(tCvmPt);
	pCvmPt->tCache.nShmId = shmid;
	pCvmPt->Ark_Idx = nArkIdx;
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->cCvmOp = SKY_RETURN;
	pTaskNode->pCvmPt = tCvmPt;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	smunload(tCvmPt);
	smfree(tCvmPt);
	return 0;
}

