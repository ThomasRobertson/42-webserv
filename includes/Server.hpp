#ifndef SERVER_HPP
# define SERVER_HPP

#include "ConfigFile.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>

class Server
{
    private:
        ConfigFile _configFile;

    public:
        Server(ConfigFile configFile);
        ~Server();

		int startServer();
        void listenClientRequest(int serverSocket);
};

struct UserRequest {
    std::string method;
    std::string root;
};

std::string manageUserResponse(UserRequest userRequest, ConfigFile configFile);
UserRequest getUserRequest(std::string requestStr);

#endif
