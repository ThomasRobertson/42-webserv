#include "ClientRequest.hpp"
#include "StartServers.hpp"

bool isValidServerName(std::string request, Server server)
{
	size_t startPos, endPos;
	std::vector<std::string> serverNames = server.getServerName();

	if (serverNames.size() == 0)
	{
		// std::cout << "NO SERVER NAME WAS SET" << std::endl;
		return true;
	}

	startPos = request.find("Host: ");
	if (startPos == std::string::npos)
		return false;
	startPos += std::strlen("Host: ");
	endPos = request.find(":", startPos);
	if (endPos == std::string::npos)
	{
		endPos = request.find("\r\n", startPos);
		if (endPos == std::string::npos)
			return false;
	}
	std::string host = request.substr(startPos, endPos - startPos);

	if(std::find(serverNames.begin(), serverNames.end(), host) == serverNames.end())
	{
		// std::cout << "SERVER NAME: " << host << " NOT FOUND IN SERVER_NAME" << std::endl;
		return false;
	}
	return true;
}

bool StartServers::isValidRequest(UserRequest requestData, std::string &status, bool isCGI, Server server)
{
    std::string httpRequest = requestData.fullStr;

    std::size_t pos = 0;
    std::string line;

    int countPost = 0;
    int countOther = 0;
    std::string method, uri, version;
	// std::cout << RED << status << DEFAULT;

	if (!isValidServerName(requestData.fullStr, server))
	{
		std::cout << RED << "NOT VALID SERVER NAME" << DEFAULT << std::endl;
		status = "400";
		return false;
	}
	std::cout << RED << status << DEFAULT;

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
		if (line.compare(0, 5, "Host:") == 0 || line.compare(0, 5, "host:") == 0)
		{
			countPost++;
		    countOther++;
		}
		else if (line.compare(0, 18, "Transfer-Encoding:") == 0 || line.compare(0, 18, "transfer-encoding:") == 0)
		{
		    if (lenghtFind == true)
		    {
		        status = "400";
		        return false;
		    }

		    lenghtFind = true;
		    countPost++;
		}
		else if (line.compare(0, 15, "Content-Length:") == 0 || line.compare(0, 15, "content-length:") == 0)
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
		
		    if (!(lengthStream >> contentLength))
		    {
		        status = "400";
		        return false;
		    }
		
		    if (contentLength < 0)
		    {
		        status = "400";
		        return false;
		    }
		
		    if (contentLength > 0 && (method == "DELETE" || method == "GET"))
		    {
		        status = "400";
		        return false;
		    }
		}
		else if (line.compare(0, 13, "Content-Type:") == 0 || line.compare(0, 13, "content-type:") == 0)
		{
		    countPost++;
		}

        if (line.length() > 8000)
		{
			status = "414";
			return false;
		}

        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos)
		{
            std::string headerKey = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            if (headerKey.empty() || headerKey.find_first_not_of(' ') == std::string::npos)
			{
				status = "400";
				return false;
			}

			if (colonPos > 0 && line[colonPos - 1] == ' ') {
        		status = "400";
        		return false;
    		}

			// if (line.length() > colonPos + 1 && line[colonPos + 1] != ' ')
			// {
			// 	status = "400";
			// 	return false;
            // }

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