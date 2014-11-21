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
#include "riack_helpers.h"
#include "riack.h"
#include "protocol/riak_msg_codes.h"

/******************************************************************************
* Map reduce
******************************************************************************/

static void _map_reduce_stream_callback(riack_client *client, void *args_raw, riack_mapred_response *result)
{
    riack_mapred_response_list** chain = (riack_mapred_response_list**)args_raw;
    riack_mapred_response_list* mapred_result_current =
            (riack_mapred_response_list*)RMALLOC(client, sizeof(riack_mapred_response_list));
    assert(chain);
    assert(result);
    riack_copy_strmapred_to_mapred(client, result, mapred_result_current);
    riack_mapred_add_to_chain(client, chain, mapred_result_current);
}

int riack_map_reduce(riack_client *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type, riack_mapred_response_list** mapred_result)
{
    if (!mapred_result) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    *mapred_result = 0;
    return riack_map_reduce_stream(client, data_len, data, content_type, _map_reduce_stream_callback, mapred_result);
}

int riack_map_reduce_stream(riack_client *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type,
        void(*callback)(riack_client*, void*, riack_mapred_response*),
        void* callback_arg)
{
    RIACK_REQ_LOCALS;
    riack_mapred_response mapred_result_current;
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


