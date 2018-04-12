/* async_server.c */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include "recursion_parallel.h"
#include "contiki_cell.h"
#include "cvm.h"
#include <math.h>
#include "cvm_api.h"
#include "async_server.h"
#include "shm_com.h"
#define PORT				4321
#define MAX_QUE_CONN_NM		5

struct sockaddr_in server_sockaddr, client_sockaddr;
int sockfd;

/* 异步信号处理函数，处理新的套结字的连接和数据 */
void* RecRmtMsg(void* arg)
{
    int sin_size;
	int client_fd;
	SKY_MESSAGE_BUFF recv_buff_temp;
	uint32_t bytes, r;
	if ((client_fd = accept(sockfd, (struct sockaddr*) &client_sockaddr,
			(socklen_t *) &sin_size)) == -1)
	{
		perror("accept");
		exit(1);
	}
	/* 调用recv函数接受客户端的请求 */
	bytes = sizeof(recv_buff_temp);
	char *buff = &(recv_buff_temp);
	while (bytes > 0)
	{
		if ((r = recv(client_fd, buff, bytes, 0)) == -1)
		{
			perror("recv");
			exit(1);
		}
		else
		{
			bytes -= r;
			buff += r;
		}
	}
	SMPT node;
	void *pNodePt;
	struct node_head_str* NodeHead = NULL;
	void* buffer;
	switch (recv_buff_temp.CMD)
	{
	case SKY_CALL:
		node.nShmId = smalloc(IPC_PRIVATE, recv_buff_temp.size);
		node.offset = 0;
		pNodePt = smload(node);
		memcpy(pNodePt, recv_buff_temp.message, recv_buff_temp.size);
		NodeHead = (struct node_head_str*) pNodePt;
		NodeHead->semid_node = semget(IPC_PRIVATE, 1, 0666);
		init_sem(NodeHead->semid_node, 1);
		NodeHead->mode = CALL_MODE_ARK;
		break;
	case SKY_RETURN:
		node.nShmId = *((int *) (&(recv_buff_temp.message[0])));
		node.offset = 0;
		pNodePt = smload(node);
		NodeHead = (struct node_head_str*) pNodePt;
		NodeHead->node_finish = 1;
		break;
	case CVM_ALLOC:
		cvmalloc((_PCVMPT) (recv_buff_temp.message));
		send(client_fd, (_PCVMPT) (recv_buff_temp.message), sizeof(_CVMPT), 0);
		break;
	case CVM_LOAD:
		;
		_PCVMPT pCvmPt;
		pCvmPt = (_PCVMPT) (recv_buff_temp.message);
		uint32_t size;
		size = pCvmPt->uSize;
		int i = 0;
		int nSendTimes = (int) ceil(size / 4194304.0);
		FRAME_PRINTF("The cvmid is %d.The size need to send:%d, the times:%d\n", pCvmPt->nCvmId, size, nSendTimes);
		for (i = 0; i < nSendTimes; i++)
		{
			bytes = 0;
			pCvmPt->uSize = size > 4194304 ? 4194304 : size;
			void *buffer = cvmload(pCvmPt);
			while (bytes < pCvmPt->uSize)
			{
				r = send(client_fd, &(buffer[bytes]), pCvmPt->uSize - bytes, 0);
				if (r == -1)
				{
					FRAME_PERROR("server cvm_load");
				}
				bytes += r;
			}
			cvmunload(pCvmPt);
			size -= bytes;
			pCvmPt->offset += bytes;
		}
		FRAME_PRINTF("server:sky cvm_load send end.\n")
		;
		break;
	case CVM_SAVE:
		;
		SMPT smptBuffer;
		smptBuffer.nShmId = smalloc(IPC_PRIVATE, 4194304);
		smptBuffer.offset = 0;
		buffer = smload(smptBuffer);
		pCvmPt = (_PCVMPT) (recv_buff_temp.message);
		size = pCvmPt->uSize;
		int nRcvTimes = ceil(size / 4194304.0);
		FRAME_PRINTF("The cvmid is %d.The size need to send:%d, the times:%d\n", pCvmPt->nCvmId, size, nSendTimes);
		for (i = 0; i < nRcvTimes; i++)
		{
			bytes = 0;
			pCvmPt->uSize = size > 4194304 ? 4194304 : size;
			pCvmPt->tCache = smptBuffer;
			while (bytes < pCvmPt->uSize)
			{
				r = recv(client_fd, &(buffer[bytes]), pCvmPt->uSize - bytes, 0);
				if (r == -1)
				{
					FRAME_PERROR("server cvm_save");
				}
				bytes += r;
			}
			pCvmPt->uSize = bytes;
			cvmsave(pCvmPt);
			size -= bytes;
			pCvmPt->offset += bytes;
			//send(client_fd, "OK", 3, 0);
		}
		FRAME_PRINTF("server:sky cvm_save recv end.\n")
		smunload(smptBuffer);
		smfree(smptBuffer);
		break;
	case CVM_FREE:
		cvmfree((_PCVMPT) (recv_buff_temp.message));
		break;
	}
	if (recv_buff_temp.CMD >= 0x08)
	{
		int nKeyTemp = BKDRHash(NodeHead->func);
		int nSemTemp = semget(nKeyTemp, 1, 0666 | IPC_CREAT);
		int nShmTemp = shmget(nKeyTemp, 0, 0666 | IPC_CREAT);
		struct task_head_str* pShmatTemp = (struct task_head_str*) shmat(
				nShmTemp, NULL, 0);

		sem_p(nSemTemp);
		NodeHead->right.shmid = pShmatTemp->stack_top.shmid;
		pShmatTemp->stack_top.shmid = node.nShmId;
		pShmatTemp->task_undone++;
		sem_v(nSemTemp);
		shmdt((void *) pShmatTemp);
		smunload(node);
	}
	//printf("%s",shm_buff_temp.buffer);
	//SendShmMessage(&shm_buff_temp);
	close(client_fd);
	pthread_exit(NULL);
}

void AcceptAsync(int sig_num)
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&thread, &attr, RecRmtMsg, NULL);
}

int SetServer()
{
	int flags;
//	char CMD[128];
//	sprintf(CMD,  "sudo ifconfig eth1 169.254.147.%d netmask 255.255.0.0", nArkIdx);
//	system(CMD);
	/* 建立socket连接 */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return (-1);
	}
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero), 8);
	int i = 1;/* 允许重复使用本地地址与套接字进行绑定 */
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
	if (bind(sockfd, (struct sockaddr *) &server_sockaddr,
			sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
		return (-1);
	}
	if (listen(sockfd, MAX_QUE_CONN_NM) == -1)
	{
		perror("listen");
		return (-1);
	}
	/* 设置异步方式 */
	signal(SIGIO, AcceptAsync); /* SIGIO信号处理函数的注册 */
	fcntl(sockfd, F_SETOWN, getpid()); /* 使套结字归属于该进程 */
	flags = fcntl(sockfd, F_GETFL); /* 获得套结字的状态标志位 */
	if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_ASYNC) < 0)
	{
		// 设置成异步访问模式
		perror("fcntl");
		return (-1);
	}
	return 0;
}

int CloseServer()
{
	if (close(sockfd) == -1)
	{

	}
	return 0;
}
