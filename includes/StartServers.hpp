#ifndef STARTSERVERS_HPP
# define STARTSERVERS_HPP

#include "ConfigFile.hpp"
#include "Server.hpp"
#include "Settings.hpp"
#include "getMethod.hpp"

#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <csignal>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>

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
        bool isValidRequest(UserRequest requestData);
        std::string listingDirectory(Client client);
        void listingOn(std::string &response, std::string &fileLocation, std::string &fileName);
        void listingOff(std::string &response, std::string &fileLocation, Server currentServer);
        void fileIsNotDirectory(std::string &response, std::string &fileLocation, std::string &contentType, std::string &status, Server currentServer);



        std::string getUserResponse(Client client);
        UserRequest getUserRequest(std::string requestStr);
        std::string getErrorPageResponse(Client client, std::string errorCode);
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
