#include "initsock.h"

CInitSock thesock;
#include <iostream>

int main()
{
	USHORT n_port = 10000;
	
	// create socket
	SOCKET s_listen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(n_port);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	
	// bind socket to local machine
	if (SOCKET_ERROR == ::bind(s_listen, (sockaddr*)&sin, sizeof(sin)))
	{
		std::cout << "Failed bind()" << std::endl;
		return -1;
	}

	// in listen model 
	::listen(s_listen, 5);

	// select model
	// 1.init a socket set fd_socked
	fd_set fd_socket;
	FD_ZERO(&fd_socket);
	FD_SET(s_listen, &fd_socket);

	while(TRUE)
	{
		// 2.copy fd_socket to select function
		fd_set fd_read = fd_socket;
		int n_ret = ::select(0, &fd_read, NULL, NULL, NULL);
		if (n_ret > 0)
		{
			// 3.compare fd_socket with fd_read
			for (int i=0; i<(int)fd_socket.fd_count; ++i)
			{
				// (1) listening socket receives a new connection
				if (FD_ISSET(fd_socket.fd_array[i], &fd_read)) 
				{
					if (fd_socket.fd_count < FD_SETSIZE)
					{
						sockaddr_in addr_remote;
						int n_addrlen = sizeof(addr_remote);
						SOCKET s_new = ::accept(s_listen,
							(SOCKADDR*)&addr_remote, 
							&n_addrlen);
						FD_SET(s_new, &fd_socket);
						std::cout << "receive a new connection"
								  << ::inet_ntoa(addr_remote.sin_addr) 
								  << std::endl;
					}
					else
					{
						std::cout << "too much connections!" << std::endl;
						continue;
					}
				}
				else
				{
					char sz_text[256];
					int n_recv = ::recv(fd_socket.fd_array[i],
						sz_text, strlen(sz_text), 0);
					if (n_recv > 0)
					{
						sz_text[n_recv] = '\0';
						std::cout << "receive data " << sz_text << std::endl;
					}
					else
					{
						::closesocket(fd_socket.fd_array[i]);
						FD_CLR(fd_socket.fd_array[i], &fd_socket);
					}
				}
			}
				
		}
	}

	return 0;
}