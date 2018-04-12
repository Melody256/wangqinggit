/*
 * cvm_api.c
 *
 *  Created on: 2015-3-28
 *      Author: wintice
 */

#include "contiki_cell.h"
#include "shm_com.h"
#include "cvm.h"
#include "cvm_api.h"

int __cvmload(PCVMPT _pCvmPt, char ispreload);

int cvmalloc(PCVMPT _pCvmPt)
{
	if (g_nArkIdx <= 0 || _pCvmPt->uSize > 0x80000000)
		return -1;
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	if (pTaskNode == NULL)
		return -1;
	SMPT tCvmPt;
	tCvmPt.nShmId = smalloc(0, sizeof(_CVMPT));
	tCvmPt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(tCvmPt);
	pCvmPt->Ark_Idx = _pCvmPt->Ark_Idx;
	pCvmPt->uSize = _pCvmPt->uSize;
	pTaskNode->cCvmOp = CVM_ALLOC;
	pTaskNode->pCvmPt = tCvmPt;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	if (pCvmPt->cState != CVM_UNLOADED)
	{
		printf("Eroor:cvmaloc failed.\n");
		return -1;
	}
	_pCvmPt->nCvmId = pCvmPt->nCvmId;
	_pCvmPt->offset = pCvmPt->offset;
	_pCvmPt->tCache = pCvmPt->tCache;
	_pCvmPt->cState = pCvmPt->cState;
	smunload(tCvmPt);
	smfree(tCvmPt);
	return 0;
}

int __cvmload(PCVMPT _pCvmPt, char ispreload)
{
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->cCvmOp = CVM_LOAD;
	SMPT smpt_cvmpt;
	smpt_cvmpt.nShmId = smalloc(0, sizeof(CVMPT));
	smpt_cvmpt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(smpt_cvmpt);
	*pCvmPt = *_pCvmPt;
	pTaskNode->pCvmPt = smpt_cvmpt;
	CALL_FUNCTION(pTaskNode, !(ispreload));
	int result = 0;
	if (ispreload)
	{
		while ((pCvmPt->cState != CVM_LOADING) && (pCvmPt->cState != CVM_LOADED))
			;
	}
	else
	{
		if (pCvmPt->cState != CVM_LOADED)
		{
			printf("cvmload() failed.");
			result = -1;
		}
	}
	*_pCvmPt = *pCvmPt;
	smunload(smpt_cvmpt);
	smfree(smpt_cvmpt);
	return result;
}

int cvmpreload(PCVMPT _pCvmPt)
{
	_pCvmPt->cState = CVM_UNLOADED;
	__cvmload(_pCvmPt, 1);
	return 0;
}

void * cvmload(PCVMPT _pCvmPt)
{
	FRAME_PRINTF("Cell %d:cvmload() has been called.\n", nCellIdx);
	_pCvmPt->cState = CVM_UNLOADED;
	__cvmload(_pCvmPt, 0);
	if (_pCvmPt->cState == CVM_LOADED)
	{
		void* cache = smload(_pCvmPt->tCache);
		FRAME_PRINTF("cvmload() end.\n");
		return cache;
	}
	else
	{
		return NULL;
	}
}

int cvmsave(PCVMPT _pCvmPt)
{

	FRAME_PRINTF("cvmsave() has been called.\n");
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	SMPT tCvmPt;
	tCvmPt.nShmId = smalloc(0, sizeof(_CVMPT));
	tCvmPt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(tCvmPt);
	*pCvmPt = *_pCvmPt;
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->cCvmOp = CVM_SAVE;
	pTaskNode->pCvmPt = tCvmPt;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	smunload(tCvmPt);
	smfree(tCvmPt);
	FRAME_PRINTF("cvmsave() end.\n");
	return 0;
}

int cvmunload(PCVMPT _pCvmPt)
{
	FRAME_PRINTF("cvmunload() has been called.\n");
	smunload(_pCvmPt->tCache);
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	SMPT tCvmPt;
	tCvmPt.nShmId = smalloc(0, sizeof(_CVMPT));
	tCvmPt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(tCvmPt);
	*pCvmPt = *_pCvmPt;
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->cCvmOp = CVM_UNLOAD;
	pTaskNode->pCvmPt = tCvmPt;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	smunload(tCvmPt);
	smfree(tCvmPt);
	FRAME_PRINTF("cvmunload() end.\n");
	return 0;
}

int cvmfree(PCVMPT _pCvmPt)
{
	TASK_NODE(cvm) *pTaskNode;
	CALL_FUNCTION_PREPARE(cvm, pTaskNode, CALL_MODE_ARK);
	SMPT tCvmPt;
	tCvmPt.nShmId = smalloc(0, sizeof(_CVMPT));
	tCvmPt.offset = 0;
	_PCVMPT pCvmPt = (_PCVMPT) smload(tCvmPt);
	*pCvmPt = *_pCvmPt;
	if (pTaskNode == NULL)
		return -1;
	pTaskNode->cCvmOp = CVM_FREE;
	;
	pTaskNode->pCvmPt = tCvmPt;
	CALL_FUNCTION(pTaskNode, CALL_BLOCKING);
	smunload(tCvmPt);
	smfree(tCvmPt);
	return 0;
}
