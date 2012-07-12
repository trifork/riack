#pragma once
#include "stdafx.h"
#include <iostream>
#include <vector>
class RiackClient
{
public:
	RiackClient();
	~RiackClient(void);

	bool connect(const std::string& host, int port);
	bool deleteValue(const std::string& bucket, const std::string& key);
	bool listBuckets(std::vector<std::string> &buckets);
	bool putText(const std::string& bucket, const std::string& key, const std::string& value);
	bool getText(const std::string& bucket, const std::string& key, std::string& value);
	bool getServerInfo(std::string &node, std::string &version);

	bool isConnected();
private:
	struct RIACK_ALLOCATOR allocator;
	bool connected;
	struct RIACK_CLIENT *client;
};

