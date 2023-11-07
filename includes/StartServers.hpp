#ifndef STARTSERVERS_HPP
# define STARTSERVERS_HPP

#include "ConfigFile.hpp"
#include "Server.hpp"
#include "Settings.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <csignal>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <iomanip>

extern bool EXIT_G;

struct UserRequest {
    std::string method;
    std::string root;
    std::string body;

    int contentLength;
    int length;
};

struct Client
{
    int fd;
    int serverIndex;
    UserRequest request;
    bool toComplete;
};

class StartServers
{
    private:
        ConfigFile _configFile;
	    std::vector<Server>	_serversVec;
        
        std::map<int, Client> _clientList;

        int _epollFd;

    public:
        StartServers(ConfigFile configFile);
        ~StartServers();

		void createServers();
        void initServers();

        bool getNewConnexion(epoll_event currentEvent);
        void processRequest(epoll_event currentEvent);
        void processResponse(epoll_event currentEvent);
        void closeServers();

        void listenClientRequest();

        void getRequestNextChunk(int userFd, std::string requestStr);
        std::string getUserResponse(Client client);
        UserRequest getUserRequest(std::string requestStr);
};

class Problem : public std::exception
{
    public:
        virtual const char *what() const throw()
        {
            return ("Problem");
        }
};

#endif
