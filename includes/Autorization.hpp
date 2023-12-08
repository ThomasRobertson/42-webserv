#pragma once

#include <string>
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include <iostream>
#include <fstream>

#define YOU_SALL_NOT_PASS 0
#define RUN_YOU_FOOLS 1
#define NO_AUTH_HEADER_FOUND 2

class Autorization
{
	public:
	Autorization(Client client, std::pair<std::string, Location> location) : _location(location), _client(client) {}
	~Autorization() {}

	int ShallYouPass();
	std::string generateErrorPage(std::string realm);

	private:
	std::pair<std::string, Location> _location;
	Client _client;
	//typedef false youShallNotPass;
	bool isAuthLocationPresent();
	bool isRequestWithAutorization();
	bool isAutorizationValid();
};
