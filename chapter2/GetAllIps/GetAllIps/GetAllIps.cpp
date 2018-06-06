// GetAllIps.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "../../common/initsock.h"
using namespace std;
CInitSock initSock;

int _tmain(int argc, _TCHAR* argv[])
{
	char szHost[256];

	// 取得主机名称
	::gethostname(szHost, 256);  

	// 通过主机名得到地址信息
	hostent *pHost = ::gethostbyname(szHost);  

	// 打印出所有IP地址
	in_addr addr;
	for (int i=0; ; ++i)
	{
		char *p = pHost->h_addr_list[i];
		if (NULL == p)
			break;

		memcpy(&addr.S_un.S_addr, p, pHost->h_length);
		char *szIp = ::inet_ntoa(addr);
		cout << "local ip address: " << szIp << endl;
	}

	return 0;
}

