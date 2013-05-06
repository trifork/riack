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

#pragma warning( disable:4005 )
#define _CRT_SECURE_NO_WARNINGS

#include "riack.h"
#include "riack_msg.h"
#include "riack_helpers.h"
#include "riack_sock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "protocol/riak_msg_codes.h"
#include "protocol/riak_kv.pb-c.h"

ProtobufCAllocator riack_pb_allocator(struct RIACK_ALLOCATOR *allocator);

struct RIACK_CLIENT* riack_new_client(struct RIACK_ALLOCATOR *allocator)
{
	struct RIACK_CLIENT* result;
	if (allocator) {
		result = allocator->alloc(0, sizeof(struct RIACK_CLIENT));
		result->allocator = *allocator;
	} else {
		result = riack_default_allocator.alloc(0, sizeof(struct RIACK_CLIENT));
		result->allocator = riack_default_allocator;
	}
	result->sockfd = -1;
	result->last_error = 0;
	result->last_error_code = 0;
	result->host = 0;
	result->port = 0;
	result->options.recv_timeout_ms = 0;
	result->options.send_timeout_ms = 0;
	return result;
}

void riack_free(struct RIACK_CLIENT *client)
{
	if (client != 0) {
		if (client->last_error) {
			RFREE(client, client->last_error);
		}
		if (client->host) {
			RFREE(client, client->host);
		}
		riack_disconnect(client);
		client->allocator.free(0, client);
	}
}

void riack_init()
{
	sock_init();
}

void riack_cleanup()
{
	sock_cleanup();
}

int riack_connect(struct RIACK_CLIENT *client, const char* host, int port,
		struct RIACK_CONNECTION_OPTIONS* options)
{
	client->sockfd = sock_open(host, port);
	if (client->sockfd > 0) {
		if (client->host && host != client->host) {
			RFREE(client, client->host);
		}
		if (host != client->host) {
			client->host = (char*)RMALLOC(client, strlen(host)+1);
			strcpy(client->host, host);
		}
		client->port = port;
		if (options) {
			client->options = *options;
			if (!sock_set_timeouts(client->sockfd, options->recv_timeout_ms, options->send_timeout_ms)) {
				sock_close(client->sockfd);
				client->sockfd = -1;
				// TODO set last error
			}
		}
		return RIACK_SUCCESS;
	}
	return RIACK_ERROR_COMMUNICATION;
}

int riack_disconnect(struct RIACK_CLIENT *client)
{
	if (client->sockfd > 0) {
		sock_close(client->sockfd);
		client->sockfd = -1;
	}
	return RIACK_SUCCESS;
}

int riack_reconnect(struct RIACK_CLIENT *client)
{
	riack_disconnect(client);
	return riack_connect(client, client->host, client->port, &client->options);
}

int riack_ping(struct RIACK_CLIENT *client)
{
	int result;
	struct RIACK_PB_MSG ping_msg;
	struct RIACK_PB_MSG *ping_response;

	result = RIACK_ERROR_COMMUNICATION;
	ping_msg.msg_code = mc_RpbPingReq;
	ping_msg.msg_len = 0;
	if (riack_send_message(client, &ping_msg) > 0) {
		if (riack_receive_message(client, &ping_response) > 0) {
			if (ping_response->msg_code == mc_RpbPingResp) {
				result = RIACK_SUCCESS;
			} else {
				result = RIACK_ERROR_RESPONSE;
			}
			riack_message_free(client, &ping_response);
		}
	}
	return result;
}

void riack_got_error_response(struct RIACK_CLIENT *client, struct RIACK_PB_MSG *msg)
{
	RpbErrorResp *resp;
	ProtobufCAllocator pb_allocator;
	if (msg->msg_code == mc_RpbErrorResp) {
		pb_allocator = riack_pb_allocator(&client->allocator);
		if (client->last_error) {
			RFREE(client, client->last_error);
		}
		resp = rpb_error_resp__unpack(&pb_allocator, msg->msg_len, msg->msg);
		if (resp) {
			client->last_error_code = resp->errcode;
			riack_copy_buffer_to_string(client, &resp->errmsg, &client->last_error);
			rpb_error_resp__free_unpacked(resp, &pb_allocator);
		}
	}
}

int riack_server_info(struct RIACK_CLIENT *client, RIACK_STRING *node, RIACK_STRING* version)
{
	int result;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	RpbGetServerInfoResp *response;
	ProtobufCAllocator pb_allocator;
	msg_req.msg_code = mc_RpbGetServerInfoReq;
	msg_req.msg_len = 0;

	pb_allocator = riack_pb_allocator(&client->allocator);
	result = RIACK_ERROR_COMMUNICATION;
	if ((riack_send_message(client, &msg_req) > 0) &&
		(riack_receive_message(client, &msg_resp) > 0)) {
		if (msg_resp->msg_code == mc_RpbGetServerInfoResp) {
			response = rpb_get_server_info_resp__unpack(&pb_allocator, msg_req.msg_len, msg_req.msg);
			if (response) {
				if (response->has_node) {
					RMALLOCCOPY(client, node->value, node->len, response->node.data, response->node.len);
				} else {
					node->len = 0;
					node->value = 0;
				}
				if (response->has_server_version) {
					RMALLOCCOPY(client, version->value, version->len,
							response->server_version.data, response->server_version.len);
				} else {
					version->len = 0;
					version->value = 0;
				}
				// Copy responses
				rpb_get_server_info_resp__free_unpacked(response, &pb_allocator);
				result = RIACK_SUCCESS;
			} else {
				result = RIACK_FAILED_PB_UNPACK;
			}
		} else {
			riack_got_error_response(client, msg_resp);
			result = RIACK_ERROR_RESPONSE;
		}
		riack_message_free(client, &msg_resp);
	}
	return result;
}

void riack_timeout_test(struct RIACK_CLIENT* client)
{
	struct RIACK_PB_MSG *msg_resp;
	riack_receive_message(client, &msg_resp);
}

