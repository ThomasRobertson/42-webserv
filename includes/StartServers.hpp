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

extern bool EXIT_G;

struct Client
{
    int fd;
    int serverIndex;
    UserRequest request;
};

class StartServers
{
    private:
        ConfigFile _configFile;
	    std::vector<Server>	_serversVec;
        
        std::vector<int> _clientsVec;
        std::map<int, Client> _clientList;

        int _epollFd;

    public:
        StartServers(ConfigFile configFile);
        ~StartServers();

		void createServers();
        void initServers();

        bool getNewConnexion(epoll_event currentEvent);
        void receiveRequest(epoll_event currentEvent);
        void sendResponse(epoll_event currentEvent);
        void closeServers();

        void listenClientRequest();

        std::string getUserResponse(Client client);
        UserRequest getUserRequest(std::string requestStr);
};

#endif
