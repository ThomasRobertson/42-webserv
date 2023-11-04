#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include <map>
#include <sstream>
#include <stdexcept>
#include "ClientResponse.hpp"

bool DEBUG_VERBOSE = false;

std::string StartServers::getUserResponse(Client client)
{
	Server currentServer = this->_serversVec[client.serverIndex];
	std::string method = client.request.method;

	std::string response;

	if (method == "GET")
		response = GenerateMethod::GETMethod(client, currentServer);
	else if (method == "POST")
		(void)response;
	else if (method == "DELETE")
		(void)response;

	#ifdef DEBUG
	else
		throw std::invalid_argument("Unknown method.");
	#endif // DEBUG

	return response;
}

UserRequest StartServers::getUserRequest(std::string requestStr)
{
    UserRequest data;

    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.root = requestStr.substr(0, spaceSepPos);
    return data;
}