/* epoll_tcp_server.c */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define READ_FAIL -1
#define READ_EOF 0
#define szExit "exit"

typedef struct _AddrBook //주소록 정보
{
	char szName[13];
	char szPhone[14];
	char szAddress[151];
} ADDRBOOK;

typedef struct _Client //Client 정보
{
	int nFd; //Client Fd
	int nReceive; //Receive Status
	char szFileName[261]; //FileName
} CLIENT;

int MyRead(int fd, void* buf, int nSize);

int main()
{
	int i = 0;
	int j = 0;
	int k = 0;
	int nIndex = 0;
	int nPort = 7000; //연동된 Port 번호
	int nMaxPending = 5; //연결 요청을 대기 시킬 수 있는 일종의 대기실
	//Listen 함수 호출을 성공하게 되면 여러 Client들이 연결을 요청해 올 것이고,
	//모든 연결 요청은 서버가 미리 만들어 놓은 대기실로 들어가 순서대로 연결요청이 수락될 때까지 기다린다.
	int nMaxConnecting = 5; //접속 가능한 Client의 최대 수
	int nListenFd = 0; //소켓 Fd
	int nClientFd = 0; //Accept 이후 리턴된 소켓 Fd
	int nEpollFd = 0; //Epoll Fd
	int nFdCount = 0; //이벤트가 발생한 Fd의 개수
	int nTimeout = -1; //Epoll 대기시간
	int nClient = 0; //연결 요청 수락된 Client의 수

	struct sockaddr_in tAddr; //서버에서 생성된 소켓의 주소 정보
	struct sockaddr_in tClientAddr; //연결 요청을 수락할 Client의 주소 정보
	
	socklen_t nAddrSize;
	nAddrSize = sizeof(tAddr);
	memset(&tAddr, 0, nAddrSize);

	ADDRBOOK tAddrBook; //주소록 정보 구조체 변수
	CLIENT tClient[nMaxConnecting]; //Client 정보 구조체 변수

	for (i = 0; i < nMaxConnecting; i++)
	{
		tClient[i].nFd = -1;
		tClient[i].nReceive = 0;
		memset(&tClient[i].szFileName, 0, sizeof(tClient[i].szFileName));
	}

	struct epoll_event tEvent; //Epoll Fd에 Listen Fd를 등록할 때 전달할 관찰 대상 이벤트 정보
	struct epoll_event* ptEvents; //동시에 일어난 이벤트 정보들을 담는 구조체 포인터
	ptEvents = (struct epoll_event*)malloc(sizeof(struct epoll_event) * nMaxConnecting);
	if (ptEvents == NULL)
	{
		fprintf(stderr, "malloc|errno[%d]\n", errno);
		return 0;
	}
	
	FILE* pFile = NULL; //Write할 파일의 포인터

	/* Listen Fd를 생성한다. */
	nListenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (nListenFd == -1)
	{
		fprintf(stderr, "socket|errno[%d]\n", errno);
		return 0;
	}
	
	/* 커널 공간에 Epoll 인스턴스를 생성한다. */
	nEpollFd = epoll_create(100);
	if (nEpollFd == -1)
	{
		fprintf(stderr, "epoll_create|errno[%d]\n", errno);
		goto EXIT;
	}

	tAddr.sin_family = AF_INET; //IPv4주소
	tAddr.sin_port = htons(nPort); //nPort=7000;
	if (tAddr.sin_port == -1)
	{
		fprintf(stderr, "htons|errno[%d]\n", errno);
		goto EXIT;
	}
	/* INADDR_ANY: 소켓을 특정 IP주소에 바인드 하지 않아도 될 때 사용됨. 사용하면 소켓이 모든 IP주소에 연결을 수락한다. */
	tAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (tAddr.sin_addr.s_addr == -1)
	{
		fprintf(stderr, "htonl|errno[%d]\n", errno);
		goto EXIT;
	}
	
	/* SO_REUSEADDR: 소켓이 다른 소켓에서 사용 중인 포트에 강제로 바인딩 할 수 있다. */
	const int flag = 1;
	if (setsockopt(nListenFd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1)
	{
		fprintf(stderr, "setsockopt|errno[%d]\n", errno);
		goto EXIT;
	}

	/* Bind를 통해 ListenFd가 가리키는 소켓에 tAddr이 가리키는 주소 정보를 할당한다. */
	if (bind(nListenFd, (struct sockaddr*)&tAddr, nAddrSize) == -1)
	{
		fprintf(stderr, "bind|errno[%d]\n", errno);
		goto EXIT;
	}
	 
	/* Client로부터의 연결 요청을 기다린다. */
	if (listen(nListenFd, nMaxPending) == -1)
	{
		fprintf(stderr, "listen|errno[%d]\n", errno);
		goto EXIT;
	}

	tEvent.data.fd = nListenFd; //서버 소켓에 대해서
	tEvent.events = EPOLLIN; //수신할 데이터가 존재하게 되는 이벤트를 관찰한다.
	if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, nListenFd, &tEvent) == -1) //이러한 정보를 Epoll 인스턴스에 등록한다.
	{
		fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
		goto EXIT;
}

	while (1)
	{
		/* Epoll Fd의 변화를 탐지한다. */
		/* Epoll Fd의 사건 발생 시 ptEvents에 Fd를 채운다. */
		/* nMaxConnecting + 1인 이유는, 다중접속 수를 초과했는데 연결 요청한 Fd를 닫아야하기 때문 */
		nFdCount = epoll_wait(nEpollFd, ptEvents, nMaxConnecting + 1, nTimeout);
		if (nFdCount == -1) //이벤트가 발생한 Fd 개수
		{
			fprintf(stderr, "epoll_wait|errno[%d]\n", errno);
			goto EXIT;
		}

		for (i = 0; i < nFdCount; i++)
		{
			if (ptEvents[i].events & EPOLLIN) /* 수신할 데이터가 존재하는 상황이라면 */
			{
				if (ptEvents[i].data.fd == nListenFd) /* Epoll events의 사용자 데이터의 Fd가 Listen Fd 라면 */
				{
					/* 연결 요청을 수락하여 데이터를 주고 받을 수 있는 상태로 설정한다. */
					/* tClientAddr: 연결 요청을 수락할 Client의 주소 정보 */
					nClientFd = accept(nListenFd, (struct sockaddr*)&tClientAddr, &nAddrSize);
					if (nClientFd == -1)
					{
						fprintf(stderr, "accept|errno[%d]\n", errno);
						goto EXIT;
					}

					/* 5개를 초과할 시 소켓을 닫음. 단, 클라이언트가 알지 못함 */
					if (nClient == 5) {
						if (close(nClientFd) == -1)
						{
							fprintf(stderr, "close|errno[%d]\n", errno);
							goto EXIT;
						}
						printf("=====================================\nClient 동시 접속 수가 5개를 초과하여 연결을 끊습니다.\n=====================================\n");
						goto WHILE;
					}
					
					/* 0인 Fd에 순차적으로 Fd를 채워준다. */
					for (k =  0; k < nMaxConnecting; k++)
					{
						if (tClient[k].nFd == -1)
						{
							tClient[k].nFd = nClientFd;
							nIndex = k;
							break;
						}
					}
					tEvent.events = EPOLLIN;
					tEvent.data.fd = tClient[nIndex].nFd;
					
					/* 새로 만들어진 Client 소켓도 Epoll 인스턴스에 등록한다. */
					if (epoll_ctl(nEpollFd, EPOLL_CTL_ADD, tClient[nIndex].nFd, &tEvent) == -1)
					{
						fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
						goto EXIT;
					}

					nClient++;
					printf("=====================================\n동시 접속 중인 Client 수: %d\n=====================================\n", nClient);
				}
				else /* Epoll events의 Fd가 Client Fd라면 */
				{
					for (j = 0; j < nMaxConnecting; j++)
					{
						if (ptEvents[i].data.fd == tClient[j].nFd)
						{
							/* 아직 리시브 받은게 없다면 파일명을 읽는다. */
							if (tClient[j].nReceive == 0) {
								memset(&tClient[j].szFileName, 0, sizeof(tClient[j].szFileName));
					
								if (MyRead(tClient[j].nFd, &tClient[j].szFileName, sizeof(tClient[j].szFileName)) == READ_FAIL)
								{
									goto EXIT;
								}
								
								tClient[j].nReceive = 1;
								printf("=====================================\n변화 감지된 Client: #%d\n전송받은 파일 경로: %s\n=====================================\n", tClient[j].nFd, tClient[j].szFileName);
							}
							else if (tClient[j].nReceive == 1) /* 파일명을 입력 받은 상태면 */
							{
								/* 주소록 정보를 읽는다. */
								if (MyRead(tClient[j].nFd, &tAddrBook, sizeof(tAddrBook)) == READ_FAIL)
								{
									goto EXIT;
								}
							
								/* Exit를 입력 받았다면 Epoll Fd에서 Client Fd를 삭제하고 죵료한다. */
								if (strcmp(szExit, tAddrBook.szName) == 0)
								{
									printf("=====================================\n변화 감지된 Client: #%d\n관찰대상에서 삭제\n=====================================\n", tClient[j].nFd);

									if (epoll_ctl(nEpollFd, EPOLL_CTL_DEL, tClient[j].nFd, NULL) == -1)
									{
										fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
									}
									if (close(tClient[j].nFd) == -1)
									{
										fprintf(stderr, "close|errno[%d]\n", errno);
										goto EXIT;
									}

									/* 삭제시킨 Client 정보를 초기화하고, 연결된 Client 수에서 1을 뺀다.*/
									nClient--;
									tClient[j].nFd = -1;
									tClient[j].nReceive = 0;
									memset(&tClient[j].szFileName, 0, sizeof(tClient[j].szFileName));	
									printf("=====================================\n동시 접속 중인 Client 수: %d\n=====================================\n", nClient);

									break;
								}

								/* 주소록 정보를 읽었으면 파일을 열고 쓴다. */
								pFile = fopen(tClient[j].szFileName, "a");
								if (pFile == NULL)
								{
									fprintf(stderr, "fopen|errno[%d]\n", errno);
									goto EXIT;
								}
								if (fwrite(&tAddrBook, sizeof(tAddrBook), 1, pFile) != 1)
								{
									fprintf(stderr, "fwrite|errno[%d]\n", errno);
								}
								if (fclose(pFile) != 0)
								{
									fprintf(stderr, "fclose|errno[%d]\n", errno);
									goto EXIT;
								}
							}
						}
					}
CONT:
					printf("goto CONT\n");

				}
			}
			else if ((ptEvents[i].events & EPOLLERR) ||
					 (ptEvents[i].events & EPOLLHUP) ||
					 (ptEvents[i].events & EPOLLRDHUP))
			{
				printf("에러 감지됨\n");
				if (epoll_ctl(nEpollFd, EPOLL_CTL_DEL, ptEvents[i].data.fd, NULL) == -1)
				{
					fprintf(stderr, "epoll_ctl|errno[%d]\n", errno);
				}
				if (close(ptEvents[i].data.fd) == -1)
				{
					fprintf(stderr, "close|errno[%d]\n", errno);
				}
				goto EXIT;
			}
		}
 WHILE:
		printf("goto WHILE\n");
	}

EXIT:
	free(ptEvents);

	if (close(nListenFd) == -1)
	{
		fprintf(stderr, "close|errno[%d]\n", errno);
	}

	return 0;
}

int MyRead(int fd, void* buf, int nSize)
{
	int nRead = 0;
	int nTotal = 0;

	while (1)
	{
 		nRead = read(fd, buf + nTotal, nSize - nTotal);
 		if (nRead == -1)
 		{
 			fprintf(stderr, "read|errno[%d]\n", errno);
			return READ_FAIL;
		}

		nTotal += nRead;
		if (nTotal == nSize)
		{
			printf("=====================================\n변화 감지된 Client: #%d\nRead 성공\n=====================================\n", fd);
			return nTotal;
		}
		else if (nTotal == 0)
		{
			return READ_EOF;
		}
	}
}
