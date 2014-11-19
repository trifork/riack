/*
   Copyright 2012 Trifork A/S
   Author: Kaspar Pedersen

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

#ifndef __RIACK_RIAK_C_H_
#define __RIACK_RIAK_C_H_

#include "riack_defines.h"

extern RIACK_ALLOCATOR riack_default_allocator;

/// Initialise should be called once on startup
RIACK_EXPORT void riack_init();
/// Cleanup should be called once before shutdown
RIACK_EXPORT void riack_cleanup();

/// Create a new RIACK client
RIACK_EXPORT RIACK_CLIENT* riack_new_client(RIACK_ALLOCATOR *allocator);

/// Free a RIACK client.
RIACK_EXPORT void riack_free(RIACK_CLIENT *client);

/// Connect to a Riak server
RIACK_EXPORT int riack_connect(RIACK_CLIENT *client, const char* host, int port, RIACK_CONNECTION_OPTIONS* options);

/// Disconnect from Riak server
RIACK_EXPORT int riack_disconnect(RIACK_CLIENT *client);

/// Reconnect to Riak server,
/// I recommended doing this when receiving an error response or timeout
RIACK_EXPORT int riack_reconnect(RIACK_CLIENT *client);

/// Check if the Riak server is responding
RIACK_EXPORT int riack_ping(RIACK_CLIENT *client);

/*************************************************************************
* List all buckets on the server (should not be used in production)
*************************************************************************/
RIACK_EXPORT int riack_list_buckets_ext(RIACK_CLIENT *client, RIACK_STRING* bucket_type,
        RIACK_STRING_LIST** bucket_list, uint32_t timeout);

RIACK_EXPORT int riack_list_buckets(RIACK_CLIENT *client, RIACK_STRING_LIST** bucket_list);


/*************************************************************************
* Key listing (should not be used in production)
*************************************************************************/

RIACK_EXPORT int riack_stream_keys_ext(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *bucket_type,
        void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING), void* callback_arg, uint32_t timeout);

RIACK_EXPORT int riack_stream_keys(RIACK_CLIENT *client, RIACK_STRING *bucket,
        void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING), void* callback_arg);

RIACK_EXPORT int riack_list_keys(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING_LINKED_LIST** keys);


RIACK_EXPORT int riack_list_keys_ext(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *bucket_type,
        RIACK_STRING_LINKED_LIST** keys, uint32_t timeout);

/*************************************************************************
* Bucket properties
*************************************************************************/

RIACK_EXPORT int riack_set_bucket_props_ext(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING* bucket_type,
        RIACK_BUCKET_PROPERTIES* properties);

RIACK_EXPORT int riack_set_bucket_props(RIACK_CLIENT *client, RIACK_STRING *bucket,
        RIACK_BUCKET_PROPERTIES* properties);

RIACK_EXPORT int riack_get_bucket_props_ext(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING* bucket_type,
        RIACK_BUCKET_PROPERTIES** properties);

RIACK_EXPORT int riack_get_bucket_props(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_BUCKET_PROPERTIES** properties);

RIACK_EXPORT int riack_reset_bucket_props(RIACK_CLIENT *client, RIACK_STRING *bucket);

RIACK_EXPORT int riack_set_bucket_type_props(RIACK_CLIENT *client, RIACK_STRING *bucket_type,
        RIACK_BUCKET_PROPERTIES* properties);
RIACK_EXPORT int riack_get_bucket_type_props(RIACK_CLIENT *client, RIACK_STRING* bucket_type,
        RIACK_BUCKET_PROPERTIES** properties);

/*************************************************************************
* Get
*************************************************************************/
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_get(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key, RIACK_GET_PROPERTIES* props,
        RIACK_GET_OBJECT** result_object);

/*************************************************************************
* Put
*************************************************************************/
/// Do a put to the server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_put(RIACK_CLIENT *client, RIACK_OBJECT *object, RIACK_OBJECT** returned_object,
        RIACK_PUT_PROPERTIES* props);

/// Do a put with simplified parameters
///  Note this function is relying on strlen which might be unsafe
RIACK_EXPORT int riack_put_simple(RIACK_CLIENT *client, char* bucket, char* key, uint8_t* data,
        size_t datalen, char* content_type);

/*************************************************************************
* 2I
*************************************************************************/

RIACK_EXPORT int riack_2i_query_exact(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *index,
        RIACK_STRING *search_key, RIACK_STRING_LIST **result_keys);

RIACK_EXPORT int riack_2i_query_range(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *index,
        RIACK_STRING *search_key_min, RIACK_STRING *search_key_max, RIACK_STRING_LIST **result_keys);

RIACK_EXPORT int riack_2i_query_ext(RIACK_CLIENT *client, RIACK_2I_QUERY_REQ *req, RIACK_STRING_LIST **result_keys,
        RIACK_STRING *continuation_token_out);

RIACK_EXPORT int riack_2i_query_stream_ext(RIACK_CLIENT *client, RIACK_2I_QUERY_REQ *req,
        RIACK_STRING *continuation_token_out, void(*callback)(RIACK_CLIENT*, void*, RIACK_STRING *key),
        void *callback_arg);

/*************************************************************************
* Datatypes
*************************************************************************/

/// Get the value of a CRDT counter, requires riak 1.4+
RIACK_EXPORT int riack_counter_get(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key,
        RIACK_COUNTER_GET_PROPERTIES *props, int64_t *result);

/// Increment a CRDT counter, requires riak 1.4+
/// if returned_value is parsed along the updated value will be returned.
RIACK_EXPORT int riack_counter_increment(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key,
        int64_t amount, RIACK_COUNTER_UPDATE_PROPERTIES *props, int64_t *returned_value);

/*************************************************************************
* Misc
*************************************************************************/


/// Retrive server info
RIACK_EXPORT int riack_server_info(RIACK_CLIENT *client, RIACK_STRING **node, RIACK_STRING** version);

/// Set the client id
RIACK_EXPORT int riack_set_clientid(RIACK_CLIENT *client, RIACK_STRING *clientid);

/// Get client id from server
RIACK_EXPORT int riack_get_clientid(RIACK_CLIENT *client, RIACK_STRING **clientid);


/// Delete an object from server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_delete(RIACK_CLIENT *client, RIACK_STRING *bucket, RIACK_STRING *key,
        RIACK_DEL_PROPERTIES *props);

/// Run a map reduce query on server
RIACK_EXPORT int riack_map_reduce(RIACK_CLIENT *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type, RIACK_MAPRED_RESPONSE_LIST** mapred_result);

/// Run a map reduce query on server, return the every result separately through the callback
RIACK_EXPORT int riack_map_reduce_stream(RIACK_CLIENT *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type,
        void(*callback)(RIACK_CLIENT*, void*, RIACK_MAPRED_RESPONSE*), void* callback_arg);


RIACK_EXPORT int riack_search(RIACK_CLIENT *client, RIACK_STRING *query, RIACK_STRING *index,
        RIACK_SEARCH_OPTIONAL_PARAMETERS* optional_parameters, RIACK_SEARCH_RESULT** search_result);

/*************************************************************************
* Memory
*************************************************************************/

RIACK_EXPORT RIACK_STRING riack_copy_from_cstring(RIACK_CLIENT* client, const char* source);

RIACK_EXPORT RIACK_STRING riack_copy_string(RIACK_CLIENT* client, RIACK_STRING source);

RIACK_EXPORT RIACK_STRING* riack_string_alloc(RIACK_CLIENT* client);

// String list
RIACK_EXPORT RIACK_STRING_LIST* riack_string_list_alloc(RIACK_CLIENT* client);
RIACK_EXPORT void riack_free_string_list_p(RIACK_CLIENT *client, RIACK_STRING_LIST **strings);

RIACK_EXPORT RIACK_OBJECT* riack_object_alloc(RIACK_CLIENT* client);
RIACK_EXPORT void riack_free_object_p(RIACK_CLIENT *client, RIACK_OBJECT **object);
RIACK_EXPORT void riack_free_object(RIACK_CLIENT *client, RIACK_OBJECT *object);

RIACK_EXPORT RIACK_GET_OBJECT* riack_get_object_alloc(RIACK_CLIENT* client);
RIACK_EXPORT void riack_free_get_object_p(RIACK_CLIENT *client, RIACK_GET_OBJECT **object);

RIACK_EXPORT void riack_free_mapred_result(RIACK_CLIENT* client, RIACK_MAPRED_RESPONSE_LIST *result);

RIACK_EXPORT void riack_free_search_result_p(RIACK_CLIENT* client, RIACK_SEARCH_RESULT** search_result);
RIACK_EXPORT RIACK_SEARCH_RESULT* riack_search_result_alloc(RIACK_CLIENT* client);

RIACK_EXPORT void riack_free_string(RIACK_CLIENT* client, RIACK_STRING* string);
RIACK_EXPORT void riack_free_string_p(RIACK_CLIENT* client, RIACK_STRING** string);

RIACK_EXPORT void riack_free_string_linked_list_p(RIACK_CLIENT *client, RIACK_STRING_LINKED_LIST **strings);

RIACK_EXPORT void riack_free_bucket_properties_p(RIACK_CLIENT *client, RIACK_BUCKET_PROPERTIES **properties);

/// For testing purpose make a recv without sending anything
RIACK_EXPORT void riack_timeout_test(RIACK_CLIENT* client);

#endif /* __RIACK_RIAK_C_H_ */
