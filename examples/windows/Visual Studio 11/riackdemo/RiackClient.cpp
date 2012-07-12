#include "stdafx.h"
#include "RiackClient.h"

void *riack_alloc(void *optional_data, size_t size) 
{
	return malloc(size);
}

void riack_free(void *optional_data, void *pointer)
{
	if (!pointer)
		return;
	free (pointer);
}

RiackClient::RiackClient(void)
{
	connected = false;
	allocator.alloc = riack_alloc;
	allocator.free = riack_free;
	client = riack_new_client(&allocator);
}

RiackClient::~RiackClient(void)
{
	riack_free(client);
}

bool RiackClient::connect(const std::string& host, int port)
{
	if (riack_connect(client, host.c_str(), port) == RIACK_SUCCESS) {
		connected = true;
	} else {
		connected = false;
	}
	return connected;
}

bool RiackClient::isConnected() {
	return connected;
}

bool RiackClient::deleteValue(const std::string& bucket, const std::string& key) 
{
	RIACK_STRING rbucket, rkey;
	rbucket.len = bucket.length();
	rbucket.value = (char*)bucket.c_str();
	rkey.len = key.length();
	rkey.value = (char*)key.c_str();
	if (riack_delete(client, rbucket, rkey, 0) == RIACK_SUCCESS) {
		return true;
	}
	return false;
}

bool RiackClient::listBuckets(std::vector<std::string> &buckets) 
{
	RIACK_STRING_LIST rbuckets;
	if (!isConnected()) {
		return false;
	}
	buckets.clear();
	if (riack_list_buckets(client, &rbuckets) == RIACK_SUCCESS) {
		for (size_t i=0; i<rbuckets.string_count; ++i) {
			size_t slen = rbuckets.strings[i].len;
			char* sdata = rbuckets.strings[i].value;
			buckets.push_back(std::string(sdata, slen));
		}
		return true;
	}
	return false;
}

bool RiackClient::getText(const std::string& bucket, const std::string& key, std::string& value)
{
	RIACK_STRING rbucket, rkey;
	RIACK_GET_OBJECT result;
	if (!isConnected()) {
		return false;
	}
	rbucket.len = bucket.length();
	rbucket.value = (char*)bucket.c_str();
	rkey.len = key.length();
	rkey.value = (char*)key.c_str();
	if (riack_get(client, rbucket, rkey, NULL, &result) == RIACK_SUCCESS) {
		if (result.object.content_count == 1) {
			char* data = (char*)result.object.content[0].data;
			size_t dataLen = result.object.content[0].data_len;
			value = std::string(data, dataLen);
			return true;
		}
	}
	return false;
}

bool RiackClient::putText(const std::string& bucket, const std::string& key, const std::string& value)
{
	struct RIACK_OBJECT obj;
	struct RIACK_CONTENT content;
	struct RIACK_OBJECT returned;

	if (!isConnected()) {
		return false;
	}
	char* plainText = "plain/text";
	memset(&content, 0, sizeof(struct RIACK_CONTENT));
	memset(&obj, 0, sizeof(struct RIACK_OBJECT));
	obj.bucket.len = bucket.length();
	obj.bucket.value = (char*)bucket.c_str();
	obj.key.len = bucket.length();
	obj.key.value = (char*)key.c_str();
	obj.content_count = 1;
	obj.content = &content;
	content.data = (uint8_t*)value.c_str();
	content.data_len = value.size();
	content.content_type.len = strlen(plainText);
	content.content_type.value = plainText;
	if (riack_put(client, obj, &returned, 0) == RIACK_SUCCESS)
		return true;

	return false;
}

bool RiackClient::getServerInfo(std::string &node, std::string &version)
{
	RIACK_STRING rnode, rversion;
	if (!isConnected()) {
		return false;
	}
	if (riack_server_info(client, &rnode, &rversion) == RIACK_SUCCESS) {
		node = std::string(rnode.value, rnode.len);
		version = std::string(rversion.value, rversion.len);
		return true;
	}
	return false;
}
