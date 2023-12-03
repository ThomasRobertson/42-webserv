#pragma once

#include "Server.hpp"

struct UserRequest {
    std::string fullStr;
    std::string fullRequest;
    std::string body;
    std::string credential;
    std::vector<std::string> cookies;

    std::string method;
    std::string route;
    std::string transferEncoding;

    bool isHeaderComplete;
    bool isBodyComplete;
    int contentLength;
    int bodySize;
};

class MaxClientBodySizeExceed : public std::exception
{
    public:
        virtual const char* what() const throw() {return "Body size limit exceed !";};
};