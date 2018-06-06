//////////////////////////////////////////////////////////////////////////
//  @file     EventSelectServer.h
//  @author   AHVincent
//  @date     28 Dec 2017
//  @remark   This code is for network
//  @note
//////////////////////////////////////////////////////////////////////////

#ifndef EVENTSELECTSERVER_H
#define EVENTSELECTSERVER_H

DWORD WINAPI ServerThread(LPVOID lpParam);


// 套接字对象
typedef struct _SOCKET_OBJ
{
	SOCKET s;				// 套接字句柄
	HANDLE event;			// 与套接字相关的事件对象句柄
	sockaddr_in addrRemote; // 客户端地址信息

	
  _SOCKET_OBJ *next;	// 指向下一个_SOCKED_OBJ 对象
} SOCKET_OBJ, *PSOCKET_OBJ;

// 线程对象
typedef struct _THREAD_OBJ
{
	HANDLE events[WSA_MAXIMUM_WAIT_EVENTS];	// 记录当前线程要等待的事件对象的句柄
	int nSocketCount;						// 记录当前线程处理的套接字数量

	PSOCKET_OBJ pSockHeader;				// 当前线程处理的套接字对象列表，表头
	PSOCKET_OBJ pSockTail;					// 表尾

	CRITICAL_SECTION cs;					// 关键代码段变量，为的是同步对本结构的访问
	_THREAD_OBJ *next;						

} THREAD_OBJ, *PTHREAD_OBJ;

// 线程列表
PTHREAD_OBJ g_pThreadHead;			// 指向线程对象列表表头
CRITICAL_SECTION g_cs;				// 同步对此全局变量的访问

// 状态信息
LONG g_nTatolConnections;
LONG g_nCurrentConnections;

// 申请一个套接字对象，初始化它的成员
PSOCKET_OBJ GetSocketObj(SOCKET s)
{
	PSOCKET_OBJ pSocket = (PSOCKET_OBJ)::GlobalAlloc(GPTR, sizeof(SOCKET_OBJ));
	if (pSocket != NULL)
	{
		pSocket->s = s;
		pSocket->event = ::WSACreateEvent();
	}

	return pSocket;
}

// 释放一个套接字对象
void FreeSocketObj(PSOCKET_OBJ pSocket)
{
	::CloseHandle(pSocket->event);
	if (pSocket->s != INVALID_SOCKET)
		::closesocket(pSocket->s);

	::GlobalFree(pSocket);
}

// 申请一个线程对象，初始化它的成员,并将它添加到线程对象列表
PTHREAD_OBJ GetThreadObj()
{
	PTHREAD_OBJ pThread = (PTHREAD_OBJ)::GlobalAlloc(GPTR, sizeof(PTHREAD_OBJ));
	if (pThread != NULL)
	{
		::InitializeCriticalSection(&pThread->cs);
		pThread->events[0] = ::WSACreateEvent();

		// 将新申请的线程对象添加到列表中
		::EnterCriticalSection(&g_cs);
		pThread->next = g_pThreadHead;
		::LeaveCriticalSection(&g_cs);
	}

	return pThread;
}

// 释放一个线程对象，并将它从线程对象列表中移除
void FreeThreadObj(PTHREAD_OBJ pThread)
{
	// 在线程对象列表中查找pThread所指的对象，如果找到就从中移除
	::EnterCriticalSection(&g_cs);
	PTHREAD_OBJ p = g_pThreadHead;
	if (p == pThread)
		g_pThreadHead = p->next;
	else
	{
		while(p != NULL && p->next != pThread)
			p = p->next;
		if (p != NULL)
			// 此时,p是pThread的前一个，
			p->next = pThread->next;
	}
	::LeaveCriticalSection(&g_cs);

	// 释放资源
	::CloseHandle(pThread->events[0]);
	::DeleteCriticalSection(&pThread->cs);
	::GlobalFree(pThread);
}

// 重新建立线程对象的events数组
void RebuildArray(PTHREAD_OBJ pThread)
{
	::EnterCriticalSection(&pThread->cs);
	PSOCKET_OBJ pSocket = pThread->pSockHeader;
	int n = 1;	// 从第一个开始写，第0个用于表示要重建
	while(pSocket != NULL)
	{
		pThread->events[n++] = pSocket->next;
		pSocket = pSocket->next;
	}
	::LeaveCriticalSection(&pThread->cs);
}

//////////////////////////////////////////////////////////////////////////

// 向一个线程的套接字列表中插入一个套接字
BOOL InsertSocketObj(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	BOOL bRet = FALSE;
	::EnterCriticalSection(&pThread->cs);
	if (pThread->nSocketCount <WSA_MAXIMUM_WAIT_EVENTS-1)
	{
		if (NULL == pThread->pSockHeader)
		{
			pThread->pSockHeader = pThread->pSockTail = pSocket;
		} 
		else
		{
			pThread->pSockTail->next = pSocket;
			pThread->pSockTail = pSocket;
		}
		pThread->nSocketCount++;
		bRet = TRUE;
	} 
	::LeaveCriticalSection(&pThread->cs);
	
	// 插入成功,说明成功处理了客户的连接请求
	if (bRet)
	{
		::InterlockedIncrement(&g_nTatolConnections);
		::InterlockedIncrement(&g_nCurrentConnections);
	}
	return bRet;
}

// 将一个套接字对象安排给空闲的线程处理
void AssignToFreeThread(PSOCKET_OBJ pSocket)
{
	pSocket->next = NULL;

	::EnterCriticalSection(&g_cs);
	PTHREAD_OBJ pThread = g_pThreadHead;
	// 试图插入到现存线程
	while(pThread != NULL)
	{
		if(InsertSocketObj(pThread, pSocket))
			break;
		pThread = pThread->next;
	}

	// 没有空闲线程，为了这个套接字创建新的线程
	if (NULL == pThread)
	{
		pThread = GetThreadObj();
		InsertSocketObj(pThread, pSocket);
		::CreateThread(NULL, 0, ServerThread, pThread, 0, NULL);
	}
	::LeaveCriticalSection(&g_cs);

	// 指示线程重建句柄数组
	::WSASetEvent(pThread->events[0]);

}

// 从给定线程的套接字对象列表中移除一个套接字对象
void RemoveSocketObj(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	::EnterCriticalSection(&g_cs);

	// 在套接字对象列表中查找制定的套接字对象，找到后将之移除
	PSOCKET_OBJ pTest = pThread->pSockHeader;
	if (pTest = pSocket)
	{
		if (pThread->pSockHeader == pThread->pSockTail)
			pThread->pSockTail = pThread->pSockHeader = pTest->next;
		else 
			pThread->pSockHeader = pTest->next;
	} 
	else
	{
		while (pTest != NULL && pTest->next != pTest)
			pTest = pTest->next;
		if (pTest != NULL)
		{
			if(pThread->pSockTail == pSocket)
				pThread->pSockTail = pTest;
			pTest->next = pSocket->next;
		}
	}
	pThread->nSocketCount--;

	::LeaveCriticalSection(&g_cs);

	// 指示线程重建句柄数组
	::WSASetEvent(pThread->events[0]);

	// 说明一个连接中断
	::InterlockedDecrement(&g_nCurrentConnections);
}

BOOL handleIO(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	// 获得具体发生的网络事件
	WSANETWORKEVENTS event;
	::WSAEnumNetworkEvents(pSocket->s, pSocket->event, &event);
	do 
	{
		if(event.lNetworkEvents & FD_READ)		// 套接字可读
		{
			if(0 == event.iErrorCode[FD_READ_BIT])
			{
				char szText[256];
				int nRecv = ::recv(pSocket->s, szText, strlen(szText), 0);
				if (nRecv > 0)
				{
					szText[nRecv] = '\0';
					cout << " receive data: " << szText << endl;
				}
			}
			else
				break;
		}
		else if (event.lNetworkEvents & FD_CLOSE)
		{
			break;
		}
		else if (event.lNetworkEvents & FD_WRITE)
		{
			if (0 == event.iErrorCode[FD_WRITE_BIT]){}
			else break;

		}
		return TRUE;
	} while (FALSE);

	// 套接字关闭，或者发生错误，程序都会转到这里来执行
	RemoveSocketObj(pThread, pSocket);
	FreeSocketObj(pSocket);
	return FALSE;

}

PSOCKET_OBJ FindSocketObj(PTHREAD_OBJ pThread, int nIndex)
{
	// 在套接字列表中查找
	PSOCKET_OBJ pSocket = pThread->pSockHeader;
	while (--nIndex)
	{
		if(NULL == pSocket)
			return NULL;
		pSocket = pSocket->next;
	}
	return pSocket;
}

DWORD WINAPI ServerThread(LPVOID lpParam)
{
	// 取得本线程对象的指针
	PTHREAD_OBJ pThread = (PTHREAD_OBJ)lpParam;
	while (TRUE)
	{
		// 等待网络事件
		int nIndex = ::WSAWaitForMultipleEvents(
			pThread->nSocketCount+1, pThread->events, FALSE, WSA_INFINITE, FALSE);
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		// 查看可受信的事件对象
		for (int i=nIndex; i<pThread->nSocketCount+1; ++i)
		{
			nIndex = ::WSAWaitForMultipleEvents(1, &pThread->events[i], TRUE, 1000, FALSE);
			if (WSA_WAIT_FAILED == nIndex || WSA_WAIT_TIMEOUT == nIndex)
				continue;
			else
			{
				if (0 == i)
				{
					RebuildArray(pThread);
					// 如果没有客户I/O要处理，本线程退出
					if (0 == pThread->nSocketCount)
					{
						FreeThreadObj(pThread);
						return 0;
					}
					::WSAResetEvent(pThread->events[0]);
				}
				else
				{
					// 查找对应的套接字对象指针,调用HandleIO处理网络事件
					PSOCKET_OBJ pSocket = (PSOCKET_OBJ)FindSocketObj(pThread, i);
					if (pSocket != NULL)
					{
						if(!handleIO(pThread, pSocket))
							RebuildArray(pThread);
					}
					else
						cout << "unable to find socket object! " << endl;
				}
			}

		}
	}
	return 0;
}

#endif  // EVENTSELECTSERVER_H