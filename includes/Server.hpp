#ifndef SERVER_HPP
# define SERVER_HPP

#include "ConfigFile.hpp"
#include "Settings.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <algorithm>

#include "Settings.hpp"
#include "utils.hpp"

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
        std::string getFileName(std::string fileName);

        int getServerSocket(int i);
        int getServerSocketSize();

        //std::string getErrorPage(std::string errorCode);
        std::string getCgiPage(std::string cgiName);
		std::pair<std::string, page> getRootDir(std::string url);
        std::string getFileRoute(std::string fileName, std::string &status, std::string method);
		std::string getErrorPageRoute(std::string errorCode);
		std::string testAccessPath(std::string location, std::string method);

        void setServerValues();

        void startServers(int epollFd);
        void addSocketToEpoll(int epollFd, int serverSocket);
        int acceptNewClient(int epollFd, int serverIndex);


};

#endif
