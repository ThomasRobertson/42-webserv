#include "StartServers.hpp"
#include "ClientRequest.hpp"

int hexStringToInt(std::string hexString)
{
    std::istringstream iss(hexString);
    int intValue;

    iss >> std::hex >> intValue;

    // if (iss.fail() || !iss.eof()) {
    //     // Conversion failed
    //     throw std::invalid_argument("Invalid hexadecimal string");
    // }

    return intValue;
}

int getBodysize(std::string requestStr, std::string transferEncoding, UserRequest &request)
{
	size_t startPos = requestStr.find("\r\n\r\n");
    if (startPos == std::string::npos)
        return 0;
    startPos += std::strlen("\r\n\r\n");

    if (transferEncoding == "default")
    {
        std::string body = requestStr.substr(startPos);
	    return body.size();
    }
    else
    {
        int size = 0;
        std::string sizeStr;

        while (true)
        {
            size_t endPos = requestStr.find("\r\n", startPos);
            if (endPos == std::string::npos)
                break;
            sizeStr = requestStr.substr(startPos, endPos - startPos);
            if (sizeStr == "0")
            {
                request.isBodyComplete = true; // if a chunk of size 0 is found, body is complete
                break;
            }
            size += hexStringToInt(sizeStr);
            startPos = endPos + std::strlen("\r\n");
            startPos = requestStr.find("\r\n", startPos);
            if (startPos == std::string::npos)
                break;
            startPos += std::strlen("\r\n");
        }
        return size;
    }
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

void StartServers::getRequestChunk(UserRequest &request, std::string requestStr)
{


	std::cout << YELLOW << "IS A NEW REQUEST : \n" << requestStr << "\n" << DEFAULT << std::endl;

    //Cookies
    // size_t pos = requestStr.find("Cookie:");
    // if (pos != std::string::npos)
    // {
    //     std::string cookieSubstr = requestStr.substr(pos + 8);
    //     std::string delimiter = "; ";
    //     size_t start = 0;
    //     size_t end = cookieSubstr.find(delimiter);
        
    //     while (end != std::string::npos)
    //     {
    //         std::string cookie = cookieSubstr.substr(start, end - start);
    //         request.cookies.push_back(cookie);
    //         start = end + delimiter.length();
    //         end = cookieSubstr.find(delimiter, start);
    //     }
        
    //     std::string lastCookie = cookieSubstr.substr(start);
    //     request.cookies.push_back(lastCookie);
    // }

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

	request.fullStr += requestStr;
    if (!request.isHeaderComplete) // if header was not complete yet, check if he is now
        request.isHeaderComplete = isHeaderComplete(request.fullStr);

    if (request.isHeaderComplete)
    {
        if (request.method.empty())
            request.method = getRequestMethod(request);

        if (request.route.empty())
            request.route = getRequestRoute(request);

        if (request.method == "POST")
        {
            if (request.transferEncoding.empty())
                request.transferEncoding = getRequestTransferEncoding(request);
            if (request.contentLength == -1)
                request.contentLength = getContentLength(request.fullStr);
            request.length = getBodysize(request.fullStr, request.transferEncoding, request);
            // std::cout << RED << "CONTENT-L: " << request.contentLength << DEFAULT << std::endl;
            // std::cout << RED << "CONTENT-E: " << request.transferEncoding << DEFAULT << std::endl;
            std::cout << RED << "BODYSIZE: " << request.length << DEFAULT << std::endl;
            // if (request.length > maxBodySize)
            //     throw error;
            if (request.transferEncoding == "default" && request.length == request.contentLength)
                request.isBodyComplete = true;
        }
        else
        {
            if (request.isHeaderComplete) // set bodyComplete to true because GET method doesn't need a body
                request.isBodyComplete = true;
        }
    }
}

void StartServers::processRequest(epoll_event currentEvent)
{
    int chunkSize = 16384;
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
            std::cout << "REQUEST UNCOMPLETE YET" << std::endl;
            return;
        }

        std::cout << "REQUEST COMPLETE" << std::endl;
        event.data.fd = currentEvent.data.fd;
        event.events = EPOLLOUT;
        epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
    }
}