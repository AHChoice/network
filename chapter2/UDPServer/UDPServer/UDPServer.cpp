// UDPServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include "../../common/initsock.h"
CInitSock initSock;

int _tmain(int argc, _TCHAR* argv[])
{
	// 创建套接字
	SOCKET s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == s)
	{
		cout << "Failed socket()" << endl;
		return 0;
	}

	// 填充sockaddr_in结构
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(10000);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	// 绑定这个套接字到一个本地地址
	if (SOCKET_ERROR == ::bind(s, (LPSOCKADDR)&sin, sizeof(sin)))
	{
		cout << "Failed bind()" << endl;
		return 0;
	}

	// 接收数据
	char buff[1024];
	sockaddr_in addr;
	int nLen = sizeof(addr);
	
	while(TRUE)
	{
		int nRecv = ::recvfrom(s, buff, 1024, 0, (sockaddr*)&addr, &nLen);
		if (nRecv > 0)
		{
			buff[nRecv] = '\0';
			cout << "接收到的数据： " << inet_ntoa(addr.sin_addr) << buff << endl;
		}
	}

	::closesocket(s);
	return 0;
}

