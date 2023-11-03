#ifndef SERVER_HPP
# define SERVER_HPP

#include "ConfigFile.hpp"
#include "Settings.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "Settings.hpp"

struct UserRequest {
    std::string method;
    std::string root;
};

struct Client;

class Server
{
    private:
        ConfigFile _configFile;
        std::string _host;
        std::vector<std::string> _port;
        int _maxClientBodySize;
        int _serverIndex;
        std::string _root;
        std::string _server_name;

        std::map<std::string, page> _htmlPageMap;
        std::map<std::string, std::string> _errorsMap;
        std::map<std::string, std::string> _cgiMap;

        std::vector<int> _serverSocketVec;

    public:
        Server(ConfigFile configFile, int serverIndex);
        ~Server();

        std::string getHost();
        std::vector<std::string> getPort();
        int getMaxClientBodySize();
        std::string getServerName();
        std::string getRoot();
        bool getListing(std::string fileLocation);


        int getServerSocket(int i);
        int getServerSocketSize();

        //std::string getErrorPage(std::string errorCode);
        std::string getCgiPage(std::string cgiName);
        std::string getFileRoute(std::string fileName, std::string &status, std::string method);
		std::string getErrorPageRoute(std::string errorCode);

        void setServerValues();

        void startServers(int epollFd);
        int addSocketToEpoll(int epollFd, int i);
        void acceptNewClient(int epollFd, int y, Client &newClient);


};

#endif
