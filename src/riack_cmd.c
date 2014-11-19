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

/* Set bucket type command */
const struct pb_command cmd_set_bucket_type = {
        mc_RpbSetBucketTypeReq,
        mc_RpbSetBucketResp,
        (size_t (*)(void const *)) rpb_set_bucket_type_req__get_packed_size,
        (size_t (*)(void const *, uint8_t *)) rpb_set_bucket_type_req__pack,
        0
};

/* Set bucket type command */
const struct pb_command cmd_set_bucket_properties = {
        mc_RpbSetBucketReq,
        mc_RpbSetBucketResp,
        (size_t (*)(void const *)) rpb_set_bucket_req__get_packed_size,
        (size_t (*)(void const *, uint8_t *)) rpb_set_bucket_req__pack,
        0
};

int riack_perform_commmand(RIACK_CLIENT *client, const struct pb_command* cmd, const struct rpb_base_req* req)
{
    int retval;
    RIACK_PB_MSG msg_req, *msg_resp;
    uint8_t *request_buffer;
    ProtobufCAllocator pb_allocator;
    size_t packed_size;

    if (!client) {
        return RIACK_ERROR_INVALID_INPUT;
    }
    retval = RIACK_ERROR_COMMUNICATION;
    packed_size = cmd->rpb_packed_size_fn(req);
    request_buffer = (uint8_t*)RMALLOC(client, packed_size);
    if (request_buffer) {
        cmd->rpb_pack(req, request_buffer);
        msg_req.msg_code = mc_RpbSetBucketTypeReq;
        msg_req.msg_len = (uint32_t) packed_size;
        msg_req.msg = request_buffer;
        if ((riack_send_message(client, &msg_req) > 0)&&
                (riack_receive_message(client, &msg_resp) > 0))
        {
            if (msg_resp->msg_code == cmd->resp_msg_code) {
                // TODO Callback to unpack
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

