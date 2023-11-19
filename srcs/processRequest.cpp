#include "StartServers.hpp"
#include "ClientRequest.hpp"

int getBodysize(std::string requestStr)
{
	size_t bodyStartPos = requestStr.find("\r\n\r\n");
    if (bodyStartPos == std::string::npos)
        return 0;

    std::string body = requestStr.substr(bodyStartPos + 4);

	return body.size();
}

int getContentLength(std::string requestStr)
{
    size_t contentLengthStartPos = requestStr.find("Content-Length: ");
    if (contentLengthStartPos == std::string::npos)
        return 0;
    contentLengthStartPos += std::strlen("Content-Length: ");

    size_t contentLengthEndPos = requestStr.find("\r", contentLengthStartPos);
    if (contentLengthEndPos == std::string::npos)
        return 0;

    std::string contentLength = requestStr.substr(contentLengthStartPos, contentLengthEndPos - contentLengthStartPos);

    return atoi(contentLength.c_str());
}

bool isHeaderComplete(std::string requestStr)
{
    if (requestStr.find("\r\n\r\n") == std::string::npos) // header file end with an empty line containing "\r\n"
        return false;
    return true;
}

void StartServers::getRequestChunk(UserRequest &request, std::string requestStr)
{
	std::cout << YELLOW << "IS A NEW REQUEST : \n" << requestStr << "\n" << DEFAULT << std::endl;

	request.fullStr += requestStr;
    if (!request.isHeaderComplete) // if header was not complete yet, check if he is now
        request.isHeaderComplete = isHeaderComplete(request.fullStr);

    if (request.isHeaderComplete)
    {
        if (request.method.empty())
        {
            size_t methodEndPos = request.fullStr.find(" ");
            if (methodEndPos != std::string::npos)
                request.method = request.fullStr.substr(0, methodEndPos);
        }

        if (request.method == "POST")
        {
            if (request.contentLength == 0)
                request.contentLength = getContentLength(request.fullStr);
            std::cout << RED << "CONTENT-L: " << request.contentLength << DEFAULT << std::endl;
            request.length = getBodysize(request.fullStr);
            // if (request.contentLength > maxBodySize)
            //     throw error;
            if (request.isHeaderComplete && request.length == request.contentLength)
                request.isBodyComplete = true;
        }
        else
        {
            if (request.isHeaderComplete)
                request.isBodyComplete = true;
        }

    }
}

void StartServers::processRequest(epoll_event currentEvent)
{
    int chunkSize = 1024;
    char buffer[chunkSize];
    ssize_t bytesRead;
    struct epoll_event event;
    Client &currentClient = _clientList[currentEvent.data.fd];

    std::cout << "----------------------- NEW REQUEST: " << currentEvent.data.fd << " -----------------------" << std::endl;
    bytesRead = read(currentEvent.data.fd, buffer, chunkSize);

    if (bytesRead <= 0) // error case
    {
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, &event);
        close(currentEvent.data.fd);
        _clientList.erase(currentEvent.data.fd);
        std::cout << "Client disconnected from error: " << currentEvent.data.fd << std::endl;
    }
    else
    {
        std::string requestData(buffer, bytesRead);
        getRequestChunk(currentClient.request, requestData); // receive chunk of the request (depending on chunkSize var)

        if (!currentClient.request.isHeaderComplete || !currentClient.request.isBodyComplete) // not opening EPOLLOUT if request is not fully complete
        {
            std::cout << "REQUEST UNCOMPLETE YET: " << currentClient.request.length << "/" << currentClient.request.contentLength << std::endl;
            return;
        }

        std::cout << "REQUEST COMPLETE: " << currentClient.request.length << "/" << currentClient.request.contentLength << std::endl;
        event.data.fd = currentEvent.data.fd;
        event.events = EPOLLOUT;
        epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
    }
}