// TcpServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
#include <stdio.h>
#include "../../common/initsock.h"
CInitSock initScok;		// ��ʼ��Winsock��

int _tmain(int argc, _TCHAR* argv[])
{
	// �����׽���
	SOCKET sListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sListen)
	{
		cout << "Failed socket()" << endl;
		return 0;
	}

	// ���sockaddr_in�ṹ
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(10000);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;

	//  ������׽��ֵ�һ�����ص�ַ
	if (SOCKET_ERROR == ::bind(sListen, (LPSOCKADDR)&sin, sizeof(sin)))
	{
		cout << "Failed bind()" << endl;
		return 0;
	}

	// �������ģʽ
	if(SOCKET_ERROR == ::listen(sListen, 2))
	{
		cout << "Failed listen()" << endl;
		return 0;
	}

	// ѭ�����ܿͻ�����������
	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	SOCKET sClient;
	char szText[] = "TCP Server Demo! \r\n";
	
	while (TRUE)
	{
		// ����һ��������
		sClient = ::accept(sListen, (SOCKADDR*)&remoteAddr, &nAddrLen);
		if (INVALID_SOCKET == sClient)
		{
			cout << "Failed accept()" << endl;
			continue;
		}
		cout << "���ܵ�һ������\r\n" << inet_ntoa(remoteAddr.sin_addr) << endl;

		// ��ͻ��˷�������
		::send(sClient, szText, strlen(szText), 0);

		// �ر�ͬ�ͻ��˵�����
		::closesocket(sClient);
	}

	// �رռ����׽���
	::closesocket(sListen);

	return 0;
}

