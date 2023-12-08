#pragma once

#include "utils.hpp"
#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include "ClientResponse.hpp"

class GenerateMethod
{
	private:
	Client _client;
	Server _server;

	std::string getRequestBody();
	std::string getChunkedRequestBody();
	std::string getFileName();

	public:
	GenerateMethod(Client client, Server server) : _client(client), _server(server) {}
	~GenerateMethod() {}

	std::string GETMethod();
	std::string POSTMethod();
	std::string DELETEMethod();
	std::string CGIMethod();
	std::string getErrorPageResponse(std::string errorCode);
	std::string listingDirectory(const std::string &fileLocation, std::string &fileName);
	std::string generateRedirect(std::string url);
};
