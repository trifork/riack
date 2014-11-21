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

extern riack_allocator riack_default_allocator;

/// Initialise should be called once on startup
RIACK_EXPORT void riack_init();
/// Cleanup should be called once before shutdown
RIACK_EXPORT void riack_cleanup();

/// Create a new RIACK client
RIACK_EXPORT riack_client* riack_new_client(riack_allocator *allocator);

/// Free a RIACK client.
RIACK_EXPORT void riack_free(riack_client *client);

/// Connect to a Riak server
RIACK_EXPORT int riack_connect(riack_client *client, const char* host, int port, riack_connection_options* options);

/// Disconnect from Riak server
RIACK_EXPORT int riack_disconnect(riack_client *client);

/// Reconnect to Riak server,
/// I recommended doing this when receiving an error response or timeout
RIACK_EXPORT int riack_reconnect(riack_client *client);

/// Check if the Riak server is responding
RIACK_EXPORT int riack_ping(riack_client *client);

/*************************************************************************
* List all buckets on the server (should not be used in production)
*************************************************************************/
RIACK_EXPORT int riack_list_buckets_ext(riack_client *client, riack_string* bucket_type,
        riack_string_list** bucket_list, uint32_t timeout);

RIACK_EXPORT int riack_list_buckets(riack_client *client, riack_string_list** bucket_list);


/*************************************************************************
* Key listing (should not be used in production)
*************************************************************************/

RIACK_EXPORT int riack_stream_keys_ext(riack_client *client, riack_string *bucket, riack_string *bucket_type,
        void(*callback)(riack_client*, void*, riack_string), void* callback_arg, uint32_t timeout);

RIACK_EXPORT int riack_stream_keys(riack_client *client, riack_string *bucket,
        void(*callback)(riack_client*, void*, riack_string), void* callback_arg);

RIACK_EXPORT int riack_list_keys(riack_client *client, riack_string *bucket, riack_string_linked_list** keys);


RIACK_EXPORT int riack_list_keys_ext(riack_client *client, riack_string *bucket, riack_string *bucket_type,
        riack_string_linked_list** keys, uint32_t timeout);

/*************************************************************************
* Bucket properties
*************************************************************************/

RIACK_EXPORT int riack_set_bucket_props_ext(riack_client *client, riack_string *bucket, riack_string* bucket_type,
        riack_bucket_properties* properties);

RIACK_EXPORT int riack_set_bucket_props(riack_client *client, riack_string *bucket,
        riack_bucket_properties* properties);

RIACK_EXPORT int riack_get_bucket_props_ext(riack_client *client, riack_string *bucket, riack_string* bucket_type,
        riack_bucket_properties** properties);

RIACK_EXPORT int riack_get_bucket_props(riack_client *client, riack_string *bucket, riack_bucket_properties** properties);

RIACK_EXPORT int riack_reset_bucket_props(riack_client *client, riack_string *bucket);

RIACK_EXPORT int riack_set_bucket_type_props(riack_client *client, riack_string *bucket_type,
        riack_bucket_properties* properties);
RIACK_EXPORT int riack_get_bucket_type_props(riack_client *client, riack_string* bucket_type,
        riack_bucket_properties** properties);

/*************************************************************************
* Get
*************************************************************************/
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_get(riack_client *client, riack_string *bucket, riack_string *key, riack_get_properties* props,
        riack_get_object** result_object);

/*************************************************************************
* Put
*************************************************************************/
/// Do a put to the server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_put(riack_client *client, riack_object *object, riack_object** returned_object,
        riack_put_properties* props);

/// Do a put with simplified parameters
///  Note this function is relying on strlen which might be unsafe
RIACK_EXPORT int riack_put_simple(riack_client *client, char* bucket, char* key, uint8_t* data,
        size_t datalen, char* content_type);

/*************************************************************************
* 2I
*************************************************************************/

typedef void(*index_query_cb_fn)(riack_client*, void*, riack_string *key);

RIACK_EXPORT int riack_2i_query_exact(riack_client *client, riack_string *bucket, riack_string *index,
        riack_string *search_key, riack_string_list **result_keys);

RIACK_EXPORT int riack_2i_query_range(riack_client *client, riack_string *bucket, riack_string *index,
        riack_string *search_key_min, riack_string *search_key_max, riack_string_list **result_keys);

RIACK_EXPORT int riack_2i_query_ext(riack_client *client, riack_2i_query_req *req, riack_string_list **result_keys,
        riack_string **continuation_token_out);

RIACK_EXPORT int riack_2i_query_stream_ext(riack_client *client, riack_2i_query_req *req,
        riack_string **continuation_token_out, index_query_cb_fn callback, void *callback_arg);

/*************************************************************************
* Datatypes
*************************************************************************/

/// Get the value of a CRDT counter, requires riak 1.4+
RIACK_EXPORT int riack_counter_get(riack_client *client, riack_string *bucket, riack_string *key,
        riack_counter_get_properties *props, int64_t *result);

/// Increment a CRDT counter, requires riak 1.4+
/// if returned_value is parsed along the updated value will be returned.
RIACK_EXPORT int riack_counter_increment(riack_client *client, riack_string *bucket, riack_string *key,
        int64_t amount, riack_counter_update_properties *props, int64_t *returned_value);

/*************************************************************************
* Misc
*************************************************************************/


/// Retrive server info
RIACK_EXPORT int riack_server_info(riack_client *client, riack_string **node, riack_string** version);

/// Set the client id
RIACK_EXPORT int riack_set_clientid(riack_client *client, riack_string *clientid);

/// Get client id from server
RIACK_EXPORT int riack_get_clientid(riack_client *client, riack_string **clientid);


/// Delete an object from server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_delete(riack_client *client, riack_string *bucket, riack_string *key,
        riack_del_properties *props);

/// Run a map reduce query on server
RIACK_EXPORT int riack_map_reduce(riack_client *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type, riack_mapred_response_list** mapred_result);

/// Run a map reduce query on server, return the every result separately through the callback
RIACK_EXPORT int riack_map_reduce_stream(riack_client *client, size_t data_len, uint8_t* data,
        enum RIACK_MAPRED_CONTENT_TYPE content_type,
        void(*callback)(riack_client*, void*, riack_mapred_response*), void* callback_arg);


RIACK_EXPORT int riack_search(riack_client *client, riack_string *query, riack_string *index,
        riack_search_optional_params* optional_parameters, riack_search_result** search_result);

/*************************************************************************
* Memory
*************************************************************************/

RIACK_EXPORT riack_string riack_copy_from_cstring(riack_client* client, const char* source);

RIACK_EXPORT riack_string riack_copy_string(riack_client* client, riack_string source);

RIACK_EXPORT riack_string* riack_string_alloc(riack_client* client);

// String list
RIACK_EXPORT riack_string_list* riack_string_list_alloc(riack_client* client);
RIACK_EXPORT void riack_free_string_list_p(riack_client *client, riack_string_list **strings);

RIACK_EXPORT riack_object* riack_object_alloc(riack_client* client);
RIACK_EXPORT void riack_free_object_p(riack_client *client, riack_object **object);
RIACK_EXPORT void riack_free_object(riack_client *client, riack_object *object);

RIACK_EXPORT riack_get_object* riack_get_object_alloc(riack_client* client);
RIACK_EXPORT void riack_free_get_object_p(riack_client *client, riack_get_object **object);

RIACK_EXPORT void riack_free_mapred_result(riack_client* client, riack_mapred_response_list *result);

RIACK_EXPORT void riack_free_search_result_p(riack_client* client, riack_search_result** search_result);
RIACK_EXPORT riack_search_result* riack_search_result_alloc(riack_client* client);

RIACK_EXPORT void riack_free_string(riack_client* client, riack_string* string);
RIACK_EXPORT void riack_free_string_p(riack_client* client, riack_string** string);

RIACK_EXPORT void riack_free_string_linked_list_p(riack_client *client, riack_string_linked_list **strings);

RIACK_EXPORT void riack_free_bucket_properties_p(riack_client *client, riack_bucket_properties **properties);

/// For testing purpose make a recv without sending anything
RIACK_EXPORT void riack_timeout_test(riack_client* client);

#endif /* __RIACK_RIAK_C_H_ */
