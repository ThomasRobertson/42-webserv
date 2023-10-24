#ifndef SERVER_HPP
# define SERVER_HPP

#include "ConfigFile.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

struct UserRequest;

class Server
{
    private:
        ConfigFile _configFile;

    public:
        Server(ConfigFile configFile);
        ~Server();

		int startServer();
        int listenClientRequest(int serverSocket, int epollFd);
};

struct UserRequest {
    std::string method;
    std::string root;
};

std::string getUserResponse(UserRequest userRequest, ConfigFile configFile, int serverIndex);
UserRequest getUserRequest(std::string requestStr);

#endif
