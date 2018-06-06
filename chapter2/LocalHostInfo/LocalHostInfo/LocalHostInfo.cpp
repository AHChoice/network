// LocalHostInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
using namespace std;
#include <iomanip>
#include <Windows.h>
#include "IPHlpApi.h"
#pragma comment(lib, "IPHlpApi.lib")
#pragma comment(lib, "WS2_32.lib")

// 全局数据
u_char	g_ucLocalMac[6];	// 本地MAC地址
DWORD	g_dwGatewayIP;		// 网关IP地址
DWORD	g_dwLcoalIP;		// 本地IP地址
DWORD	g_dwMask;			// 子网掩码

BOOL GetGlobalData()
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;
	ULONG ulLen = 0;

	// 为适配器结构申请内存
	::GetAdaptersInfo(pAdapterInfo, &ulLen);
	pAdapterInfo = (PIP_ADAPTER_INFO)::GlobalAlloc(GPTR, ulLen);

	// 取得本地适配器结构信息
	if (ERROR_SUCCESS == ::GetAdaptersInfo(pAdapterInfo, &ulLen))
	{
		if(pAdapterInfo != NULL)
		{
			memcpy(g_ucLocalMac, pAdapterInfo->Address, 6);
			g_dwGatewayIP = inet_addr(pAdapterInfo->GatewayList.IpAddress.String);
			g_dwLcoalIP = inet_addr(pAdapterInfo->IpAddressList.IpAddress.String);
			g_dwMask = inet_addr(pAdapterInfo->IpAddressList.IpMask.String);
		}
	}

	cout << "\n-------------local host information-------------\n";
	in_addr in;
	in.S_un.S_addr = g_dwLcoalIP;
	cout << setw(20) <<" IP Address :" << ::inet_ntoa(in) << endl;
	in.S_un.S_addr = g_dwMask;
	cout << setw(20) << " Subnet Mask :" << ::inet_ntoa(in) << endl;
	in.S_un.S_addr = g_dwGatewayIP;
	cout << setw(20) <<" Default GateWay :" << ::inet_ntoa(in) << endl;
	u_char *p = g_ucLocalMac;
	cout << setw(20) << "MAC Address :";
	printf("%02X-%02X-%02X-%02X-%02X-%02X \n", p[0], p[1], p[2], p[3], p[4], p[5]);
	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	GetGlobalData();
	return 0;
}

