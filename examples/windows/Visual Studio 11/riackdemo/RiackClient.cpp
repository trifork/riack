#include "stdafx.h"
#include "RiackClient.h"

RiackClient::RiackClient(void)
{
	connected = false;
	client = riack_new_client(0);
}


RiackClient::~RiackClient(void)
{
	riack_free(client);
}

bool RiackClient::connect(std::string& host, int port)
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

bool RiackClient::putText(std::string& bucket, std::string& key, std::string& value)
{
	struct RIACK_OBJECT obj;
	struct RIACK_CONTENT content;
	if (!connected) {
		throw new std::runtime_error("Error not connected to riak server");
	}
	char* plainText = "plain/text";
	memset(&content, 0, sizeof(struct RIACK_CONTENT));
	obj.bucket.len = bucket.size();
	obj.bucket.value = (char*)bucket.c_str();
	obj.key.len = bucket.size();
	obj.key.value = (char*)key.c_str();
	obj.content_count = 1;
	obj.content = &content;
	content.data = (uint8_t*)value.c_str();
	content.data_len = value.size();
	content.content_type.len = strlen(plainText);
	content.content_type.value = plainText;
	if (riack_put(client, obj, 0, 0) == RIACK_SUCCESS)
		return true;

	return false;
}
