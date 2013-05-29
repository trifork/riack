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
#include "protocol/riak_msg_codes.h"
#include "protocol/riak_kv.pb-c.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void riack_got_error_response(struct RIACK_CLIENT *client, struct RIACK_PB_MSG *msg);
ProtobufCAllocator riack_pb_allocator(struct RIACK_ALLOCATOR *allocator);

void riak_set_object_response_values(struct RIACK_CLIENT* client, struct RIACK_OBJECT *pobject, RpbPutResp* pput_resp)
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
		RMALLOCCOPY(client, pput_resp->key.data, pput_resp->key.len, pobject->key.value, pobject->key.len);
	}
	pobject->vclock.len = 0;
	pobject->vclock.clock = 0;
	if (pput_resp->has_vclock) {
		RMALLOCCOPY(client, pput_resp->vclock.data, pput_resp->vclock.len,
				pobject->vclock.clock, pobject->vclock.len);
	}
	content_cnt = pput_resp->n_content;
	pobject->content_count = content_cnt;
	if (content_cnt > 0) {
		pobject->content = RMALLOC(client, sizeof(struct RIACK_CONTENT)*content_cnt);
		for (i=0; i<content_cnt; ++i) {
			riack_copy_rpbcontent_to_content(client, pput_resp->content[i], &pobject->content[i]);
		}
	}
}

void riak_set_object_response_values_get(struct RIACK_CLIENT* client, struct RIACK_GET_OBJECT *object, RpbGetResp* getresp)
{
	size_t cnt, i;
	if (!object || !getresp) {
		return;
	}
	object->unchanged_present = getresp->has_unchanged ? 1 : 0;
	object->unchanged = getresp->unchanged ? 1 : 0;
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
		object->object.content = RMALLOC(client, sizeof(struct RIACK_CONTENT)*cnt);
		for (i=0; i<cnt; ++i) {
			riack_copy_rpbcontent_to_content(client, getresp->content[i], &object->object.content[i]);
		}
	}
}


int riack_put_simple(struct RIACK_CLIENT *client, char* bucket, char* key, uint8_t* data, size_t datalen, char* content_type)
{
	int result;
	struct RIACK_OBJECT object;
	object.bucket.value = bucket;
	object.bucket.len = strlen(bucket);
	object.key.value = key;
	object.key.len = strlen(key);
	object.vclock.len = 0;
	object.content = (struct RIACK_CONTENT*)RMALLOC(client, sizeof(struct RIACK_CONTENT));
	memset(object.content, 0, sizeof(struct RIACK_CONTENT));
	object.content[0].content_type.value = content_type;
	object.content[0].content_type.len = strlen(content_type);
	object.content[0].data_len = datalen;
	object.content[0].data = data;
	result = riack_put(client, object, 0, (struct RIACK_PUT_PROPERTIES*)0);
	RFREE(client, object.content);
	return result;
}

void riack_set_object_properties(struct RIACK_PUT_PROPERTIES* pprops, RpbPutReq* pput_req)
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

void riack_set_get_properties(struct RIACK_CLIENT *client, struct RIACK_GET_PROPERTIES* props, RpbGetReq* get_req)
{
	if (props) {
		get_req->has_basic_quorum = props->basic_quorum_present;
		get_req->basic_quorum = props->basic_quorum;
		get_req->has_deletedvclock = props->deletedvclock_present;
		get_req->deletedvclock = props->deletedvclock;
		get_req->has_head = props->head_use;
		get_req->head = props->head;
		get_req->has_if_modified = 0;
		get_req->has_notfound_ok = props->notfound_ok_present;
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

void riack_set_del_properties(struct RIACK_CLIENT *client, struct RIACK_DEL_PROPERTIES* props, RpbDelReq* del_req)
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

int riack_get(struct RIACK_CLIENT *client,
			 RIACK_STRING bucket,
			 RIACK_STRING key,
			 struct RIACK_GET_PROPERTIES* props,
			 struct RIACK_GET_OBJECT* result_object)
{
	int result;
	size_t packed_size;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	ProtobufCAllocator pb_allocator;
	RpbGetReq get_req;
	RpbGetResp *get_resp;
	uint8_t *request_buffer;

	if (!bucket.value || !key.value || key.len == 0 || bucket.len == 0 || !client) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);

	result = RIACK_ERROR_COMMUNICATION;
	rpb_get_req__init(&get_req);
	get_req.key.data = (uint8_t *) key.value;
	get_req.key.len = key.len;
	get_req.bucket.data = (uint8_t *) bucket.value;
	get_req.bucket.len = bucket.len;
	riack_set_get_properties(client, props, &get_req);

	packed_size = rpb_get_req__get_packed_size(&get_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_get_req__pack(&get_req, request_buffer);
		msg_req.msg_code = mc_RpbGetReq;
		msg_req.msg_len = packed_size;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0))
		{
			if (msg_resp->msg_code == mc_RpbGetResp) {
				get_resp = rpb_get_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
				if (get_resp) {
					riak_set_object_response_values_get(client, result_object, get_resp);
					rpb_get_resp__free_unpacked(get_resp, &pb_allocator);
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

int riack_put(struct RIACK_CLIENT *client,
			struct RIACK_OBJECT object,
			struct RIACK_OBJECT* preturned_object,
			struct RIACK_PUT_PROPERTIES* properties)
{
	int result;
	size_t packed_size;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	ProtobufCAllocator pb_allocator;
	RpbPutReq put_req;
	RpbPutResp *put_resp;
	uint8_t *request_buffer;

	if (!client || !object.bucket.value || !object.key.value) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);
	result = RIACK_ERROR_COMMUNICATION;
	rpb_put_req__init(&put_req);
	riack_copy_object_to_rpbputreq(client, &object, &put_req);
	riack_set_object_properties(properties, &put_req);

	packed_size = rpb_put_req__get_packed_size(&put_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_put_req__pack(&put_req, request_buffer);
		msg_req.msg_code = mc_RpbPutReq;
		msg_req.msg_len = packed_size;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0))
		{
			if (msg_resp->msg_code == mc_RpbPutResp) {
				result = RIACK_SUCCESS;
				if (preturned_object) {
					put_resp = rpb_put_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
					if (put_resp) {
						riak_set_object_response_values(client, preturned_object, put_resp);
						rpb_put_resp__free_unpacked(put_resp, &pb_allocator);
					} else {
						result = RIACK_FAILED_PB_UNPACK;
					}
				}
			} else {
				riack_got_error_response(client, msg_resp);
				result = RIACK_ERROR_RESPONSE;
			}
			riack_message_free(client, &msg_resp);
		}
		RFREE(client, request_buffer);
	}
	riack_free_copied_rpb_put_req(client, &put_req);
	return result;
}

int riack_delete(struct RIACK_CLIENT *client, RIACK_STRING bucket, RIACK_STRING key, struct RIACK_DEL_PROPERTIES *props)
{
	int result;
	size_t packed_size;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	RpbDelReq del_req;
	uint8_t *request_buffer;

	if (!client || !bucket.value || !key.value || key.len == 0 || bucket.len == 0) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	result = RIACK_ERROR_COMMUNICATION;
	rpb_del_req__init(&del_req);

	del_req.bucket.len = bucket.len;
	del_req.bucket.data = (uint8_t *) bucket.value;
	del_req.key.len = key.len;
	del_req.key.data = (uint8_t *) key.value;
	riack_set_del_properties(client, props, &del_req);

	packed_size = rpb_del_req__get_packed_size(&del_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_del_req__pack(&del_req, request_buffer);
		msg_req.msg_code = mc_RpbDelReq;
		msg_req.msg_len = packed_size;
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

int riack_get_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t *n_val, uint8_t *allow_mult)
{
	int result;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	ProtobufCAllocator pb_allocator;
	size_t packed_size;
	uint8_t *request_buffer;
	RpbGetBucketResp *response;
	RpbGetBucketReq get_request = RPB_GET_BUCKET_REQ__INIT;

	if (!client || !bucket.value || bucket.len == 0) {
		return RIACK_ERROR_INVALID_INPUT;
	}
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
				response = rpb_get_bucket_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
				if (response) {
					if (response->props->has_allow_mult) {
						*allow_mult = response->props->allow_mult ? 1 : 0;
					}
					if (response->props->has_n_val) {
						*n_val = response->props->n_val;
					}
					rpb_get_bucket_resp__free_unpacked(response, &pb_allocator);
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

static void _list_keys_stream_callback(struct RIACK_CLIENT *client, void *args_raw, RIACK_STRING key)
{
	struct RIACK_STRING_LINKED_LIST **current = (struct RIACK_STRING_LINKED_LIST**)args_raw;
	RIACK_STRING new_string;
	assert(current);
	RMALLOCCOPY(client, new_string.value, new_string.len, key.value, key.len);
	riack_string_linked_list_add(client, current, new_string);
}

int riack_list_keys(struct RIACK_CLIENT *client, RIACK_STRING bucket, struct RIACK_STRING_LINKED_LIST** keys)
{
	if (!keys) {
		return RIACK_ERROR_INVALID_INPUT;
	}
	*keys = 0;
	return riack_stream_keys(client, bucket, _list_keys_stream_callback, keys);
}

int riack_stream_keys(struct RIACK_CLIENT *client, RIACK_STRING bucket,
					  void(*callback)(struct RIACK_CLIENT*, void*, RIACK_STRING), void *callback_arg)
{
	int result;
	struct RIACK_PB_MSG msg_req;
	struct RIACK_PB_MSG *msg_resp;
	RpbListKeysReq list_req;
	RpbListKeysResp *list_resp;
	RIACK_STRING current_string;
	ProtobufCAllocator pb_allocator;
	size_t packed_size, num_keys, i;
	uint8_t *request_buffer, recvdone;

	if (!client || !callback || bucket.len == 0) {
		return RIACK_ERROR_INVALID_INPUT;
	}
	pb_allocator = riack_pb_allocator(&client->allocator);
	result = RIACK_ERROR_COMMUNICATION;
	rpb_list_keys_req__init(&list_req);
	list_req.bucket.len = bucket.len;
	list_req.bucket.data = bucket.value;
	packed_size = rpb_list_keys_req__get_packed_size(&list_req);
	request_buffer = RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_list_keys_req__pack(&list_req, request_buffer);
		msg_req.msg_code = mc_RpbListKeysReq;
		msg_req.msg_len = packed_size;
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
								result = RIACK_SUCCESS;
							}
							num_keys = list_resp->n_keys;
							for (i=0; i<num_keys; ++i) {
								current_string.value = (char*)list_resp->keys[i].data;
								current_string.len   = list_resp->keys[i].len;
								callback(client, callback_arg, current_string);
							}
							rpb_list_keys_resp__free_unpacked(list_resp, &pb_allocator);
						} else {
							result = RIACK_FAILED_PB_UNPACK;
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
	return result;
}

int riack_list_buckets(struct RIACK_CLIENT *client, RIACK_STRING_LIST* bucket_list)
{
	int result;
	struct RIACK_PB_MSG msg_req;
	struct RIACK_PB_MSG *msg_resp;
	RpbListBucketsResp *list_resp;
	ProtobufCAllocator pb_allocator;
	size_t i;

	if (!client || !bucket_list) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);
	result = RIACK_ERROR_COMMUNICATION;

	msg_req.msg_code = mc_RpbListBucketsReq;
	msg_req.msg_len = 0;
	bucket_list->string_count = 0;
	if ((riack_send_message(client, &msg_req) > 0)&&
		(riack_receive_message(client, &msg_resp) > 0)) {
		if (msg_resp->msg_code == mc_RpbListBucketsResp) {
			list_resp = rpb_list_buckets_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
			if (list_resp) {
				bucket_list->string_count = list_resp->n_buckets;
				bucket_list->strings = (RIACK_STRING*)RMALLOC(client, sizeof(RIACK_STRING) * list_resp->n_buckets);
				for (i=0; i<list_resp->n_buckets; ++i) {
					RMALLOCCOPY(client,
								bucket_list->strings[i].value,
								bucket_list->strings[i].len,
								list_resp->buckets[i].data,
								list_resp->buckets[i].len);
				}
				rpb_list_buckets_resp__free_unpacked(list_resp, &pb_allocator);
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


int riack_set_clientid(struct RIACK_CLIENT *client, RIACK_STRING clientid)
{
	int result;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	RpbSetClientIdReq req;
	size_t packed_size;
	uint8_t *request_buffer;

	rpb_set_client_id_req__init(&req);
	req.client_id.len = clientid.len;
	req.client_id.data = clientid.value;

	packed_size = rpb_set_client_id_req__get_packed_size(&req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	result = RIACK_ERROR_COMMUNICATION;
	if (request_buffer) {
		rpb_set_client_id_req__pack(&req, request_buffer);
		msg_req.msg_len = packed_size;
		msg_req.msg_code = mc_RpbSetClientIdReq;
		msg_req.msg = request_buffer;
		if ((riack_send_message(client, &msg_req) > 0)&&
			(riack_receive_message(client, &msg_resp) > 0)) {
			if (msg_resp->msg_code == mc_RpbSetClientIdResp) {
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

int riack_get_clientid(struct RIACK_CLIENT *client, RIACK_STRING *clientid)
{
	int result;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	RpbGetClientIdResp *id_resp;
	ProtobufCAllocator pb_allocator;

	pb_allocator = riack_pb_allocator(&client->allocator);
	msg_req.msg = 0;
	msg_req.msg_len = 0;
	msg_req.msg_code = mc_RpbGetClientIdReq;
	result = RIACK_ERROR_COMMUNICATION;
	if ((riack_send_message(client, &msg_req) > 0) &&
		(riack_receive_message(client, &msg_resp) > 0)) {
		if (msg_resp->msg_code == mc_RpbGetClientIdResp) {
			id_resp = rpb_get_client_id_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
			if (id_resp) {
				RMALLOCCOPY(client, clientid->value, clientid->len, id_resp->client_id.data, id_resp->client_id.len);
				rpb_get_client_id_resp__free_unpacked(id_resp, &pb_allocator);
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

static void _map_reduce_stream_callback(struct RIACK_CLIENT *client, void *args_raw, struct RIACK_MAPRED_STREAM_RESULT *result)
{
	struct RIACK_MAPRED_RESULT** chain = (struct RIACK_MAPRED_RESULT**)args_raw;
	struct RIACK_MAPRED_RESULT* mapred_result_current =
			(struct RIACK_MAPRED_RESULT*)RMALLOC(client, sizeof(struct RIACK_MAPRED_RESULT));
	assert(chain);
	assert(result);
	riack_copy_strmapred_to_mapred(client, result, mapred_result_current);
	riack_mapred_add_to_chain(client, chain, mapred_result_current);
}

int riack_map_redue(struct RIACK_CLIENT *client, size_t data_len, uint8_t* data,
		enum RIACK_MAPRED_CONTENT_TYPE content_type, struct RIACK_MAPRED_RESULT** mapred_result)
{
	if (!mapred_result) {
		return RIACK_ERROR_INVALID_INPUT;
	}
	*mapred_result = 0;
	return riack_map_reduce_stream(client, data_len, data, content_type, _map_reduce_stream_callback, mapred_result);
}

int riack_map_reduce_stream(struct RIACK_CLIENT *client,
										 size_t data_len,
										 uint8_t* data,
										 enum RIACK_MAPRED_CONTENT_TYPE content_type,
										 void(*callback)(struct RIACK_CLIENT*, void*, struct RIACK_MAPRED_STREAM_RESULT*),
										 void* callback_arg)
{
	struct RIACK_MAPRED_STREAM_RESULT mapred_result_current;
	int result;
	size_t packed_size;
	struct RIACK_PB_MSG msg_req, *msg_resp;
	RpbMapRedReq mr_req;
	RpbMapRedResp *mr_resp;
	ProtobufCAllocator pb_allocator;
	uint8_t *request_buffer;
	char* content_type_sz;
	uint8_t last_message;

	memset(&mapred_result_current, 0, sizeof(mapred_result_current));

	if (!client || !data || data_len == 0 || !callback) {
		return RIACK_ERROR_INVALID_INPUT;
	}

	pb_allocator = riack_pb_allocator(&client->allocator);
	result = RIACK_ERROR_COMMUNICATION;

	rpb_map_red_req__init(&mr_req);
	if (content_type == APPLICATION_JSON) {
		content_type_sz = "application/json";
	} else if (content_type == APPLICATION_ERLANG_TERM) {
		content_type_sz = "application/x-erlang-binary";
	} else {
		return RIACK_ERROR_INVALID_INPUT;
	}
	mr_req.content_type.len = strlen(content_type_sz);
	mr_req.content_type.data = content_type_sz;
	mr_req.request.len = data_len;
	mr_req.request.data = data;
	packed_size = rpb_map_red_req__get_packed_size(&mr_req);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_map_red_req__pack(&mr_req, request_buffer);
		msg_req.msg_code = mc_RpbMapRedReq;
		msg_req.msg_len = packed_size;
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
							result = RIACK_SUCCESS;
							last_message = 1;
						}
						rpb_map_red_resp__free_unpacked(mr_resp, &pb_allocator);
					} else {
						result = RIACK_FAILED_PB_UNPACK;
					}
				} else {
					riack_got_error_response(client, msg_resp);
					result = RIACK_ERROR_RESPONSE;
					last_message = 1;
				}
				riack_message_free(client, &msg_resp);
			}
		}
		RFREE(client, request_buffer);
	}

	return result;
}

int riack_2i_query(struct RIACK_CLIENT *client, RpbIndexReq* request, RIACK_STRING_LIST* result_keys)
{
	struct RIACK_PB_MSG msg_req, *msg_resp;
	ProtobufCAllocator pb_allocator;
	RpbIndexResp *index_resp;
	int result;
	size_t packed_size, keys, i;
	uint8_t *request_buffer;
	result = RIACK_ERROR_COMMUNICATION;
	pb_allocator = riack_pb_allocator(&client->allocator);
	packed_size = rpb_index_req__get_packed_size(request);
	request_buffer = (uint8_t*)RMALLOC(client, packed_size);
	if (request_buffer) {
		rpb_index_req__pack(request, request_buffer);
		msg_req.msg = request_buffer;
		msg_req.msg_code = mc_RpbIndexReq;
		msg_req.msg_len = packed_size;
		if ((riack_send_message(client, &msg_req) > 0) &&
			(riack_receive_message(client, &msg_resp) > 0)) {
			if (msg_resp->msg_code == mc_RpbIndexResp) {
				index_resp = rpb_index_resp__unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
				if (index_resp) {
					keys = index_resp->n_keys;
					result_keys->string_count = keys;
					result_keys->strings = RMALLOC(client, sizeof(RIACK_STRING) * keys);
					for (i=0; i<keys; ++i) {
						RMALLOCCOPY(client, result_keys->strings[i].value, result_keys->strings[i].len,
								index_resp->keys[i].data, index_resp->keys[i].len);
					}
					rpb_index_resp__free_unpacked(index_resp, &pb_allocator);
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

int riack_2i_query_exact(struct RIACK_CLIENT *client, RIACK_STRING bucket,
		RIACK_STRING index, RIACK_STRING search_key, RIACK_STRING_LIST* result_keys)
{
	int result;
	RpbIndexReq req;
	rpb_index_req__init(&req);
	req.bucket.len = bucket.len;
	req.bucket.data = (uint8_t*)bucket.value;
	req.has_key = 1;
	req.key.len = search_key.len;
	req.key.data = (uint8_t*)search_key.value;
	req.index.len = index.len;
	req.index.data = (uint8_t*)index.value;
	req.qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__eq;
	result = riack_2i_query(client, &req, result_keys);
	return result;
}

int riack_2i_query_range(struct RIACK_CLIENT *client, RIACK_STRING bucket,
		RIACK_STRING index, RIACK_STRING search_key_min,
		RIACK_STRING search_key_max, RIACK_STRING_LIST* result_keys)
{
	int result;
	RpbIndexReq req;
	rpb_index_req__init(&req);
	req.bucket.len = bucket.len;
	req.bucket.data = (uint8_t*)bucket.value;
	req.has_range_min = 1;
	req.range_min.len = search_key_min.len;
	req.range_min.data = (uint8_t*)search_key_min.value;
	req.has_range_max = 1;
	req.range_max.len = search_key_max.len;
	req.range_max.data = (uint8_t*)search_key_max.value;
	req.index.len = index.len;
	req.index.data = (uint8_t*)index.value;
	req.qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__range;
	result = riack_2i_query(client, &req, result_keys);
	return result;
}
