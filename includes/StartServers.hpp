#ifndef STARTSERVERS_HPP
# define STARTSERVERS_HPP

#include "ConfigFile.hpp"
#include "Server.hpp"
#include "Settings.hpp"
#include "ClientRequest.hpp"

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

// struct Client
// {
//     int fd;
//     int serverIndex;
//     UserRequest request;
//     bool toComplete;
// };

struct Client
{
    int fd;

    Server *server;
    UserRequest request;

    time_t lastActionDate;
};

class StartServers
{
    private:
        ConfigFile _configFile;
	    std::vector<Server>	_serversVec;
        
        std::map<int, Client> _clientList;

        int _epollFd;

		bool isCGIFile(Server server, std::string request);

    public:
        StartServers(ConfigFile configFile);
        ~StartServers();

		void createServers();
        void initServers();

        void checkTimeout();
        bool getNewConnexion(epoll_event currentEvent);
        void processRequest(epoll_event currentEvent);
        void processResponse(epoll_event currentEvent);
        void closeServers();

        void listenClientRequest();

        void getRequestChunk(UserRequest &request, std::string requestStr, int maxBodySize);
        UserRequest getUserRequest(std::string requestStr);
        std::string getUserResponse(Client &client);


        void createFile(UserRequest request, Server currentServer);
        int deleteFiles(UserRequest request, Server currentServer);
        bool isValidRequest(UserRequest requestData, std::string &status, bool isCGI);
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
