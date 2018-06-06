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
	// 创建套接字
	SOCKET s = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == s)
	{
		cout << "Failed socked()" << endl;
		return 0;
	}

	// 也可以在这里调用bind函数绑定一个本地地址
	// 否则系统将会自动安排

	//　填写地址远程信息
	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(10000);
	// 注意，这里要填写服务器程序所在机器的地址的IP地址
	// 如果你的计算机没有联网，直接使用127.0.0.1即可
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	if (-1 == ::connect(s, (sockaddr*)&servAddr, sizeof(servAddr)))
	{
		cout << "Failed connect()!" << endl;
		return 0;
	}

	// 接收数据
	char buff[256];
	int nRecv = ::recv(s, buff, 256, 0);
	if (nRecv > 0)
	{
		buff[nRecv] = '\0';
		cout << "接收到的数据" << buff << endl;
	}

	// 关闭套接字
	::closesocket(s);
	return 0;

}

