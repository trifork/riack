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

extern struct RIACK_ALLOCATOR riack_default_allocator;

/// Initialise should be called once on startup
RIACK_EXPORT void riack_init();
/// Cleanup should be called once before shutdown
RIACK_EXPORT void riack_cleanup();

/// Create a new RIACK client
RIACK_EXPORT struct RIACK_CLIENT* riack_new_client(struct RIACK_ALLOCATOR *allocator);

/// Free a RIACK client.
RIACK_EXPORT void riack_free(struct RIACK_CLIENT *client);

/// Connect to a Riak server
RIACK_EXPORT int riack_connect(struct RIACK_CLIENT *client, const char* host, int port,
		struct RIACK_CONNECTION_OPTIONS* options);

/// Disconnect from Riak server
RIACK_EXPORT int riack_disconnect(struct RIACK_CLIENT *client);

/// Reconnect to Riak server,
/// I recommended doing this when receiving an error response or timeout
RIACK_EXPORT int riack_reconnect(struct RIACK_CLIENT *client);

/// Check if the Riak server is responding
RIACK_EXPORT int riack_ping(struct RIACK_CLIENT *client);

////////////////////////////////////////////////////////////////////////
// Riak 2.0+ only

/// List all buckets on the server (should not be used in production)
RIACK_EXPORT int riack_list_buckets_ext(struct RIACK_CLIENT *client, struct RIACK_BUCKET_TYPE_OPTIONAL bucket_type,
        RIACK_STRING_LIST* bucket_list, uint32_t timeout);

RIACK_EXPORT int riack_list_keys_ext(struct RIACK_CLIENT *client,
        RIACK_STRING bucket,
        struct RIACK_BUCKET_TYPE_OPTIONAL bucket_type,
        struct RIACK_STRING_LINKED_LIST** keys, uint32_t timeout);

/// Return all keys through the callback.
/// This function shall be used instead of riack_list_keys(..) if your Bucket contains considerably large amount of keys.
RIACK_EXPORT int riack_stream_keys_ext(struct RIACK_CLIENT *client, RIACK_STRING bucket,
        struct RIACK_BUCKET_TYPE_OPTIONAL bucket_type,
        void(*callback)(struct RIACK_CLIENT*, void*, RIACK_STRING),
        void* callback_arg,
        uint32_t timeout);

////////////////////////////////////////////////////////////////////////
// Riak 1.4+ only

/// Set extended bucket properties Riak 1.4+ required
RIACK_EXPORT int riack_set_bucket_props_ext(struct RIACK_CLIENT *client,
        RIACK_STRING bucket, struct RIACK_BUCKET_PROPERTIES* properties);

RIACK_EXPORT int riack_get_bucket_props_ext(struct RIACK_CLIENT *client,
        RIACK_STRING bucket, struct RIACK_BUCKET_PROPERTIES** properties);

/// Reset bucket properties to default Riak 1.4+ required
RIACK_EXPORT int riack_reset_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket);

/// Get the value of a CRDT counter, requires riak 1.4+
RIACK_EXPORT int riack_counter_get(struct RIACK_CLIENT *client,
        RIACK_STRING bucket,
        RIACK_STRING key,
        struct RIACK_COUNTER_GET_PROPERTIES *props,
        int64_t *result);

/// Increment a CRDT counter, requires riak 1.4+
/// if returned_value is parsed along the updated value will be returned.
RIACK_EXPORT int riack_counter_increment(struct RIACK_CLIENT *client,
        RIACK_STRING bucket,
        RIACK_STRING key,
        int64_t amount,
        struct RIACK_COUNTER_UPDATE_PROPERTIES *props,
        int64_t *returned_value);


////////////////////////////////////////////////////////////////////////
// Riak 1.2

/// List all buckets on the server (should not be used in production)
RIACK_EXPORT int riack_list_buckets(struct RIACK_CLIENT *client, RIACK_STRING_LIST* bucket_list);

RIACK_EXPORT int riack_list_keys(struct RIACK_CLIENT *client, RIACK_STRING bucket, struct RIACK_STRING_LINKED_LIST** keys);

/// Return all keys through the callback.
/// This function shall be used instead of riack_list_keys(..) if your Bucket contains considerably large amount of keys.
RIACK_EXPORT int riack_stream_keys(struct RIACK_CLIENT *client, RIACK_STRING bucket,
								   void(*callback)(struct RIACK_CLIENT*, void*, RIACK_STRING), void* callback_arg);

/// Set bucket properties
RIACK_EXPORT int riack_set_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t n_val, uint8_t allow_mult);

/// Get bucket properties
/// Note if the server chooses not to respond with n_val or allow_mult it will not be set
RIACK_EXPORT int riack_get_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t *n_val, uint8_t *allow_mult);

/// Retrive server info
RIACK_EXPORT int riack_server_info(struct RIACK_CLIENT *client, RIACK_STRING *node, RIACK_STRING* version);

/// Set the client id
RIACK_EXPORT int riack_set_clientid(struct RIACK_CLIENT *client, RIACK_STRING clientid);

/// Get client id from server
RIACK_EXPORT int riack_get_clientid(struct RIACK_CLIENT *client, RIACK_STRING *clientid);

/// Get an object from server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_get(struct RIACK_CLIENT *client,
			  RIACK_STRING bucket,
			  RIACK_STRING key,
			  struct RIACK_GET_PROPERTIES* props,
			  struct RIACK_GET_OBJECT* result_object);

/// Delete an object from server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_delete(struct RIACK_CLIENT *client,
				RIACK_STRING bucket,
				RIACK_STRING key,
				struct RIACK_DEL_PROPERTIES *props);

/// Run a map reduce query on server
RIACK_EXPORT int riack_map_reduce(struct RIACK_CLIENT *client,
								 size_t data_len,
								 uint8_t* data,
								 enum RIACK_MAPRED_CONTENT_TYPE content_type,
                                 struct RIACK_MAPRED_RESPONSE_LIST** mapred_result);

/// Run a map reduce query on server, return the every result separately through the callback
RIACK_EXPORT int riack_map_reduce_stream(struct RIACK_CLIENT *client,
										 size_t data_len,
										 uint8_t* data,
										 enum RIACK_MAPRED_CONTENT_TYPE content_type,
                                         void(*callback)(struct RIACK_CLIENT*, void*, struct RIACK_MAPRED_RESPONSE*),
										 void* callback_arg);

/// Do a put to the server
/// props are optional and can be NULL in which case defaults will be used.
RIACK_EXPORT int riack_put(struct RIACK_CLIENT *client,
			  struct RIACK_OBJECT object,
			  struct RIACK_OBJECT* ppreturned_object,
			  struct RIACK_PUT_PROPERTIES* props);

/// Do a put with simplified parameters
///  Note this function is relying on strlen which might be unsafe
RIACK_EXPORT int riack_put_simple(struct RIACK_CLIENT *client,
								   char* bucket,
								   char* key,
								   uint8_t* data,
								   size_t datalen,
								   char* content_type);

RIACK_EXPORT int riack_2i_query_exact(struct RIACK_CLIENT *client,
								   	  RIACK_STRING bucket,
								   	  RIACK_STRING index,
								   	  RIACK_STRING search_key,
                                      RIACK_STRING_LIST *result_keys);

RIACK_EXPORT int riack_2i_query_range(struct RIACK_CLIENT *client,
								   	  RIACK_STRING bucket,
								   	  RIACK_STRING index,
								   	  RIACK_STRING search_key_min,
                                      RIACK_STRING search_key_max,
								   	  RIACK_STRING_LIST *result_keys);

RIACK_EXPORT int riack_2i_query_ext(struct RIACK_CLIENT *client,
                                    struct RIACK_2I_QUERY_REQ *req,
                                    RIACK_STRING_LIST *result_keys,
                                    RIACK_STRING *continuation_token_out);

RIACK_EXPORT int riack_2i_query_stream_ext(struct RIACK_CLIENT *client,
                                           struct RIACK_2I_QUERY_REQ *req,
                                           RIACK_STRING *continuation_token_out,
                                           void(*callback)(struct RIACK_CLIENT*, void*, RIACK_STRING *key),
                                           void *callback_arg);

RIACK_EXPORT int riack_search(struct RIACK_CLIENT *client,
                              RIACK_STRING query,
                              RIACK_STRING index,
                              struct RIACK_SEARCH_OPTIONAL_PARAMETERS* optional_parameters,
                              struct RIACK_SEARCH_RESULT* search_result);

RIACK_EXPORT RIACK_STRING riack_copy_from_cstring(struct RIACK_CLIENT* client, const char* source);

RIACK_EXPORT RIACK_STRING riack_copy_string(struct RIACK_CLIENT* client, RIACK_STRING source);

RIACK_EXPORT void riack_free_object(struct RIACK_CLIENT* client, struct RIACK_OBJECT *pobject);

RIACK_EXPORT void riack_free_get_object(struct RIACK_CLIENT* client, struct RIACK_GET_OBJECT *pobject);

RIACK_EXPORT void riack_free_mapred_result(struct RIACK_CLIENT* client, struct RIACK_MAPRED_RESPONSE_LIST *result);

RIACK_EXPORT void riack_free_search_result(struct RIACK_CLIENT* client, struct RIACK_SEARCH_RESULT* search_result);

RIACK_EXPORT void riack_free_string(struct RIACK_CLIENT* client, RIACK_STRING* string);

RIACK_EXPORT void riack_free_string_list(struct RIACK_CLIENT* client, RIACK_STRING_LIST* strings);

RIACK_EXPORT void riack_free_string_linked_list(struct RIACK_CLIENT* client, struct RIACK_STRING_LINKED_LIST** strings);

RIACK_EXPORT void riack_free_bucket_properties(struct RIACK_CLIENT *client, struct RIACK_BUCKET_PROPERTIES** properties);

/// For testing purpose make a recv without sending anything
RIACK_EXPORT void riack_timeout_test(struct RIACK_CLIENT* client);

#endif /* __RIACK_RIAK_C_H_ */
