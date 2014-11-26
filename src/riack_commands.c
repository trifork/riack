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

#include "riack.h"
#include "riack_internal.h"
#include "riack_helpers.h"
#include "protocol/riak_msg_codes.h"

/******************************************************************************
* Commands
******************************************************************************/

/* Ping command */
const struct pb_command cmd_ping = {
        mc_RpbPingReq,
        mc_RpbPingResp,
        0,
        0,
        0,
        0
};

/* Set bucket type command */
const struct pb_command cmd_set_bucket_type = {
        mc_RpbSetBucketTypeReq,
        mc_RpbSetBucketResp,
        (rpb_packed_size_fn) rpb_set_bucket_type_req__get_packed_size,
        (rpb_pack_fn) rpb_set_bucket_type_req__pack,
        0,
        0
};

/* Set bucket type command */
const struct pb_command cmd_set_bucket_properties = {
        mc_RpbSetBucketReq,
        mc_RpbSetBucketResp,
        (rpb_packed_size_fn) rpb_set_bucket_req__get_packed_size,
        (rpb_pack_fn) rpb_set_bucket_req__pack,
        0,
        0
};

/* Reset bucket properties command */
const struct pb_command cmd_reset_bucket_properties = {
        mc_RpbResetBucketReq,
        mc_RpbResetBucketResp,
        (rpb_packed_size_fn) rpb_reset_bucket_req__get_packed_size,
        (rpb_pack_fn) rpb_reset_bucket_req__pack,
        0,
        0
};

/* Get bucket properties command */
const struct pb_command cmd_get_bucket_properties = {
        mc_RpbGetBucketReq,
        mc_RpbGetBucketResp,
        (rpb_packed_size_fn) rpb_get_bucket_req__get_packed_size,
        (rpb_pack_fn) rpb_get_bucket_req__pack,
        (rpb_unpack_fn) rpb_get_bucket_resp__unpack,
        (rpb_free_unpacked_fn) rpb_get_bucket_resp__free_unpacked
};

/* Get bucket type properties command */
const struct pb_command cmd_get_type_properties = {
        mc_RpbGetBucketTypeReq,
        mc_RpbGetBucketResp,
        (rpb_packed_size_fn) rpb_get_bucket_type_req__get_packed_size,
        (rpb_pack_fn) rpb_get_bucket_type_req__pack,
        // Response is same as for get bucket properties
        (rpb_unpack_fn) rpb_get_bucket_resp__unpack,
        (rpb_free_unpacked_fn) rpb_get_bucket_resp__free_unpacked
};

/* Get server info command */
const struct pb_command cmd_get_server_info = {
        mc_RpbGetServerInfoReq,
        mc_RpbGetServerInfoResp,
        0,
        0,
        // Response is same as for get bucket properties
        (rpb_unpack_fn) rpb_get_server_info_resp__unpack,
        (rpb_free_unpacked_fn) rpb_get_server_info_resp__free_unpacked
};

/* Counter get command */
const struct pb_command cmd_index_query = {
        mc_RpbIndexReq,
        mc_RpbIndexResp,
        (rpb_packed_size_fn) rpb_index_req__get_packed_size,
        (rpb_pack_fn) rpb_index_req__pack,
        (rpb_unpack_fn) rpb_index_resp__unpack,
        (rpb_free_unpacked_fn) rpb_index_resp__free_unpacked
};

/* Counter increment command */
const struct pb_command cmd_counter_increment = {
        mc_RpbCounterUpdateReq,
        mc_RpbCounterUpdateResp,
        (rpb_packed_size_fn) rpb_counter_update_req__get_packed_size,
        (rpb_pack_fn) rpb_counter_update_req__pack,
        (rpb_unpack_fn) rpb_counter_update_resp__unpack,
        (rpb_free_unpacked_fn) rpb_counter_update_resp__free_unpacked
};

/* Counter get command */
const struct pb_command cmd_counter_get = {
        mc_RpbCounterGetReq,
        mc_RpbCounterGetResp,
        (rpb_packed_size_fn) rpb_counter_get_req__get_packed_size,
        (rpb_pack_fn) rpb_counter_get_req__pack,
        (rpb_unpack_fn) rpb_counter_get_resp__unpack,
        (rpb_free_unpacked_fn) rpb_counter_get_resp__free_unpacked
};

/* Get command */
const struct pb_command cmd_get = {
        mc_RpbGetReq,
        mc_RpbGetResp,
        (rpb_packed_size_fn) rpb_get_req__get_packed_size,
        (rpb_pack_fn) rpb_get_req__pack,
        (rpb_unpack_fn) rpb_get_resp__unpack,
        (rpb_free_unpacked_fn) rpb_get_resp__free_unpacked
};

/* Put command */
const struct pb_command cmd_put = {
        mc_RpbPutReq,
        mc_RpbPutResp,
        (rpb_packed_size_fn) rpb_put_req__get_packed_size,
        (rpb_pack_fn) rpb_put_req__pack,
        (rpb_unpack_fn) rpb_put_resp__unpack,
        (rpb_free_unpacked_fn) rpb_put_resp__free_unpacked
};

/* Delete command */
const struct pb_command cmd_delete = {
        mc_RpbDelReq,
        mc_RpbDelResp,
        (rpb_packed_size_fn) rpb_del_req__get_packed_size,
        (rpb_pack_fn) rpb_del_req__pack,
        0,
        0
};

/* Set client id command */
const struct pb_command cmd_set_clientid = {
        mc_RpbSetClientIdReq,
        mc_RpbSetClientIdResp,
        (rpb_packed_size_fn) rpb_set_client_id_req__get_packed_size,
        (rpb_pack_fn) rpb_set_client_id_req__pack,
        0,
        0
};

/* Get client command */
const struct pb_command cmd_get_clientid = {
        mc_RpbGetClientIdReq,
        mc_RpbGetClientIdResp,
        0,
        0,
        (rpb_unpack_fn) rpb_get_client_id_resp__unpack,
        (rpb_free_unpacked_fn) rpb_get_client_id_resp__free_unpacked
};

/* Search command */
const struct pb_command cmd_search = {
        mc_RpbSearchQueryReq,
        mc_RbpSearchQueryResp,
        (rpb_packed_size_fn) rpb_search_query_req__get_packed_size,
        (rpb_pack_fn) rpb_search_query_req__pack,
        (rpb_unpack_fn) rpb_search_query_resp__unpack,
        (rpb_free_unpacked_fn) rpb_search_query_resp__free_unpacked
};

/* List buckets command */
const struct pb_command cmd_list_buckets = {
        mc_RpbListBucketsReq,
        mc_RpbListBucketsResp,
        (rpb_packed_size_fn) rpb_list_buckets_req__get_packed_size,
        (rpb_pack_fn) rpb_list_buckets_req__pack,
        (rpb_unpack_fn) rpb_list_buckets_resp__unpack,
        (rpb_free_unpacked_fn) rpb_list_buckets_resp__free_unpacked
};

/* List keys command */
const struct pb_command cmd_list_keys = {
        mc_RpbListKeysReq,
        mc_RpbListKeysResp,
        (rpb_packed_size_fn) rpb_list_keys_req__get_packed_size,
        (rpb_pack_fn) rpb_list_keys_req__pack,
        (rpb_unpack_fn) rpb_list_keys_resp__unpack,
        (rpb_free_unpacked_fn) rpb_list_keys_resp__free_unpacked
};

/* Map/Reduce command */
const struct pb_command cmd_map_reduce = {
        mc_RpbMapRedReq,
        mc_RpbMapRedResp,
        (rpb_packed_size_fn) rpb_map_red_req__get_packed_size,
        (rpb_pack_fn) rpb_map_red_req__pack,
        (rpb_unpack_fn) rpb_map_red_resp__unpack,
        (rpb_free_unpacked_fn) rpb_map_red_resp__free_unpacked
};


int riack_perform_commmand(riack_client *client, const struct pb_command* cmd, const struct rpb_base_req* req,
        cmd_response_cb cb, void** cb_arg)
{
    riack_pb_msg msg_req, *msg_resp;
    ProtobufCAllocator pb_allocator;
    void* resp;
    uint8_t *request_buffer;
    size_t packed_size;
    int retval;
    if (!client) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    retval = RIACK_ERROR_COMMUNICATION;
    pb_allocator = riack_pb_allocator(&client->allocator);
    request_buffer = 0;
    packed_size = 0;

    // Not all commands have data
    if (cmd->packed_size_fn != NULL && cmd->pack_fn != NULL) {
        packed_size = cmd->packed_size_fn(req);
        if (packed_size > 0) {
            request_buffer = (uint8_t *) RMALLOC(client, packed_size);
        }
    }
    if (request_buffer != NULL || packed_size == 0) {
        if (packed_size > 0 && cmd->pack_fn != NULL) {
            // Pack if we have data
            cmd->pack_fn(req, request_buffer);
        }
        msg_req.msg_code = cmd->req_msg_code;
        msg_req.msg_len = (uint32_t) packed_size;
        msg_req.msg = request_buffer;
        if (riack_send_message(client, &msg_req) > 0) {
            int done = 0;
            while (!done && riack_receive_message(client, &msg_resp) > 0) {
                done = 1;
                if (msg_resp->msg_code == cmd->resp_msg_code) {
                    if (cmd->unpack_fn != NULL && cb != NULL) {
                        resp = cmd->unpack_fn(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                        if (resp) {
                            retval = RIACK_SUCCESS;
                            if (cb(client, resp, cb_arg) == RIACK_CMD_CONTINUE) {
                                done = 0;
                            }
                            cmd->free_unpacked_fn(resp, &pb_allocator);
                        } else {
                            retval = RIACK_FAILED_PB_UNPACK;
                        }
                    } else {
                        retval = RIACK_SUCCESS;
                    }
                } else {
                    riack_got_error_response(client, msg_resp);
                    retval = RIACK_ERROR_RESPONSE;
                }
                riack_message_free(client, &msg_resp);
            }
        }
        if (request_buffer != 0) {
            RFREE(client, request_buffer);
        }
    }
    return retval;
}

