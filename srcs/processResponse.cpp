#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include "ClientResponse.hpp"
#include "GenerateMethod.hpp"
#include "utils.hpp"
#include <vector>

bool DEBUG_VERBOSE = false;

bool StartServers::isCGIFile(Server server, std::string request)
{
	size_t dotPos = request.find_last_of(".");
	std::map<std::string, std::string> CGI = server.getCgiPages();
	if (dotPos != std::string::npos)
	{
		std::string extension = request.substr(dotPos + 1);
		if (CGI.find(extension) != CGI.end())
			return true;
	}
	return false;
}

void StartServers::processResponse(epoll_event currentEvent)
{
	std::string response;
	
	Client currentClient = _clientList[currentEvent.data.fd];
	Server currentServer = _serversVec[currentClient.serverIndex];
	GenerateMethod genMethod(currentClient, currentServer);


	std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;

	if (isCGIFile(currentServer, currentClient.request.root))
	{
		response = genMethod.CGIMethod();
	}
	else if (currentClient.request.method == "GET")
	{
		response = genMethod.GETMethod();
	}
	else if (currentClient.request.method == "POST")
	{
		// std::cout << currentClient.request.body << std::endl;
		response = genMethod.POSTMethod();
	}
	else if (currentClient.request.method == "DELETE")
	{
		// std::cout << "DELETE METH" << std::endl;
		response = genMethod.DELETEMethod();
	}
	else
	{
		response = genMethod.getErrorPageResponse("405");
	}

	// if (isValidRequest(client.request))
	// else
		// response = GenerateMethod::getErrorPageResponse(client, server, "400");

	write(currentEvent.data.fd, response.c_str(), response.length());

	_clientList.erase(currentEvent.data.fd);
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
	close(currentEvent.data.fd);

	std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;
}