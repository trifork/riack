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
#include "riack.h"
#include "riack_msg.h"
#include "riack_helpers.h"
#include "protocol/riak_msg_codes.h"
#include <string.h>

#define RIACK_SET_BUCKETTYPE_AND_TIMEOUT(REQ)  if (bucket_type && bucket_type->len > 0) { \
                            REQ.has_type = 1; \
                            REQ.type.len = bucket_type->len; \
                            REQ.type.data = (uint8_t *) bucket_type->value; } \
                        if (timeout > 0) { REQ.has_timeout = 1; REQ.timeout = timeout; }

ProtobufCAllocator riack_pb_allocator(RIACK_ALLOCATOR *allocator);

void riak_set_object_response_values(RIACK_CLIENT* client, RIACK_OBJECT *pobject, RpbPutResp* pput_resp)
{
	size_t content_cnt, i;
	if (!pput_resp || !pobject) {
		return;
	}
	pobject->bucket.value = 0;
	pobject->bucket.len = 0;
	pobject->key.len = 0;
	pobject->key.value = 0;
	if (pput_resp->has_key) {
		RMALLOCCOPY(client, pobject->key.value, pobject->key.len, pput_resp->key.data, pput_resp->key.len);
	}
	pobject->vclock.len = 0;
	pobject->vclock.clock = 0;
	if (pput_resp->has_vclock) {
		RMALLOCCOPY(client, pobject->vclock.clock, pobject->vclock.len, 
                pput_resp->vclock.data, pput_resp->vclock.len);
	}
	content_cnt = pput_resp->n_content;
	pobject->content_count = content_cnt;
	if (content_cnt > 0) {
		pobject->content = RMALLOC(client, sizeof(RIACK_CONTENT)*content_cnt);
		for (i=0; i<content_cnt; ++i) {
			riack_copy_rpbcontent_to_content(client, pput_resp->content[i], &pobject->content[i]);
		}
	}
}

void riak_set_object_response_values_get(RIACK_CLIENT* client, RIACK_GET_OBJECT *object, RpbGetResp* getresp)
{
	size_t cnt, i;
	if (!object || !getresp) {
		return;
	}
	object->unchanged_present = (uint8_t) (getresp->has_unchanged ? 1 : 0);
	object->unchanged = (uint8_t) (getresp->unchanged ? 1 : 0);
	object->object.bucket.value = 0;
	object->object.bucket.len = 0;
	object->object.key.value = 0;
	object->object.key.len = 0;

	object->object.vclock.len = 0;
	object->object.vclock.clock = 0;
	if (getresp->has_vclock) {
		object->object.vclock.len = getresp->vclock.len;
		object->object.vclock.clock = (uint8_t*)RMALLOC(client, getresp->vclock.len);
		memcpy(object->object.vclock.clock, getresp->vclock.data, getresp->vclock.len);
	}
	cnt = getresp->n_content;
	object->object.content_count = cnt;
	if (cnt > 0) {
		object->object.content = RMALLOC(client, sizeof(RIACK_CONTENT)*cnt);
		for (i=0; i<cnt; ++i) {
			riack_copy_rpbcontent_to_content(client, getresp->content[i], &object->object.content[i]);
		}
	}
}


int riack_put_simple(RIACK_CLIENT *client, char* bucket, char* key, uint8_t* data, size_t datalen, char* content_type)
{
	int result;
	RIACK_OBJECT object;
	object.bucket.value = bucket;
	object.bucket.len = strlen(bucket);
	object.key.value = key;
	object.key.len = strlen(key);
	object.vclock.len = 0;
	object.content = (RIACK_CONTENT*)RMALLOC(client, sizeof(RIACK_CONTENT));
	memset(object.content, 0, sizeof(RIACK_CONTENT));
	object.content[0].content_type.value = content_type;
	object.content[0].content_type.len = strlen(content_type);
	object.content[0].data_len = datalen;
	object.content[0].data = data;
	result = riack_put(client, &object, 0, (RIACK_PUT_PROPERTIES*)0);
	RFREE(client, object.content);
	return result;
}

void riack_set_object_properties(RIACK_PUT_PROPERTIES* pprops, RpbPutReq* pput_req)
{
	if (pprops) {
		pput_req->has_w = pprops->w_use;
		pput_req->w = pprops->w;
		pput_req->has_dw = pprops->dw_use;
		pput_req->dw = pprops->dw;
		pput_req->has_pw = pprops->pw_use;
		pput_req->pw = pprops->pw;
		pput_req->has_if_none_match = pprops->if_none_match_use;
		pput_req->if_none_match = pprops->if_none_match;
		pput_req->has_if_not_modified = pprops->if_not_modified_use;
		pput_req->if_not_modified = pprops->if_not_modified;
		pput_req->has_return_body = pprops->return_body_use;
		pput_req->return_body = pprops->return_body;
		pput_req->has_return_head = pprops->return_head_use;
        pput_req->return_head = pprops->return_head;
	} else {
		pput_req->has_w = 0;
		pput_req->has_dw = 0;
		pput_req->has_pw = 0;
		pput_req->has_if_none_match = 0;
		pput_req->has_if_not_modified = 0;
		pput_req->has_return_body = 0;
        pput_req->has_return_head = 0;
	}
}

void riack_set_get_properties(RIACK_CLIENT *client, RIACK_GET_PROPERTIES* props, RpbGetReq* get_req)
{
	if (props) {
        get_req->has_basic_quorum = props->basic_quorum_use;
		get_req->basic_quorum = props->basic_quorum;
        get_req->has_deletedvclock = props->deletedvclock_use;
		get_req->deletedvclock = props->deletedvclock;
		get_req->has_head = props->head_use;
		get_req->head = props->head;
		get_req->has_if_modified = 0;
        get_req->has_notfound_ok = props->notfound_ok_use;
		get_req->notfound_ok = props->notfound_ok;
		get_req->has_pr = props->pr_use;
		get_req->pr = props->pr;
		get_req->has_r = props->r_use;
		get_req->r = props->r;

		get_req->has_if_modified = props->if_modified_use;
		if (props->if_modified_use) {
			get_req->if_modified.len = props->if_modified.len;
			get_req->if_modified.data = (uint8_t*)RMALLOC(client, props->if_modified.len);
			memcpy(get_req->if_modified.data, props->if_modified.clock, props->if_modified.len);
		} else {
			get_req->if_modified.data = 0;
			get_req->if_modified.len = 0;
		}
	} else {
		get_req->has_basic_quorum = 0;
		get_req->has_deletedvclock = 0;
		get_req->has_head = 0;
		get_req->has_if_modified = 0;
		get_req->has_notfound_ok = 0;
		get_req->has_pr = 0;
		get_req->has_r = 0;
	}
}

void riack_set_del_properties(RIACK_CLIENT *client, RIACK_DEL_PROPERTIES* props, RpbDelReq* del_req)
{
	if (props) {
		del_req->has_dw = props->dw_use;
		del_req->dw = props->dw;
		del_req->has_pr = props->pr_use;
		del_req->pr = props->pr;
		del_req->has_pw = props->pw_use;
		del_req->pw = props->pw;
		del_req->has_r = props->r_use;
		del_req->r = props->r;
		del_req->has_rw = props->rw_use;
		del_req->rw = props->rw;
		del_req->has_w = props->w_use;
		del_req->w = props->w;
		del_req->has_vclock = 0;
		del_req->vclock.data = 0;
		if (props->vclock.clock != 0) {
			del_req->has_vclock = 1;
			del_req->vclock.len = props->vclock.len;
			del_req->vclock.data = RMALLOC(client, props->vclock.len);
			memcpy(del_req->vclock.data, props->vclock.clock, props->vclock.len);
		}
	} else {
		del_req->has_dw = 0;
		del_req->has_pr = 0;
		del_req->has_pw = 0;
		del_req->has_r = 0;
		del_req->has_rw = 0;
		del_req->has_w = 0;
		del_req->has_vclock = 0;
		del_req->vclock.data = 0;
	}
}

int riack_get(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key, RIACK_GET_PROPERTIES* props,
        RIACK_GET_OBJECT** result_object)
{
    RIACK_REQ_LOCALS;
	RpbGetReq get_req;
    RpbGetResp *get_resp;

	if (!RSTR_HAS_CONTENT_P(key) || !RSTR_HAS_CONTENT_P(bucket) || !client || !result_object) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);

    retval = RIACK_ERROR_COMMUNICATION;
	rpb_get_req__init(&get_req);
	get_req.key.data = (uint8_t *) key->value;
	get_req.key.len = key->len;
	get_req.bucket.data = (uint8_t *) bucket->value;
	get_req.bucket.len = bucket->len;
	riack_set_get_properties(client, props, &get_req);

	packed_size = rpb_get_req__get_packed_size(&get_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_get_req__pack(&get_req, request_buffer);
		msg_req.msg_code = mc_RpbGetReq;
		msg_req.msg_len = (uint32_t) packed_size;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0))
		{
			if (msg_resp->msg_code == mc_RpbGetResp) {
				get_resp = rpb_get_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
				if (get_resp) {
                    *result_object = riack_get_object_alloc(client);
					riak_set_object_response_values_get(client, *result_object, get_resp);
					rpb_get_resp__free_unpacked(get_resp, &pb_allocator);
                    retval = RIACK_SUCCESS;
				} else {
                    retval = RIACK_FAILED_PB_UNPACK;
				}
			} else {
				riack_got_error_response(client, msg_resp);
                retval = RIACK_ERROR_RESPONSE;
			}
			riack_message_free(client, &msg_resp);
		}
		RFREE(client, request_buffer);
	}
    return retval;
}

int riack_put(RIACK_CLIENT *client, RIACK_OBJECT *object, RIACK_OBJECT**returned_object,
        RIACK_PUT_PROPERTIES* properties)
{
    RIACK_REQ_LOCALS;
	RpbPutReq put_req;
    RpbPutResp *put_resp;

    if (!client || !object || !object->bucket.value) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);
    retval = RIACK_ERROR_COMMUNICATION;
	rpb_put_req__init(&put_req);
	riack_copy_object_to_rpbputreq(client, object, &put_req);
    riack_set_object_properties(properties, &put_req);

	packed_size = rpb_put_req__get_packed_size(&put_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_put_req__pack(&put_req, request_buffer);
		msg_req.msg_code = mc_RpbPutReq;
		msg_req.msg_len = (uint32_t) packed_size;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0))
		{
			if (msg_resp->msg_code == mc_RpbPutResp) {
                retval = RIACK_SUCCESS;
				if (returned_object) {
                    *returned_object = riack_object_alloc(client);
					put_resp = rpb_put_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
					if (put_resp) {
						riak_set_object_response_values(client, *returned_object, put_resp);
						rpb_put_resp__free_unpacked(put_resp, &pb_allocator);
					} else {
                        retval = RIACK_FAILED_PB_UNPACK;
					}
				}
			} else {
				riack_got_error_response(client, msg_resp);
                retval = RIACK_ERROR_RESPONSE;
			}
			riack_message_free(client, &msg_resp);
		}
		RFREE(client, request_buffer);
	}
    riack_free_copied_rpb_put_req(client, &put_req);
    return retval;
}

int riack_delete(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key, RIACK_DEL_PROPERTIES *props)
{
	int result;
	size_t packed_size;
	RIACK_PB_MSG msg_req, *msg_resp;
	RpbDelReq del_req;
    uint8_t *request_buffer;

	if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(key)) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	result = RIACK_ERROR_COMMUNICATION;
	rpb_del_req__init(&del_req);

	del_req.bucket.len = bucket->len;
	del_req.bucket.data = (uint8_t *) bucket->value;
	del_req.key.len = key->len;
	del_req.key.data = (uint8_t *) key->value;
	riack_set_del_properties(client, props, &del_req);

	packed_size = rpb_del_req__get_packed_size(&del_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_del_req__pack(&del_req, request_buffer);
		msg_req.msg_code = mc_RpbDelReq;
		msg_req.msg_len = (uint32_t) packed_size;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0))
		{
			if (msg_resp->msg_code == mc_RpbDelResp) {
				result = RIACK_SUCCESS;
			} else {
				riack_got_error_response(client, msg_resp);
				result = RIACK_ERROR_RESPONSE;
			}
			riack_message_free(client, &msg_resp);
		}
		RFREE(client, request_buffer);
	}

    RFREE(client, del_req.vclock.data);
    return result;
}

static void _list_keys_stream_callback(RIACK_CLIENT *client, void *args_raw, RIACK_STRING key)
{
	RIACK_STRING_LINKED_LIST **current = (RIACK_STRING_LINKED_LIST**)args_raw;
	RIACK_STRING new_string;
	assert(current);
	RMALLOCCOPY(client, new_string.value, new_string.len, key.value, key.len);
	riack_string_linked_list_add(client, current, new_string);
}

int riack_list_keys_ext(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *bucket_type,
        RIACK_STRING_LINKED_LIST** keys, uint32_t timeout)
{
    if (!keys) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *keys = 0;
    return riack_stream_keys_ext(client, bucket, bucket_type, _list_keys_stream_callback, keys, timeout);
}

int riack_list_keys(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING_LINKED_LIST** keys)
{
	return riack_list_keys_ext(client, bucket, NULL, keys, 0);
}

int riack_stream_keys_ext(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING* bucket_type,
        void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING),
        void* callback_arg, uint32_t timeout)
{
    RIACK_REQ_LOCALS;
    RpbListKeysReq list_req;
    RpbListKeysResp *list_resp;
    RIACK_STRING current_string;
    size_t num_keys, i;
    uint8_t recvdone;

    if (!client || !callback || !RSTR_HAS_CONTENT_P(bucket)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    pb_allocator = riack_pb_allocator(&client->allocator);
    retval = RIACK_ERROR_COMMUNICATION;
    rpb_list_keys_req__init(&list_req);
    list_req.bucket.len = bucket->len;
    list_req.bucket.data = (uint8_t*)bucket->value;

    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(list_req)
    packed_size = rpb_list_keys_req__get_packed_size(&list_req);
    request_buffer = RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_list_keys_req__pack(&list_req, request_buffer);
        msg_req.msg_code = mc_RpbListKeysReq;
        msg_req.msg_len = (uint32_t) packed_size;
        msg_req.msg = request_buffer;
        if (riack_send_message(client, &msg_req) > 0)
        {
            recvdone = 0;
            while (!recvdone) {
                if (riack_receive_message(client, &msg_resp) > 0) {
                    if (msg_resp->msg_code == mc_RpbListKeysResp) {
                        list_resp = rpb_list_keys_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                        if (list_resp) {
                            if (list_resp->has_done && list_resp->done) {
                                recvdone = 1;
                                retval = RIACK_SUCCESS;
                            }
                            num_keys = list_resp->n_keys;
                            for (i=0; i<num_keys; ++i) {
                                current_string.value = (char*)list_resp->keys[i].data;
                                current_string.len   = list_resp->keys[i].len;
                                callback(client, callback_arg, current_string);
                            }
                            rpb_list_keys_resp__free_unpacked(list_resp, &pb_allocator);
                        } else {
                            retval = RIACK_FAILED_PB_UNPACK;
                        }
                    } else {
                        riack_got_error_response(client, msg_resp);
                        recvdone = 1;
                    }
                    riack_message_free(client, &msg_resp);
                }
            }
        }
        RFREE(client, request_buffer);
    }
    return retval;
}

int riack_stream_keys(RIACK_CLIENT *client, RIACK_STRING *bucket,
					  void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING), void *callback_arg)
{
    return riack_stream_keys_ext(client, bucket, NULL, callback, callback_arg, 0);
}

int riack_list_buckets(RIACK_CLIENT *client, RIACK_STRING_LIST** bucket_list)
{
    return riack_list_buckets_ext(client, NULL, bucket_list, 0);
}

int riack_list_buckets_ext(RIACK_CLIENT *client, RIACK_STRING* bucket_type,
        RIACK_STRING_LIST** bucket_list, uint32_t timeout)
{
    RIACK_REQ_LOCALS;
    RpbListBucketsReq list_req;
    RpbListBucketsResp *list_resp;
    size_t i;

    if (!client || !bucket_list) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    pb_allocator = riack_pb_allocator(&client->allocator);
    retval = RIACK_ERROR_COMMUNICATION;
    *bucket_list = riack_string_list_alloc(client);

    rpb_list_buckets_req__init(&list_req);
    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(list_req)
    packed_size = rpb_list_buckets_req__get_packed_size(&list_req);
    request_buffer = RMALLOC(client, packed_size);
    if (request_buffer && packed_size > 0) {
        rpb_list_buckets_req__pack(&list_req, request_buffer);
    }
    msg_req.msg_code = mc_RpbListBucketsReq;
    msg_req.msg_len = (uint32_t) packed_size;
    msg_req.msg = request_buffer;
    (*bucket_list)->string_count = 0;
    if ((riack_send_message(client, &msg_req) > 0) &&
            (riack_receive_message(client, &msg_resp) > 0)) {
        if (msg_resp->msg_code == mc_RpbListBucketsResp) {
            list_resp = rpb_list_buckets_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
            if (list_resp) {
                (*bucket_list)->string_count = list_resp->n_buckets;
                (*bucket_list)->strings = (RIACK_STRING *) RMALLOC(client, sizeof(RIACK_STRING) * list_resp->n_buckets);
                for (i = 0; i < list_resp->n_buckets; ++i) {
                    RMALLOCCOPY(client,
                            (*bucket_list)->strings[i].value,
                            (*bucket_list)->strings[i].len,
                            list_resp->buckets[i].data,
                            list_resp->buckets[i].len);
                }
                rpb_list_buckets_resp__free_unpacked(list_resp, &pb_allocator);
                retval = RIACK_SUCCESS;
            } else {
                retval = RIACK_FAILED_PB_UNPACK;
            }
        } else {
            riack_got_error_response(client, msg_resp);
            retval = RIACK_ERROR_RESPONSE;
        }
        riack_message_free(client, &msg_resp);
    }
    RFREE(client, request_buffer);
    return retval;
}

int riack_set_clientid(RIACK_CLIENT *client, RIACK_STRING *clientid)
{
    RIACK_REQ_LOCALS;
    RpbSetClientIdReq req;

	rpb_set_client_id_req__init(&req);
	req.client_id.len = clientid->len;
    req.client_id.data = (uint8_t*)clientid->value;

	packed_size = rpb_set_client_id_req__get_packed_size(&req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    retval = RIACK_ERROR_COMMUNICATION;
	if (request_buffer) {
		rpb_set_client_id_req__pack(&req, request_buffer);
		msg_req.msg_len = (uint32_t) packed_size;
		msg_req.msg_code = mc_RpbSetClientIdReq;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0)) {
			if (msg_resp->msg_code == mc_RpbSetClientIdResp) {
                retval = RIACK_SUCCESS;
			} else {
				riack_got_error_response(client, msg_resp);
                retval = RIACK_ERROR_RESPONSE;
			}
			riack_message_free(client, &msg_resp);
		}
		RFREE(client, request_buffer);
	}
    return retval;
}

int riack_get_clientid(RIACK_CLIENT *client, RIACK_STRING **clientid)
{
    int retval;
    ProtobufCAllocator pb_allocator;
    RIACK_PB_MSG msg_req, *msg_resp;
	RpbGetClientIdResp *id_resp;
    // TODO Validate input
	pb_allocator = riack_pb_allocator(&client->allocator);
	msg_req.msg = 0;
	msg_req.msg_len = 0;
	msg_req.msg_code = mc_RpbGetClientIdReq;
    retval = RIACK_ERROR_COMMUNICATION;
	if ((riack_send_message(client, &msg_req) > 0) &&
		(riack_receive_message(client, &msg_resp) > 0)) {
		if (msg_resp->msg_code == mc_RpbGetClientIdResp) {
			id_resp = rpb_get_client_id_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
			if (id_resp) {
                *clientid = riack_string_alloc(client);
				RMALLOCCOPY(client, (*clientid)->value, (*clientid)->len, id_resp->client_id.data, id_resp->client_id.len);
				rpb_get_client_id_resp__free_unpacked(id_resp, &pb_allocator);
                retval = RIACK_SUCCESS;
			} else {
                retval = RIACK_FAILED_PB_UNPACK;
			}
		} else {
			riack_got_error_response(client, msg_resp);
            retval = RIACK_ERROR_RESPONSE;
		}
		riack_message_free(client, &msg_resp);
	}
    return retval;
}

/***************************
* Map reduce
***************************/

static void _map_reduce_stream_callback(RIACK_CLIENT *client, void *args_raw, RIACK_MAPRED_RESPONSE *result)
{
    RIACK_MAPRED_RESPONSE_LIST** chain = (RIACK_MAPRED_RESPONSE_LIST**)args_raw;
    RIACK_MAPRED_RESPONSE_LIST* mapred_result_current =
            (RIACK_MAPRED_RESPONSE_LIST*)RMALLOC(client, sizeof(RIACK_MAPRED_RESPONSE_LIST));
	assert(chain);
	assert(result);
	riack_copy_strmapred_to_mapred(client, result, mapred_result_current);
	riack_mapred_add_to_chain(client, chain, mapred_result_current);
}

int riack_map_reduce(RIACK_CLIENT *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type, RIACK_MAPRED_RESPONSE_LIST** mapred_result)
{
	if (!mapred_result) {
		return RIACK_ERROR_INVALID_INPUT;
	}
	*mapred_result = 0;
	return riack_map_reduce_stream(client, data_len, data, content_type, _map_reduce_stream_callback, mapred_result);
}

int riack_map_reduce_stream(RIACK_CLIENT *client,
										 size_t data_len,
										 uint8_t* data,
										 enum RIACK_MAPRED_CONTENT_TYPE content_type,
                                         void(*callback)(RIACK_CLIENT*, void*, RIACK_MAPRED_RESPONSE*),
										 void* callback_arg)
{
    RIACK_REQ_LOCALS;
    RIACK_MAPRED_RESPONSE mapred_result_current;
	RpbMapRedReq mr_req;
    RpbMapRedResp *mr_resp;
	char* content_type_sz;
	uint8_t last_message;

	memset(&mapred_result_current, 0, sizeof(mapred_result_current));

	if (!client || !data || data_len == 0 || !callback) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);
    retval = RIACK_ERROR_COMMUNICATION;

	rpb_map_red_req__init(&mr_req);
	if (content_type == APPLICATION_JSON) {
		content_type_sz = "application/json";
	} else if (content_type == APPLICATION_ERLANG_TERM) {
		content_type_sz = "application/x-erlang-binary";
	} else {
		return RIACK_ERROR_INVALID_INPUT;
	}
	mr_req.content_type.len = strlen(content_type_sz);
    mr_req.content_type.data = (uint8_t*)content_type_sz;
	mr_req.request.len = data_len;
	mr_req.request.data = data;
	packed_size = rpb_map_red_req__get_packed_size(&mr_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_map_red_req__pack(&mr_req, request_buffer);
		msg_req.msg_code = mc_RpbMapRedReq;
		msg_req.msg_len = (uint32_t) packed_size;
		msg_req.msg = request_buffer;
		if (riack_send_message(client, &msg_req) > 0) {
			last_message = 0;
			while ((last_message == 0) && (riack_receive_message(client, &msg_resp) > 0)) {
				if (msg_resp->msg_code == mc_RpbMapRedResp) {
					mr_resp = rpb_map_red_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
					if (mr_resp) {
						riack_link_strmapred_with_rpbmapred(client, mr_resp, &mapred_result_current);
						callback(client, callback_arg, &mapred_result_current);
						if (mr_resp->has_done && mr_resp->done) {
                            retval = RIACK_SUCCESS;
							last_message = 1;
						}
						rpb_map_red_resp__free_unpacked(mr_resp, &pb_allocator);
					} else {
                        retval = RIACK_FAILED_PB_UNPACK;
					}
				} else {
					riack_got_error_response(client, msg_resp);
                    retval = RIACK_ERROR_RESPONSE;
					last_message = 1;
				}
				riack_message_free(client, &msg_resp);
			}
		}
		RFREE(client, request_buffer);
	}

    return retval;
}

/***************************
* Secondary indexes
***************************/

int riack_2i_query(RIACK_CLIENT *client, RpbIndexReq* request, RIACK_STRING_LIST** result_keys, RIACK_STRING* continuation_token,
                   void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING *key), void *callback_arg)
{
    RIACK_REQ_LOCALS;
    RpbIndexResp *index_resp;
    size_t keys, i;
    uint8_t last_message;
    retval = RIACK_ERROR_COMMUNICATION;
    if (result_keys) {
        *result_keys = riack_string_list_alloc(client);
    }
	pb_allocator = riack_pb_allocator(&client->allocator);
	packed_size = rpb_index_req__get_packed_size(request);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_index_req__pack(request, request_buffer);
		msg_req.msg = request_buffer;
		msg_req.msg_code = mc_RpbIndexReq;
		msg_req.msg_len = (uint32_t) packed_size;
        if (riack_send_message(client, &msg_req) > 0) {
            last_message = 0;
            while ((last_message == 0) && (riack_receive_message(client, &msg_resp) > 0)) {
                if (msg_resp->msg_code == mc_RpbIndexResp) {
                    index_resp = rpb_index_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                    if (index_resp) {
                        keys = index_resp->n_keys;
                        // If we have result keys then we are not streaming
                        if (result_keys) {
                            (*result_keys)->string_count = keys;
                            (*result_keys)->strings = RMALLOC(client, sizeof(RIACK_STRING) * keys);
                            for (i=0; i<keys; ++i) {
                                RMALLOCCOPY(client, (*result_keys)->strings[i].value, (*result_keys)->strings[i].len,
                                        index_resp->keys[i].data, index_resp->keys[i].len);
                            }
                            last_message = 1;
                        }
                        // If streaming
                        if (callback) {
                            for (i=0; i<keys; ++i) {
                                RIACK_STRING key;
                                key.len = index_resp->keys[i].len;
                                key.value = (char*)index_resp->keys[i].data;
                                callback(client, callback_arg, &key);
                            }
                            if (index_resp->has_done && index_resp->done) {
                                retval = RIACK_SUCCESS;
                                last_message = 1;
                            }
                        }
                        if (continuation_token && index_resp->has_continuation) {
                            RMALLOCCOPY(client, continuation_token->value, continuation_token->len,
                                    index_resp->continuation.data, index_resp->continuation.len);
                        } else if (continuation_token) {
                            continuation_token->len = 0;
                            continuation_token->value = 0;
                        }
                        rpb_index_resp__free_unpacked(index_resp, &pb_allocator);
                        retval = RIACK_SUCCESS;
                    } else {
                        last_message = 1;
                        retval = RIACK_FAILED_PB_UNPACK;
                    }
                } else {
                    last_message = 1;
                    riack_got_error_response(client, msg_resp);
                    retval = RIACK_ERROR_RESPONSE;
                }
                riack_message_free(client, &msg_resp);
            }
		}
		RFREE(client, request_buffer);
	}
    return retval;
}

int riack_2i_query_exact(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *index,
        RIACK_STRING *search_key, RIACK_STRING_LIST** result_keys)
{
	int result;
	RpbIndexReq req;
    // TODO Validate input
	rpb_index_req__init(&req);
	req.bucket.len = bucket->len;
	req.bucket.data = (uint8_t*)bucket->value;
	req.has_key = 1;
	req.key.len = search_key->len;
	req.key.data = (uint8_t*)search_key->value;
	req.index.len = index->len;
	req.index.data = (uint8_t*)index->value;
	req.qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__eq;
    result = riack_2i_query(client, &req, result_keys, 0, 0, 0);
	return result;
}

int riack_2i_query_range(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *index, RIACK_STRING *search_key_min,
		RIACK_STRING *search_key_max, RIACK_STRING_LIST** result_keys)
{
	int result;
	RpbIndexReq req;
    // TODO Validate input
    rpb_index_req__init(&req);
	req.bucket.len = bucket->len;
	req.bucket.data = (uint8_t*)bucket->value;
	req.has_range_min = 1;
	req.range_min.len = search_key_min->len;
	req.range_min.data = (uint8_t*)search_key_min->value;
	req.has_range_max = 1;
	req.range_max.len = search_key_max->len;
	req.range_max.data = (uint8_t*)search_key_max->value;
	req.index.len = index->len;
	req.index.data = (uint8_t*)index->value;
	req.qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__range;
    result = riack_2i_query(client, &req, result_keys, 0, 0, 0);
	return result;
}

void riack_set_index_req_from_riack_req(RIACK_2I_QUERY_REQ *req, RpbIndexReq *pbreq)
{
    pbreq->bucket.len = req->bucket.len;
    pbreq->bucket.data = (uint8_t*)req->bucket.value;
    pbreq->index.len = req->index.len;
    pbreq->index.data = (uint8_t*)req->index.value;
    if (RSTR_HAS_CONTENT(req->search_exact)) {
        // Exact query
        pbreq->has_key = 1;
        pbreq->key.data = (uint8_t*)req->search_exact.value;
        pbreq->key.len = req->search_exact.len;
        pbreq->qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__eq;
    } else {
        // Ranged query
        pbreq->has_range_min = 1;
        pbreq->range_min.data = (uint8_t*)req->search_min.value;
        pbreq->range_min.len = req->search_min.len;
        pbreq->has_range_max = 1;
        pbreq->range_max.data = (uint8_t*)req->search_max.value;
        pbreq->range_max.len = req->search_max.len;
        pbreq->qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__range;
    }
    if (req->max_results > 0) {
        pbreq->has_max_results = 1;
        pbreq->max_results = req->max_results;
    }
    if (RSTR_HAS_CONTENT(req->continuation_token)) {
        pbreq->has_continuation = 1;
        pbreq->continuation.data = (uint8_t*)req->continuation_token.value;
        pbreq->continuation.len = req->continuation_token.len;
    }
}

int riack_2i_query_ext(RIACK_CLIENT *client,
                       RIACK_2I_QUERY_REQ *req,
                       RIACK_STRING_LIST **result_keys,
                       RIACK_STRING *continuation_token_out)
{
    int result;
    RpbIndexReq pbreq;
    if (!client || !req || !result_keys) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_index_req__init(&pbreq);
    riack_set_index_req_from_riack_req(req, &pbreq);
    result = riack_2i_query(client, &pbreq, result_keys, continuation_token_out, 0, 0);
    return result;
}

int riack_2i_query_stream_ext(RIACK_CLIENT *client, RIACK_2I_QUERY_REQ *req, RIACK_STRING *continuation_token_out,
                              void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING *key), void *callback_arg)
{
    int result;
    RpbIndexReq pbreq;
    if (!client || !req) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_index_req__init(&pbreq);
    riack_set_index_req_from_riack_req(req, &pbreq);
    pbreq.stream = pbreq.has_stream = 1;
    result = riack_2i_query(client, &pbreq, 0, continuation_token_out, callback, callback_arg);
    return result;
}

/***************************
* CRDTs
***************************/

int riack_counter_get(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key,
        RIACK_COUNTER_GET_PROPERTIES *props, int64_t *result)
{
    RIACK_REQ_LOCALS;
    RpbCounterGetReq pbreq;
    RpbCounterGetResp *pbresp;

    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(key)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    retval = RIACK_ERROR_COMMUNICATION;
    rpb_counter_get_req__init(&pbreq);

    pbreq.bucket.data = (uint8_t*)bucket->value;
    pbreq.bucket.len = bucket->len;
    pbreq.key.data = (uint8_t*)key->value;
    pbreq.key.len = key->len;
    if (props) {
        pbreq.has_basic_quorum = props->basic_quorum_use;
        pbreq.basic_quorum = props->basic_quorum;
        pbreq.has_notfound_ok = props->notfound_ok_use;
        pbreq.notfound_ok = props->notfound_ok;
        pbreq.has_pr = props->pr_use;
        pbreq.pr = props->pr;
        pbreq.has_r = props->r_use;
        pbreq.r = props->r;
    }

    pb_allocator = riack_pb_allocator(&client->allocator);
    packed_size = rpb_counter_get_req__get_packed_size(&pbreq);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_counter_get_req__pack(&pbreq, request_buffer);
        msg_req.msg = request_buffer;
        msg_req.msg_code = mc_RpbCounterGetReq;
        msg_req.msg_len = (uint32_t) packed_size;
        if ((riack_send_message(client, &msg_req) > 0) &&
            (riack_receive_message(client, &msg_resp) > 0)) {
            if (msg_resp->msg_code == mc_RpbCounterGetResp) {
                pbresp = rpb_counter_get_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                if (pbresp) {
                    if (pbresp->has_value && result) {
                        *result = pbresp->value;
                    } else if (result) {
                        *result = 0;
                    }
                    retval = RIACK_SUCCESS;
                    rpb_counter_get_resp__free_unpacked(pbresp, &pb_allocator);
                } else {
                    retval = RIACK_FAILED_PB_UNPACK;
                }
            } else {
                riack_got_error_response(client, msg_resp);
                retval = RIACK_ERROR_RESPONSE;
            }
            riack_message_free(client, &msg_resp);
        }
        RFREE(client, request_buffer);
    }

    return retval;
}

int riack_counter_increment(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key, int64_t amount,
        RIACK_COUNTER_UPDATE_PROPERTIES *props, int64_t *returned_value)
{
    RIACK_REQ_LOCALS;
    RpbCounterUpdateReq pbreq;
    RpbCounterUpdateResp *pbresp;

    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(key)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    retval = RIACK_ERROR_COMMUNICATION;
    rpb_counter_update_req__init(&pbreq);

    pbreq.bucket.data = (uint8_t*)bucket->value;
    pbreq.bucket.len = bucket->len;
    pbreq.key.data = (uint8_t*)key->value;
    pbreq.key.len = key->len;
    pbreq.amount = amount;
    pbreq.has_returnvalue = 1;
    if (returned_value) {
        pbreq.returnvalue = 1;
    } else {
        pbreq.returnvalue = 0;
    }
    if (props) {
        pbreq.has_dw = props->dw_use;
        pbreq.dw = props->dw;
        pbreq.has_pw = props->pw_use;
        pbreq.pw = props->pw;
        pbreq.has_w = props->w_use;
        pbreq.w = props->w;
    }
    pb_allocator = riack_pb_allocator(&client->allocator);
    packed_size = rpb_counter_update_req__get_packed_size(&pbreq);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_counter_update_req__pack(&pbreq, request_buffer);
        msg_req.msg = request_buffer;
        msg_req.msg_code = mc_RpbCounterUpdateReq;
        msg_req.msg_len = (uint32_t) packed_size;
        if ((riack_send_message(client, &msg_req) > 0) &&
            (riack_receive_message(client, &msg_resp) > 0)) {
            if (msg_resp->msg_code == mc_RpbCounterUpdateResp) {
                pbresp = rpb_counter_update_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                if (pbresp) {
                    if (returned_value && pbresp->has_value) {
                        *returned_value = pbresp->value;
                    } else if (returned_value) {
                        *returned_value = 0;
                    }
                    retval = RIACK_SUCCESS;
                    rpb_counter_update_resp__free_unpacked(pbresp, &pb_allocator);
                } else {
                    retval = RIACK_FAILED_PB_UNPACK;
                }
            } else {
                riack_got_error_response(client, msg_resp);
                retval = RIACK_ERROR_RESPONSE;
            }
        }
        RFREE(client, request_buffer);
    }
    return retval;
}
