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
#include <string.h>

#define RIACK_SET_BUCKETTYPE_AND_TIMEOUT(REQ)  if (bucket_type && bucket_type->len > 0) { \
                            REQ.has_type = 1; \
                            REQ.type.len = bucket_type->len; \
                            REQ.type.data = (uint8_t *) bucket_type->value; } \
                        if (timeout > 0) { REQ.has_timeout = 1; REQ.timeout = timeout; }

void riak_set_object_response_values(riack_client* client, riack_object *pobject, RpbPutResp* pput_resp);
void riak_set_object_response_values_get(riack_client* client, riack_get_object *object, RpbGetResp* getresp);
void riack_set_object_properties(riack_put_properties* pprops, RpbPutReq* pput_req);
void riack_set_get_properties(riack_client *client, riack_get_properties* props, RpbGetReq* get_req);
void riack_set_del_properties(riack_client *client, riack_del_properties* props, RpbDelReq* del_req);

/******************************************************************************
* Get
******************************************************************************/

riack_cmd_cb_result riack_get_cb(riack_client *client, RpbGetResp* response, riack_get_object** result_object)
{
    *result_object = riack_get_object_alloc(client);
    riak_set_object_response_values_get(client, *result_object, response);
    return RIACK_CMD_DONE;
}

int riack_get(riack_client *client, riack_string *bucket, riack_string *key, riack_get_properties* props,
        riack_get_object** result_object)
{
    return riack_get_ext(client, bucket, key, props, 0, result_object, 0);
}

int riack_get_ext(riack_client *client, riack_string *bucket, riack_string *key,
        riack_get_properties* props, riack_string *bucket_type, riack_get_object** result_object, uint32_t timeout)
{
    RpbGetReq get_req;
    if (!RSTR_HAS_CONTENT_P(key) || !RSTR_HAS_CONTENT_P(bucket) || !client || !result_object) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_get_req__init(&get_req);
    get_req.key.data = (uint8_t *) key->value;
    get_req.key.len = key->len;
    get_req.bucket.data = (uint8_t *) bucket->value;
    get_req.bucket.len = bucket->len;
    riack_set_get_properties(client, props, &get_req);
    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(get_req)

    return riack_perform_commmand(client, &cmd_get, (struct rpb_base_req const *) &get_req,
            (cmd_response_cb) riack_get_cb, (void **) result_object);
}

/******************************************************************************
* Put
******************************************************************************/


int riack_put_simple(riack_client *client, char* bucket, char* key, uint8_t* data, size_t datalen, char* content_type)
{
    int result;
    riack_object object;
    object.bucket.value = bucket;
    object.bucket.len = strlen(bucket);
    object.key.value = key;
    object.key.len = strlen(key);
    object.vclock.len = 0;
    object.content = (riack_content*)RMALLOC(client, sizeof(riack_content));
    memset(object.content, 0, sizeof(riack_content));
    object.content[0].content_type.value = content_type;
    object.content[0].content_type.len = strlen(content_type);
    object.content[0].data_len = datalen;
    object.content[0].data = data;
    result = riack_put(client, &object, NULL, NULL);
    RFREE(client, object.content);
    return result;
}

riack_cmd_cb_result riack_put_cb(riack_client *client, RpbPutResp* response, riack_object** returned_object)
{
    if (returned_object) {
        *returned_object = riack_object_alloc(client);
        riak_set_object_response_values(client, *returned_object, response);
    }
    return RIACK_CMD_DONE;
}

int riack_put(riack_client *client, riack_object *object, riack_object**returned_object,
        riack_put_properties* props)
{
    return riack_put_ext(client, object, NULL, returned_object, props, 0);
}

int riack_put_ext(riack_client *client, riack_object *object, riack_string *bucket_type,
        riack_object** returned_object, riack_put_properties* props, uint32_t timeout)
{
    RpbPutReq put_req;
    int retval;
    if (!client || !object || !object->bucket.value) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_put_req__init(&put_req);
    riack_copy_object_to_rpbputreq(client, object, &put_req);
    riack_set_object_properties(props, &put_req);
    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(put_req)
    retval = riack_perform_commmand(client, &cmd_put, (struct rpb_base_req const *) &put_req,
            (cmd_response_cb) riack_put_cb, (void **) returned_object);
    riack_free_copied_rpb_put_req(client, &put_req);
    return retval;
}

/******************************************************************************
* Delete
******************************************************************************/

int riack_delete(riack_client *client, riack_string *bucket, riack_string *key, riack_del_properties *props)
{
    return riack_delete_ext(client, bucket, NULL, key, props, 0);
}

int riack_delete_ext(riack_client *client, riack_string *bucket, riack_string *bucket_type,
        riack_string *key, riack_del_properties *props, uint32_t timeout)
{
    int result;
    RpbDelReq del_req;
    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(key)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_del_req__init(&del_req);
    del_req.bucket.len = bucket->len;
    del_req.bucket.data = (uint8_t *) bucket->value;
    del_req.key.len = key->len;
    del_req.key.data = (uint8_t *) key->value;
    riack_set_del_properties(client, props, &del_req);
    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(del_req)
    result = riack_perform_commmand(client, &cmd_delete, (struct rpb_base_req const *) &del_req, NULL, NULL);
    // TODO No need to copy and free vclock
    RFREE(client, del_req.vclock.data);
    return result;
}

/******************************************************************************
* List keys
******************************************************************************/

static void _list_keys_stream_callback(riack_client *client, void *args_raw, riack_string key)
{
    riack_string_linked_list **current = (riack_string_linked_list**)args_raw;
	riack_string new_string;
	assert(current);
	RMALLOCCOPY(client, new_string.value, new_string.len, key.value, key.len);
	riack_string_linked_list_add(client, current, new_string);
}

int riack_list_keys_ext(riack_client *client, riack_string *bucket, riack_string *bucket_type,
        riack_string_linked_list** keys, uint32_t timeout)
{
    if (!keys) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *keys = 0;
    return riack_stream_keys_ext(client, bucket, bucket_type, _list_keys_stream_callback, keys, timeout);
}

int riack_list_keys(riack_client *client, riack_string *bucket, riack_string_linked_list** keys)
{
	return riack_list_keys_ext(client, bucket, NULL, keys, 0);
}

riack_cmd_cb_result riack_stream_keys_ext_cb(riack_client *client, RpbListKeysResp *response,
        riack_stream_cb_params* user_cb_arg)
{
    riack_string current_string;
    size_t i;
    for (i=0; i<response->n_keys; ++i) {
        current_string.value = (char*)response->keys[i].data;
        current_string.len   = response->keys[i].len;
        user_cb_arg->callback(client, user_cb_arg->user_cb_arg, current_string);
    }
    if (response->has_done && response->done) {
        return RIACK_CMD_DONE;
    }
    return RIACK_CMD_CONTINUE;
}

int riack_stream_keys_ext(riack_client *client, riack_string *bucket, riack_string* bucket_type,
        list_keys_stream_cb callback, void* callback_arg, uint32_t timeout)
{
    RpbListKeysReq list_req;
    riack_stream_cb_params user_cb;

    if (!client || !callback || !RSTR_HAS_CONTENT_P(bucket)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_list_keys_req__init(&list_req);
    list_req.bucket.len = bucket->len;
    list_req.bucket.data = (uint8_t*)bucket->value;
    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(list_req)
    user_cb.callback = callback;
    user_cb.user_cb_arg = callback_arg;
    return riack_perform_commmand(client, &cmd_list_keys, (struct rpb_base_req const *) &list_req,
            (cmd_response_cb) riack_stream_keys_ext_cb, (void **) &user_cb);
}

int riack_stream_keys(riack_client *client, riack_string *bucket,
					  void(*callback)(riack_client*, void*, riack_string), void *callback_arg)
{
    return riack_stream_keys_ext(client, bucket, NULL, callback, callback_arg, 0);
}

/******************************************************************************
* List buckets
******************************************************************************/

int riack_list_buckets(riack_client *client, riack_string_list** bucket_list)
{
    return riack_list_buckets_ext(client, NULL, bucket_list, 0);
}

riack_cmd_cb_result riack_list_buckets_ext_cb(riack_client *client, RpbListBucketsResp *response,
        riack_string_list** bucket_list)
{
    size_t i;
    (*bucket_list)->string_count = response->n_buckets;
    if (response->n_buckets > 0) {
        (*bucket_list)->strings = (riack_string *) RMALLOC(client, sizeof(riack_string) * response->n_buckets);
        for (i = 0; i < response->n_buckets; ++i) {
            RMALLOCCOPY(client,
                    (*bucket_list)->strings[i].value,
                    (*bucket_list)->strings[i].len,
                    response->buckets[i].data,
                    response->buckets[i].len);
        }
    }
    return RIACK_CMD_DONE;
}

int riack_list_buckets_ext(riack_client *client, riack_string* bucket_type,
        riack_string_list** bucket_list, uint32_t timeout)
{
    RpbListBucketsReq list_req;
    if (!client || !bucket_list) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *bucket_list = riack_string_list_alloc(client);
    (*bucket_list)->string_count = 0;

    rpb_list_buckets_req__init(&list_req);
    RIACK_SET_BUCKETTYPE_AND_TIMEOUT(list_req)

    return riack_perform_commmand(client, &cmd_list_buckets, (struct rpb_base_req const *) &list_req,
            (cmd_response_cb) riack_list_buckets_ext_cb, (void **) bucket_list);
}

/******************************************************************************
* Get/Set client id
******************************************************************************/

int riack_set_clientid(riack_client *client, riack_string *clientid)
{
    RpbSetClientIdReq req;
	rpb_set_client_id_req__init(&req);
	req.client_id.len = clientid->len;
    req.client_id.data = (uint8_t*)clientid->value;
    return riack_perform_commmand(client, &cmd_set_clientid, (struct rpb_base_req const *) &req, NULL, NULL);
}

riack_cmd_cb_result riack_get_clientid_cb(riack_client *client, RpbGetClientIdResp* response, riack_string **clientid)
{
    *clientid = riack_string_alloc(client);
    RMALLOCCOPY(client, (*clientid)->value, (*clientid)->len, response->client_id.data, response->client_id.len);
    return RIACK_CMD_DONE;
}

int riack_get_clientid(riack_client *client, riack_string **clientid)
{
    if (!client || !clientid) {
        return RIACK_ERROR_INVALID_INPUT;
    }
	return riack_perform_commmand(client, &cmd_get_clientid, NULL,
            (cmd_response_cb) riack_get_clientid_cb, (void **) clientid);
}

/******************************************************************************
* Helper functions
******************************************************************************/

void riak_set_object_response_values(riack_client* client, riack_object *pobject, RpbPutResp* pput_resp)
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
        pobject->content = RMALLOC(client, sizeof(riack_content)*content_cnt);
        for (i=0; i<content_cnt; ++i) {
            riack_copy_rpbcontent_to_content(client, pput_resp->content[i], &pobject->content[i]);
        }
    }
}

void riak_set_object_response_values_get(riack_client* client, riack_get_object *object, RpbGetResp* getresp)
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
        object->object.content = RMALLOC(client, sizeof(riack_content)*cnt);
        for (i=0; i<cnt; ++i) {
            riack_copy_rpbcontent_to_content(client, getresp->content[i], &object->object.content[i]);
        }
    }
}


void riack_set_object_properties(riack_put_properties* pprops, RpbPutReq* pput_req)
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


void riack_set_get_properties(riack_client *client, riack_get_properties* props, RpbGetReq* get_req)
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


void riack_set_del_properties(riack_client *client, riack_del_properties* props, RpbDelReq* del_req)
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
