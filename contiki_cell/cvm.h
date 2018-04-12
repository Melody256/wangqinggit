/*
 * cvm.h
 *
 *  Created on: 2014-12-1
 *      Author: wintice
 */

#ifndef CVM_H_
#define CVM_H_

#include <stdlib.h>
#include <stddef.h>
#include "recursion_parallel.h"

#define CVM_CREATE 0x01
#define CVM_ALLOC 0x02
#define CVM_LOAD 0x04
#define CVM_SAVE 0x05
#define CVM_UNLOAD 0x06
#define CVM_FREE 0x07
#define SKY_CALL 0x08
#define SKY_RETURN 0x09

#define CVM_ERROR 0x00
#define CVM_UNLOADED 0x01
#define CVM_LOADING 0x02
#define CVM_LOADED 0x03

typedef struct
{
	uint32_t nCvmId;
	int Ark_Idx;
	uint32_t uSize;
	uint32_t offset;
	SMPT tCache;
	volatile char cState;
} _CVMPT, *_PCVMPT;

typedef struct
{
	char cHalfKBTable[0x400000];
	int nSectorsUsed;
	FILE* cvmio;
} CVMPAGE, *PCVMPAGE;

typedef struct _CVMBLOCK
{
	int nCvmId;
	char cFreeMark;
	int nAttachCount;
	uint32_t nSectorsNum;
	struct _CVMBLOCK* next;
} CVMBLOCK, *PCVMBLOCK;

typedef struct _CVMMAP
{
	int nCvmId;
	int Ark_Idx;
	uint32_t offset;
	uint32_t uSize;
	SMPT tCache;
	volatile char cState;
	int nAttachCount;
	struct _CVMMAP* next;
} CVMMAP, *PCVMMAP;

TASK_NODE(cvm)
{
	NODE_HEAD()
	char cCvmOp;SHM_PARA pCvmPt; //CVMPT
};

extern int func_idx_cvm;
extern char FUNC_SWITCHER_cvm;
RECURSION_FUNCTION(cvm);
void init_func_cvm();
void end_func_cvm();
int root_func_cvm();

#endif /* CVM_H_ */
