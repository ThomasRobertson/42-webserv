#include "StartServers.hpp"

StartServers::StartServers(ConfigFile configFile) : _configFile(configFile) {}

StartServers::~StartServers() {}

void StartServers::createServers()
{
	for (int i = 0; i <= this->_configFile.getServerNumber(); i++)
	{
		Server newServ(this->_configFile, i);
		this->_serversVec.push_back(newServ);
	}
}

void StartServers::initServers()
{
	std::vector<Server>::iterator it;

    this->_epollFd = epoll_create1(0); // check if fail
    for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
    {
        it->startServers(this->_epollFd);
    }
    listenClientRequest();
}

bool StartServers::getNewConnexion(epoll_event currentEvent)
{
    std::vector<Server>::iterator it;
    struct Client newClient;
    int serverIndex = 0;
    
    for (it = this->_serversVec.begin() ; it != this->_serversVec.end() ; it++)
    {
        for (int portIndex = 0; portIndex < it->getServerSocketSize(); portIndex++)
        {
            if (currentEvent.data.fd == it->getServerSocket(portIndex))
            {
                it->acceptNewClient(_epollFd, portIndex, newClient);
                newClient.serverIndex = serverIndex;
                newClient.toComplete = false;
                _clientList[newClient.fd] = newClient;
                return true;
            } 
        }
        serverIndex++;
    }
    return false;
}

void StartServers::closeServers()
{
    std::vector<Server>::iterator serverIt;
    std::vector<std::string> portsVec;
    std::vector<std::string>::iterator portIt;
    int portIndex;

    close(_epollFd);

    for (serverIt = this->_serversVec.begin(); serverIt < this->_serversVec.end(); serverIt++)
    {
        portsVec = serverIt->getPort();
        for (portIt = portsVec.begin(), portIndex = 0 ; portIt != portsVec.end() ; portIndex++, portIt++)
        {
            close(serverIt->getServerSocket(portIndex));
            std::cout << YELLOW << "[i] Server shutdown: " << serverIt->getHost() << "::" << *portIt << DEFAULT << std::endl;
        }
    }
}

void StartServers::listenClientRequest()
{
    epoll_event events[10];

    while (true)
    {
        int numEvents = epoll_wait(_epollFd, events, 10, -1);
        if (EXIT_G == true)
            break;
        for (int i = 0; i < numEvents; i++)
        {
            bool isNewClient = getNewConnexion(events[i]);

            if (!isNewClient)
            {
                // if (events[i].events & EPOLLERR) check epoll error
                if (events[i].events & EPOLLIN)
                    processRequest(events[i]);
                if (events[i].events & EPOLLOUT)
                    processResponse(events[i]);
            }

        }
    }
    closeServers();
}