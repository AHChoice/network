#include "../../common/initsock.h"
#include <iostream>
#include <stdio.h>
#define  WM_SOCKET WM_USER + 101  
CInitSock the_sock;

LRESULT CALLBACK(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

int main()
{
	char sz_classname[] = TEXT("MainClass");
	WNDCLASSEX wndclass;

	// Fill the WNDCLASSEX structure with parameters describing the main window
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = WindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
	wndclass.lpszClassName = sz_classname;
	wndclass.hIconSm = NULL;
	::RegisterClassEx(&wndclass);

	// create main window
	HWND hwnd = ::CreateWindowEx(
		0,
		sz_classname,
		TEXT(""),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL);
	if (NULL == hwnd)
	{
		::MessageBoxEx(NULL, TEXT("create error"), TEXT("ERROR"), MB_OK);
		return -1;
	}

	USHORT nport = 4567;
	
	// create listen socket
	SOCKET s_listen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(nport);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	
	// bind socket to local machine
	if (SOCKET_ERROR == ::bind(s_listen, (sockaddr*)&sin, sizeof(sin)))
	{
		std::cout << "Failed bind()" << std::endl;
		return -1;
	}

	::WSAAsyncSelect(s_listen, hwnd, WM_SOCKET, FD_ACCEPT|FD_CLOSE);

	::listen(s_listen, 5);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return msg.wParam;
}