#ifndef STARTSERVERS_HPP
# define STARTSERVERS_HPP

#include "ConfigFile.hpp"
#include "Server.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

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
        
        // int _epollFd;

        std::vector<int> _clientsVec;
        std::map<int, Client> _clientList;




    public:
        StartServers(ConfigFile configFile);
        ~StartServers();

		void createServers();
        void initServers();

        int listenClientRequest(int epollFd);
        void newResponseAndRequest(int epollFd, epoll_event &events, int clientIndex);

        std::string getUserResponse(Client client);
        UserRequest getUserRequest(std::string requestStr);
};

#endif
