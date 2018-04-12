/*
 * cvm.c
 *
 *  Created on: 2014-12-1
 *      Author: wintice
 */

#include <math.h>
#include "contiki_cell.h"
#include "cvm.h"
#include "cvm_api.h"
#include "fcntl.h"
#include "client.h"

char FUNC_SWITCHER_cvm = 0;
int func_idx_cvm = 0;
PCVMPAGE gCvmPages[16] =
{ NULL };
PCVMBLOCK pBlockStateTable = NULL;
PCVMMAP pBlockMapTable = NULL;
pthread_mutex_t gCvmMutex;

#define MAP2PT(map,pt) \
	pt.nShmId = smalloc(IPC_PRIVATE,sizeof(_CVMPT));\
	pt.offset=0;\
	_PCVMPT ADDR = (_PCVMPT)smload(pt);\
	ADDR->nCvmId=map->nCvmId;\
	ADDR->Ark_Idx=map->Ark_Idx;\
	ADDR->offset=map->offset;\
	ADDR->uSize=map->uSize;\
	ADDR->tCache=map->tCache;\
	ADDR->cState=map->cState;\
	smunload(pt);

RECURSION_FUNCTION(cvm)
{
	FUNCTION_INIT(cvm)
		PCVMMAP pMapNodePrev;
		PCVMMAP pMapNode;
		_PCVMPT pCvmPt = (_PCVMPT) smload(INPUTPARA(pCvmPt));
		uint8_t uPageIdx;
		uint32_t uBlockOffset;
		pthread_mutex_lock(&gCvmMutex);
		switch (INPUTPARA(cCvmOp))
		{
		case SKY_CALL:
			client(INPUTPARA(cCvmOp), pCvmPt->tCache.nShmId, pCvmPt->uSize);
			break;
		case SKY_RETURN:
			client(INPUTPARA(cCvmOp), pCvmPt->tCache.nShmId, pCvmPt->Ark_Idx);
			break;
		case CVM_ALLOC:
			if (pCvmPt->Ark_Idx != g_nArkIdx)
			{
				client(INPUTPARA(cCvmOp), INPUTPARA(pCvmPt).nShmId,
						pCvmPt->Ark_Idx);
			}
			else
			{
				int nSectorsNeed = ceil(pCvmPt->uSize / 512.0);
				uint8_t i = 0;
				for (i = 0; i < 16; i++)
				{
					if (gCvmPages[i] == NULL)
					{
						char strCvmFile[7];
						sprintf(strCvmFile, "%c%c.cvm", i / 10 + '0',
								i % 10 + '0');
						while ((gCvmPages[i] = malloc(sizeof(CVMPAGE))) == NULL)
							;
						while ((gCvmPages[i]->cvmio = fopen(strCvmFile, "wb+"))
								== NULL)
							;
						memset(gCvmPages[i]->cHalfKBTable, 0,
								0x400000 * sizeof(char));
						gCvmPages[i]->nSectorsUsed = 0;
					}
					if ((gCvmPages[i]->nSectorsUsed)
							<= (0x400000 - nSectorsNeed))
					{
						int unused = 0, j = 0;
						for (j = 0; j < 0x400000; j++)
						{
							if (gCvmPages[i]->cHalfKBTable[j] == 0)
							{
								unused++;
								if (unused == nSectorsNeed)
									break;
							}
							else
							{
								unused = 0;
							}
						}
						if (unused == nSectorsNeed)
						{
							pCvmPt->Ark_Idx = g_nArkIdx;
							pCvmPt->tCache.nShmId = -1;
							pCvmPt->tCache.offset = 0;
							gCvmPages[i]->nSectorsUsed += nSectorsNeed;
							for (; unused> 0; unused--)
							{
								gCvmPages[i]->cHalfKBTable[j--] = 1;
							}
							pCvmPt->nCvmId = (i << 23) + (j + 1);
							pCvmPt->offset = 0;
							pCvmPt->cState = CVM_UNLOADED;
							PCVMBLOCK BlockNode = (PCVMBLOCK) malloc(
									sizeof(CVMBLOCK));
							BlockNode->nCvmId = pCvmPt->nCvmId;
							BlockNode->nSectorsNum = nSectorsNeed;
							BlockNode->cFreeMark = 0;
							BlockNode->nAttachCount = 0;
							BlockNode->next = pBlockStateTable->next;
							pBlockStateTable->next = BlockNode;
							break;
						}
					}
				}
				if (i == 16)
				{
					pCvmPt->nCvmId = -1;
				}
			}
			break;
		case CVM_LOAD:
			FRAME_PRINTF("CVM_LOAD!\n")
			;
			pMapNode = pBlockMapTable->next;
			for (; pMapNode != NULL; pMapNode = pMapNode->next)
			{
                //FRAME_PRINTF("offset:%d\n", pCvmPt->offset);
                //FRAME_PRINTF("size:%d\n", pCvmPt->uSize);
				if (pMapNode->nCvmId == pCvmPt->nCvmId
						&& pMapNode->offset <= pCvmPt->offset
						&& pMapNode->uSize
								>=(pCvmPt->offset - pMapNode->offset + pCvmPt->uSize)
						&& pMapNode->Ark_Idx == pCvmPt->Ark_Idx)
				{
                    FRAME_PRINTF("CVM_LOAD:Got a mother map!\n");
					while (pMapNode->cState != CVM_LOADED)
						;
					pCvmPt->tCache = pMapNode->tCache;
					pCvmPt->cState = pMapNode->cState;
					pCvmPt->tCache.offset += pCvmPt->offset - pMapNode->offset;
					pMapNode->nAttachCount++;
					break;
				}
			}
			if (pMapNode == NULL)
			{
				FRAME_PRINTF("Create a new map!\n");
				uint8_t uPageIdx = (pCvmPt->nCvmId >> 23) & 0x0f;
				uint32_t uBlockOffset = ((pCvmPt->nCvmId) & 0x7fffffU) * 512U;
				int nMapShmId;
				while ((nMapShmId = smalloc(IPC_PRIVATE, pCvmPt->uSize)) == -1)
				{
                    FRAME_PRINTF("smalloc failed.Try to release some space.\n");
					pMapNodePrev = pBlockMapTable;
					pMapNode = pBlockMapTable->next;
					for (; pMapNode != NULL;)
					{
						if (pMapNode->nAttachCount == 0)
						{
							if (pMapNode->Ark_Idx != g_nArkIdx)
							{
								SMPT shmidtemp;
								MAP2PT(pMapNode, shmidtemp);
								client(CVM_SAVE, shmidtemp.nShmId,
										pMapNode->Ark_Idx);
								smfree(shmidtemp);
							}
							else
							{
								uPageIdx = (pMapNode->nCvmId >> 23) & 0x0f;
								uBlockOffset = ((pMapNode->nCvmId) & 0x7fffffU)
										* 512U;
								void *cache = smload(pMapNode->tCache);
								fseek(gCvmPages[uPageIdx]->cvmio,
										uBlockOffset + pMapNode->offset,
										SEEK_SET);
								fwrite(cache, pMapNode->uSize, 1,
										gCvmPages[uPageIdx]->cvmio);
								fflush(gCvmPages[uPageIdx]->cvmio);
								smunload(pMapNode->tCache);
							}FRAME_PRINTF(
									"cvm-map-node not in used, free it.\n");
							smfree(pMapNode->tCache);
							pMapNodePrev->next = pMapNode->next;
							free(pMapNode);
						}
						else
							pMapNodePrev = pMapNode;
						pMapNode = pMapNodePrev->next;
					}
				}
				if (nMapShmId != -1)
				{
                    FRAME_PRINTF("New Map ShmId:%d\n",nMapShmId);
					PCVMBLOCK pBlockNode = pBlockStateTable->next;
					if (pCvmPt->Ark_Idx == g_nArkIdx)
					{
						for (; pBlockNode != NULL; pBlockNode =
								pBlockNode->next)
						{
							if (pBlockNode->nCvmId == pCvmPt->nCvmId)
							{
								pBlockNode->nAttachCount++;
								break;
							}
						}
					}
					if (pBlockNode != NULL || pCvmPt->Ark_Idx != g_nArkIdx)
					{
						pMapNode = (PCVMMAP) malloc(sizeof(CVMMAP));
						pMapNode->nCvmId = pCvmPt->nCvmId;
						pMapNode->Ark_Idx = pCvmPt->Ark_Idx;
						pMapNode->offset = pCvmPt->offset;
						pMapNode->uSize = pCvmPt->uSize;
						pMapNode->cState = pCvmPt->cState = CVM_LOADING;
						pMapNode->nAttachCount = 1;
						pMapNode->next = pBlockMapTable->next;
						pBlockMapTable->next = pMapNode;
						pCvmPt->tCache.nShmId = pMapNode->tCache.nShmId =
								nMapShmId;
						pCvmPt->tCache.offset = pMapNode->tCache.offset = 0;
						if (pCvmPt->Ark_Idx != g_nArkIdx)
						{
							FRAME_PRINTF(
									"Try to get the cvm data from other ark.\n");
							client(INPUTPARA(cCvmOp), INPUTPARA(pCvmPt).nShmId,
									pCvmPt->Ark_Idx);
						}
						else
						{
							void *cache = smload(pCvmPt->tCache);
							fseek(gCvmPages[uPageIdx]->cvmio,
									uBlockOffset + pCvmPt->offset, SEEK_SET);
							fread(cache, pCvmPt->uSize, 1,
									gCvmPages[uPageIdx]->cvmio);
							fflush(gCvmPages[uPageIdx]->cvmio);
							smunload(pCvmPt->tCache);
						}
						pMapNode->cState = pCvmPt->cState = CVM_LOADED;
					}
				}
			}
			break;
		case CVM_SAVE:
			pMapNode = pBlockMapTable->next;
			for (; pMapNode != NULL; pMapNode = pMapNode->next)
			{
				if (pMapNode->nCvmId == pCvmPt->nCvmId
						&& pMapNode->offset <= pCvmPt->offset
						&& pMapNode->uSize
								>= (pCvmPt->offset - pMapNode->offset + pCvmPt->uSize)
						&& pMapNode->Ark_Idx == pCvmPt->Ark_Idx)
				{
					if (pMapNode->tCache.nShmId != pCvmPt->tCache.nShmId)
					{
                        FRAME_PRINTF("CVM_SAVE:Got a mother map!\n");
						void* dest = smload(pMapNode->tCache);
						void* src = smload(pCvmPt->tCache);
						memcpy(
								&(((char *) dest)[pCvmPt->offset
										- pMapNode->offset]), src,
								pCvmPt->uSize);
						smunload(pMapNode->tCache);
						smunload(pCvmPt->tCache);
					}

					break;
				}
			}
			if (pMapNode == NULL && pCvmPt->Ark_Idx == g_nArkIdx)
			{
				uPageIdx = (pCvmPt->nCvmId >> 23) & 0x0f;
				uBlockOffset = ((pCvmPt->nCvmId) & 0x7fffffU) * 512U;
				void *cache = smload(pCvmPt->tCache);
				fseek(gCvmPages[uPageIdx]->cvmio, uBlockOffset + pCvmPt->offset,
				SEEK_SET);
				fwrite(cache, pCvmPt->uSize, 1, gCvmPages[uPageIdx]->cvmio);
				fflush(gCvmPages[uPageIdx]->cvmio);
				smunload(pCvmPt->tCache);
			}
			else if (pCvmPt->Ark_Idx != g_nArkIdx)
			{
				client(INPUTPARA(cCvmOp), INPUTPARA(pCvmPt).nShmId,
						pCvmPt->Ark_Idx);
			}
			break;
		case CVM_UNLOAD:
			FRAME_PRINTF("CVM_UNLOAD!\n")
			;
			pMapNode = pBlockMapTable->next;
			for (; pMapNode != NULL; pMapNode = pMapNode->next)
			{
				if (pCvmPt->tCache.nShmId == pMapNode->tCache.nShmId)
				{
					pMapNode->nAttachCount--;
				}
			}
			PCVMBLOCK pBlockNode;
			PCVMBLOCK pBlockNodePrev;
			if (pCvmPt->Ark_Idx == g_nArkIdx)
			{
				pBlockNode = pBlockStateTable->next;
				pBlockNodePrev = pBlockStateTable;
				for (; pBlockNode != NULL;)
				{
					if (pBlockNode->nCvmId == pCvmPt->nCvmId)
					{
						pBlockNode->nAttachCount--;
						if ((pBlockNode->cFreeMark) && (pBlockNode->nAttachCount == 0))
						{
							uint32_t uSectorsNum = pBlockNode->nSectorsNum;
							uint32_t uStartSector = (pCvmPt->nCvmId)
									& 0x7fffffU;
							uint8_t uPageIdx = (pCvmPt->nCvmId >> 23) & 0x0f;
							gCvmPages[uPageIdx]->nSectorsUsed -=
									pBlockNode->nSectorsNum;
							for (; uSectorsNum > 0; uSectorsNum--)
							{
								gCvmPages[uPageIdx]->cHalfKBTable[uStartSector++] =
										0;
							}
							pBlockNodePrev->next = pBlockNode->next;
							free(pBlockNode);
						}
						break;
					}
					pBlockNodePrev = pBlockNodePrev->next;
					pBlockNode = pBlockNodePrev->next;
				}
			}
			break;
		case CVM_FREE:
			uPageIdx = (pCvmPt->nCvmId >> 23) & 0x0f;
			pMapNodePrev = pBlockMapTable;
			for (pMapNode = pBlockMapTable->next; pMapNode != NULL;)
			{
				if (pMapNode->Ark_Idx == pCvmPt->Ark_Idx
						&& pMapNode->nCvmId == pCvmPt->nCvmId)
				{
					smfree(pMapNode->tCache);
					pMapNodePrev->next = pMapNode->next;
					free(pMapNode);
				}
				else
				{
					pMapNodePrev = pMapNode;
				}
				pMapNode = pMapNodePrev->next;
			}
			if (pCvmPt->Ark_Idx != g_nArkIdx)
			{
				client(INPUTPARA(cCvmOp), INPUTPARA(pCvmPt).nShmId,
						pCvmPt->Ark_Idx);
			}
			else
			{
				pBlockNodePrev = pBlockStateTable;
				pBlockNode = pBlockStateTable->next;
				for (; pBlockNode != NULL;)
				{
					if (pBlockNode->nCvmId == pCvmPt->nCvmId)
					{
						if (pBlockNode->nAttachCount == 0)
						{
							uint32_t uSectorsNum = pBlockNode->nSectorsNum;
							uint32_t uStartSector = (pCvmPt->nCvmId)
									& 0x7fffffU;
							uint8_t uPageIdx = (pCvmPt->nCvmId >> 23) & 0x0f;
							gCvmPages[uPageIdx]->nSectorsUsed -=
									pBlockNode->nSectorsNum;
							for (; uSectorsNum > 0; uSectorsNum--)
							{
								gCvmPages[uPageIdx]->cHalfKBTable[uStartSector++] =
										0;
							}
							pBlockNodePrev->next = pBlockNode->next;
							free(pBlockNode);
						}
						else
							pBlockNode->cFreeMark = 1;
						break;
					}
					pBlockNodePrev = pBlockNodePrev->next;
					pBlockNode = pBlockNodePrev->next;
				}
			}
			break;
		}
		smunload(INPUTPARA(pCvmPt));
		pthread_mutex_unlock(&gCvmMutex);
		NODE_FINISH()
		;
		FUNC_END();
}

int root_func_cvm()
{
	PCVMPT pCvmPt = malloc(sizeof(CVMPT));
	pCvmPt->Ark_Idx = 2;
	pCvmPt->uSize = 0x100000;
	cvmalloc(pCvmPt);
	char* cache = (char *) cvmload(pCvmPt);
	if (cache == NULL)
	{
		FRAME_PRINTF("Error: cache is NULL.");
	}
	memset(cache, 'a', 0xfffff);
	cache[0xfffff] = '\0';
	cvmsave(pCvmPt);
	cvmunload(pCvmPt);
//	cache = (char *)cvmpreload(pCvmPt);
//	if(pCvmPt->cState == CVM_LOADING) {
//		FRAME_PRINTF("cvmpreload sucessfully!!\n.");
//	}
//	cache = (char *)cvmload(pCvmPt);
//	cvmunload(pCvmPt);
	cvmfree(pCvmPt);
	free(pCvmPt);
	return 0;
}

void init_func_cvm()
{
	pBlockStateTable = (PCVMBLOCK) malloc(sizeof(CVMBLOCK));
	pBlockStateTable->next = NULL;
	pBlockMapTable = (PCVMMAP) malloc(sizeof(CVMMAP));
	pBlockMapTable->next = NULL;
	pthread_mutex_init(&gCvmMutex, NULL);
}

void end_func_cvm()
{
	int i;
	PCVMMAP pMapNode = pBlockMapTable->next;
	for (; pMapNode != NULL; pMapNode = pBlockMapTable->next)
	{
		smfree(pMapNode->tCache);
		pBlockMapTable->next = pMapNode->next;
		free(pMapNode);
	}
	PCVMBLOCK pBlockNode = pBlockStateTable->next;
	for (; pBlockNode != NULL; pBlockNode = pBlockStateTable->next)
	{
		pBlockStateTable->next = pBlockNode->next;
		free(pMapNode);
	}
	pthread_mutex_destroy(&gCvmMutex);
	for (i = 0; i < 16; i++)
	{
		if (gCvmPages[i] != NULL)
		{
			fclose(gCvmPages[i]->cvmio);
			free(gCvmPages[i]);
		}
	}
}
