// riackdemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RiackHelper.h"
#include "RiackClient.h"

using namespace std;

void getValue(RiackClient *client);
void setValue(RiackClient *client);
void deleteValue(RiackClient *client);
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
			break;
		case 's':
		case 'S':
			setValue(client);
		case 'g':
		case 'G':
			getValue(client);
			break;
		case 'd':
		case 'D':
			deleteValue(client);
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

void getValue(RiackClient *client) {
	string bucket, key;

}

void setValue(RiackClient *client) {
}

void deleteValue(RiackClient *client) {
}

void connect(RiackClient *client) {
	char host[1024];
	int port;
	cout << "Input host: ";
	cin >> host;
	cout << endl << "Input port: ";
	cin >> port;
	cout << endl;
	if (client->connect(string(host), port)) {
		cout << "Connected!" << endl;
	} else {
		cout << "Failed to connect" << std::endl;
	}
}
