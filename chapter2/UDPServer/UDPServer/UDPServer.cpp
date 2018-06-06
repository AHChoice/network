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
	// �����׽���
	SOCKET s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == s)
	{
		cout << "Failed socket()" << endl;
		return 0;
	}

	// ���sockaddr_in�ṹ
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(10000);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	// ������׽��ֵ�һ�����ص�ַ
	if (SOCKET_ERROR == ::bind(s, (LPSOCKADDR)&sin, sizeof(sin)))
	{
		cout << "Failed bind()" << endl;
		return 0;
	}

	// ��������
	char buff[1024];
	sockaddr_in addr;
	int nLen = sizeof(addr);
	
	while(TRUE)
	{
		int nRecv = ::recvfrom(s, buff, 1024, 0, (sockaddr*)&addr, &nLen);
		if (nRecv > 0)
		{
			buff[nRecv] = '\0';
			cout << "���յ������ݣ� " << inet_ntoa(addr.sin_addr) << buff << endl;
		}
	}

	::closesocket(s);
	return 0;
}

