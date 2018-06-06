
// UPDClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace std;
#include "../../common/initsock.h"
CInitSock initSock;


int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == s)
	{
		cout << "Failed socket()" << endl;
		return 0;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10000);
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	char szText[] = "UDP Server \r\n";
	while (TRUE)
	::sendto(s, szText, strlen(szText), 0, (sockaddr*)&addr, sizeof(addr));

	::closesocket(s);
	return 0;
}

