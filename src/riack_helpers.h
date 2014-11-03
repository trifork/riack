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

void riack_free_copied_rpb_put_req(RIACK_CLIENT* client, RpbPutReq* pput_req);
void riack_free_copied_pair(RIACK_CLIENT* client, RIACK_PAIR *ppair);

void riack_copy_string_to_buffer(RIACK_CLIENT* client, char* str, ProtobufCBinaryData* target);
void riack_copy_buffer_to_string(RIACK_CLIENT* client, ProtobufCBinaryData* src, char** str);

void riack_copy_content_to_rpbcontent(RIACK_CLIENT* client, RIACK_CONTENT *pcontent, RpbContent* ppbc_content);
void riack_copy_rpbcontent_to_content(RIACK_CLIENT* client, RpbContent* ppbc_content, RIACK_CONTENT *pcontent);

void riack_copy_object_to_rpbputreq(RIACK_CLIENT* client, RIACK_OBJECT *pobject, RpbPutReq* pput_req);

void riack_copy_rpbpair_to_pair(RIACK_CLIENT* client, RpbPair* rpc_pair, RIACK_PAIR* rpair);

/// This function will not allocate dynamic memory for data
void riack_link_strmapred_with_rpbmapred(RIACK_CLIENT* client, RpbMapRedResp* source,
                                         RIACK_MAPRED_RESPONSE* target);

void riack_copy_strmapred_to_mapred(RIACK_CLIENT* client, RIACK_MAPRED_RESPONSE* source,
                                    RIACK_MAPRED_RESPONSE_LIST* target);

RIACK_STRING_LINKED_LIST* riack_string_linked_list_add(RIACK_CLIENT *client,
		RIACK_STRING_LINKED_LIST** base,
		RIACK_STRING string_new);

void riack_mapred_add_to_chain(RIACK_CLIENT *client,
        RIACK_MAPRED_RESPONSE_LIST** base,
        RIACK_MAPRED_RESPONSE_LIST* mapred_new);

#endif // __RIACK__HELPERS__H__
