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

std::string StartServers::getErrorPageResponse(Client client, std::string errorCode)
{
    std::string response, fileLocation, contentType, status;
	Server currentServer = this->_serversVec[client.serverIndex];
	
	status = "400 OK";
	contentType = "text/html";
    
    fileLocation = currentServer.getErrorPageRoute(errorCode);
	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		fileLocation = currentServer.getErrorPageRoute("500");
	}
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	ClientResponse clientReponse(status, contentType, content);

	response = clientReponse.getReponse();
	return response;
}

UserRequest StartServers::getUserRequest(std::string requestStr)
{
    UserRequest data;

	data.fullRequest = requestStr;
    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.root = requestStr.substr(0, spaceSepPos);
    return data;
}