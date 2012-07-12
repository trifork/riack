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
#include <stdint.h>

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
RIACK_EXPORT int riack_connect(struct RIACK_CLIENT *client, const char* host, int port);

/// Check if the Riak server is responding
RIACK_EXPORT int riack_ping(struct RIACK_CLIENT *client);

/// List all buckets on the server (should not be used in production)
RIACK_EXPORT int riack_list_buckets(struct RIACK_CLIENT *client, RIACK_STRING_LIST* bucket_list);

RIACK_EXPORT int riack_list_keys(struct RIACK_CLIENT *client, RIACK_STRING bucket, RIACK_STRING_LIST* keys);

/// Set bucket properties
RIACK_EXPORT int riack_set_bucket_props(struct RIACK_CLIENT *client, RIACK_STRING bucket, uint32_t n_val, uint8_t allow_mult);

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
RIACK_EXPORT int riack_map_redue(struct RIACK_CLIENT *client,
								 size_t data_len,
								 uint8_t* data,
								 enum RIACK_MAPRED_CONTENT_TYPE content_type,
								 struct RIACK_MAPRED_RESULT** mapred_result);

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

RIACK_EXPORT void riack_free_object(struct RIACK_CLIENT* client, struct RIACK_OBJECT *pobject);

RIACK_EXPORT void riack_free_get_object(struct RIACK_CLIENT* client, struct RIACK_GET_OBJECT *pobject);

RIACK_EXPORT void riack_free_mapred_result(struct RIACK_CLIENT* client, struct RIACK_MAPRED_RESULT *result);

RIACK_EXPORT void riack_free_string(struct RIACK_CLIENT* client, RIACK_STRING* string);

RIACK_EXPORT void riack_free_string_list(struct RIACK_CLIENT* client, RIACK_STRING_LIST* strings);


#endif /* __RIACK_RIAK_C_H_ */
