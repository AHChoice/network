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

	// ȡ����������
	::gethostname(szHost, 256);  

	// ͨ���������õ���ַ��Ϣ
	hostent *pHost = ::gethostbyname(szHost);  

	// ��ӡ������IP��ַ
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

