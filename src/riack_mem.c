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
#pragma warning( disable:4005 )

#include "riack.h"
#include <stdio.h>
#include "protocol/riak.pb-c.h"

void *riack_pb_alloc(void *allocator_data, size_t size);
void riack_pb_free (void *allocator_data, void *data);

riack_allocator riack_default_allocator =
{
	riack_pb_alloc,
	riack_pb_free,
};

ProtobufCAllocator riack_pb_allocator(riack_allocator *allocator)
{
	ProtobufCAllocator result;
	result.alloc = allocator->alloc;
	result.free = allocator->free;
	result.allocator_data = allocator->allocator_optional_data;
	return result;
}

void *riack_pb_alloc(void *allocator_data, size_t size)
{
	void *rv;
	(void) allocator_data;
	if (size == 0) {
		return 0;
	}
	rv = malloc(size);
	if (rv == 0) {
		fprintf(stderr, "Out of memory\n");
		abort();
	}
	return rv;
}

void riack_pb_free (void *allocator_data, void *data)
{
	(void) allocator_data;
	if (data) {
		free(data);
	}
}
