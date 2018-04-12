/*client.c*/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <math.h>
#include "async_server.h"
#include "cvm.h"

#define PORT	4321
#define BUFFER_SIZE 1024

void client(char CMD, int shmid, int num)
{
	SKY_MESSAGE_BUFF send_buff_temp;
	int sockfd;
	uint32_t bytes, r;
	struct hostent *host;
	struct sockaddr_in serv_addr;
	char strDestAddr[16];
	send_buff_temp.CMD = CMD;
	void *addr;
	switch (CMD)
	{
	case SKY_CALL:
		sprintf(strDestAddr, "169.254.147.%d", GetAvailArk());
		send_buff_temp.size = num;
		addr = shmat(shmid, NULL, 0);
		memcpy(send_buff_temp.message, addr, num);
		shmdt(addr);
		break;
	case SKY_RETURN:
		sprintf(strDestAddr, "169.254.147.%d", num);
		send_buff_temp.size = sizeof(int);
		memcpy(send_buff_temp.message, &shmid, sizeof(int));
		break;
	case CVM_ALLOC:
	case CVM_LOAD:
	case CVM_SAVE:
	case CVM_UNLOAD:
	case CVM_FREE:
		sprintf(strDestAddr, "169.254.147.%d", num);
		send_buff_temp.size = sizeof(_CVMPT);
		addr = shmat(shmid, NULL, 0);
		memcpy(send_buff_temp.message, addr, sizeof(_CVMPT));
		shmdt(addr);
		break;
	}
	/*地址解析函数*/
	if ((host = gethostbyname(strDestAddr)) == NULL)
	{
		perror("gethostbyname");
		return;
	}
	/*创建socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		return;
	}
	/*设置sockaddr_in 结构体中相关参数*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr *) host->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	/*调用connect函数主动发起对服务器端的连接*/
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr))
			== -1)
	{
		perror("connect");
		return;
	}
	/*发送消息给服务器端*/
	bytes = sizeof(send_buff_temp);
	char *buff = (char *) &(send_buff_temp);
	while (bytes > 0)
	{
		if ((r = send(sockfd, buff, bytes, 0)) == -1)
		{
			perror("send");
			return;
		}
		else
		{
			bytes -= r;
			buff += r;
		}
	}
    printf("Client send a cmd\nCODE:%d\nCVMID:%d\nSIZE:%d\nOFFSET:%d\n", CMD, ((_PCVMPT) send_buff_temp.message)->nCvmId, ((_PCVMPT) send_buff_temp.message)->uSize, ((_PCVMPT) send_buff_temp.message)->offset);
	if (CMD == CVM_ALLOC)
	{
		addr = shmat(shmid, NULL, 0);
		recv(sockfd, addr, sizeof(_CVMPT), 0);
		shmdt(addr);
	}
	else if (CMD == CVM_SAVE)
	{
		_PCVMPT pCvmPt;
		pCvmPt = (_PCVMPT) (send_buff_temp.message);
		char* buffer = (char *) smload(pCvmPt->tCache);
		char bufferans[3];
		uint32_t size, sendsize;
		size = pCvmPt->uSize;
		int i = 0;
		int nSendTimes = (int) ceil(size / 4194304.0);
		FRAME_PRINTF("The cvmid is %d.The size need to send:%d, the times:%d\n", pCvmPt->nCvmId, size, nSendTimes);
		for (i = 0; i < nSendTimes; i++)
		{
			bytes = 0;
			sendsize = size > 4194304 ? 4194304 : size;
			while (bytes < sendsize)
			{
				r = send(sockfd, &(buffer[i * 4194304 + bytes]),
						sendsize - bytes, 0);
				if (r == -1)
				{
					FRAME_PERROR("client send in cvm_save");
                    FRAME_PRINTF("%x",buffer);
                    while(1);
				}
				else
				{
					bytes += r;
				}
			}
			size -= bytes;
			bytes = 0;
			//while (bytes < 3)
			//{
			//	r = recv(sockfd, bufferans, 3, 0);
			//	bytes += r;
			//}
		}
		FRAME_PRINTF("client:sky cvm_save send end.\n");
		smunload(pCvmPt->tCache);
	}
	else if (CMD == CVM_LOAD)
	{
		_PCVMPT pCvmPt;
		pCvmPt = (_PCVMPT) (send_buff_temp.message);
		char* buffer = (char *) smload(pCvmPt->tCache);
		uint32_t size, rcvsize;
		size = pCvmPt->uSize;
		int i = 0;
		int nSendTimes = (int) ceil(size / 4194304.0);
		FRAME_PRINTF("The cvmid is %d.The size need to recieve:%d, the times:%d\n", pCvmPt->nCvmId, size, 
				nSendTimes);
		for (i = 0; i < nSendTimes; i++)
		{
			bytes = 0;
			rcvsize = size > 4194304 ? 4194304 : size;
			while (bytes < rcvsize)
			{
				r = recv(sockfd, &(buffer[i * 4194304 + bytes]),
						rcvsize - bytes, 0);
				if (r == -1)
				{
					FRAME_PERROR("client cvm_load");
				}
				else
				{
					bytes += r;
				}
			}
			size -= bytes;
		}
		smunload(pCvmPt->tCache);
		FRAME_PRINTF("client:sky cvm_load recv end.\n");
	}
	close(sockfd);
	return;
}
