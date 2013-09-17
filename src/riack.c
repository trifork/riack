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
    size_t packed_size;
    RpbResetBucketReq reset_req;
    struct RIACK_PB_MSG msg_req;
    struct RIACK_PB_MSG *msg_resp;
    uint8_t *request_buffer;

    if (!client || !RSTR_HAS_CONTENT(bucket)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    result = RIACK_ERROR_COMMUNICATION;
    reset_req.bucket.data = (uint8_t*)bucket.value;
    reset_req.bucket.len = bucket.len;
    packed_size = rpb_reset_bucket_req__get_packed_size(&reset_req);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer)  {
        rpb_reset_bucket_req__pack(&reset_req, request_buffer);
        msg_req.msg_code = mc_RpbResetBucketReq;
        msg_req.msg_len = packed_size;
        msg_req.msg = request_buffer;
        if ((riack_send_message(client, &msg_req) > 0)&&
            (riack_receive_message(client, &msg_resp) > 0))
        {
            if (msg_resp->msg_code == mc_RpbResetBucketResp) {
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

enum RIACK_REPLICATION_MODE riack_replication_mode_from_replmode(RpbBucketProps__RpbReplMode rpb_repl_mode) {
    switch (rpb_repl_mode) {
    case RPB_BUCKET_PROPS__RPB_REPL_MODE__TRUE:
        return REALTIME_AND_FULLSYNC;
    case RPB_BUCKET_PROPS__RPB_REPL_MODE__REALTIME:
        return REALTIME;
    case RPB_BUCKET_PROPS__RPB_REPL_MODE__FULLSYNC:
        return FULLSYNC;
    }
    return DISABLED;
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

RpbCommitHook** riack_hooks_to_rpb_hooks(struct RIACK_CLIENT *client,
                                         struct RIACK_COMMIT_HOOK* hooks, size_t hook_count) {
    size_t i;
    RpbCommitHook** result;
    if (hook_count == 0) {
        return NULL;
    }
    result = RMALLOC(client, sizeof(RpbCommitHook *) * hook_count);
    for (i=0; i<hook_count; ++i) {
        if (RSTR_HAS_CONTENT(hooks[i].name)) {
            result[i]->has_name = 1;
            RMALLOCCOPY(client, result[i]->name.data, result[i]->name.len,
                        hooks[i].name.value, hooks[i].name.len);
        } else {
            result[i]->has_name = 0;
            result[i]->name.data = 0;
            result[i]->name.len = 0;
        }
        result[i]->modfun = (RpbModFun*)RMALLOC(client, sizeof(RpbModFun));
        RMALLOCCOPY(client, result[i]->modfun->function.data, result[i]->modfun->function.len,
                    hooks[i].modfun.function.value, hooks[i].modfun.function.len);
        RMALLOCCOPY(client, result[i]->modfun->module.data, result[i]->modfun->module.len,
                    hooks[i].modfun.module.value, hooks[i].modfun.module.len);
    }
    return result;
}

// Frees all members of a RpbModFun, but not the RpbModFun* itself
void riack_free_copied_rpb_mod_fun(struct RIACK_CLIENT *client, RpbModFun* rpb_modfun) {
    if (rpb_modfun->function.len > 0 && rpb_modfun->function.data) {
        RFREE(client, rpb_modfun->function.data);
    }
    if (rpb_modfun->module.len > 0 && rpb_modfun->module.data) {
        RFREE(client, rpb_modfun->module.data);
    }
}

// Frees a RpbCommitHook, but not the RpbCommitHook* itself
void riack_free_copied_commit_hook(struct RIACK_CLIENT *client, RpbCommitHook* rpb_hook) {
    if (rpb_hook->has_name && rpb_hook->name.len > 0) {
        RFREE(client, rpb_hook->name.data);
    }
    if (rpb_hook->modfun) {
        riack_free_copied_rpb_mod_fun(client, rpb_hook->modfun);
        RFREE(client, rpb_hook->modfun);
    }
}

// Frees all members of a RpbBucketProps, but not the RpbBucketProps* itself
void riack_free_copied_rpb_bucket_props(struct RIACK_CLIENT *client, RpbBucketProps *rpb_props) {
    if (rpb_props->has_backend && rpb_props->backend.len > 0) {
        RFREE(client, rpb_props->backend.data);
    }
    if (rpb_props->chash_keyfun) {
        riack_free_copied_rpb_mod_fun(client, rpb_props->chash_keyfun);
        RFREE(client, rpb_props->chash_keyfun);
    }
    if (rpb_props->linkfun) {
        riack_free_copied_rpb_mod_fun(client, rpb_props->linkfun);
        RFREE(client, rpb_props->linkfun);
    }
    if (rpb_props->n_postcommit > 0) {
        size_t i;
        for (i=0; i<rpb_props->n_postcommit; ++i) {
            riack_free_copied_commit_hook(client, rpb_props->postcommit[i]);
            RFREE(client, rpb_props->postcommit[i]);
        }
        RFREE(client, rpb_props->postcommit);
    }
    if (rpb_props->n_precommit > 0) {
        size_t i;
        for (i=0; i<rpb_props->n_precommit; ++i) {
            riack_free_copied_commit_hook(client, rpb_props->precommit[i]);
            RFREE(client, rpb_props->precommit[i]);
        }
        RFREE(client, rpb_props->precommit);
    }
}

void riack_free_commit_hooks(struct RIACK_CLIENT *client, struct RIACK_COMMIT_HOOK* hooks, size_t hook_count) {
    size_t i;
    for (i=0; i<hook_count; ++i) {
        RSTR_SAFE_FREE(client, hooks[i].name);
        RSTR_SAFE_FREE(client, hooks[i].modfun.function);
        RSTR_SAFE_FREE(client, hooks[i].modfun.module);
    }
    RFREE(client, hooks);
}

void riack_free_bucket_properties(struct RIACK_CLIENT *client, struct RIACK_BUCKET_PROPERTIES** properties) {
    if (*properties) {
        if (RSTR_HAS_CONTENT((*properties)->backend)) {
            RFREE(client, (*properties)->backend.value);
        }
        riack_free_commit_hooks(client, (*properties)->precommit_hooks, (*properties)->precommit_hook_count);
        riack_free_commit_hooks(client, (*properties)->postcommit_hooks, (*properties)->postcommit_hook_count);
        if ((*properties)->chash_keyfun_use) {
            RSTR_SAFE_FREE(client, (*properties)->chash_keyfun.function);
            RSTR_SAFE_FREE(client, (*properties)->chash_keyfun.module);
        }
        if ((*properties)->linkfun_use) {
            RSTR_SAFE_FREE(client, (*properties)->linkfun.function);
            RSTR_SAFE_FREE(client, (*properties)->linkfun.module);
        }
        RFREE(client, *properties);
    }
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
        rpb_props->postcommit = riack_hooks_to_rpb_hooks(client, props->postcommit_hooks, props->postcommit_hook_count);
    }
    if (props->has_precommit_hooks) {
        rpb_props->precommit = riack_hooks_to_rpb_hooks(client, props->precommit_hooks, props->precommit_hook_count);
        rpb_props->n_precommit = props->precommit_hook_count;
    }
    if (RSTR_HAS_CONTENT(props->backend)) {
        rpb_props->has_backend = 1;
        rpb_props->backend.len = props->backend.len;
        rpb_props->backend.data = (uint8_t*)RMALLOC(client, props->backend.len);
        memcpy(rpb_props->backend.data, props->backend.value, props->backend.len);
    }
    if (props->chash_keyfun_use) {
        rpb_props->chash_keyfun = (RpbModFun*)RMALLOC(client, sizeof(RpbModFun));
        RMALLOCCOPY(client, rpb_props->chash_keyfun->function.data, rpb_props->chash_keyfun->function.len,
                    props->chash_keyfun.function.value, props->chash_keyfun.function.len);
        RMALLOCCOPY(client, rpb_props->chash_keyfun->module.data, rpb_props->chash_keyfun->module.len,
                    props->chash_keyfun.module.value, props->chash_keyfun.module.len);
    }
    if (props->linkfun_use) {
        rpb_props->linkfun = (RpbModFun*)RMALLOC(client, sizeof(RpbModFun));
        RMALLOCCOPY(client, rpb_props->linkfun->function.data, rpb_props->linkfun->function.len,
                    props->linkfun.function.value, props->linkfun.function.len);
        RMALLOCCOPY(client, rpb_props->linkfun->module.data, rpb_props->linkfun->module.len,
                    props->linkfun.module.value, props->linkfun.module.len);
    }
    if (props->replication_mode_use) {
        rpb_props->has_repl = 1;
        rpb_props->repl = replmode_from_riack_replication_mode(props->replication_mode);
    }
}

int riack_set_bucket_props_base(struct RIACK_CLIENT *client, RpbSetBucketReq *set_request) {
    struct RIACK_PB_MSG msg_req, *msg_resp;
    uint8_t *request_buffer;
    size_t packed_size;
    int result;
    result = RIACK_ERROR_COMMUNICATION;
    packed_size = rpb_set_bucket_req__get_packed_size(set_request);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_set_bucket_req__pack(set_request, request_buffer);
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

int riack_set_bucket_props_ext(struct RIACK_CLIENT *client, RIACK_STRING bucket, struct RIACK_BUCKET_PROPERTIES* properties)
{
    RpbSetBucketReq set_request = RPB_SET_BUCKET_REQ__INIT;
    RpbBucketProps bck_props = RPB_BUCKET_PROPS__INIT;
    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    riack_set_rpb_bucket_props(client, properties, &bck_props);
    set_request.props = &bck_props;
    set_request.bucket.len = bucket.len;
    set_request.bucket.data = (uint8_t*)bucket.value;

    return riack_set_bucket_props_base(client, &set_request);
}

int riack_set_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t n_val, uint8_t allow_mult)
{
    RpbSetBucketReq set_request = RPB_SET_BUCKET_REQ__INIT;
    RpbBucketProps bck_props = RPB_BUCKET_PROPS__INIT;
    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    bck_props.has_allow_mult = 1;
    bck_props.allow_mult = allow_mult;
    bck_props.has_n_val = 1;
    bck_props.n_val = n_val;
    set_request.props = &bck_props;
    set_request.bucket.len = bucket.len;
    set_request.bucket.data = (uint8_t*)bucket.value;
    return riack_set_bucket_props_base(client, &set_request);
}

struct RIACK_MODULE_FUNCTION riack_rpb_modfun_to_modfun(struct RIACK_CLIENT *client, RpbModFun *rpb_modfun) {
    struct RIACK_MODULE_FUNCTION result;
    memset(&result, 0, sizeof(struct RIACK_MODULE_FUNCTION));
    if (rpb_modfun && rpb_modfun->function.len > 0) {
        RMALLOCCOPY(client, result.function.value, result.function.len, rpb_modfun->function.data, rpb_modfun->function.len);
    }
    if (rpb_modfun && rpb_modfun->module.len > 0) {
        RMALLOCCOPY(client, result.module.value, result.module.len, rpb_modfun->module.data, rpb_modfun->module.len);
    }
    return result;
}

struct RIACK_COMMIT_HOOK* riack_rpb_hooks_to_hooks(struct RIACK_CLIENT *client, RpbCommitHook ** rpb_hooks, size_t hook_count)
{
    size_t i;
    struct RIACK_COMMIT_HOOK* result;
    if (hook_count == 0) return 0;

    result = RCALLOC(client, sizeof(struct RIACK_COMMIT_HOOK*) * hook_count);
    for (i=0; i<hook_count; ++i) {
        if (rpb_hooks[i]->has_name) {
            RMALLOCCOPY(client, result[i].name.value, result[i].name.len, rpb_hooks[i]->name.data, rpb_hooks[i]->name.len);
        }
        result[i].modfun = riack_rpb_modfun_to_modfun(client, rpb_hooks[i]->modfun);
    }
    return result;
}

struct RIACK_BUCKET_PROPERTIES* riack_riack_bucket_props_from_rpb(struct RIACK_CLIENT *client, RpbBucketProps* rpb_props) {
#define COPY_PROPERTY_USE_TO_HAS(FROM, TO, PROP_NAME_FROM, PROP_NAME_TO) if (FROM->has_##PROP_NAME_FROM) { \
                                                                            TO->PROP_NAME_TO##_use = 1; \
                                                                            TO->PROP_NAME_TO = FROM->PROP_NAME_FROM;}
    struct RIACK_BUCKET_PROPERTIES* result = NULL;
    result = RMALLOC(client, sizeof(struct RIACK_BUCKET_PROPERTIES));
    if (result) {
        memset(result, 0, sizeof(struct RIACK_BUCKET_PROPERTIES));
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, allow_mult, allow_mult);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, basic_quorum, basic_quorum);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, big_vclock, big_vclock);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, dw, dw);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, last_write_wins, last_write_wins);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, notfound_ok, notfound_ok);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, notfound_ok, notfound_ok);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, n_val, n_val);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, old_vclock, old_vclock);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, pr, pr);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, pw, pw);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, r, r);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, rw, rw);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, search, search);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, small_vclock, small_vclock);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, w, w);
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, young_vclock, young_vclock);
        if (rpb_props->has_backend) {
            RMALLOCCOPY(client, result->backend.value, result->backend.len, rpb_props->backend.data, rpb_props->backend.len);
        }
        if (rpb_props->has_has_postcommit) {
            result->has_postcommit_hooks = 1;
            result->postcommit_hook_count = rpb_props->n_postcommit;
            if (rpb_props->n_postcommit > 0) {
                result->postcommit_hooks = riack_rpb_hooks_to_hooks(client, rpb_props->postcommit, rpb_props->n_postcommit);
            }
        }
        if (rpb_props->has_has_precommit) {
            result->has_precommit_hooks = 1;
            result->precommit_hook_count = rpb_props->n_precommit;
            if (rpb_props->n_precommit > 0) {
                result->precommit_hooks = riack_rpb_hooks_to_hooks(client, rpb_props->precommit, rpb_props->n_precommit);
            }
        }
        if (rpb_props->chash_keyfun) {
            result->chash_keyfun_use = 1;
            result->chash_keyfun = riack_rpb_modfun_to_modfun(client, rpb_props->chash_keyfun);
        }
        if (rpb_props->linkfun) {
            result->linkfun_use = 1;
            result->linkfun = riack_rpb_modfun_to_modfun(client, rpb_props->linkfun);
        }
        if (rpb_props->has_repl) {
            result->replication_mode_use = 1;
            result->replication_mode = riack_replication_mode_from_replmode(rpb_props->repl);
        }
    }
    return result;
}

int riack_get_bucket_base(struct RIACK_CLIENT *client, RIACK_STRING bucket, RpbGetBucketResp **response) {
    int result;
    struct RIACK_PB_MSG msg_req, *msg_resp;
    ProtobufCAllocator pb_allocator;
    size_t packed_size;
    uint8_t *request_buffer;
    RpbGetBucketReq get_request = RPB_GET_BUCKET_REQ__INIT;
    pb_allocator = riack_pb_allocator(&client->allocator);
    result = RIACK_ERROR_COMMUNICATION;
    get_request.bucket.len = bucket.len;
    get_request.bucket.data = (uint8_t*)bucket.value;
    packed_size = rpb_get_bucket_req__get_packed_size(&get_request);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_get_bucket_req__pack(&get_request, request_buffer);
        msg_req.msg_code = mc_RpbGetBucketReq;
        msg_req.msg_len = packed_size;
        msg_req.msg = request_buffer;
        if ((riack_send_message(client, &msg_req) > 0)&&
            (riack_receive_message(client, &msg_resp) > 0))
        {
            if (msg_resp->msg_code == mc_RpbGetBucketResp) {
                *response = rpb_get_bucket_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                if (*response) {
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
        RFREE(client, request_buffer);
    }
    return result;
}

int riack_get_bucket_props_ext(struct RIACK_CLIENT *client, RIACK_STRING bucket, struct RIACK_BUCKET_PROPERTIES** properties)
{
    ProtobufCAllocator pb_allocator;
    int result;
    RpbGetBucketResp *response;
    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    pb_allocator = riack_pb_allocator(&client->allocator);
    result = riack_get_bucket_base(client, bucket, &response);
    *properties = NULL;
    if (result == RIACK_SUCCESS) {
        *properties = riack_riack_bucket_props_from_rpb(client, response->props);
        rpb_get_bucket_resp__free_unpacked(response, &pb_allocator);
    }
    return result;
}


int riack_get_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t *n_val, uint8_t *allow_mult)
{
    ProtobufCAllocator pb_allocator;
    int result;
    RpbGetBucketResp *response;
    if (!client || !bucket.value || bucket.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    pb_allocator = riack_pb_allocator(&client->allocator);
    result = riack_get_bucket_base(client, bucket, &response);
    if (result == RIACK_SUCCESS) {
        if (response->props->has_allow_mult) {
            *allow_mult = response->props->allow_mult ? 1 : 0;
        }
        if (response->props->has_n_val) {
            *n_val = response->props->n_val;
        }
        rpb_get_bucket_resp__free_unpacked(response, &pb_allocator);
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

