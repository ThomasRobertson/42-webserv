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
        std::vector<std::string> _server_name;
        int _maxClientBodySize;
        int _serverIndex;
        std::string _root;
        std::string _postRoot;

        std::map<std::string, Location> _htmlLocationMap;
        std::map<std::string, std::string> _errorsMap;
        std::map<std::string, std::string> _cgiMap;

        std::vector<int> _serverSocketVec;

    public:
        Server(ConfigFile configFile, int serverIndex);
        ~Server();

        std::string getHost();
        std::string getRoot();
        int getMaxClientBodySize();
        std::vector<std::string> getPort();
        bool getListing(std::string fileLocation);
        std::string getFileName(std::string fileName);
        std::string getPostRoot(std::string fileName);
        Location getLocation(std::string route);
        std::vector<std::string> getServerName();

        int getServerSocket(int i);
        int getServerSocketSize();

        std::map<std::string, std::string> getCgiPages();
		std::pair<std::string, Location> getRootDir(std::string url);
        std::string getFileRoute(std::string fileName, std::string &status, std::string method, bool &is_dir, bool isCGI = false);
		std::string getErrorPageRoute(std::string errorCode);
		std::string testAccessPath(std::string location, std::string method);

        void setServerValues();

        void startServers(int epollFd);
        void addSocketToEpoll(int epollFd, int serverSocket);
        int acceptNewClient(int epollFd, int serverIndex);


};

#endif