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
#define RFREE(client, pointer) client->allocator.free(0, pointer)
/* Allocate and copy memory */
#define RMALLOCCOPY(client, target, target_len, source, len) target = (void*)RMALLOC(client, len); memcpy(target, source, len); target_len=len

#define RSTR_HAS_CONTENT(s) (s.len > 0 && s.value)

struct RIACK_ALLOCATOR
{
	void *(*alloc)(void *optional_data, size_t size);
	void (*free)(void *optional_data, void *pointer);
	void *allocator_optional_data;
};

/* Socket connection options */
struct RIACK_CONNECTION_OPTIONS {
	uint32_t recv_timeout_ms;
	uint32_t send_timeout_ms;
};

/* Riack's base string type */
typedef struct {
	char* value;
	size_t len;
} RIACK_STRING;

/* List of strings */
typedef struct {
	RIACK_STRING* strings;
	size_t string_count;
} RIACK_STRING_LIST;

/* Linked list of strings */
struct RIACK_STRING_LINKED_LIST {
	RIACK_STRING string;
	struct RIACK_STRING_LINKED_LIST* next;
};

/* Riack client */
struct RIACK_CLIENT {
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
	struct RIACK_CONNECTION_OPTIONS options;

    /* Allocator to use with this client */
	struct RIACK_ALLOCATOR allocator;
};

/* Link to an object */
struct RIACK_LINK
{
	RIACK_STRING bucket;
	RIACK_STRING key;
    /* Link tag */
	RIACK_STRING tag;
};

/* key/value pair */
struct RIACK_PAIR
{
	RIACK_STRING key;
	uint8_t  value_present;
	size_t   value_len;
	uint8_t* value;
};

struct RIACK_CONTENT
{
	size_t data_len;
	uint8_t *data;
	RIACK_STRING content_type;
	RIACK_STRING charset;
	RIACK_STRING content_encoding;
	RIACK_STRING vtag;
	size_t link_count;
	struct RIACK_LINK* links;
	//
	uint8_t last_modified_present;
	uint32_t last_modified;
	uint8_t last_modified_usecs_present;
	uint32_t last_modified_usecs;
	uint8_t deleted_present;
	uint8_t deleted;
	//
	size_t usermeta_count;
	struct RIACK_PAIR *usermetas;
	size_t index_count;
	struct RIACK_PAIR *indexes;
};


struct RIACK_MODULE_FUNCTION {
    RIACK_STRING module;
    RIACK_STRING function;
};

struct RIACK_COMMIT_HOOK {
    struct RIACK_MODULE_FUNCTION modfun;
    RIACK_STRING name;
};

/* Riak 1.4+ replication mode bucket setting */
enum RIACK_REPLICATION_MODE {
    REALTIME_AND_FULLSYNC, // TRUE in pbc interface
    REALTIME,
    FULLSYNC,
    DISABLED  // FALSE in pbc interface
};

/* Riak bucket properties for riak 1.4+ */
struct RIACK_BUCKET_PROPERTIES {
    uint8_t n_val_use;
    uint32_t n_val;
    uint8_t allow_mult_use;
    uint8_t allow_mult;

    // Riak 1.4+ properties
    uint8_t last_write_wins_use;
    uint8_t last_write_wins;

    uint8_t has_precommit_hooks;
    size_t precommit_hook_count;
    struct RIACK_COMMIT_HOOK* precommit_hooks;

    uint8_t has_postcommit_hooks;
    size_t postcommit_hook_count;
    struct RIACK_COMMIT_HOOK* postcommit_hooks;

    uint8_t linkfun_use;
    struct RIACK_MODULE_FUNCTION linkfun;
    uint8_t crash_keyfun_use;
    struct RIACK_MODULE_FUNCTION crash_keyfun;

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

    RIACK_STRING backend;

    uint8_t search_use;
    uint8_t search;

    uint8_t replication_mode_use;
    enum RIACK_REPLICATION_MODE replication_mode;

};

/* MapReduce content type */
enum RIACK_MAPRED_CONTENT_TYPE {
	APPLICATION_JSON,
	APPLICATION_ERLANG_TERM
};

/* MapReduce response structure */
struct RIACK_MAPRED_RESPONSE {
    /* What phase is this response from */
	uint8_t phase_present;
	uint32_t phase;

	size_t data_size;
	uint8_t* data;
};

/* Mapreduce response list */
struct RIACK_MAPRED_RESPONSE_LIST {
    struct RIACK_MAPRED_RESPONSE response;
    struct RIACK_MAPRED_RESPONSE_LIST* next_result;
};

struct RIACK_VECTOR_CLOCK {
	size_t len;
	uint8_t* clock;
};

struct RIACK_OBJECT {
	RIACK_STRING bucket;
	RIACK_STRING key;
	struct RIACK_VECTOR_CLOCK vclock;
	size_t content_count;
	struct RIACK_CONTENT* content;
};

struct RIACK_GET_OBJECT {
	struct RIACK_OBJECT object;
	uint8_t unchanged_present;
	uint8_t unchanged;
};

struct RIACK_GET_PROPERTIES
{
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
	struct RIACK_VECTOR_CLOCK if_modified;
};

struct RIACK_PUT_PROPERTIES
{
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
};

struct RIACK_DEL_PROPERTIES
{
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
	struct RIACK_VECTOR_CLOCK vclock;
};

/*
  RIAK SEARCH
*/

/* Optional search parameters */
struct RIACK_SEARCH_OPTIONAL_PARAMETERS {
    uint8_t rowlimit_present;
    uint32_t rowlimit;
    uint8_t start_present;
    uint32_t start;
    uint8_t sort_present;
    RIACK_STRING sort;
    uint8_t filter_present;
    RIACK_STRING filter;
    uint8_t default_field_present;
    RIACK_STRING default_field;
    uint8_t default_operation_present;
    RIACK_STRING default_operation;
    uint8_t presort_present;
    RIACK_STRING presort;
    size_t field_limits_count;
    RIACK_STRING *field_limits;
};

/* Search document */
struct RIACK_SEARCH_DOCUMENT {
    size_t field_count;
    struct RIACK_PAIR *fields;
};

/* Result from a search */
struct RIACK_SEARCH_RESULT {
    size_t document_count;
    struct RIACK_SEARCH_DOCUMENT* documents;
    uint8_t max_score_present;
    float max_score;
    uint8_t num_found_present;
    uint32_t num_found;
};

#endif // __RIACK__DEFINES__H__
