/*
 * shm_com.h
 *
 *  Created on: 2014-4-7
 *      Author: user
 */

#ifndef SHM_COM_H_
#define SHM_COM_H_
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define CONTROLER 0;

typedef struct
{
	int nShmId;
	size_t offset;
} SMPT, *PSMPT;

typedef struct _SMMAP
{
	int nShmId;
	void* pMapAddr;
	int nAttachCount;
	char cFreeMark;
	struct _SMMAP* prev;
	struct _SMMAP* next;
} SMMAP, *PSMMAP;

inline void SmTableInit();
PSMMAP SmTableSearch(int nShmId);
inline int smalloc(int nKey, size_t size);
inline void * smload(SMPT pSMPT);
inline int smunload(SMPT pSMPT);
inline int smfree(SMPT pSMPT);
void SmTableDel();
#endif /* SHM_COM_H_ */

