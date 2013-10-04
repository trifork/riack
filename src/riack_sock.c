/*
   Copyright 2012 Trifork A/S
   Author: Kaspar Pedersen

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
#define _CRT_SECURE_NO_WARNINGS
#include "riack_sock.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#define __USE_POSIX
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/protosw.h>
#endif

int sock_init(void)
{
#ifdef WIN32
	WSADATA wsaData;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
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
	if (sockfd > 0) {
	    #ifdef WIN32
		closesocket(sockfd);
		#else
		close(sockfd);
		#endif
	}
}

int sock_open(const char* host, int port)
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
	if (rv != 0) {
		printf("getaddrinfo failed: %d\n", rv);
		return -1;
	}

	for (p = servinfo; p !=NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0) {
			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
                int flag = 1;
                setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (void*)&flag, sizeof(int));
				break;
			} else {
				sock_close(sockfd);
				sockfd = -1;
			}
		}
	}
	freeaddrinfo(servinfo);

	return sockfd;
}

#ifndef WIN32
struct timeval timeval_from_ms(uint32_t ms)
{
	uint32_t timeout_secs, timeout_usecs;
	struct timeval timeout;
	timeout_secs = ms / 1000;
	timeout_usecs = (ms - (timeout_secs * 1000)) * 1000;
	timeout.tv_sec =  timeout_secs;
	timeout.tv_usec = timeout_usecs;
	return timeout;
}
#endif

int sock_set_keep_alive(int sockfd)
{
    int one;
    one = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void*)&one, sizeof(one)) < 0) {
        return 0;
    }
    return 1;
}

int sock_set_timeouts(int sockfd, uint32_t recv_timeout, uint32_t send_timeout)
{
#ifdef WIN32
	uint32_t recv_timeout_rdy, send_timeout_rdy;
	recv_timeout_rdy = recv_timeout;
	send_timeout_rdy = send_timeout;
#else
	struct timeval recv_timeout_rdy, send_timeout_rdy;
	recv_timeout_rdy = timeval_from_ms(recv_timeout);
	send_timeout_rdy = timeval_from_ms(send_timeout);
#endif
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (void*)&recv_timeout_rdy, sizeof(recv_timeout_rdy)) < 0) {
		return 0;
	}
	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (void*)&send_timeout_rdy, sizeof(send_timeout_rdy)) < 0) {
		return 0;
	}
	return 1;
}

int sock_recv(int sockfd, uint8_t* buff, int len)
{
	int recieved;
	int offset = 0;
	while (offset < len) {
		recieved = recv(sockfd, (char*)buff + offset, len - offset, 0);
		if (recieved < 0) {
			if (errno == EINTR) {
				/* Try again */
				continue;
			}
			return recieved;
		}
		if (recieved == 0) {
			break;
		}
		offset += recieved;
	}
	return offset;
}

int sock_send(int sockfd, uint8_t* data, int len)
{
	int sent;
	int offset = 0;
	while (offset < len) {
		sent = send(sockfd, (char*)data + offset, len - offset, 0);
		if (sent < 0) {
			if (errno == EINTR) {
			/* Try again */
				continue;
			}
			return sent;
		} else if (sent == 0) {
			break;
		}
		offset += sent;
	}
	return offset;
}
