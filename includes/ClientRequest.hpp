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
    int length;
};

// class ClientRequest
// {
// 	private:
//         std::string _fullStr;
//         std::string method;
//         std::string route;

//         bool isHeaderComplete;
//         bool isBodyComplete;
//         int contentLength;
//         int length;

// 	public:
//         ClientRequest();
//         ~ClientRequest() {}

// };