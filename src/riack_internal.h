
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

//#define RIACK_CMD_DONE 0
//#define RIACK_CMD_REC_MORE 1

struct rpb_base_req {
    ProtobufCMessage base;
};

struct rpb_base_resp {
    ProtobufCMessage base;
};

typedef enum {
    RIACK_CMD_DONE = 1,
    RIACK_CMD_CONTINUE
} riack_cmd_cb_result;

typedef riack_cmd_cb_result (*cmd_response_cb)(RIACK_CLIENT*, struct rpb_base_resp*, void**);

typedef size_t (*rpb_packed_size_fn)(const void *);
typedef size_t (*rpb_pack_fn)(const void *, uint8_t*);
typedef struct rpb_base_resp* (*rpb_unpack_fn)(ProtobufCAllocator*, size_t, const uint8_t*);

typedef void (*rpb_free_unpacked_fn)(struct rpb_base_resp*, ProtobufCAllocator*);

struct pb_command {
    uint8_t req_msg_code;
    uint8_t resp_msg_code;
    rpb_packed_size_fn packed_size_fn;
    rpb_pack_fn pack_fn;
    rpb_unpack_fn unpack_fn;
    rpb_free_unpacked_fn free_unpacked_fn;
};

struct riack_server_info {
    RIACK_STRING *node;
    RIACK_STRING *server_version;
};

/************************************
* riack_cmd.c
************************************/
extern const struct pb_command cmd_ping;
extern const struct pb_command cmd_set_bucket_type;
extern const struct pb_command cmd_set_bucket_properties;
extern const struct pb_command cmd_reset_bucket_properties;
extern const struct pb_command cmd_get_bucket_properties;
extern const struct pb_command cmd_get_type_properties;
extern const struct pb_command cmd_get_server_info;
extern const struct pb_command cmd_counter_increment;
extern const struct pb_command cmd_counter_get;
extern const struct pb_command cmd_index_query;
extern const struct pb_command cmd_get;
extern const struct pb_command cmd_put;
extern const struct pb_command cmd_set_clientid;
extern const struct pb_command cmd_get_clientid;

int riack_perform_commmand(RIACK_CLIENT *client, const struct pb_command* cmd, const struct rpb_base_req* req,
        cmd_response_cb cb, void** cb_arg);

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
