/*
   Copyright 2014 Trifork A/S
   Author: Kaspar Bach Pedersen

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

#include <string.h>
#include "riack_internal.h"
#include "protocol/riak_kv.pb-c.h"

/******************************************************************************
* Secondary indexes
******************************************************************************/


struct riack_2i_cb_args {
    index_query_cb_fn user_cb;
    void* user_cb_arg;
    riack_string_list** result_keys;
    riack_string** continuation_token;
};

riack_cmd_cb_result riack_2i_query_cb(riack_client *client, RpbIndexResp* response, struct riack_2i_cb_args* args)
{
    size_t keys, i;
    riack_cmd_cb_result retval;
    retval = RIACK_CMD_DONE;
    keys = response->n_keys;
    // If we have result keys then we are not streaming
    if (args->result_keys) {
        (*args->result_keys)->string_count = keys;
        (*args->result_keys)->strings = RMALLOC(client, sizeof(riack_string) * keys);
        for (i=0; i<keys; ++i) {
            RMALLOCCOPY(client, (*args->result_keys)->strings[i].value, (*args->result_keys)->strings[i].len,
                    response->keys[i].data, response->keys[i].len);
        }
    }
    // If streaming
    if (args->user_cb) {
        retval = RIACK_CMD_CONTINUE;
        for (i=0; i<keys; ++i) {
            riack_string key;
            key.len = response->keys[i].len;
            key.value = (char*)response->keys[i].data;
            args->user_cb(client, args->user_cb_arg, &key);
        }
        if (response->has_done && response->done) {
            retval = RIACK_CMD_DONE;
        }
    }
    if (args->continuation_token && response->has_continuation) {
        *args->continuation_token = riack_string_alloc(client);
        RMALLOCCOPY(client, (*args->continuation_token)->value, (*args->continuation_token)->len,
                response->continuation.data, response->continuation.len);
    } else if (args->continuation_token) {
        *args->continuation_token = 0;
    }
    return retval;
}


int riack_2i_query_exact(riack_client *client, riack_string *bucket, riack_string *index,
        riack_string *search_key, riack_string_list** result_keys)
{
    return riack_2i_query_exact_ext(client, bucket, NULL, index, search_key, result_keys);
}

int riack_2i_query_exact_ext(riack_client *client, riack_string *bucket, riack_string *bucket_type, riack_string *index,
        riack_string *search_key, riack_string_list** result_keys)
{
    int result;
    RpbIndexReq req;
    if (!client || !result_keys || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(index) || !RSTR_HAS_CONTENT_P(search_key)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_index_req__init(&req);
    req.bucket.len = bucket->len;
    req.bucket.data = (uint8_t*)bucket->value;
    req.has_key = 1;
    req.key.len = search_key->len;
    req.key.data = (uint8_t*)search_key->value;
    req.index.len = index->len;
    req.index.data = (uint8_t*)index->value;
    req.qtype = RPB_INDEX_REQ__INDEX_QUERY_TYPE__eq;
    if (RSTR_HAS_CONTENT_P(bucket_type)) {
        req.type.data = (uint8_t *) bucket_type->value;
        req.type.len = bucket_type->len;
    }
    result = riack_perform_2i_query(client, &req, result_keys, 0, 0, 0);
    return result;
}


int riack_2i_query_range(riack_client *client, riack_string *bucket, riack_string *index, riack_string *search_key_min,
        riack_string *search_key_max, riack_string_list** result_keys)
{
    return riack_2i_query_range_ext(client, bucket, NULL, index, search_key_min, search_key_max, result_keys);
}

int riack_2i_query_range_ext(riack_client *client, riack_string *bucket, riack_string *bucket_type, riack_string *index,
        riack_string *search_key_min, riack_string *search_key_max,
        riack_string_list **result_keys)
{
    int result;
    RpbIndexReq req;
    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(index) || !RSTR_HAS_CONTENT_P(search_key_min) ||
            !RSTR_HAS_CONTENT_P(search_key_max) || !result_keys) {
        return RIACK_ERROR_INVALID_INPUT;
    }
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
    if (RSTR_HAS_CONTENT_P(bucket_type)) {
        req.type.data = (uint8_t *) bucket_type->value;
        req.type.len = bucket_type->len;
    }
    result = riack_perform_2i_query(client, &req, result_keys, 0, 0, 0);
    return result;
}

void riack_set_index_req_from_riack_req(riack_2i_query_req *req, RpbIndexReq *pbreq)
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
    if (RSTR_HAS_CONTENT(req->bucket_type)) {
        pbreq->has_type = 1;
        pbreq->type.data = (uint8_t *) req->bucket_type.value;
        pbreq->type.len = req->bucket_type.len;
    }
}

int riack_2i_query(riack_client *client, riack_2i_query_req *req,
        riack_string_list **result_keys, riack_string **continuation_token_out)
{
    int result;
    RpbIndexReq pbreq;
    if (!client || !req || !result_keys) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_index_req__init(&pbreq);
    riack_set_index_req_from_riack_req(req, &pbreq);
    result = riack_perform_2i_query(client, &pbreq, result_keys, continuation_token_out, 0, 0);
    return result;
}

int riack_2i_query_stream(riack_client *client, riack_2i_query_req *req, riack_string **continuation_token_out,
        void(*callback)(riack_client*, void*, riack_string *key), void *callback_arg)
{
    int result;
    RpbIndexReq pbreq;
    if (!client || !req) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    rpb_index_req__init(&pbreq);
    riack_set_index_req_from_riack_req(req, &pbreq);
    pbreq.stream = pbreq.has_stream = 1;
    result = riack_perform_2i_query(client, &pbreq, 0, continuation_token_out, callback, callback_arg);
    return result;
}


int riack_perform_2i_query(riack_client *client, RpbIndexReq* request, riack_string_list** result_keys,
        riack_string** continuation_token, index_query_cb_fn callback, void *callback_arg)
{
    struct riack_2i_cb_args cb_args_cmd;
    if (result_keys) {
        *result_keys = riack_string_list_alloc(client);
    }
    cb_args_cmd.result_keys = result_keys;
    cb_args_cmd.user_cb = callback;
    cb_args_cmd.user_cb_arg = callback_arg;
    cb_args_cmd.continuation_token = continuation_token;
    return riack_perform_commmand(client, &cmd_index_query, (struct rpb_base_req const *) request,
            (cmd_response_cb) riack_2i_query_cb, (void **) &cb_args_cmd);

}
