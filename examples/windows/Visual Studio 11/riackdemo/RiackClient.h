#pragma once
#include "stdafx.h"
#include <iostream>

class RiackClient
{
public:
	RiackClient();
	~RiackClient(void);

	bool connect(std::string& host, int port);
	bool putText(std::string& bucket, std::string& key, std::string& value);

	bool isConnected();
private:
	bool connected;
	struct RIACK_CLIENT *client;
};

