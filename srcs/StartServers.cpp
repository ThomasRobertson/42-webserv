#include "StartServers.hpp"
#include "ClientRequest.hpp"
#include "utils.hpp"

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
    std::vector<Server>::iterator serverIt;
    struct Client newClient;
    int serverIndex = 0;

    for (serverIt = this->_serversVec.begin() ; serverIt != this->_serversVec.end() ; serverIt++)
    {
        for (int portIndex = 0; portIndex < serverIt->getServerSocketSize(); portIndex++)
        {
            if (currentEvent.data.fd == serverIt->getServerSocket(portIndex))
            {
                newClient.fd = serverIt->acceptNewClient(_epollFd, portIndex);
                newClient.lastActionDate = getDate();
                newClient.server = &(*serverIt);
                newClient.request.isBodyComplete = false; // default value
                newClient.request.isHeaderComplete = false; // default value
				newClient.request.isBodyTooLarge = false; // default value
                newClient.request.contentLength = -1; // default value
                newClient.request.isCGI = false; // default value
                newClient.request.bodySize = 0; // default value
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
            if (serverIt->getServerSocket(portIndex) != -1)
                close(serverIt->getServerSocket(portIndex));
            std::cout << YELLOW << "[i] Server shutdown: " << serverIt->getServerName() << "::" << *portIt << DEFAULT << std::endl;
        }
    }

    std::map<int, Client>::iterator clientIt;

    for (clientIt = this->_clientList.begin() ; clientIt != this->_clientList.end() ; clientIt++)
    {
        close(clientIt->first);
    }
}

void StartServers::checkTimeout()
{
    std::map<int, Client>::iterator clientIter;
    int timeoutValue = 60;

    for (clientIter = this->_clientList.begin(); clientIter != this->_clientList.end();)
    {
        if (getDate() - clientIter->second.lastActionDate > timeoutValue)
        {
            int clientFd = clientIter->second.fd;
            std::cout << RED << "[i] Client disconnected from timeout: " << clientFd << DEFAULT << std::endl;
            clientIter++;
            _clientList.erase(clientFd);
            epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
            close(clientFd);
        }
        else
            clientIter++;
    }
}

void StartServers::disconnectClient(int clientFd)
{
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
    close(clientFd);
    _clientList.erase(clientFd);
    std::cout << RED << "[i] Client disconnected from error: " << clientFd << DEFAULT << std::endl;
}

void StartServers::listenClientRequest()
{
    epoll_event events[64];

    while (true)
    {
        int numEvents = epoll_wait(_epollFd, events, 64, 1000);
        if (numEvents == -1)
            return;
        checkTimeout();
        if (EXIT_G == true)
            break;
        for (int i = 0; i < numEvents; i++)
        {
            bool isNewClient = getNewConnexion(events[i]);

            if (!isNewClient)
            {
                if (events[i].events & EPOLLERR)
                    disconnectClient(events[i].data.fd);
                if (events[i].events & EPOLLIN)
                    processRequest(events[i]);
                if (events[i].events & EPOLLOUT)
                    processResponse(events[i]);
            }

        }
    }
}