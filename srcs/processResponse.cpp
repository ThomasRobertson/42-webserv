#include "Server.hpp"
#include "ConfigFile.hpp"
#include "Settings.hpp"
#include "StartServers.hpp"
#include "ClientRequest.hpp"
#include "ClientResponse.hpp"
#include "GenerateMethod.hpp"
#include "utils.hpp"
#include <vector>

bool DEBUG_VERBOSE = false;

bool StartServers::isCGIFile(Server server, std::string request)
{
	std::map<std::string, std::string> CGI = server.getCgiPages();
	// std::cout << "cgi : " << std::string(parseFileExtension(request)) << std::endl;
	if (CGI.find(parseFileExtension(request)) != CGI.end())
	{
		return true;
	}
	return false;
}

void StartServers::processResponse(epoll_event currentEvent)
{
	std::string response, status;
	
	Client currentClient = _clientList[currentEvent.data.fd];
	Server currentServer = *(currentClient.server);
	GenerateMethod genMethod(currentClient, currentServer);

	// std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;

	try
	{
		bool isCGI = isCGIFile(currentServer, currentClient.request.route);
		if (!isValidRequest(currentClient.request, status, isCGI))
		{
			std::cout << RED << "Invalid header, generating error page.\n" << DEFAULT;
			response = genMethod.getErrorPageResponse(status);
		}
		else if (isCGI)
		{
			std::cout << GREEN << "Launching CGI.\n" << DEFAULT;
			response = genMethod.CGIMethod();
		}
		else if (currentClient.request.method == "GET")
		{
			std::cout << GREEN << "Launching GET Method.\n" << DEFAULT;
			response = genMethod.GETMethod();
		}
		else if (currentClient.request.method == "POST")
		{
			std::cout << GREEN << "Launching POST Method.\n" << DEFAULT;
			// std::cout << currentClient.request.body << std::endl;
			response = genMethod.POSTMethod();
		}
		else if (currentClient.request.method == "DELETE")
		{
			std::cout << GREEN << "Launching DELETE Method.\n" << DEFAULT;
			// std::cout << "DELETE METH" << std::endl;
			response = genMethod.DELETEMethod();
		}
		else
		{
			std::cout << RED << "No valid method found, return error.\n" << DEFAULT;
			response = genMethod.getErrorPageResponse("405");
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		response = genMethod.getErrorPageResponse("500");
	}

	write(currentEvent.data.fd, response.c_str(), response.length());
	// std::cout << YELLOW << response << DEFAULT << std::endl;

	_clientList.erase(currentEvent.data.fd);
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
	close(currentEvent.data.fd);

	std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;
}