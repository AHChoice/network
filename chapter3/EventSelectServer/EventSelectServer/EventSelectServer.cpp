// EventSelectServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../common/initsock.h"
#include <iostream>
using namespace std;
#include "EventSelectServer.h"

int _tmain(int argc, _TCHAR* argv[])
{
	USHORT nPort = 10000;  // 次服务器监听的端口号

	// 创建监听套接字
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (SOCKET_ERROR == ::bind(sListen, (sockaddr*)&sin, sizeof(sin)))
	{
		cout << " failed bind()! " << endl;
		return 0;
	}
	::listen(sListen, 200);

	// 创建事件对象，并关联到监听的套接字
	WSAEVENT event = ::WSACreateEvent();
	::WSAEventSelect(sListen, event, FD_ACCEPT|FD_CLOSE);

	::InitializeCriticalSection(&g_cs);

	// 处理客户的连接请求，打印状态信息
	while (TRUE)
	{
		int nRet = ::WaitForSingleObject(event, 5*1000);
		if (WSA_WAIT_FAILED == nRet)
		{
			cout << " failed WaitForSingleObject() " << endl;
			break;
		}
		else if(WSA_WAIT_TIMEOUT)
		{
			cout << "\n";
			cout << " TotalConnections: " << g_nTatolConnections << endl;
			cout << " CurrentConnections: " << g_nCurrentConnections << endl;
			continue;
		}
		else
		{
			::ResetEvent(event);
			// 循环处理所有未解决的连接请求
			while (TRUE)
			{
				sockaddr_in si;
				int nLen = sizeof(si);
				SOCKET sNew = ::accept(sListen, (sockaddr*)&si, &nLen);
				if (SOCKET_ERROR == sNew)
					break;
				PSOCKET_OBJ pSocket = GetSocketObj(sNew);
				pSocket->addrRemote = si;
				::WSAEventSelect(pSocket->s, pSocket->event, FD_READ|FD_CLOSE|FD_WRITE);
				AssignToFreeThread(pSocket);
			}
		}
	}

	::DeleteCriticalSection(&g_cs);
	return 0;
}

