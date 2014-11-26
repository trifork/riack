
#ifndef __RIACK_INTERNAL__H__
#define __RIACK_INTERNAL__H__

#include "protobuf-c/protobuf-c.h"
#include "protocol/riak_kv.pb-c.h"
#include "protocol/riak.pb-c.h"
#include "protocol/riak_search.pb-c.h"
#include "riack_msg.h"
#include "riack.h"

#define FAILED_TO_SET_SOCKET_OPTION_KEEPALIVE "Failed to set keep-alive socket option"
#define FAILED_TO_SET_SOCKET_TIMEOUTS "Failed to timeout options on socket"

#define RIACK_REQ_LOCALS int retval; \
                         riack_pb_msg msg_req, *msg_resp; \
                         uint8_t *request_buffer; \
                         ProtobufCAllocator pb_allocator; \
                         size_t packed_size

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

typedef riack_cmd_cb_result (*cmd_response_cb)(riack_client*, struct rpb_base_resp*, void**);

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
    riack_string *node;
    riack_string *server_version;
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
extern const struct pb_command cmd_delete;
extern const struct pb_command cmd_set_clientid;
extern const struct pb_command cmd_get_clientid;
extern const struct pb_command cmd_search;
extern const struct pb_command cmd_list_buckets;
extern const struct pb_command cmd_list_keys;
extern const struct pb_command cmd_map_reduce;

int riack_perform_commmand(riack_client *client, const struct pb_command* cmd, const struct rpb_base_req* req,
        cmd_response_cb cb, void** cb_arg);

/************************************
* riack.c
************************************/
void riack_got_error_response(riack_client *client, riack_pb_msg *msg);

/************************************
* riack_kv.c
************************************/
typedef struct {
    list_keys_stream_cb callback;
    void* user_cb_arg;
} riack_stream_cb_params;

/************************************
* riack_mapreduce.c
************************************/
typedef struct {
    map_reduce_stream_cb callback;
    void* user_cb_arg;
} riack_mapreduce_cb_params;


/************************************
* riack_2i.c
************************************/
int riack_perform_2i_query(riack_client *client, RpbIndexReq* request, riack_string_list** result_keys,
        riack_string** continuation_token, index_query_cb_fn callback, void *callback_arg);

/************************************
* riack_mem.c
************************************/
ProtobufCAllocator riack_pb_allocator(riack_allocator *allocator);

#endif // __RIACK_INTERNAL__H__
