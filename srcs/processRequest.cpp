#include "StartServers.hpp"
#include "ClientRequest.hpp"

int getBodysize(std::string requestStr)
{
	size_t startPos = requestStr.find("\r\n\r\n");

    if (startPos == std::string::npos)
        return 0;

    startPos += std::strlen("\r\n\r\n");
    std::string body = requestStr.substr(startPos);

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

std::string getRequestMethod(UserRequest request)
{
    size_t methodEndPos = request.fullStr.find(" ");
    if (methodEndPos != std::string::npos)
        return request.fullStr.substr(0, methodEndPos);
    return "";
}

std::string getRequestRoute(UserRequest request)
{
    size_t startPos = request.fullStr.find(" ");
    if (startPos != std::string::npos)
    {
        startPos++;
        size_t endPos = request.fullStr.find(" ", startPos);
        return request.fullStr.substr(startPos, endPos - startPos);
    }
    return "";
}

std::string getRequestTransferEncoding(UserRequest request)
{
    size_t transferStartPos = request.fullStr.find("Transfer-Encoding: ");
    if (transferStartPos != std::string::npos)
    {
        transferStartPos += std::strlen("Transfer-Encoding: ");
        size_t transferEndPos = request.fullStr.find("\r\n", transferStartPos);
        return request.fullStr.substr(transferStartPos, transferEndPos - transferStartPos);
    }
    return "default";
}

bool isLastChunkReceived(std::string str)
{
    if (str.find("\r\n0\r\n\r\n") != std::string::npos)
        return true;
    return false;
}

void StartServers::getRequestChunk(Client &client, std::string requestStr)
{
    // Cookies
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
            client.request.cookies.push_back(cookie);
            start = end + delimiter.length();
            end = cookieSubstr.find(delimiter, start);
        }
        
        std::string lastCookie = cookieSubstr.substr(start, cookieSubstr.find_first_of("\n\r"));
        client.request.cookies.push_back(lastCookie);
    }

    // Authorization
    // size_t authPos = requestStr.find("Authorization:");
    // if (authPos != std::string::npos)
    // {
    //     std::string authHeader = requestStr.substr(authPos);

    //     if (authHeader.find("Basic") != std::string::npos)
    //     {
    //         size_t spacePos = authHeader.find(' ');
    //         if (spacePos != std::string::npos)
    //             std::string credentials = authHeader.substr(spacePos + 1);
    //     }
    // }

	client.request.fullStr += requestStr;

    if (!client.request.isHeaderComplete) // if header was not complete yet, check if it is now
        client.request.isHeaderComplete = isHeaderComplete(client.request.fullStr);

    if (!client.request.isHeaderComplete) // if header is still not complete, stop here
        return;

    if (client.request.method.empty())
        client.request.method = getRequestMethod(client.request);

    if (client.request.route.empty())
        client.request.route = getRequestRoute(client.request);

    if (client.request.method == "POST")
    {
        if (client.request.transferEncoding.empty())
            client.request.transferEncoding = getRequestTransferEncoding(client.request);

        if (client.request.contentLength == -1)
            client.request.contentLength = getContentLength(client.request.fullStr);

        if (!client.request.isCGI)
            client.request.isCGI = isCGIFile(*(client.server), client.request.route);

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
    // int chunkSize = 65536;
    int chunkSize = 1048576;
    char buffer[chunkSize];
    ssize_t bytesRead;
    struct epoll_event event;
    Client &currentClient = _clientList[currentEvent.data.fd];

    // std::cout << "----------------------- NEW REQUEST: " << currentEvent.data.fd << " -----------------------" << std::endl;

    currentClient.lastActionDate = getDate(); // update timeout

    bytesRead = read(currentEvent.data.fd, buffer, chunkSize);
    if (bytesRead <= 0) // error case
    {
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, &event);
        close(currentEvent.data.fd);
        _clientList.erase(currentEvent.data.fd);
        std::cout << "Client disconnected from error: " << currentEvent.data.fd << std::endl;
        return;
    }

    std::string requestData(buffer, bytesRead);
    // std::cout << requestData << std::endl;

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
    {
        // std::cout << RED << "REQUEST UNCOMPLETE" << DEFAULT << std::endl;
        // std::cout << currentClient.request.bodySize << "/" << currentClient.request.contentLength << std::endl;
        return;
    }

    std::cout << GREEN << "REQUEST COMPLETE" << DEFAULT << std::endl;
    event.data.fd = currentEvent.data.fd;
    event.events = EPOLLOUT;
    epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
}

// To check:
// - Multiple files in one POST request
// - Body in DELETE request
// - Chunked response