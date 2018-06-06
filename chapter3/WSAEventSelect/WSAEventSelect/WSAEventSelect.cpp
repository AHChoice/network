// WSAEventSelect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//#include <windows.h>
#include <iostream>
using namespace std;
#include "../../common/initsock.h"
#include <stdio.h>
CInitSock initSock;

int _tmain(int argc, _TCHAR* argv[])
{
	// 事件句柄和套接字句柄表
	WSAEVENT	eventArray[WSA_MAXIMUM_WAIT_EVENTS];
	SOCKET		sockArray[WSA_MAXIMUM_WAIT_EVENTS];
	int nEventTotal = 0;

	USHORT nPort = 10000;

	// 创建监听套接字
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nPort);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	if (SOCKET_ERROR == ::bind(sListen, (sockaddr*)&sin, sizeof(sin)))
	{
		cout << "Failed bind()" << endl;
		return -1;
	}
	::listen(sListen, 5);

	// 创建事件对象，并关联到新的套接字
	WSAEVENT event = ::WSACreateEvent();
	::WSAEventSelect(sListen, event, FD_ACCEPT|FD_CLOSE);

	// 添加到表中
	eventArray[nEventTotal] = event;
	sockArray[nEventTotal] = sListen;
	nEventTotal++;

	// 处理网络事件
	while (TRUE)
	{
		// 在所有的事件对象上等待
		int nIndex = ::WSAWaitForMultipleEvents(nEventTotal, eventArray, FALSE, WSA_INFINITE, FALSE);
		// 对每个事件调用WSAWaitForMultipleEvents函数，以便确定它的状态
		nIndex = nIndex - WSA_WAIT_EVENT_0;
		for (int i=nIndex; i<nEventTotal; ++i)
		{
			nIndex = ::WSAWaitForMultipleEvents(1, &eventArray[i], TRUE, 1000, FALSE);
			if (nIndex == WSA_WAIT_FAILED || nIndex == WSA_WAIT_TIMEOUT)
			{
				continue;
			}
			else
			{
				// 获取来自通知的消息，WSAEnumNetworkEvents函数会自动重置受信事件
				WSANETWORKEVENTS event;
				::WSAEnumNetworkEvents(sockArray[i], eventArray[i], &event);
				if (event.lNetworkEvents & FD_ACCEPT)	// 处理FD_ACCEPT消息
				{
					if (event.iErrorCode[FD_ACCEPT_BIT] == 0)
					{
						if (nEventTotal > WSA_MAXIMUM_WAIT_EVENTS)
						{
							cout << "too many connections!" << endl;
							continue;
						}
						SOCKET sNew = ::accept(sockArray[i], NULL, NULL);
						WSAEVENT event =  ::WSACreateEvent();
						::WSAEventSelect(sNew, event, FD_READ|FD_CLOSE|FD_WRITE);
						// 添加到表中
						eventArray[nEventTotal] = event;
						sockArray[nEventTotal] = sNew;
						nEventTotal++;
					}
				}
				else if (event.lNetworkEvents & FD_READ)
				{
					if (event.iErrorCode[FD_READ_BIT] == 0)
					{
						char szText[256];
						int nRecv = ::recv(sockArray[i], szText, strlen(szText), 0);
						if (nRecv > 0)
						{
							szText[nRecv] = '\0';
							cout << "接收到的数据 " << szText << endl;
						}
					}
				}
				else if (event.lNetworkEvents & FD_CLOSE)
				{
					if (event.iErrorCode[FD_CLOSE_BIT] == 0)
					{
						::closesocket(sockArray[i]);
						for (int j=i; j<nEventTotal-1; ++j)
						{
							sockArray[j] = sockArray[j+1];
							sockArray[j] = sockArray[j+1];
						}
						nEventTotal--;
					}
				}
				else if (event.lNetworkEvents & FD_WRITE)
				{
				}
			}
		}
	}
	return 0;
}
