
#include "StartServers.hpp"


StartServers::StartServers(ConfigFile configFile) : _configFile(configFile)
{
	return ;
}

StartServers::~StartServers()
{
	return ;
}

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

    int epollFd = epoll_create1(0);
    for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
    {
        (*it).startServers(epollFd);
    }

    listenClientRequest(epollFd);
}

void Server::startServers(int epollFd)
{
    std::vector<std::string> portsVec = this->getPort();
    
    int i = 0;

    for (std::vector<std::string>::iterator it = portsVec.begin() ; it != portsVec.end() ; it++)
    {

        std::string hostStr = this->getHost();
        std::string portStr = *it;

        const char *host = hostStr.c_str();
        const char *port = portStr.c_str();

        std::cout << host << std::endl;
        std::cout << port << std::endl;

        memset(&_hints, 0, sizeof(struct addrinfo));
        _hints.ai_family = AF_UNSPEC;
        _hints.ai_socktype = SOCK_STREAM;
        _hints.ai_flags = AI_PASSIVE;

        if (getaddrinfo(host, port, &_hints, &_result) != 0)
        {
            std::cerr << "Error retrieving host information" << std::endl;
            return ;
        }

        _serverSocketVec.push_back(-1);

        for (_rp = _result; _rp != NULL; _rp = _rp->ai_next)
        {
            _serverSocketVec[i] = socket(_rp->ai_family, _rp->ai_socktype, _rp->ai_protocol);
            if (_serverSocketVec[i] == -1)
                continue;

            int iSetOption = 1;
            setsockopt(_serverSocketVec[i], SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

            if (bind(_serverSocketVec[i], _rp->ai_addr, _rp->ai_addrlen) == 0)
                break;

            close(_serverSocketVec[i]);
        }

        freeaddrinfo(_result);

        if (_rp == NULL)
        {
            std::cerr << "Socket binding error" << std::endl;
            return ;
        }

        if (listen(_serverSocketVec[i], 5) == -1)
        {
            std::cerr << "Socket listening error" << std::endl;
            close(_serverSocketVec[i]);
            return ;
        }
descriptor
        std::cout << "Server listening on port " << port << "..." << std::endl;

        int flags = fcntl(_serverSocketVec[i], F_GETFL, 0);
        fcntl(_serverSocketVec[i], F_SETFL, flags | O_NONBLOCK);

        this->addSocketToEpoll(epollFd, i);
        i++;
    }
}

int StartServers::listenClientRequest(int epollFd)
{
    std::vector<Server>::iterator it;
    epoll_event events[10];
    epoll_event event;
    std::string response;
    UserRequest userRequest;
    Client currentClient;
    std::map<int, Client> clientList;

    while (true)
    {
        int numEvents = epoll_wait(epollFd, events, 10, -1);
        for (int i = 0; i < numEvents; i++)
        {
            int serverIndex = 0;
	        for (it = this->_serversVec.begin() ; it != this->_serversVec.end() ; it++)
            {
                for (int y = 0; y < (*it).getServerSocketSize(); y++)
                {
                    if (events[i].data.fd == (*it).getServerSocket(y))
                    {
                        currentClient.fd = events[i].data.fd;
                        currentClient.serverIndex = serverIndex;
                        (*it).acceptNewClient(epollFd, y);
                        clientList[currentClient.fd] = currentClient;
                        std::cout << "ServerSocketSize: " << (*it).getServerSocketSize() << "" << std::endl;

                    }
                }
                serverIndex++;
            }


            if (events[i].events & EPOLLOUT)
            {
                currentClient = clientList[events[i].data.fd];
                std::cout << "----------------------- NEW REPONSE: " << events[i].data.fd << " -----------------------" << std::endl;
                response = getUserResponse(currentClient);
            
                ssize_t bytesSent = write(events[i].data.fd, response.c_str(), response.length());
                std::cout << "response sent" << response.substr(0, 200) << std::endl;
            
                epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                currentClient = clientList[events[i].data.fd];
                std::cout << "----------------------- NEW REQUEST: " << events[i].data.fd << " -----------------------" << std::endl;
                char buffer[1024];
                ssize_t bytesRead = read(events[i].data.fd, buffer, 1024);
                if (bytesRead <= 0)
                {
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
	        	    clientList.erase(currentClient.fd);
                    std::cout << "Client disconnected: " << events[i].data.fd << std::endl;
                }
                else
                {
                    std::string requestData(buffer, bytesRead);
                    currentClient.request = getUserRequest(requestData);
                    std::cout << "Received HTTP request:\n" << requestData << std::endl;
            
                    event.data.fd = events[i].data.fd;
                    event.events = EPOLLOUT;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &event);
                }
            }
        }
    }

    close(epollFd);
    for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
        for (int z = 0; z < (*it).getServerSocketSize(); z++)
            close((*it).getServerSocket(z));

    return 1;
}

