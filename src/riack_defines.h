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
#ifndef __RIACK__DEFINES__H__
#define __RIACK__DEFINES__H__

#include "ints.h"
// #include "riack_compat.h"
#include <stdlib.h>
#include <stddef.h>

#if defined(RIACK_SHARED) && defined(_WIN32)
  #if defined(riack_EXPORTS)
    #define  RIACK_EXPORT __declspec(dllexport)
  #else
    #define  RIACK_EXPORT __declspec(dllimport)
  #endif
#else /* defined (_WIN32) */
 #define RIACK_EXPORT
#endif

/* Success */
#define RIACK_SUCCESS 1
/* Communication failed, socket closed etc. */
#define RIACK_ERROR_COMMUNICATION -1
/* Riak returned an unexpected response */
#define RIACK_ERROR_RESPONSE -2
/* Bad argument or invalid arguments */
#define RIACK_ERROR_INVALID_INPUT -3
/* Failed to unpack an pb message (most likely low memory conditions) */
#define RIACK_FAILED_PB_UNPACK -4

/* Allacate memory using client allocator */
#define RMALLOC(client, size) client->allocator.alloc(0, size)
/* Allacate memory and fill it with zeroes, using client allocator */
#define RCALLOC(client, size) memset(client->allocator.alloc(0, size), 0, size)
/* Free allocated memory */
#define RFREE(client, pointer) if (pointer) { client->allocator.free(0, pointer); }
/* Allocate and copy memory */
#define RMALLOCCOPY(client, target, target_len, source, len) target = (void*)RMALLOC(client, len); memcpy(target, source, len); target_len=len

#define RSTR_HAS_CONTENT(s) (s.len > 0 && s.value)
#define RSTR_HAS_CONTENT_P(s) (s && s->len > 0 && s->value)

#define RSTR_SAFE_FREE(client, str) if (RSTR_HAS_CONTENT(str)) { RFREE(client, str.value); str.len = 0; str.value=0; }

#define RSTR_COPY(client, target, source) target.len = source.len; \
                                          if (source.len > 0) {  \
                                              target.value = RMALLOC(client, source.len); \
                                              memcpy(target.value, source.value, source.len); \
                                          } else { \
                                              target.value = 0; }

typedef struct _riack_allocator
{
	void *(*alloc)(void *optional_data, size_t size);
	void (*free)(void *optional_data, void *pointer);
	void *allocator_optional_data;
} riack_allocator;

/* Socket connection options */
typedef struct _riack_connection_options {
	uint32_t recv_timeout_ms;
	uint32_t send_timeout_ms;
    uint8_t keep_alive_enabled;
} riack_connection_options;

/* Riack's base string type */
typedef struct _riack_string {
	char* value;
	size_t len;
} riack_string;

/* List of strings */
typedef struct _riack_string_list {
    riack_string* strings;
	size_t string_count;
} riack_string_list;

/* Linked list of strings */
typedef struct _riack_string_linked_list {
    riack_string string;
	struct _riack_string_linked_list* next;
} riack_string_linked_list;

/* Riack client */
typedef struct _riack_client {
    /* Socket handle */
	int sockfd;
    /* Last error text zero terminated */
	char* last_error;
    /* Last error code */
	uint32_t last_error_code;
    /* Riak host, zero terminated */
	char* host;
    /* Riak port number (protocol buffers port) */
	int port;
    /* Connection options */
    riack_connection_options options;

    /* Allocator to use with this client */
	riack_allocator allocator;
} riack_client;

/* Link to an object */
typedef struct _riack_link
{
	riack_string bucket;
    riack_string key;
    /* Link tag */
    riack_string tag;
} riack_link;

/* key/value pair */
typedef struct _riack_pair
{
    riack_string key;
	uint8_t  value_present;
	size_t   value_len;
	uint8_t* value;
} riack_pair;

typedef struct _riack_content
{
	size_t data_len;
	uint8_t *data;
    riack_string content_type;
    riack_string charset;
    riack_string content_encoding;
    riack_string vtag;
	size_t link_count;
	riack_link* links;
	//
	uint8_t last_modified_present;
	uint32_t last_modified;
	uint8_t last_modified_usecs_present;
	uint32_t last_modified_usecs;
	uint8_t deleted_present;
	uint8_t deleted;
	//
	size_t usermeta_count;
    riack_pair *usermetas;
	size_t index_count;
    riack_pair *indexes;
} riack_content;


typedef struct _riack_module_function {
    riack_string module;
    riack_string function;
} riack_module_function;

typedef struct _riack_commit_hook {
    riack_module_function modfun;
    riack_string name;
} riack_commit_hook;

/* Riak 1.4+ replication mode bucket setting */
enum RIACK_REPLICATION_MODE {
    REALTIME_AND_FULLSYNC = 1, // TRUE in pbc interface
    REALTIME,
    FULLSYNC,
    DISABLED  // FALSE in pbc interface
};

/* Riak bucket properties for riak 1.4+ */
typedef struct _riack_bucket_properties {
    uint8_t n_val_use;
    uint32_t n_val;
    uint8_t allow_mult_use;
    uint8_t allow_mult;

    // Riak 1.4+ properties
    uint8_t last_write_wins_use;
    uint8_t last_write_wins;

    uint8_t has_precommit_hooks;
    size_t precommit_hook_count;
    riack_commit_hook* precommit_hooks;

    uint8_t has_postcommit_hooks;
    size_t postcommit_hook_count;
    riack_commit_hook* postcommit_hooks;

    uint8_t linkfun_use;
    riack_module_function linkfun;
    uint8_t chash_keyfun_use;
    riack_module_function chash_keyfun;

    uint8_t old_vclock_use;
    uint32_t old_vclock;
    uint8_t young_vclock_use;
    uint32_t young_vclock;
    uint8_t small_vclock_use;
    uint32_t small_vclock;
    uint8_t big_vclock_use;
    uint32_t big_vclock;

    uint8_t pr_use;
    uint32_t pr;
    uint8_t r_use;
    uint32_t r;
    uint8_t w_use;
    uint32_t w;
    uint8_t dw_use;
    uint32_t dw;
    uint8_t pw_use;
    uint32_t pw;
    uint8_t rw_use;
    uint32_t rw;

    uint8_t basic_quorum_use;
    uint8_t basic_quorum;
    uint8_t notfound_ok_use;
    uint8_t notfound_ok;

    riack_string backend;

    uint8_t search_use;
    uint8_t search;

    uint8_t replication_mode_use;
    enum RIACK_REPLICATION_MODE replication_mode;

    uint8_t search_index_use;
    riack_string search_index;

    uint8_t datatype_use;
    riack_string datatype;

    uint8_t consistent_use;
    uint8_t consistent;
} riack_bucket_properties;

/* MapReduce content type */
enum RIACK_MAPRED_CONTENT_TYPE {
	APPLICATION_JSON = 1,
	APPLICATION_ERLANG_TERM
};

/* MapReduce response structure */
typedef struct _riack_mapred_response {
    /* What phase is this response from */
	uint8_t phase_present;
	uint32_t phase;

	size_t data_size;
	uint8_t* data;
} riack_mapred_response;

/* Mapreduce response list */
typedef struct _riack_mapred_response_list{
    riack_mapred_response response;
    struct _riack_mapred_response_list* next_result;
} riack_mapred_response_list;

typedef struct _riack_vector_clock {
	size_t len;
	uint8_t* clock;
} riack_vector_clock;

typedef struct _riack_object {
	riack_string bucket;
    riack_string key;
    riack_vector_clock vclock;
	size_t content_count;
	riack_content* content;
} riack_object;

typedef struct _riack_get_object {
	riack_object object;
	uint8_t unchanged_present;
	uint8_t unchanged;
} riack_get_object;

typedef struct _riack_bucket_type_optional {
    uint8_t bucket_type_present;
    riack_string bucket_type;
} riack_bucket_type_optional;

typedef struct _riack_get_properties {
	uint8_t r_use;
	uint32_t r;
	uint8_t pr_use;
	uint32_t pr;
    uint8_t basic_quorum_use;
	uint8_t basic_quorum;
    uint8_t notfound_ok_use;
	uint8_t notfound_ok;

	uint8_t head_use;
	uint8_t head;
    uint8_t deletedvclock_use;
	uint8_t deletedvclock;

	uint8_t if_modified_use;
	riack_vector_clock if_modified;
    riack_bucket_type_optional bucket_type;
} riack_get_properties;

typedef struct _riack_put_properties {
	uint8_t w_use;
	uint32_t w;
	uint8_t dw_use;
	uint32_t dw;
	uint8_t pw_use;
	uint32_t pw;
	uint8_t if_not_modified_use;
	uint8_t if_not_modified;
	uint8_t if_none_match_use;
	uint8_t if_none_match;
	uint8_t return_body_use;
	uint8_t return_body;
	uint8_t return_head_use;
    uint8_t return_head;

    riack_bucket_type_optional bucket_type;
} riack_put_properties;

typedef struct _riack_del_properties {
	uint8_t rw_use;
	uint32_t rw;
	uint8_t r_use;
	uint32_t r;
	uint8_t w_use;
	uint32_t w;
	uint8_t pr_use;
	uint32_t pr;
	uint8_t pw_use;
	uint32_t pw;
	uint8_t dw_use;
	uint32_t dw;
	riack_vector_clock vclock;
    riack_bucket_type_optional bucket_type;
} riack_del_properties;

//************************
// Secondary Indexes
//************************
typedef struct _riack_2i_query_req {
    riack_string bucket;
    riack_string bucket_type;
    riack_string index;
    riack_string search_exact;
    riack_string search_min;
    riack_string search_max;
    uint32_t max_results;
    riack_string continuation_token;
} riack_2i_query_req;

//************************
// Search
//************************

/* Optional search parameters */
typedef struct _riack_search_optional_params {
    uint8_t rowlimit_present;
    uint32_t rowlimit;
    uint8_t start_present;
    uint32_t start;
    uint8_t sort_present;
    riack_string sort;
    uint8_t filter_present;
    riack_string filter;
    uint8_t default_field_present;
    riack_string default_field;
    uint8_t default_operation_present;
    riack_string default_operation;
    uint8_t presort_present;
    riack_string presort;
    size_t field_limits_count;
    riack_string *field_limits;
} riack_search_optional_params;

/* Search document */
typedef struct _riack_search_doc {
    size_t field_count;
    riack_pair *fields;
} riack_search_doc;

/* Result from a search */
typedef struct _riack_search_result {
    size_t document_count;
    riack_search_doc* documents;
    uint8_t max_score_present;
    float max_score;
    uint8_t num_found_present;
    uint32_t num_found;
} riack_search_result;

//************************
// CRDTs
//************************

typedef struct _riack_counter_update_properties {
    uint8_t w_use;
    uint32_t w;
    uint8_t dw_use;
    uint32_t dw;
    uint8_t pw_use;
    uint32_t pw;
} riack_counter_update_properties;

typedef struct _riack_counter_get_properties {
    uint8_t r_use;
    uint32_t r;
    uint8_t pr_use;
    uint32_t pr;
    uint8_t basic_quorum_use;
    uint8_t basic_quorum;
    uint8_t notfound_ok_use;
    uint8_t notfound_ok;
} riack_counter_get_properties;

#endif // __RIACK__DEFINES__H__
