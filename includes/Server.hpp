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

        struct epoll_event _event;

        int _clientAddr;
        int _clientSocket;
        socklen_t _clientAddrLen;
        std::vector<int> _clientsVec;

        std::string _response;
        UserRequest _userRequest;


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

        std::string getUserResponse(UserRequest userRequest);
        UserRequest getUserRequest(std::string requestStr);

        int getClient(int index);
        int getClientsVecSize();

        void setServerValues();

        int startServers();
        int addSocketToEpoll(int epollFd);
        int getServerSocket();
        void acceptNewClient(int epollFd);
        void newResponseAndRequest(int epollFd, epoll_event &events, int clientIndex);


};

#endif
