/*
 * sem_com.h
 *
 *  Created on: 2014-4-7
 *      Author: user
 */

#ifndef SEM_COM_H_
#define SEM_COM_H_
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
extern int init_sem(int sem_id, int init_value);
extern int del_sem(int sem_id);
extern int sem_p(int sem_id);
extern int sem_v(int sem_id);
#endif /* SEM_COM_H_ */
