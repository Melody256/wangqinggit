/*
 * shm_com.c
 *
 *  Created on: 2015-2-15
 *      Author: wintice
 */

#include "shm_com.h"
#include <stdlib.h>
#include <pthread.h>
#include "recursion_parallel.h"
pthread_mutex_t frame_sm_mutex;
PSMMAP pSmTableHead;

inline void SmTableInit()
{
	pSmTableHead = malloc(sizeof(SMMAP));
	pSmTableHead->nShmId = 0;
	pSmTableHead->pMapAddr = NULL;
	pSmTableHead->nAttachCount = 0;
	pSmTableHead->cFreeMark = 0;
	pSmTableHead->prev = pSmTableHead->next = NULL;
	pthread_mutex_init(&frame_sm_mutex, NULL);
}

PSMMAP SmTableSearch(int nShmId)
{
	PSMMAP pNode = pSmTableHead;
	while ((pNode = pNode->next) != NULL)
	{
		if (pNode->nShmId == nShmId)
			break;
	}
	return pNode;
}

inline int smalloc(int nKey, unsigned int size)
{
	int nShmId = shmget(nKey, size, 0666 | IPC_CREAT | IPC_EXCL);
	if (nShmId == -1)
	{
		FRAME_PERROR("smalloc:");
		return -1;
	}
	return nShmId;
}

inline void * smload(SMPT smpt)
{
	PSMMAP pNode = NULL;
	pthread_mutex_lock(&frame_sm_mutex);
	if ((pNode = SmTableSearch(smpt.nShmId)) == NULL)
	{
		pNode = malloc(sizeof(SMMAP));
		pNode->nShmId = smpt.nShmId;
		pNode->pMapAddr = shmat(smpt.nShmId, (void*) 0, 0);
		if (pNode->pMapAddr == (void*) -1)
		{
			free(pNode);
			perror("smload:");
			pthread_mutex_unlock(&frame_sm_mutex);
			return NULL;
		}
		pNode->cFreeMark = 0;
		pNode->nAttachCount = 1;
		pNode->prev = pSmTableHead;
		if (pSmTableHead->next != NULL)
			pSmTableHead->next->prev = pNode;
		pNode->next = pSmTableHead->next;
		pSmTableHead->next = pNode;
	}
	else
	{
		if (pNode->cFreeMark)
		{
			printf("Error:the smpt has been freed!\n");
			pthread_mutex_unlock(&frame_sm_mutex);
			return NULL;
		}
		pNode->nAttachCount++;
	}
	pthread_mutex_unlock(&frame_sm_mutex);
	return (pNode->pMapAddr + smpt.offset);

}

inline int smunload(SMPT smpt)
{
	PSMMAP pNode = NULL;
	pthread_mutex_lock(&frame_sm_mutex);
	if ((pNode = SmTableSearch(smpt.nShmId)) == NULL)
	{
		printf(
				"error:the shared memory pointer doesn't pointed at any attached space.\n");
		pthread_mutex_unlock(&frame_sm_mutex);
		return -1;
	}
	else
	{
		pNode->nAttachCount--;
		if (pNode->nAttachCount == 0)
		{
            printf("No used this shm anymore.Detach it.\n");
			while (shmdt(pNode->pMapAddr) == -1)
				;
			if (pNode->next != NULL)
				pNode->next->prev = pNode->prev;
			pNode->prev->next = pNode->next;
			free(pNode);
		}
		pthread_mutex_unlock(&frame_sm_mutex);
		return 0;
	}
}

inline int smfree(SMPT smpt)
{
	PSMMAP pNode = NULL;
	pthread_mutex_lock(&frame_sm_mutex);
	if ((pNode = SmTableSearch(smpt.nShmId)) != NULL)
	{
		pNode->cFreeMark = 1;
	}
	if (shmctl(smpt.nShmId, IPC_RMID, NULL) == -1)
	{
		perror("smfree:");
		pthread_mutex_unlock(&frame_sm_mutex);
		return -1;
	}
	pthread_mutex_unlock(&frame_sm_mutex);
	return (0);
}

void SmTableDel()
{
	PSMMAP pNodeTemp, pNode = pSmTableHead->next;
	while (pNode != NULL)
	{
		pNodeTemp = pNode;
		pNode = pNodeTemp->next;
		free(pNodeTemp);
	}
	free(pSmTableHead);
}
