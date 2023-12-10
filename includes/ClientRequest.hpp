#pragma once

#include "Server.hpp"

struct UserRequest {
    std::string fullStr;
    std::string credential;
    std::vector<std::string> cookies;

    std::string method;
    std::string route;
    std::string transferEncoding;
    std::string contentType;

    bool isHeaderComplete;
    bool isBodyComplete;
	bool isBodyTooLarge;
    bool isCGI;

    int contentLength;
    int bodySize;
};

class MaxClientBodySizeExceed : public std::exception
{
    public:
        virtual const char* what() const throw() {return "Body size limit exceed !";};
};

bool isValidRequest(UserRequest request);