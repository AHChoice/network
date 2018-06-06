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


// �׽��ֶ���
typedef struct _SOCKET_OBJ
{
	SOCKET s;				// �׽��־��
	HANDLE event;			// ���׽�����ص��¼�������
	sockaddr_in addrRemote; // �ͻ��˵�ַ��Ϣ

	
  _SOCKET_OBJ *next;	// ָ����һ��_SOCKED_OBJ ����
} SOCKET_OBJ, *PSOCKET_OBJ;

// �̶߳���
typedef struct _THREAD_OBJ
{
	HANDLE events[WSA_MAXIMUM_WAIT_EVENTS];	// ��¼��ǰ�߳�Ҫ�ȴ����¼�����ľ��
	int nSocketCount;						// ��¼��ǰ�̴߳�����׽�������

	PSOCKET_OBJ pSockHeader;				// ��ǰ�̴߳�����׽��ֶ����б���ͷ
	PSOCKET_OBJ pSockTail;					// ��β

	CRITICAL_SECTION cs;					// �ؼ�����α�����Ϊ����ͬ���Ա��ṹ�ķ���
	_THREAD_OBJ *next;						

} THREAD_OBJ, *PTHREAD_OBJ;

// �߳��б�
PTHREAD_OBJ g_pThreadHead;			// ָ���̶߳����б��ͷ
CRITICAL_SECTION g_cs;				// ͬ���Դ�ȫ�ֱ����ķ���

// ״̬��Ϣ
LONG g_nTatolConnections;
LONG g_nCurrentConnections;

// ����һ���׽��ֶ��󣬳�ʼ�����ĳ�Ա
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

// �ͷ�һ���׽��ֶ���
void FreeSocketObj(PSOCKET_OBJ pSocket)
{
	::CloseHandle(pSocket->event);
	if (pSocket->s != INVALID_SOCKET)
		::closesocket(pSocket->s);

	::GlobalFree(pSocket);
}

// ����һ���̶߳��󣬳�ʼ�����ĳ�Ա,��������ӵ��̶߳����б�
PTHREAD_OBJ GetThreadObj()
{
	PTHREAD_OBJ pThread = (PTHREAD_OBJ)::GlobalAlloc(GPTR, sizeof(PTHREAD_OBJ));
	if (pThread != NULL)
	{
		::InitializeCriticalSection(&pThread->cs);
		pThread->events[0] = ::WSACreateEvent();

		// ����������̶߳�����ӵ��б���
		::EnterCriticalSection(&g_cs);
		pThread->next = g_pThreadHead;
		::LeaveCriticalSection(&g_cs);
	}

	return pThread;
}

// �ͷ�һ���̶߳��󣬲��������̶߳����б����Ƴ�
void FreeThreadObj(PTHREAD_OBJ pThread)
{
	// ���̶߳����б��в���pThread��ָ�Ķ�������ҵ��ʹ����Ƴ�
	::EnterCriticalSection(&g_cs);
	PTHREAD_OBJ p = g_pThreadHead;
	if (p == pThread)
		g_pThreadHead = p->next;
	else
	{
		while(p != NULL && p->next != pThread)
			p = p->next;
		if (p != NULL)
			// ��ʱ,p��pThread��ǰһ����
			p->next = pThread->next;
	}
	::LeaveCriticalSection(&g_cs);

	// �ͷ���Դ
	::CloseHandle(pThread->events[0]);
	::DeleteCriticalSection(&pThread->cs);
	::GlobalFree(pThread);
}

// ���½����̶߳����events����
void RebuildArray(PTHREAD_OBJ pThread)
{
	::EnterCriticalSection(&pThread->cs);
	PSOCKET_OBJ pSocket = pThread->pSockHeader;
	int n = 1;	// �ӵ�һ����ʼд����0�����ڱ�ʾҪ�ؽ�
	while(pSocket != NULL)
	{
		pThread->events[n++] = pSocket->next;
		pSocket = pSocket->next;
	}
	::LeaveCriticalSection(&pThread->cs);
}

//////////////////////////////////////////////////////////////////////////

// ��һ���̵߳��׽����б��в���һ���׽���
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
	
	// ����ɹ�,˵���ɹ������˿ͻ�����������
	if (bRet)
	{
		::InterlockedIncrement(&g_nTatolConnections);
		::InterlockedIncrement(&g_nCurrentConnections);
	}
	return bRet;
}

// ��һ���׽��ֶ����Ÿ����е��̴߳���
void AssignToFreeThread(PSOCKET_OBJ pSocket)
{
	pSocket->next = NULL;

	::EnterCriticalSection(&g_cs);
	PTHREAD_OBJ pThread = g_pThreadHead;
	// ��ͼ���뵽�ִ��߳�
	while(pThread != NULL)
	{
		if(InsertSocketObj(pThread, pSocket))
			break;
		pThread = pThread->next;
	}

	// û�п����̣߳�Ϊ������׽��ִ����µ��߳�
	if (NULL == pThread)
	{
		pThread = GetThreadObj();
		InsertSocketObj(pThread, pSocket);
		::CreateThread(NULL, 0, ServerThread, pThread, 0, NULL);
	}
	::LeaveCriticalSection(&g_cs);

	// ָʾ�߳��ؽ��������
	::WSASetEvent(pThread->events[0]);

}

// �Ӹ����̵߳��׽��ֶ����б����Ƴ�һ���׽��ֶ���
void RemoveSocketObj(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	::EnterCriticalSection(&g_cs);

	// ���׽��ֶ����б��в����ƶ����׽��ֶ����ҵ���֮�Ƴ�
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

	// ָʾ�߳��ؽ��������
	::WSASetEvent(pThread->events[0]);

	// ˵��һ�������ж�
	::InterlockedDecrement(&g_nCurrentConnections);
}

BOOL handleIO(PTHREAD_OBJ pThread, PSOCKET_OBJ pSocket)
{
	// ��þ��巢���������¼�
	WSANETWORKEVENTS event;
	::WSAEnumNetworkEvents(pSocket->s, pSocket->event, &event);
	do 
	{
		if(event.lNetworkEvents & FD_READ)		// �׽��ֿɶ�
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

	// �׽��ֹرգ����߷������󣬳��򶼻�ת��������ִ��
	RemoveSocketObj(pThread, pSocket);
	FreeSocketObj(pSocket);
	return FALSE;

}

PSOCKET_OBJ FindSocketObj(PTHREAD_OBJ pThread, int nIndex)
{
	// ���׽����б��в���
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
	// ȡ�ñ��̶߳����ָ��
	PTHREAD_OBJ pThread = (PTHREAD_OBJ)lpParam;
	while (TRUE)
	{
		// �ȴ������¼�
		int nIndex = ::WSAWaitForMultipleEvents(
			pThread->nSocketCount+1, pThread->events, FALSE, WSA_INFINITE, FALSE);
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		// �鿴�����ŵ��¼�����
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
					// ���û�пͻ�I/OҪ�������߳��˳�
					if (0 == pThread->nSocketCount)
					{
						FreeThreadObj(pThread);
						return 0;
					}
					::WSAResetEvent(pThread->events[0]);
				}
				else
				{
					// ���Ҷ�Ӧ���׽��ֶ���ָ��,����HandleIO���������¼�
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