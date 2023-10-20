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
        void listenClientRequest(int serverSocket);

        int testServer(std::string hostStr, std::string portStr);
        int testListenClientRequest(int serverSocket, int epollFd);
        void epollOut(UserRequest &userRequest, std::string &response, int &clientSocket);
        void epollIn(UserRequest &userRequest, int &clientSocket);
};

struct UserRequest {
    std::string method;
    std::string root;
};

std::string getUserResponse(UserRequest userRequest, ConfigFile configFile);
UserRequest getUserRequest(std::string requestStr);

#endif
