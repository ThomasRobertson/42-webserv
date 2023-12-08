#include "Server.hpp"
#include "ConfigFile.hpp"
#include "Settings.hpp"
#include "StartServers.hpp"
#include "ClientRequest.hpp"
#include "ClientResponse.hpp"
#include "GenerateMethod.hpp"
#include "utils.hpp"
#include <vector>
#include "Autorization.hpp"

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

std::string StartServers::generateResponse(Server server, Client client)
{
	std::string status;
	GenerateMethod genMethod(client, server);
	std::pair<std::string, Location> location;
	try
	{
		location = server.getRootDir(client.request.route);
	}
	catch (const std::exception&)
	{
		status = "404";
		std::cout << RED << "Invalid header, generating error page.\n" << DEFAULT;
		return genMethod.getErrorPageResponse(status);
	}

	if (!location.second.redirect.empty())
		return genMethod.generateRedirect(location.second.redirect);

	Autorization autorizationCheck(client, location);

	switch (autorizationCheck.ShallYouPass()) {
	case YOU_SALL_NOT_PASS:
		return autorizationCheck.generateErrorPage(location.second.authBasic);
		break;
	case RUN_YOU_FOOLS:
		break;
	}

	try
	{
		bool isCGI = isCGIFile(server, client.request.route);

		if (!isValidRequest(client.request, status, isCGI))
		{
			std::cout << RED << "Invalid header, generating error page.\n" << DEFAULT;
			return genMethod.getErrorPageResponse(status);
		}
		else if (isCGI)
		{
			std::cout << GREEN << "Launching CGI.\n" << DEFAULT;
			return genMethod.CGIMethod();
		}
		else if (client.request.method == "GET")
		{
			std::cout << GREEN << "Launching GET Method.\n" << DEFAULT;
			return genMethod.GETMethod();
		}
		else if (client.request.method == "POST")
		{
			std::cout << GREEN << "Launching POST Method.\n" << DEFAULT;
			// std::cout << client.request.body << std::endl;
			return genMethod.POSTMethod();
		}
		else if (client.request.method == "DELETE")
		{
			std::cout << GREEN << "Launching DELETE Method.\n" << DEFAULT;
			// std::cout << "DELETE METH" << std::endl;
			return genMethod.DELETEMethod();
		}
		else
		{
			std::cout << RED << "No valid method found, return error.\n" << DEFAULT;
			return genMethod.getErrorPageResponse("405");
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return genMethod.getErrorPageResponse("500");
	}
}

void StartServers::processResponse(epoll_event currentEvent)
{
	std::string response;
	Client currentClient = _clientList[currentEvent.data.fd];
	Server currentServer = *(currentClient.server);

	// std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;

	response = generateResponse(currentServer, currentClient);

	write(currentEvent.data.fd, response.c_str(), response.length());
	// std::cout << YELLOW << response << DEFAULT << std::endl;

	_clientList.erase(currentEvent.data.fd);
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
	close(currentEvent.data.fd);

	std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;
}
