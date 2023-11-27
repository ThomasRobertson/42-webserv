#include "StartServers.hpp"

int getBodysize(std::string requestStr)
{
	size_t bodyStartPos = requestStr.find("\r\n\r\n") + 4;
	std::string body = requestStr.substr(bodyStartPos);
	return body.size();
}

int getContentLength(std::string requestStr)
{
		size_t contentLengthStartPos = requestStr.find("Content-Length:") + 16; // 16 being the length of "Content-Length: "
		size_t contentLengthEndPos = requestStr.find("\r", contentLengthStartPos);
		std::string contentLength = requestStr.substr(contentLengthStartPos, contentLengthEndPos - contentLengthStartPos);
		return atoi(contentLength.c_str());
}

UserRequest StartServers::getUserRequest(std::string requestStr)
{
    UserRequest request;

    request.fullRequest = requestStr;
	std::cout << YELLOW << "IS A NEW REQUEST :\n" << requestStr << "\n" << DEFAULT << std::endl;




    //Cookies
    size_t pos = requestStr.find("Cookie:");
    if (pos != std::string::npos)
    {
        std::string cookieSubstr = requestStr.substr(pos + 8);
        std::string delimiter = "; ";
        size_t start = 0;
        size_t end = cookieSubstr.find(delimiter);
        
        while (end != std::string::npos)
        {
            std::string cookie = cookieSubstr.substr(start, end - start);
            request.cookies.push_back(cookie);
            start = end + delimiter.length();
            end = cookieSubstr.find(delimiter, start);
        }
        
        std::string lastCookie = cookieSubstr.substr(start);
        request.cookies.push_back(lastCookie);
    }



    //Authorization
    size_t authPos = requestStr.find("Authorization:");
    if (authPos != std::string::npos)
    {
        std::string authHeader = requestStr.substr(authPos);

        if (authHeader.find("Basic") != std::string::npos)
        {
            size_t spacePos = authHeader.find(' ');
            if (spacePos != std::string::npos)
                std::string credentials = authHeader.substr(spacePos + 1);
        }
    }



    size_t spaceSepPos = requestStr.find(" "); // first space char after "GET /scripts/script.js HTTP/1.1"
    request.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    request.root = requestStr.substr(0, spaceSepPos);

	if (request.method == "POST")
	{
		request.contentLength = getContentLength(requestStr);
		request.body = requestStr;
		request.length = getBodysize(request.body);
		std::cout << RED << request.body << DEFAULT << std::endl;
		std::cout << CYAN << request.length << DEFAULT << std::endl;
	}
	else
	{
		request.contentLength = 0;
		request.length = 0;
	}

    return request;
}

void StartServers::getRequestNextChunk(int userFd, std::string requestStr)
{
	std::cout << YELLOW << "IS UNCOMPLETE REQUEST" << DEFAULT << std::endl;
	_clientList[userFd].request.body += requestStr;
	_clientList[userFd].request.length = getBodysize(_clientList[userFd].request.body);
	std::cout << RED << _clientList[userFd].request.body << DEFAULT << std::endl;
	std::cout << CYAN << _clientList[userFd].request.length << DEFAULT << std::endl;
}

void StartServers::processRequest(epoll_event currentEvent)
{
    char buffer[131072]; // set to maxBodySize and read
    ssize_t bytesRead;
    struct epoll_event event;

    std::cout << "----------------------- NEW REQUEST: " << currentEvent.data.fd << " -----------------------" << std::endl;
    bytesRead = read(currentEvent.data.fd, buffer, 131072);

    if (bytesRead <= 0)
    {
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, &event);
        close(currentEvent.data.fd);
        _clientList.erase(currentEvent.data.fd);
        std::cout << "Client disconnected from error: " << currentEvent.data.fd << std::endl;
    }
    else
    {
        std::string requestData(buffer, bytesRead);
        Client currentClient = _clientList[currentEvent.data.fd];

        if (_clientList[currentEvent.data.fd].toComplete)
            getRequestNextChunk(currentEvent.data.fd, requestData);
        else
            _clientList[currentEvent.data.fd].request = getUserRequest(requestData);

        if (_clientList[currentEvent.data.fd].request.length != _clientList[currentEvent.data.fd].request.contentLength) // not opening EPOLLOUT if request is not complete
        {
            std::cout << "REQUEST UNCOMPLETE YET: " << _clientList[currentEvent.data.fd].request.length << "/" << _clientList[currentEvent.data.fd].request.contentLength << std::endl;
            _clientList[currentEvent.data.fd].toComplete = true;
            return;
        }

        event.data.fd = currentEvent.data.fd;
        event.events = EPOLLOUT;
        epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
    }
}