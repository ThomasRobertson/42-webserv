#ifndef SERVER_HPP
# define SERVER_HPP

#include "ConfigFile.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

struct UserRequest {
    std::string method;
    std::string root;
};

class Server
{
    private:
        ConfigFile _configFile;
        std::string _host;
        std::vector<std::string> _port;
        int _maxClientBodySize;
        int _serverIndex;

        std::map<std::string, page> _htmlPageMap;
        std::map<std::string, std::string> _errorsMap;
        std::map<std::string, std::string> _cgiMap;

        int _serverSocket;
        struct addrinfo _hints;
        struct addrinfo *_result;
        struct addrinfo *_rp;
        UserRequest _userRequest;
        std::string _response;
        int _clientAddr;
        int _clientSocket;
        struct epoll_event _events[10];
        socklen_t _clientAddrLen;

    public:
        Server(ConfigFile configFile, int serverIndex);
        ~Server();

        std::string getHost();
        std::vector<std::string> getPort();
        int getMaxClientBodySize();
        int getServerIndex();

        std::string getErrorPage(std::string errorCode);
        std::string getCgiPage(std::string cgiName);
        std::string getFileRoute(std::string location, std::string &status);

        std::string getUserResponse(UserRequest userRequest, ConfigFile configFile);
        UserRequest getUserRequest(std::string requestStr);

        void setServerValues();



};

#endif
