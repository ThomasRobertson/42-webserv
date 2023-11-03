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

void StartServers::receiveRequest(epoll_event currentEvent)
{
    char buffer[1024];
    ssize_t bytesRead;
    struct epoll_event event;

    std::cout << "----------------------- NEW REQUEST: " << currentEvent.data.fd << " -----------------------" << std::endl;
    bytesRead = read(currentEvent.data.fd, buffer, 1024);

    if (bytesRead <= 0)
    {
        epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, &event);
        close(currentEvent.data.fd);
        _clientList.erase(currentEvent.data.fd);
        std::cout << "Client disconnected from error: " << currentEvent.data.fd << std::endl;
    }
    else
    {
        std::string requestData(buffer, bytesRead);

        std::cout << requestData << std::endl;
        if (_clientList[currentEvent.data.fd].toComplete)
            getRequestNextChunk(currentEvent.data.fd, requestData);
        else
            _clientList[currentEvent.data.fd].request = getUserRequest(requestData);

        event.data.fd = currentEvent.data.fd;
        event.events = EPOLLOUT;
        epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
    }
}

void StartServers::sendResponse(epoll_event currentEvent)
{
    std::string response;
    struct epoll_event event;

    std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;
    if (_clientList[currentEvent.data.fd].request.length != _clientList[currentEvent.data.fd].request.finalLength)
    {
        std::cout << "RESPONSE NOT SENT BECAUSE IS NOT COMPLETE" << std::endl;
        _clientList[currentEvent.data.fd].toComplete = true;
        event.data.fd = currentEvent.data.fd;
        event.events = EPOLLIN;
        epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
        return;
    }
    response = getUserResponse(_clientList[currentEvent.data.fd]);

    write(currentEvent.data.fd, response.c_str(), response.length());
    std::cout << "response sent: " << response.substr(0, 200) << std::endl;

    // event.data.fd = currentEvent.data.fd;
    // event.events = EPOLLIN;
    // epoll_ctl(_epollFd, EPOLL_CTL_MOD, currentEvent.data.fd, &event);
    _clientList.erase(currentEvent.data.fd);
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
    close(currentEvent.data.fd);
    std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;

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
                    receiveRequest(events[i]);
                if (events[i].events & EPOLLOUT)
                    sendResponse(events[i]);
            }

        }
    }
    closeServers();
}