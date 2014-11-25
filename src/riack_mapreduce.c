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

riack_cmd_cb_result riack_map_reduce_stream_cb(riack_client *client, RpbMapRedResp *response, riack_mapreduce_cb_params *cb_params)
{
    riack_mapred_response mapred_result_current;
    memset(&mapred_result_current, 0, sizeof(mapred_result_current));
    riack_link_strmapred_with_rpbmapred(client, response, &mapred_result_current);
    cb_params->callback(client, cb_params->user_cb_arg, &mapred_result_current);
    if (response->has_done && response->done) {
        return RIACK_CMD_DONE;
    }
    return RIACK_CMD_CONTINUE;
}

int riack_map_reduce_stream(riack_client *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type, map_reduce_stream_cb callback, void* callback_arg)
{
    RpbMapRedReq mr_req;
    riack_mapreduce_cb_params cb_params;
    char* content_type_sz;

    if (!client || !data || data_len == 0 || !callback) {
        return RIACK_ERROR_INVALID_INPUT;
    }
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
    cb_params.user_cb_arg = callback_arg;
    cb_params.callback = callback;
    return riack_perform_commmand(client, &cmd_map_reduce, (struct rpb_base_req const *) &mr_req,
            (cmd_response_cb) riack_map_reduce_stream_cb, (void **) &cb_params);
}


