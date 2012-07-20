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

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#if defined (_WIN32) 
  #if defined(riack_EXPORTS)
    #define  RIACK_EXPORT __declspec(dllexport)
  #else
    #define  RIACK_EXPORT __declspec(dllimport)
  #endif
#else /* defined (_WIN32) */
 #define RIACK_EXPORT
#endif

#define RIACK_SUCCESS 1
#define RIACK_ERROR_COMMUNICATION -1
#define RIACK_ERROR_RESPONSE -2
#define RIACK_ERROR_INVALID_INPUT -3

#define RMALLOC(client, size) client->allocator.alloc(0, size)
#define RFREE(client, pointer) client->allocator.free(0, pointer)

#define RMALLOCCOPY(client, target, target_len, source, len) target = RMALLOC(client, len); memcpy(target, source, len); target_len=len

struct RIACK_ALLOCATOR
{
	void *(*alloc)(void *optional_data, size_t size);
	void (*free)(void *optional_data, void *pointer);
	void *allocator_optional_data;
};

struct RIACK_CONNECTION_OPTIONS {
	uint32_t recv_timeout_ms;
	uint32_t send_timeout_ms;
};

typedef struct {
	char* value;
	size_t len;
} RIACK_STRING;

typedef struct {
	RIACK_STRING* strings;
	size_t string_count;
} RIACK_STRING_LIST;

struct RIACK_STRING_LINKED_LIST {
	RIACK_STRING string;
	struct RIACK_STRING_LINKED_LIST* next;
};

struct RIACK_CLIENT {
	int sockfd;
	char* last_error;
	uint32_t last_error_code;
	char* host;
	int port;
	struct RIACK_CONNECTION_OPTIONS options;

	struct RIACK_ALLOCATOR allocator;
};

struct RIACK_LINK
{
	RIACK_STRING bucket;
	RIACK_STRING key;
	RIACK_STRING tag;
};

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
	struct RIACK_LINK** links;
	//
	uint8_t last_modified_present;
	uint32_t last_modified;
	uint8_t last_modified_usecs_present;
	uint32_t last_modified_usecs;
	uint8_t deleted_present;
	uint8_t deleted;
	//
	size_t usermeta_count;
	struct RIACK_PAIR **usermetas;
	size_t index_count;
	struct RIACK_PAIR *indexes;
};

enum RIACK_MAPRED_CONTENT_TYPE {
	APPLICATION_JSON,
	APPLICATION_ERLANG_TERM
};

struct RIACK_MAPRED_RESULT {
	uint8_t phase_present;
	uint32_t phase;
	size_t data_size;
	uint8_t* data;
	struct RIACK_MAPRED_RESULT* next_result;
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
	uint8_t basic_quorum_present;
	uint8_t basic_quorum;
	uint8_t notfound_ok_present;
	uint8_t notfound_ok;

	uint8_t head_use;
	uint8_t head;
	uint8_t deletedvclock_present;
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

#endif // __RIACK__DEFINES__H__
