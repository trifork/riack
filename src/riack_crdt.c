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
* CRDTs
******************************************************************************/

int riack_counter_get_cb(riack_client *client, RpbCounterGetResp *response, int64_t *returned_value)
{
    if (returned_value && response->has_value) {
        *returned_value = response->value;
    } else if (returned_value) {
        *returned_value = 0;
    }
    return RIACK_CMD_DONE;
}

int riack_counter_get(riack_client *client, riack_string *bucket, riack_string *key,
        riack_counter_get_properties *props, int64_t *result)
{
    RpbCounterGetReq pbreq;
    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(key)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
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
    return riack_perform_commmand(client, &cmd_counter_get, (struct rpb_base_req const *) &pbreq,
            (cmd_response_cb) riack_counter_get_cb, (void **) result);
}

riack_cmd_cb_result riack_counter_increment_cb(riack_client *client, RpbCounterUpdateResp *response, int64_t *returned_value)
{
    if (returned_value && response->has_value) {
        *returned_value = response->value;
    } else if (returned_value) {
        *returned_value = 0;
    }
    return RIACK_CMD_DONE;
}

int riack_counter_increment(riack_client *client, riack_string *bucket, riack_string *key, int64_t amount,
        riack_counter_update_properties *props, int64_t *returned_value)
{
    RpbCounterUpdateReq pbreq;
    if (!client || !RSTR_HAS_CONTENT_P(bucket) || !RSTR_HAS_CONTENT_P(key)) {
        return RIACK_ERROR_INVALID_INPUT;
    }
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
    return riack_perform_commmand(client, &cmd_counter_increment, (struct rpb_base_req const *) &pbreq,
            (cmd_response_cb) riack_counter_increment_cb, (void **) returned_value);
}

