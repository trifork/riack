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
#ifndef __RIACK__SOCK__H__
#define __RIACK__SOCK__H__

#include "ints.h"

int sock_init(void);
void sock_cleanup(void);

int sock_open(const char* host, int port);
void sock_close(int sockfd);

int sock_set_timeouts(int sockfd, uint32_t recv_timeout, uint32_t send_timeout);
int sock_set_keep_alive(int sockfd);

int sock_send(int sockfd, uint8_t* data, int len);
int sock_recv(int sockfd, uint8_t* buff, int len);

int ssl_sock_send(void *ssl, uint8_t* data, int len);
int ssl_sock_recv(void *ssl, uint8_t* buff, int len);

#endif // __RIACK__SOCK__H__
