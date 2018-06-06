// TcpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include "../../common/initsock.h"
CInitSock initScok;		// 初始化Winsock库

int _tmain(int argc, _TCHAR* argv[])
{
	// 创建套接字
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sListen)
	{
		cout << "Failed socket()" << endl;
		return 0;
	}

	// 填充sockaddr_in结构
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(10000);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	//  绑定这个套接字到一个本地地址
	if (SOCKET_ERROR == ::bind(sListen, (LPSOCKADDR)&sin, sizeof(sin)))
	{
		cout << "Failed bind()" << endl;
		return 0;
	}

	// 进入监听模式
	if(SOCKET_ERROR == ::listen(sListen, 2))
	{
		cout << "Failed listen()" << endl;
		return 0;
	}

	// 循环接受客户的连接请求
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	SOCKET sClient;
	char szText[] = "TCP Server Demo! \r\n";
	
	while (TRUE)
	{
		// 接受一个新连接
		sClient = ::accept(sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (INVALID_SOCKET == sClient)
		{
			cout << "Failed accept()" << endl;
			continue;
		}
		cout << "接受到一个连接\r\n" << inet_ntoa(remoteAddr.sin_addr) << endl;

		// 向客户端发送数据
		::send(sClient, szText, strlen(szText), 0);

		// 关闭同客户端的连接
		::closesocket(sClient);
	}

	// 关闭监听套接字
	::closesocket(sListen);

	return 0;
}

