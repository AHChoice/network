// TcpClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../common/initsock.h"
#include <iostream>
using namespace std;
#include <stdio.h>
CInitSock initSock;

int _tmain(int argc, _TCHAR* argv[])
{
	// �����׽���
	SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == s)
	{
		cout << "Failed socked()" << endl;
		return 0;
	}

	// Ҳ�������������bind������һ�����ص�ַ
	// ����ϵͳ�����Զ�����

	//����д��ַԶ����Ϣ
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(10000);
	// ע�⣬����Ҫ��д�������������ڻ����ĵ�ַ��IP��ַ
	// �����ļ����û��������ֱ��ʹ��127.0.0.1����
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (-1 == ::connect(s, (sockaddr*)&servAddr, sizeof(servAddr)))
	{
		cout << "Failed connect()!" << endl;
		return 0;
	}

	// ��������
	char buff[256];
	int nRecv = ::recv(s, buff, 256, 0);
	if (nRecv > 0)
	{
		buff[nRecv] = '\0';
		cout << "���յ�������" << buff << endl;
	}

	// �ر��׽���
	::closesocket(s);
	return 0;

}

