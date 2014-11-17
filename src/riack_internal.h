
#ifndef __RIACK_INTERNAL__H__
#define __RIACK_INTERNAL__H__

#include "riack_msg.h"
#include "protobuf-c/protobuf-c.h"
#include "protocol/riak.pb-c.h"

#define FAILED_TO_SET_SOCKET_OPTION_KEEPALIVE "Failed to set keep-alive socket option"
#define FAILED_TO_SET_SOCKET_TIMEOUTS "Failed to timeout options on socket"

#define RIACK_REQ_LOCALS int retval; \
                         RIACK_PB_MSG msg_req, *msg_resp; \
                         uint8_t *request_buffer; \
                         ProtobufCAllocator pb_allocator; \
                         size_t packed_size

/************************************
* riack.c
************************************/
void riack_got_error_response(RIACK_CLIENT *client, RIACK_PB_MSG *msg);
void riack_set_rpb_bucket_props(RIACK_CLIENT *client, RIACK_BUCKET_PROPERTIES* props, RpbBucketProps *rpb_props);
RIACK_BUCKET_PROPERTIES* riack_riack_bucket_props_from_rpb(RIACK_CLIENT *client, RpbBucketProps* rpb_props);
void riack_free_copied_rpb_mod_fun(RIACK_CLIENT *client, RpbModFun* rpb_modfun);
RpbCommitHook** riack_hooks_to_rpb_hooks(RIACK_CLIENT *client, RIACK_COMMIT_HOOK* hooks, size_t hook_count);

/************************************
* riack_mem.c
************************************/
ProtobufCAllocator riack_pb_allocator(RIACK_ALLOCATOR *allocator);

#endif // __RIACK_INTERNAL__H__
