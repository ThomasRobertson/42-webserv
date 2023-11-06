#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include <exception>
#include <map>
#include <sstream>
#include <stdexcept>
#include "ClientResponse.hpp"
#include "utils.hpp"

// std::string StartServers::getUserResponse(Client client)
// {
// 	Server currentServer = this->_serversVec[client.serverIndex];
// 	std::string method = client.request.method;

// 	std::string response;

// 	if (method == "GET")
// 		response = GenerateMethod::GETMethod(client, currentServer);
// 	else if (method == "POST")
// 		(void)response;
// 	else if (method == "DELETE")
// 		(void)response;

// 	#ifdef DEBUG
// 	else
// 		throw std::invalid_argument("Unknown method.");
// 	#endif // DEBUG

// 	return response;
// }