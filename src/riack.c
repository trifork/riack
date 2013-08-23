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

int riack_reset_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket)
{
    int result;
    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    result = RIACK_ERROR_COMMUNICATION;
    //
    return result;
}

RpbBucketProps__RpbReplMode replmode_from_riack_replication_mode(enum RIACK_REPLICATION_MODE replication_mode) {
    switch (replication_mode) {
    case REALTIME_AND_FULLSYNC:
        return RPB_BUCKET_PROPS__RPB_REPL_MODE__TRUE;
    case REALTIME:
        return RPB_BUCKET_PROPS__RPB_REPL_MODE__REALTIME;
    case FULLSYNC:
        return RPB_BUCKET_PROPS__RPB_REPL_MODE__FULLSYNC;
    case DISABLED:
        break;
    }
    return RPB_BUCKET_PROPS__RPB_REPL_MODE__FALSE;
}

void riack_set_rpb_bucket_props(struct RIACK_CLIENT *client, struct RIACK_BUCKET_PROPERTIES* props, RpbBucketProps *rpb_props)
{
#define COPY_PROPERTY_HAS_TO_USE(FROM, TO, PROP_NAME_FROM, PROP_NAME_TO) if (FROM->PROP_NAME_FROM##_use) { \
                                                                            TO->has_##PROP_NAME_TO = 1; \
                                                                            TO->PROP_NAME_TO = FROM->PROP_NAME_FROM; \
                                                                            }
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, allow_mult, allow_mult);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, basic_quorum, basic_quorum);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, big_vclock, big_vclock);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, small_vclock, small_vclock);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, young_vclock, young_vclock);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, old_vclock, old_vclock);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, dw, dw);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, w, w);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, pw, pw);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, rw, rw);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, pr, pr);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, r, r);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, n_val, n_val);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, last_write_wins, last_write_wins);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, notfound_ok, notfound_ok);
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, search, search);
    if (props->has_postcommit_hooks) {
        rpb_props->n_postcommit = props->postcommit_hook_count;
        // TODO The actual hooks
    }
    if (props->has_precommit_hooks) {
        rpb_props->n_precommit = props->precommit_hook_count;
        // TODO
    }
    if (RSTR_HAS_CONTENT(props->backend)) {
        rpb_props->has_backend = 1;
        rpb_props->backend.len = props->backend.len;
        rpb_props->backend.data = (uint8_t*)RMALLOC(client, props->backend.len);
        memcpy(rpb_props->backend.data, props->backend.value, props->backend.len);
    }
    if (props->crash_keyfun_use) {
        // TODO
    }
    if (props->linkfun_use) {
        // TODO
    }
    if (props->replication_mode_use) {
        rpb_props->has_repl = 1;
        rpb_props->repl = replmode_from_riack_replication_mode(props->replication_mode);
    }

/*
  RpbCommitHook **precommit;
  RpbCommitHook **postcommit;
  RpbModFun *chash_keyfun;
  RpbModFun *linkfun;
*/
    /*
struct  _RpbModFun
{
  ProtobufCBinaryData module;
  ProtobufCBinaryData function;
};
*/
}

int riack_set_bucket_props_ext(struct RIACK_CLIENT *client, RIACK_STRING bucket, struct RIACK_BUCKET_PROPERTIES* properties)
{
    int result;
    RpbSetBucketReq set_request = RPB_SET_BUCKET_REQ__INIT;
    RpbBucketProps bck_props = RPB_BUCKET_PROPS__INIT;
    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    result = RIACK_ERROR_COMMUNICATION;
    //
    return result;
}


int riack_set_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t n_val, uint8_t allow_mult)
{
    int result;
    struct RIACK_PB_MSG msg_req, *msg_resp;
    size_t packed_size;
    uint8_t *request_buffer;
    RpbSetBucketReq set_request = RPB_SET_BUCKET_REQ__INIT;
    RpbBucketProps bck_props = RPB_BUCKET_PROPS__INIT;

    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }

    result = RIACK_ERROR_COMMUNICATION;
    bck_props.has_allow_mult = 1;
    bck_props.allow_mult = allow_mult;
    bck_props.has_n_val = 1;
    bck_props.n_val = n_val;
    set_request.props = &bck_props;
    set_request.bucket.len = bucket.len;
    set_request.bucket.data = (uint8_t*)bucket.value;
    packed_size = rpb_set_bucket_req__get_packed_size(&set_request);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_set_bucket_req__pack(&set_request, request_buffer);
        msg_req.msg_code = mc_RpbSetBucketReq;
        msg_req.msg_len = packed_size;
        msg_req.msg = request_buffer;
        if ((riack_send_message(client, &msg_req) > 0)&&
            (riack_receive_message(client, &msg_resp) > 0))
        {
            if (msg_resp->msg_code == mc_RpbSetBucketResp) {
                result = RIACK_SUCCESS;
            } else {
                riack_got_error_response(client, msg_resp);
                result = RIACK_ERROR_RESPONSE;
            }
            riack_message_free(client, &msg_resp);
        }
        RFREE(client, request_buffer);
    }
    return result;
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

