#pragma once

#include "Server.hpp"

struct UserRequest {
    std::string fullStr;

    std::string method;
    std::string route;

    bool isHeaderComplete;
    bool isBodyComplete;
    int contentLength;
    int length;
};

struct Client
{
    int fd;
    Server *server;
    UserRequest request;
};

class ClientRequest
{
	private:
        std::string _fullStr;
        std::string method;
        std::string route;

        bool isHeaderComplete;
        bool isBodyComplete;
        int contentLength;
        int length;

	public:
        ClientRequest();
        ~ClientRequest() {}

};