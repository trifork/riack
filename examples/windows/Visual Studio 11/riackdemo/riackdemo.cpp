// riackdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RiackHelper.h"
#include "RiackClient.h"
#include <string>

using namespace std;

void getValue(RiackClient *client);
void setValue(RiackClient *client);
void deleteValue(RiackClient *client);
void listBuckets(RiackClient *client);
void serverInfo(RiackClient *client);
void connect(RiackClient *client);

int _tmain(int argc, _TCHAR* argv[])
{
	bool connected = false;
	bool quit = false;
	RiackHelper riack;
	RiackClient *client = new RiackClient();
	while (!quit) {
		std::cout << "************************" << std::endl;
		if (client->isConnected()) {
			cout << " G Get a value" << endl;
			cout << " S Set a value" << endl;
			cout << " I Serverinfo" << endl;
			cout << " L List buckets" << endl;
			cout << " D Delete a value" << endl;
		} else {
			cout << " C Connect" << endl;
		}
		cout << "************************" << endl;
		cout << " Q Quit" << endl;
		char choice;
		cin >> choice;
		switch (choice)
		{
		case 'c':
		case 'C':
			connect(client);
			break;
		case 's':
		case 'S':
			setValue(client);
			break;
		case 'g':
		case 'G':
			getValue(client);
			break;
		case 'd':
		case 'D':
			deleteValue(client);
			break;
		case 'i':
		case 'I':
			serverInfo(client);
			break;
		case 'l':
		case 'L':
			listBuckets(client);
			break;
		case 'Q':
		case 'q':
			quit = true;
			break;
		default:
			break;
		}
 	}
	delete client;
	return 0;
}

void getValue(RiackClient *client) 
{
	string bucket, key, value;
	cout << "Input bucket: ";
	cin >> bucket;
	cout << endl << "Input keyname: ";
	cin >> key;
	if (client->getText(bucket, key, value)) {
		cout << "Value: " << value << endl;
	} else {
		cout << "Failed to get value" << endl;
	}
}

void setValue(RiackClient *client) 
{
	string bucket, key, value;
	cout << "Input target bucket: ";
	cin >> bucket;
	cout << endl << "Input target keyname: ";
	cin >> key;
	cout << endl << "Input value: ";
	cin >> value;
	cout << endl;
	if (client->putText(bucket, key, value)) {
		cout << "Success" << endl;
	} else {
		cout << "Failed to put value" << endl;
	}
}

void deleteValue(RiackClient *client) 
{
	string bucket, key;
	cout << "Input bucket: ";
	cin >> bucket;
	cout << endl << "Input key: ";
	cin >> key;
	cout << endl;
	if (client->deleteValue(bucket, key)) {
		cout << "Successfully delete" << endl;
	} else {
		cout << "Failed to delete" << endl;
	}
}

void listBuckets(RiackClient *client) 
{
	vector<string> buckets;
	if (client->listBuckets(buckets)) {
		vector<string>::const_iterator iter;
		for (iter = buckets.begin(); iter != buckets.end(); ++iter) {
			cout << *iter << endl;
		}
	} else {
		cout << "Failed to list buckets" << endl;
	}
}

void serverInfo(RiackClient *client) 
{
	string node, version;
	if (client->getServerInfo(node, version)) {
		cout << "Node: " << node.c_str() << endl;
		cout << "Version: " << version.c_str() << endl;
	} else {
		cout << "Failed to get server info" << endl;
	}
}

void connect(RiackClient *client) 
{
	string host;
	int port;
	cout << "Input host: ";
	cin >> host;
	cout << endl << "Input port: ";
	cin >> port;
	cout << endl;
	if (client->connect(host, port)) {
		cout << "Connected!" << endl;
	} else {
		cout << "Failed to connect" << std::endl;
	}
}
