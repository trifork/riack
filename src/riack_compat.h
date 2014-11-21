/*
   Copyright 2014 Trifork A/S
   Author: Kaspar Bach Pedersen

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
#ifndef __RIACK__COMPAT__H__
#define __RIACK__COMPAT__H__


/************************************
* riack 1.x compatibility defines
************************************/

#define RIACK_ALLOCATOR _riack_allocator
#define RIACK_CONNECTION_OPTIONS _riack_connection_options
#define RIACK_STRING riack_string
#define RIACK_STRING_LIST _riack_string_list
#define RIACK_STRING_LINKED_LIST _riack_string_linked_list
#define RIACK_CLIENT _riack_client
#define RIACK_LINK _riack_link
#define RIACK_PAIR _riack_pair
#define RIACK_CONTENT _riack_content
#define RIACK_MODULE_FUNCTION _riack_module_function
#define RIACK_COMMIT_HOOK _riack_commit_hook
#define RIACK_BUCKET_PROPERTIES _riack_bucket_properties
#define RIACK_MAPRED_RESPONSE _riack_mapred_response
#define RIACK_MAPRED_RESPONSE_LIST _riack_mapred_response_list
#define RIACK_VECTOR_CLOCK _riack_vector_clock
#define RIACK_OBJECT _riack_object
#define RIACK_GET_OBJECT _riack_get_object
#define RIACK_GET_PROPERTIES _riack_get_properties
#define RIACK_PUT_PROPERTIES _riack_put_properties
#define RIACK_DEL_PROPERTIES _riack_del_properties
#define RIACK_2I_QUERY_REQ _riack_2i_query_req

#define RIACK_SEARCH_OPTIONAL_PARAMETERS _riack_search_optional_params
#define RIACK_SEARCH_DOCUMENT _riack_search_doc
#define RIACK_SEARCH_RESULT _riack_search_result
#define RIACK_COUNTER_UPDATE_PROPERTIES _riack_counter_update_properties
#define RIACK_COUNTER_GET_PROPERTIES _riack_counter_get_properties

#endif
