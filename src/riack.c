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

#include "riack_internal.h"
#include "riack_helpers.h"
#include "riack_sock.h"
#include <string.h>
#include <protocol/riak_msg_codes.h>

void riack_set_rpb_bucket_props(riack_client *client, riack_bucket_properties* props, RpbBucketProps *rpb_props);
riack_bucket_properties* riack_riack_bucket_props_from_rpb(riack_client *client, RpbBucketProps* rpb_props);
RpbCommitHook** riack_hooks_to_rpb_hooks(riack_client *client, riack_commit_hook* hooks, size_t hook_count);


riack_client* riack_new_client(riack_allocator *allocator)
{
    /* Creates a new riack_client instance.
     * Remeber to call riack_free when the client is no longer needed */
	riack_client* result;
	if (allocator) {
		result = allocator->alloc(0, sizeof(riack_client));
		result->allocator = *allocator;
	} else {
		result = riack_default_allocator.alloc(0, sizeof(riack_client));
		result->allocator = riack_default_allocator;
	}
	result->sockfd = -1;
	result->last_error = 0;
	result->last_error_code = 0;
	result->host = 0;
	result->port = 0;
	result->options.recv_timeout_ms = 0;
	result->options.send_timeout_ms = 0;
    result->options.keep_alive_enabled = 0;
	return result;
}

void riack_free(riack_client *client)
{
    /* Free a riack_client  */
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

int riack_connect(riack_client *client, const char* host, int port, riack_connection_options* options)
{
    /* Connect a client to a riak server. options are applied to socket connection */
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
                client->last_error_code = 0;
                client->last_error = RMALLOC(client, sizeof(FAILED_TO_SET_SOCKET_TIMEOUTS));
                strcpy(client->last_error, FAILED_TO_SET_SOCKET_TIMEOUTS);
                return RIACK_ERROR_COMMUNICATION;
			}
            if (client->options.keep_alive_enabled == 1) {
                if (!sock_set_keep_alive(client->sockfd)) {
                    sock_close(client->sockfd);
                    client->sockfd = -1;
                    client->last_error_code = 0;
                    client->last_error = RMALLOC(client, sizeof(FAILED_TO_SET_SOCKET_OPTION_KEEPALIVE));
                    strcpy(client->last_error, FAILED_TO_SET_SOCKET_OPTION_KEEPALIVE);
                    return RIACK_ERROR_COMMUNICATION;
                }
            }
		}
		return RIACK_SUCCESS;
	}
	return RIACK_ERROR_COMMUNICATION;
}

int riack_disconnect(riack_client *client)
{
    /* Disconnect from the server, if we are connected */
	if (client->sockfd > 0) {
		sock_close(client->sockfd);
		client->sockfd = -1;
	}
	return RIACK_SUCCESS;
}

int riack_reconnect(riack_client *client)
{
    /* Reconnect to server */
	riack_disconnect(client);
	return riack_connect(client, client->host, client->port, &client->options);
}

int riack_ping(riack_client *client)
{
	return riack_perform_commmand(client, &cmd_ping, 0, 0, 0);
}

int riack_reset_bucket_props(riack_client *client, riack_string *bucket)
{
    RpbResetBucketReq reset_req;
    if (!client || !RSTR_HAS_CONTENT_P(bucket)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    reset_req.bucket.data = (uint8_t*)bucket->value;
    reset_req.bucket.len = bucket->len;
    return riack_perform_commmand(client, &cmd_reset_bucket_properties, (struct rpb_base_req *) &reset_req, 0, 0);
}

int riack_set_bucket_props_ext(riack_client *client, riack_string *bucket,
        riack_string* bucket_type, riack_bucket_properties* properties)
{
    RpbSetBucketReq set_request = RPB_SET_BUCKET_REQ__INIT;
    RpbBucketProps bck_props = RPB_BUCKET_PROPS__INIT;
    if (!properties || !client || !RSTR_HAS_CONTENT_P(bucket)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    riack_set_rpb_bucket_props(client, properties, &bck_props);
    if (bucket_type) {
        set_request.has_type = 1;
        set_request.type.len = bucket_type->len;
        set_request.type.data = (uint8_t *) bucket_type->value;
    }
    set_request.props = &bck_props;
    set_request.bucket.len = bucket->len;
    set_request.bucket.data = (uint8_t*)bucket->value;

    return riack_perform_commmand(client, &cmd_set_bucket_properties, (struct rpb_base_req const *) &set_request, 0, 0);
}

int riack_set_bucket_props(riack_client *client, riack_string *bucket, riack_bucket_properties* properties)
{
    return riack_set_bucket_props_ext(client, bucket, NULL, properties);
}

riack_cmd_cb_result riack_get_bucket_base_cb(riack_client *client, RpbGetBucketResp* response, riack_bucket_properties** out)
{
    *out = riack_riack_bucket_props_from_rpb(client, response->props);
    return RIACK_CMD_DONE;
}

int riack_get_bucket_base(riack_client *client, riack_string *bucket, riack_string *bucket_type,
        riack_bucket_properties** properties) {
    RpbGetBucketReq get_request = RPB_GET_BUCKET_REQ__INIT;
    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !properties) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *properties = NULL;
    get_request.bucket.data = (uint8_t *) bucket->value;
    get_request.bucket.len = bucket->len;
    if (bucket_type) {
        get_request.has_type = 1;
        get_request.type.len = bucket_type->len;
        get_request.type.data = (uint8_t *) bucket_type->value;
    }
    return riack_perform_commmand(client, &cmd_get_bucket_properties, (struct rpb_base_req const *) &get_request,
            (cmd_response_cb) riack_get_bucket_base_cb, (void **) properties);
}

int riack_get_bucket_props(riack_client *client, riack_string *bucket, riack_bucket_properties** properties)
{
    return riack_get_bucket_base(client, bucket, 0, properties);
}

int riack_get_bucket_props_ext(riack_client *client, riack_string *bucket, riack_string* bucket_type,
        riack_bucket_properties** properties)
{
    return riack_get_bucket_base(client, bucket, bucket_type, properties);
}

int riack_set_bucket_type_props(riack_client *client, riack_string *bucket_type,
        riack_bucket_properties* properties)
{
    RpbSetBucketTypeReq set_request = RPB_SET_BUCKET_TYPE_REQ__INIT;
    RpbBucketProps bck_props = RPB_BUCKET_PROPS__INIT;
    if (!client || !properties || !RSTR_HAS_CONTENT_P(bucket_type)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    set_request.type.data = (uint8_t *) bucket_type->value;
    set_request.type.len = bucket_type->len;
    riack_set_rpb_bucket_props(client, properties, &bck_props);
    set_request.props = &bck_props;
    return riack_perform_commmand(client, &cmd_set_bucket_type, (struct rpb_base_req *) &set_request, 0, 0);
}

int riack_get_bucket_type_props(riack_client *client, riack_string* bucket_type, riack_bucket_properties** properties)
{
    RpbGetBucketTypeReq get_request = RPB_GET_BUCKET_TYPE_REQ__INIT;
    if (!client || !RSTR_HAS_CONTENT_P(bucket_type) || properties == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *properties = NULL;
    get_request.type.len = bucket_type->len;
    get_request.type.data = (uint8_t *) bucket_type->value;
    return riack_perform_commmand(client, &cmd_get_type_properties, (struct rpb_base_req const *) &get_request,
            (cmd_response_cb) riack_get_bucket_base_cb, (void **) properties);
}

riack_cmd_cb_result riack_server_info_cb(riack_client *client, RpbGetServerInfoResp* response,
        struct riack_server_info** server_info)
{
    *server_info = RMALLOC(client, sizeof(struct riack_server_info));
    if (response->has_node) {
        (*server_info)->node = riack_string_alloc(client);
        RMALLOCCOPY(client, (*server_info)->node->value, (*server_info)->node->len, response->node.data, response->node.len);
    } else {
        (*server_info)->node = 0;
    }
    if (response->has_server_version) {
        (*server_info)->server_version = riack_string_alloc(client);
        RMALLOCCOPY(client, (*server_info)->server_version->value, (*server_info)->server_version->len,
                response->server_version.data, response->server_version.len);
    } else {
        (*server_info)->server_version = 0;
    }
    return RIACK_CMD_DONE;
}

int riack_server_info(riack_client *client, riack_string **node, riack_string** version)
{
    int retval;
    struct riack_server_info* server_info;
    if (!client || node == 0 || version == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *node = 0;
    *version = 0;
    retval = riack_perform_commmand(client, &cmd_get_server_info, 0,
            (cmd_response_cb) riack_server_info_cb, (void **) &server_info);
    if (server_info) {
        *node = server_info->node;
        *version = server_info->server_version;
        RFREE(client, server_info);
    }
    return retval;
}

void riack_timeout_test(riack_client* client)
{
	riack_pb_msg *msg_resp;
	riack_receive_message(client, &msg_resp);
}

/******************************************************************************
* Memory management functions
******************************************************************************/

void riack_free_commit_hooks(riack_client *client, riack_commit_hook** hooks, size_t hook_count) {
    /* Frees a RIACK_COMMIT_HOOK including all members */
    size_t i;
    if (hooks && *hooks) {
        for (i = 0; i < hook_count; ++i) {
            RSTR_SAFE_FREE(client, (*hooks)[i].name);
            RSTR_SAFE_FREE(client, (*hooks)[i].modfun.function);
            RSTR_SAFE_FREE(client, (*hooks)[i].modfun.module);
        }
        RFREE(client, *hooks);
        *hooks = 0;
    }
}


void riack_free_bucket_properties_p(riack_client *client, riack_bucket_properties **properties) {
    /* Frees a RIACK_BUCKET_PROPERTIES including all members */
    if (*properties) {
        if (RSTR_HAS_CONTENT((*properties)->backend)) {
            RFREE(client, (*properties)->backend.value);
        }
        riack_free_commit_hooks(client, &(*properties)->precommit_hooks, (*properties)->precommit_hook_count);
        riack_free_commit_hooks(client, &(*properties)->postcommit_hooks, (*properties)->postcommit_hook_count);
        if ((*properties)->chash_keyfun_use) {
            RSTR_SAFE_FREE(client, (*properties)->chash_keyfun.function);
            RSTR_SAFE_FREE(client, (*properties)->chash_keyfun.module);
        }
        if ((*properties)->search_index_use) {
            RSTR_SAFE_FREE(client, (*properties)->search_index)
        }
        if ((*properties)->datatype_use) {
            RSTR_SAFE_FREE(client, (*properties)->datatype)
        }
        if ((*properties)->linkfun_use) {
            RSTR_SAFE_FREE(client, (*properties)->linkfun.function);
            RSTR_SAFE_FREE(client, (*properties)->linkfun.module);
        }
        RFREE(client, *properties);
    }
}

/******************************************************************************
* Helper functions
******************************************************************************/

void riack_got_error_response(riack_client *client, riack_pb_msg *msg)
{
    /* Copies a riak error to a riak client
     * that way it is possible to get last error from the client */
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


enum RIACK_REPLICATION_MODE riack_replication_mode_from_replmode(RpbBucketProps__RpbReplMode rpb_repl_mode) {
    switch (rpb_repl_mode) {
        case RPB_BUCKET_PROPS__RPB_REPL_MODE__TRUE:
            return REALTIME_AND_FULLSYNC;
        case RPB_BUCKET_PROPS__RPB_REPL_MODE__REALTIME:
            return REALTIME;
        case RPB_BUCKET_PROPS__RPB_REPL_MODE__FULLSYNC:
            return FULLSYNC;
        default:
            return DISABLED;
    }
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

riack_module_function riack_rpb_modfun_to_modfun(riack_client *client, RpbModFun *rpb_modfun) {
    /* Deep copies a RpbModFun to a RIACK_MODULE_FUNCTION */
    riack_module_function result;
    memset(&result, 0, sizeof(riack_module_function));
    if (rpb_modfun && rpb_modfun->function.len > 0) {
        RMALLOCCOPY(client, result.function.value, result.function.len, rpb_modfun->function.data, rpb_modfun->function.len);
    }
    if (rpb_modfun && rpb_modfun->module.len > 0) {
        RMALLOCCOPY(client, result.module.value, result.module.len, rpb_modfun->module.data, rpb_modfun->module.len);
    }
    return result;
}

riack_commit_hook* riack_rpb_hooks_to_hooks(riack_client *client, RpbCommitHook ** rpb_hooks, size_t hook_count)
{
    /* Deep copies an array of pointers to RpbCommitHook's and returns it as an array of RIACK_COMMIT_HOOK's */
    size_t i;
    riack_commit_hook* result;
    if (hook_count == 0) return 0;

    result = RCALLOC(client, sizeof(riack_commit_hook) * hook_count);
    for (i=0; i<hook_count; ++i) {
        if (rpb_hooks[i]->has_name) {
            RMALLOCCOPY(client, result[i].name.value, result[i].name.len, rpb_hooks[i]->name.data, rpb_hooks[i]->name.len);
        }
        result[i].modfun = riack_rpb_modfun_to_modfun(client, rpb_hooks[i]->modfun);
    }
    return result;
}

riack_bucket_properties* riack_riack_bucket_props_from_rpb(riack_client *client, RpbBucketProps* rpb_props) {
/* Copy all properties from RpbBucketProps to RIACK_BUCKET_PROPERTIES */

#define COPY_PROPERTY_USE_TO_HAS(FROM, TO, PROP_NAME_FROM, PROP_NAME_TO) if (FROM->has_##PROP_NAME_FROM) { \
                                                                            TO->PROP_NAME_TO##_use = 1; \
                                                                            TO->PROP_NAME_TO = FROM->PROP_NAME_FROM;}
    riack_bucket_properties* result = NULL;
    result = RMALLOC(client, sizeof(riack_bucket_properties));
    if (result) {
        memset(result, 0, sizeof(riack_bucket_properties));
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
        COPY_PROPERTY_USE_TO_HAS(rpb_props, result, consistent, consistent);
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
        if (rpb_props->has_datatype) {
            RMALLOCCOPY(client, result->datatype.value, result->datatype.len, rpb_props->datatype.data, rpb_props->datatype.len);
        }
        if (rpb_props->has_search_index) {
            RMALLOCCOPY(client, result->search_index.value, result->search_index.len, rpb_props->search_index.data, rpb_props->search_index.len);
        }
    }
    return result;
}


void riack_set_rpb_bucket_props(riack_client *client, riack_bucket_properties* props, RpbBucketProps *rpb_props)
{
/* Copy all properties from RIACK_BUCKET_PROPERTIES to RpbBucketProps */

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
    COPY_PROPERTY_HAS_TO_USE(props, rpb_props, consistent, consistent);
    if (props->has_postcommit_hooks) {
        rpb_props->has_postcommit = rpb_props->has_has_postcommit = 1;
        rpb_props->n_postcommit = props->postcommit_hook_count;
        rpb_props->postcommit = riack_hooks_to_rpb_hooks(client, props->postcommit_hooks, props->postcommit_hook_count);
    }
    if (props->has_precommit_hooks) {
        rpb_props->has_precommit = rpb_props->has_has_precommit = 1;
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
        rpb_mod_fun__init(rpb_props->chash_keyfun);
        RMALLOCCOPY(client, rpb_props->chash_keyfun->function.data, rpb_props->chash_keyfun->function.len,
                props->chash_keyfun.function.value, props->chash_keyfun.function.len);
        RMALLOCCOPY(client, rpb_props->chash_keyfun->module.data, rpb_props->chash_keyfun->module.len,
                props->chash_keyfun.module.value, props->chash_keyfun.module.len);
    }
    if (props->linkfun_use) {
        rpb_props->linkfun = (RpbModFun*)RMALLOC(client, sizeof(RpbModFun));
        rpb_mod_fun__init(rpb_props->linkfun);
        RMALLOCCOPY(client, rpb_props->linkfun->function.data, rpb_props->linkfun->function.len,
                props->linkfun.function.value, props->linkfun.function.len);
        RMALLOCCOPY(client, rpb_props->linkfun->module.data, rpb_props->linkfun->module.len,
                props->linkfun.module.value, props->linkfun.module.len);
    }
    if (props->replication_mode_use) {
        rpb_props->has_repl = 1;
        rpb_props->repl = replmode_from_riack_replication_mode(props->replication_mode);
    }
    if (props->search_index_use) {
        rpb_props->has_search_index = 1;
        rpb_props->search_index.len = props->search_index.len;
        rpb_props->search_index.data = (uint8_t*)RMALLOC(client, props->search_index.len);
        memcpy(rpb_props->search_index.data, props->search_index.value, props->search_index.len);
    }
    if (props->datatype_use) {
        rpb_props->has_datatype = 1;
        rpb_props->datatype.len = props->datatype.len;
        rpb_props->datatype.data = (uint8_t*)RMALLOC(client, props->datatype.len);
        memcpy(rpb_props->datatype.data, props->datatype.value, props->datatype.len);
    }
}


RpbCommitHook** riack_hooks_to_rpb_hooks(riack_client *client, riack_commit_hook* hooks, size_t hook_count) {
    size_t i;
    RpbCommitHook** result;
    if (hook_count == 0) {
        return NULL;
    }
    result = (RpbCommitHook **)RMALLOC(client, sizeof(RpbCommitHook *) * hook_count);
    for (i=0; i<hook_count; ++i) {
        result[i] = (RpbCommitHook *)RCALLOC(client, sizeof(RpbCommitHook));
        rpb_commit_hook__init(result[i]);
        if (RSTR_HAS_CONTENT(hooks[i].name)) {
            // Js function
            result[i]->has_name = 1;
            RMALLOCCOPY(client, result[i]->name.data, result[i]->name.len,
                    hooks[i].name.value, hooks[i].name.len);
        } else {
            // Erlang function
            result[i]->modfun = (RpbModFun*)RCALLOC(client, sizeof(RpbModFun));
            rpb_mod_fun__init(result[i]->modfun);
            RMALLOCCOPY(client, result[i]->modfun->function.data, result[i]->modfun->function.len,
                    hooks[i].modfun.function.value, hooks[i].modfun.function.len);
            RMALLOCCOPY(client, result[i]->modfun->module.data, result[i]->modfun->module.len,
                    hooks[i].modfun.module.value, hooks[i].modfun.module.len);
        }
    }
    return result;
}

