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
#include "riack_msg.h"
#include "riack_helpers.h"
#include "riack_sock.h"
#include "protocol/riak_msg_codes.h"


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
        (size_t (*)(void const *)) rpb_set_bucket_type_req__get_packed_size,
        (size_t (*)(void const *, uint8_t *)) rpb_set_bucket_type_req__pack,
        0,
        0
};

/* Set bucket type command */
const struct pb_command cmd_set_bucket_properties = {
        mc_RpbSetBucketReq,
        mc_RpbSetBucketResp,
        (size_t (*)(void const *)) rpb_set_bucket_req__get_packed_size,
        (size_t (*)(void const *, uint8_t *)) rpb_set_bucket_req__pack,
        0,
        0
};

/* Reset bucket properties command */
const struct pb_command cmd_reset_bucket_properties = {
        mc_RpbResetBucketReq,
        mc_RpbResetBucketResp,
        (size_t (*)(void const *)) rpb_reset_bucket_req__get_packed_size,
        (size_t (*)(void const *, uint8_t *)) rpb_reset_bucket_req__pack,
        0,
        0
};

/* Get bucket properties command */
const struct pb_command cmd_get_bucket_props = {
        mc_RpbGetBucketReq,
        mc_RpbGetBucketResp,
        (size_t (*)(void const *)) rpb_get_bucket_req__get_packed_size,
        (size_t (*)(void const *, uint8_t *)) rpb_get_bucket_req__pack,
        (struct rpb_base_resp *(*)(ProtobufCAllocator *, size_t, uint8_t const *)) rpb_get_bucket_resp__unpack,
        (void (*)(struct rpb_base_resp *, ProtobufCAllocator *)) rpb_get_bucket_resp__free_unpacked
// TODO Find some way of hiding all these long function pointer casts
};


int riack_perform_commmand(RIACK_CLIENT *client, const struct pb_command* cmd, const struct rpb_base_req* req,
        int (*response_cb)(RIACK_CLIENT*, struct rpb_base_resp*))
{
    RIACK_PB_MSG msg_req, *msg_resp;
    ProtobufCAllocator pb_allocator;
    struct rpb_base_resp* resp;
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
    if (cmd->rpb_packed_size_fn != 0 && cmd->rpb_pack != 0) {
        packed_size = cmd->rpb_packed_size_fn(req);
        request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    }
    if (request_buffer != 0 || cmd->rpb_packed_size_fn == 0) {
        if (cmd->rpb_packed_size_fn != 0 && cmd->rpb_pack != 0) {
            // Pack if we have data
            cmd->rpb_pack(req, request_buffer);
        }
        msg_req.msg_code = cmd->req_msg_code;
        msg_req.msg_len = (uint32_t) packed_size;
        msg_req.msg = request_buffer;
        if ((riack_send_message(client, &msg_req) > 0)&&
                (riack_receive_message(client, &msg_resp) > 0)) {
            if (msg_resp->msg_code == cmd->resp_msg_code) {
                if (cmd->rpb_unpack != 0 && response_cb != 0) {
                    resp = cmd->rpb_unpack(&pb_allocator, msg_resp->msg_len, msg_resp->msg);
                    if (resp) {
                        retval = response_cb(client, resp);
                        cmd->rpb_free_unpacked(resp, &pb_allocator);
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
        if (request_buffer != 0) {
            RFREE(client, request_buffer);
        }
    }
    return retval;
}

