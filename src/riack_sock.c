/*
   Copyright 2012 Kaspar Pedersen

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "riack_sock.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#define __USE_POSIX
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#endif

int sock_init(void)
{
#ifdef WIN32
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return -1;
	}
#endif
	return 0;
}

void sock_cleanup()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void sock_close(int sockfd)
{
	if (sockfd > 0)
	{
	    #ifdef WIN32
		closesocket(sockfd);
		#else
		close(sockfd);
		#endif
	}
}

int sock_open(char* host, int port)
{
	char szPort[10];
	struct addrinfo hints;
	struct addrinfo *servinfo, *p;
	int rv, sockfd = -1;

	sprintf(szPort, "%d", port);
	memset( &hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	rv = getaddrinfo(host, szPort, &hints, &servinfo);
	if (rv != 0)
	{
		printf("getaddrinfo failed: %d\n", rv);
		return -1;
	}

	for (p = servinfo; p !=NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0)
		{
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0)
			{
				break;
			}
			else
			{
				sock_close(sockfd);
				sockfd = -1;
			}
		}
	}
	freeaddrinfo(servinfo);

	return sockfd;
}

int sock_recv(int sockfd, uint8_t* buff, int len)
{
	return recv(sockfd, (char*)buff, len, 0);
}

int sock_send(int sockfd, uint8_t* data, int len)
{
	return send(sockfd, (char*)data, len, 0);
}
