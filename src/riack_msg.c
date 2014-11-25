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
#include "riack_msg.h"
#include "riack_sock.h"
#include "protocol/riak_msg_codes.h"

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <netinet/in.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


void dbg_print_message(riack_pb_msg * pmsg)
{
	char buff_print[100];
    if (pmsg != 0) {
		printf("***********************\n");
		riak_get_msg_description(pmsg->msg_code, buff_print, sizeof(buff_print));
		printf("* %s\n", buff_print);
		printf("***********************\n");
    } else {
		printf("pmsg = 0\n");
	}
}

void riack_message_free(riack_client *client, riack_pb_msg ** ppMsg)
{
	riack_pb_msg * pMsg = *ppMsg;
    if (pMsg != 0) {
        if (pMsg->msg_len > 0 && pMsg->msg != 0) {
			RFREE(client, pMsg->msg);
		}
		RFREE(client, pMsg);
	}
	*ppMsg = 0;
}

int riack_receive_message(riack_client *client, riack_pb_msg ** msg)
{
	riack_pb_msg * recvMsg;
	uint32_t msgLenN;
	int rcvBytes = 0;
	if (msg == 0)
		return 0;
	*msg = 0;
	recvMsg = (riack_pb_msg *)RMALLOC(client, sizeof(riack_pb_msg));
	recvMsg->msg_len = 0;
	recvMsg->msg = 0;
	rcvBytes = sock_recv(client->sockfd, (uint8_t*)&msgLenN, 4);
    if (rcvBytes == 4) {
		rcvBytes = sock_recv(client->sockfd, &recvMsg->msg_code, 1);
        if (rcvBytes == 1) {
			recvMsg->msg_len = ntohl(msgLenN)-1;
            if (recvMsg->msg_len > 0) {
				recvMsg->msg = (uint8_t*)RMALLOC(client, recvMsg->msg_len);
				rcvBytes = sock_recv(client->sockfd, recvMsg->msg, recvMsg->msg_len);
                if (rcvBytes == recvMsg->msg_len) {
					*msg = recvMsg;
					return recvMsg->msg_len + 5;
				}
            } else {
				*msg = recvMsg;
				return 5;
			}
		}
	}
	riack_message_free(client, &recvMsg);
	return -1;
}

int riack_send_message(riack_client *client, riack_pb_msg * msg)
{
	uint8_t *buf;
	uint32_t netlen, sendlen;

	sendlen = 5 + msg->msg_len;
	buf = (uint8_t*)RMALLOC(client, sendlen);
	netlen = htonl(msg->msg_len+1);
	memcpy(buf, &netlen, 4);
	buf[4] = msg->msg_code;
    if (msg->msg_len > 0) {
		memcpy(buf+5, msg->msg, (int)msg->msg_len);
	}
	// first send header
    if (sock_send(client->sockfd, buf, sendlen) != sendlen) {
        // printf("sock_send failed to send correct number of bytes\n");
		// Error failed to send bytes most have lost connection
		RFREE(client, buf);
		return -1;
	}
	RFREE(client, buf);
	return sendlen;
}
