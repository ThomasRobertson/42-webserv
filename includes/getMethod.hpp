#pragma once

#include "utils.hpp"
#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include "ClientResponse.hpp"

class GenerateMethod
{
	private:

	public:
	GenerateMethod() {}
	~GenerateMethod() {}

	static std::string GETMethod(Client client, Server server);
};
