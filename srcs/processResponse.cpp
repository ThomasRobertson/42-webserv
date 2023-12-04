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

	std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;

	try
	{
		bool isCGI = isCGIFile(currentServer, currentClient.request.route);
		if (!isValidRequest(currentClient.request, status, isCGI))
		{
			response = genMethod.getErrorPageResponse(status);
		}
		else if (isCGI)
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
	}
	catch (const std::exception&)
	{
		response = genMethod.getErrorPageResponse("500");
	}

	write(currentEvent.data.fd, response.c_str(), response.length());
	std::cout << YELLOW << response << DEFAULT << std::endl;

	_clientList.erase(currentEvent.data.fd);
	epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
	close(currentEvent.data.fd);

	std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;
}

bool StartServers::isValidRequest(UserRequest requestData, std::string &status, bool isCGI)
{
    // std::cout << "requestData: fullRequest: " << requestData.fullRequest << std::endl << "END" << std::endl;

    std::string httpRequest = requestData.fullStr;

    std::size_t pos = 0;
    std::string line;

    int countPost = 0;
    int countOther = 0;
    std::string method, uri, version;

	if (!isCGI && requestData.isBodyTooLarge)
	{
		status = "413";
		return false;
	}
	if (httpRequest.empty())
	{
		status = "400";
		return false;
	}

	pos = httpRequest.find("\r\n");
    if (pos != std::string::npos)
    {
        line = httpRequest.substr(0, pos);
        httpRequest.erase(0, pos + 2);
    }
    else
        line = httpRequest;
    std::istringstream lineStream(line);
    lineStream >> method >> uri >> version;
    if (method != "GET" && method != "POST" && method != "DELETE")
    {   
		status = "405"; 
		return false;
	}
    if (version != "HTTP/1.1")
    {   
		status = "505"; 
		return false;
	}
    // if (uri.find('/') != 0) //!Ne marche pas
    // {   
	// 	status = "400"; 
	// 	return false;
	// }
    if (line.length() > 8000)
    {   
		status = "414"; 
		return false;
	}
    if (std::count(line.begin(), line.end(), ' ') > 2)
    {   
		status = "400"; 
		return false;
	}

	bool lenghtFind = false;
    while (httpRequest.find("\r\n") != std::string::npos)
    {
		pos = httpRequest.find("\r\n");
        if (pos != std::string::npos)
        {
            line = httpRequest.substr(0, pos);
            httpRequest.erase(0, pos + 2);
        }
        else
            line = httpRequest;

        if (line.empty())
            break;
        else if (line.find("Host:") != std::string::npos || line.find("host:") != std::string::npos)
        {
            countPost++;
            countOther++;

        }
        else if (line.find("Transfer-Encoding:") != std::string::npos || line.find("transfer-encoding:") != std::string::npos)
        {
			if (lenghtFind == true)
			{
				status = "400";
				return false;
			}

			lenghtFind = true;
			countPost++;
        }
        else if (line.find("Content-Length:") != std::string::npos || line.find("content-length:") != std::string::npos)
        {
			if (lenghtFind == true)
			{
				status = "400";
				return false;
			}

			lenghtFind = true;
			countPost++;
            std::string lengthValue = line.substr(16); // Extract the value after "Content-Length:" or "content-length:"
            std::istringstream lengthStream(lengthValue);
            int contentLength;

			lengthStream >> contentLength;

            if (contentLength <= 0)
			{
				status = "400";
				return false;
			}
                
        }
        else if (line.find("Content-Type:") != std::string::npos || line.find("content-type:") != std::string::npos)
            countPost++;

        if (line.length() > 8000)
		{
			status = "414";
			return false;
		}

        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            std::string headerKey = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            if (headerKey.empty() || headerKey.find_first_not_of(' ') == std::string::npos)
			{
				status = "400";
				return false;
			}
        }
    }


    if (countOther == 1 && (method == "GET" || method == "DELETE"))
        return true;
    else if (countPost >= 3 && method == "POST")
        return true;
    else
	{
		status = "400";
		return false;
	}
}