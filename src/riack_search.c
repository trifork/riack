/*
   Copyright 2013 Trifork A/S
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

#include "riack.h"
#include "riack_msg.h"
#include "riack_helpers.h"
#include "protocol/riak_msg_codes.h"
#include "protocol/riak_search.pb-c.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void riack_got_error_response(struct RIACK_CLIENT *client, struct RIACK_PB_MSG *msg);
ProtobufCAllocator riack_pb_allocator(struct RIACK_ALLOCATOR *allocator);

void riack_free_copied_rpb_search_req(struct RIACK_CLIENT *client,
                                      RpbSearchQueryReq* search_req)
{
    if (search_req->has_df) {
        RFREE(client, search_req->df.data);
    }
    if (search_req->has_filter) {
        RFREE(client, search_req->filter.data);
    }
    if (search_req->has_op) {
        RFREE(client, search_req->op.data);
    }
    if (search_req->has_presort) {
        RFREE(client, search_req->presort.data);
    }
    if (search_req->has_sort) {
        RFREE(client, search_req->sort.data);
    }
    if (search_req->n_fl > 0) {
        size_t i;
        for (i=0; i<search_req->n_fl; ++i) {
            RFREE(client, search_req->fl[i].data);
        }
        RFREE(client, search_req->fl);
    }
}

/// Copies all optional search parameters to the protobuffers request.
void riack_set_optional_search_properties_on_req(struct RIACK_CLIENT *client,
                                                 struct RIACK_SEARCH_OPTIONAL_PARAMETERS* props,
                                                 RpbSearchQueryReq* search_req)
{
    size_t cnt;
    if (props->default_field_present) {
        search_req->has_df = 1;
        RMALLOCCOPY(client,
                    search_req->df.data, search_req->df.len,
                    props->default_field.value, props->default_field.len);
    }
    if (props->default_operation_present) {
        search_req->has_op = 1;
        RMALLOCCOPY(client,
                    search_req->op.data, search_req->op.len,
                    props->default_operation.value, props->default_operation.len);
    }
    if (props->filter_present) {
        search_req->has_filter = 1;
        RMALLOCCOPY(client,
                    search_req->filter.data, search_req->filter.len,
                    props->filter.value, props->filter.len);
    }
    if (props->presort_present) {
        search_req->has_presort = 1;
        RMALLOCCOPY(client,
                    search_req->presort.data, search_req->presort.len,
                    props->presort.value, props->presort.len);
    }
    if (props->rowlimit_present) {
        search_req->has_rows = 1;
        search_req->rows = props->rowlimit;
    }
    if (props->sort_present) {
        search_req->has_sort = 1;
        RMALLOCCOPY(client,
                    search_req->sort.data, search_req->sort.len,
                    props->sort.value, props->sort.len);
    }
    if (props->start_present) {
        search_req->has_start = 1;
        search_req->start = props->start;
    }
    cnt = props->field_limits_count;
    search_req->n_fl = cnt;
    if (cnt > 0) {
        size_t i;
        search_req->fl = RMALLOC(client, sizeof(ProtobufCBinaryData) * cnt);
        for (i = 0; i<cnt; ++i) {
            RMALLOCCOPY(client,
                        search_req->fl[i].data, search_req->fl[i].len,
                        props->field_limits[i].value, props->field_limits[i].len);
        }
    }
}


int riack_search(struct RIACK_CLIENT *client,
                 RIACK_STRING query,
                 RIACK_STRING index,
                 struct RIACK_SEARCH_OPTIONAL_PARAMETERS* optional_parameters,
                 struct RIACK_SEARCH_RESULT** search_result)
{
    int result;
    struct RIACK_PB_MSG msg_req, *msg_resp;
    ProtobufCAllocator pb_allocator;
    size_t packed_size;
    uint8_t *request_buffer;
    RpbSearchQueryResp *response;
    RpbSearchQueryReq search_req = RPB_SEARCH_QUERY_REQ__INIT;

    if (!client || !query.value || query.len == 0 || !index.value || index.len == 0) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    pb_allocator = riack_pb_allocator(&client->allocator);
    result = RIACK_ERROR_COMMUNICATION;

    search_req.q.data = (uint8_t*)query.value;
    search_req.q.len = query.len;
    search_req.index.data = (uint8_t*)index.value;
    search_req.index.len = index.len;
    riack_set_optional_search_properties_on_req(client, optional_parameters, &search_req);
    packed_size = rpb_search_query_req__get_packed_size(&search_req);

    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        rpb_search_query_req__pack(&search_req, request_buffer);
        msg_req.msg_code = mc_RpbSearchQueryReq;
        msg_req.msg_len = packed_size;
        msg_req.msg = request_buffer;
        if ((riack_send_message(client, &msg_req) > 0)&&
            (riack_receive_message(client, &msg_resp) > 0))
        {
            if (msg_resp->msg_code == mc_RbpSearchQueryResp) {
                //
            } else {
                riack_got_error_response(client, msg_resp);
                result = RIACK_ERROR_RESPONSE;
            }
            riack_message_free(client, &msg_resp);
        }
        RFREE(client, request_buffer);
    }
    riack_free_copied_rpb_search_req(client, &search_req);
    return result;
}
