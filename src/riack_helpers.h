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
#ifndef __RIACK__HELPERS__H__
#define __RIACK__HELPERS__H__

#include "riack_defines.h"
#include "protocol/riak.pb-c.h"
#include "protocol/riak_kv.pb-c.h"

void riack_free_copied_rpb_put_req(riack_client* client, RpbPutReq* pput_req);
void riack_free_copied_pair(riack_client* client, riack_pair *ppair);

void riack_copy_string_to_buffer(riack_client* client, char* str, ProtobufCBinaryData* target);
void riack_copy_buffer_to_string(riack_client* client, ProtobufCBinaryData* src, char** str);

void riack_copy_content_to_rpbcontent(riack_client* client, riack_content *pcontent, RpbContent* ppbc_content);
void riack_copy_rpbcontent_to_content(riack_client* client, RpbContent* ppbc_content, riack_content *pcontent);

void riack_copy_object_to_rpbputreq(riack_client* client, riack_object *pobject, RpbPutReq* pput_req);

void riack_copy_rpbpair_to_pair(riack_client* client, RpbPair* rpc_pair, riack_pair* rpair);

/// This function will not allocate dynamic memory for data
void riack_link_strmapred_with_rpbmapred(riack_client* client, RpbMapRedResp* source, riack_mapred_response* target);

void riack_copy_strmapred_to_mapred(riack_client* client, riack_mapred_response* source,
                                    riack_mapred_response_list* target);

riack_string_linked_list* riack_string_linked_list_add(riack_client *client, riack_string_linked_list** base,
		riack_string string_new);

void riack_mapred_add_to_chain(riack_client *client, riack_mapred_response_list** base,
        riack_mapred_response_list* mapred_new);

#endif // __RIACK__HELPERS__H__
