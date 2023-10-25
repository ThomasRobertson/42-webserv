#ifndef STARTSERVERS_HPP
# define STARTSERVERS_HPP

#include "ConfigFile.hpp"
#include "Server.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>

class StartServers
{
    private:
        ConfigFile _configFile;
	    std::vector<Server>	_serversVec;

    public:
        StartServers(ConfigFile configFile);
        ~StartServers();

		void createServers();
        void initServers();

        int startServers();
        int listenClientRequest(int serverSocket, int epollFd);

};

#endif
