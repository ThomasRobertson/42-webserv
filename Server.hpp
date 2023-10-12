#ifndef SERVER_HPP
# define SERVER_HPP

#include "ConfigFile.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>

class Server
{
    public:
        Server();
        ~Server();

		int testServer(std::string host, std::string port);
};

#endif
