#include "StartServers.hpp"
#include "ClientRequest.hpp"

// -------------------------------- Body -------------------------------- //

int getBodysize(std::string requestStr)
{
	size_t startPos = requestStr.find("\r\n\r\n");

    if (startPos == std::string::npos)
        return 0;

    startPos += std::strlen("\r\n\r\n");
    std::string body = requestStr.substr(startPos);

    return body.size();
}

int getHeaderContentLength(std::string requestStr)
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

std::string getHeaderRequestMethod(std::string requestStr)
{
    size_t methodEndPos = requestStr.find(" ");
    if (methodEndPos != std::string::npos)
        return requestStr.substr(0, methodEndPos);
    return "";
}

std::string getHeaderRequestRoute(std::string requestStr)
{
    size_t startPos = requestStr.find(" ");
    if (startPos != std::string::npos)
    {
        startPos++;
        size_t endPos = requestStr.find(" ", startPos);
        return requestStr.substr(startPos, endPos - startPos);
    }
    return "";
}

std::string getHeaderRequestTransferEncoding(std::string requestStr)
{
    size_t transferStartPos = requestStr.find("Transfer-Encoding: ");
    if (transferStartPos != std::string::npos)
    {
        transferStartPos += std::strlen("Transfer-Encoding: ");
        size_t transferEndPos = requestStr.find("\r\n", transferStartPos);
        return requestStr.substr(transferStartPos, transferEndPos - transferStartPos);
    }
    return "default";
}

bool isLastChunkReceived(std::string requestStr)
{
    if (requestStr.find("\r\n0\r\n\r\n") != std::string::npos)
        return true;
    return false;
}

// -------------------------------- COOKIE / AUTH -------------------------------- //

std::vector<std::string> getHeaderCookie(std::string requestStr)
{
    std::vector<std::string> cookies;
    size_t startPos, endPos;
    std::string cookieSubStr, cookie;

    startPos = requestStr.find("Cookie: ");
    if (startPos == std::string::npos)
        return cookies;

    startPos += std::strlen("Cookie: ");
    endPos = requestStr.find("\r\n", startPos);
    if (endPos == std::string::npos)
        return cookies;

    cookieSubStr = requestStr.substr(startPos, endPos - startPos);
    startPos = 0;
    endPos = cookieSubStr.find("; ");
    while (endPos != std::string::npos)
    {
        cookie = cookieSubStr.substr(startPos, endPos - startPos);
        cookies.push_back(cookie);
        startPos = endPos + std::strlen("; ");
        endPos = cookieSubStr.find(";", startPos);
    }
    cookie = cookieSubStr.substr(startPos);
    cookies.push_back(cookie);
    return cookies;
}

std::string getHeaderAuth(std::string requestStr)
{
 	size_t authPos = requestStr.find("Authorization:");
    if (authPos != std::string::npos)
    {
        std::string authHeader = requestStr.substr(authPos);

        if (authHeader.find("Basic ") != std::string::npos)
        {
			size_t startPos = authHeader.find("Basic ") + 6;
			size_t endPost = authHeader.find("\r\n");
			std::string value = authHeader.substr(startPos, endPost - startPos);
			if (!value.empty())
				return value;
        }
    }
	return "";
}

// -------------------------------- GETTER -------------------------------- //

std::string getHeaderContentType(std::string requestStr)
{
    size_t startPos, endPos;

    startPos = requestStr.find("Content-Type: ");
    if (startPos == std::string::npos)
        return "";
    startPos += std::strlen("Content-Type: ");
    endPos = requestStr.find(";", startPos);
    if (endPos == std::string::npos)
    {
        endPos = requestStr.find("\r\n", startPos);
        if (endPos == std::string::npos)
            return "";
    }
    return requestStr.substr(startPos, endPos - startPos);
}

void StartServers::getRequestChunk(Client &client, std::string chunkStr)
{
	client.request.fullStr += chunkStr;

    if (!client.request.isHeaderComplete) // if header was not complete yet, check if it is now
    {
        client.request.isHeaderComplete = isHeaderComplete(client.request.fullStr);
        if (!client.request.isHeaderComplete) // if header is still not complete, stop here ; else, get and set all header data
            return;
        client.request.method = getHeaderRequestMethod(client.request.fullStr);
        client.request.route = getHeaderRequestRoute(client.request.fullStr);
        client.request.transferEncoding = getHeaderRequestTransferEncoding(client.request.fullStr);
        client.request.contentType = getHeaderContentType(client.request.fullStr);
        client.request.contentLength = getHeaderContentLength(client.request.fullStr);
        client.request.isCGI = isCGIFile(*(client.server), client.request.route);
        client.request.cookies = getHeaderCookie(client.request.fullStr);
		client.request.credential = getHeaderAuth(client.request.fullStr);
    }

    if (client.request.method == "POST")
    {
        client.request.bodySize = getBodysize(client.request.fullStr);

        if (!client.request.isCGI && client.request.bodySize > client.server->getMaxClientBodySize())
            throw MaxClientBodySizeExceed();

        if (client.request.transferEncoding == "default" && client.request.bodySize == client.request.contentLength)
            client.request.isBodyComplete = true;
        else if (client.request.transferEncoding == "chunked" && isLastChunkReceived(client.request.fullStr))
            client.request.isBodyComplete = true;
    }
    else
    {
        if (client.request.isHeaderComplete) // set isBodyComplete to true because GET and DELETE methods don't need a body
            client.request.isBodyComplete = true;
    }
}

// This is how we process the request:

// - We read the client fd to receive the request as a string in the buffer.
// - If the size of bytes read is 0 then either the client disconnected or an error occured.
// - If the size is not 0, request was received successfully.
// - We parse the request string and put all the data in the client.request struct.
// - While parsing the request, we check that the request is complete (different methods for GET, POST OR DELETE).
// - If the request is complete, we open EPOLLOUT which means the request is ready for a response.
// - If the request is NOT complete, we let EPOLLIN open so the next part can be received.

void StartServers::processRequest(epoll_event currentEvent)
{
    int chunkSize = 65536;
    char buffer[chunkSize];
    ssize_t bytesRead;
    struct epoll_event event;
    Client &currentClient = _clientList[currentEvent.data.fd];
    Server currentServer = *(currentClient.server);

    // std::cout << "----------------------- NEW REQUEST: " << currentEvent.data.fd << " -----------------------" << std::endl;
    std::cout << YELLOW << "[R] New request received from client " << currentEvent.data.fd << DEFAULT << std::endl;

    currentClient.lastActionDate = getDate(); // update timeout

    bytesRead = read(currentEvent.data.fd, buffer, chunkSize);
    if (bytesRead <= 0) // error case
    {
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
        close(currentEvent.data.fd);
        _clientList.erase(currentEvent.data.fd);
        std::cout << RED << "[i] Client disconnected from error: " << currentEvent.data.fd << DEFAULT << std::endl;
        return;
    }

    std::string requestData(buffer, bytesRead);
    print(requestData);

    try
    {
        getRequestChunk(currentClient, requestData);
    }
    catch (std::exception &e) 
    {
        std::cout << RED << e.what() << DEFAULT << std::endl;
        currentClient.request.isHeaderComplete = true;
        currentClient.request.isBodyComplete = true;
		currentClient.request.isBodyTooLarge = true;
    }

    if (!currentClient.request.isHeaderComplete || !currentClient.request.isBodyComplete) // not opening EPOLLOUT if request is not fully complete
        return;

    currentClient.response = generateResponse(currentServer, currentClient);

    int fd = open("test.txt", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    epoll_event testEvent;
    testEvent.data.fd = fd;
    testEvent.events = EPOLLOUT;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == 0)
        std::cout << "file added to epoll: " << fd << std::endl;
    else
        std::cout << "error while adding file to epoll: " << fd << " " << strerror(errno) << std::endl;

    // if (currentClient.filesToCreate.size() > 0)
    // {
    //     for (std::vector<FileToCreate>::iterator it = currentClient.filesToCreate.begin() ; it != currentClient.filesToCreate.end() ; it++)
    //     {
    //         _clientList[it->fd] = currentClient;
    //         event.data.fd = it->fd;
    //         event.events = EPOLLOUT;
    //         if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, it->fd, &event) == 0)
    //             std::cout << "file added to epoll: " << it->fd << std::endl;
    //         else
    //             std::cout << "error while adding file to epoll: " << it->fd << " " << strerror(errno) << std::endl;
    //     }
    //     return;
    // }

    event.data.fd = currentEvent.data.fd;
    event.events = EPOLLOUT;
    epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
}