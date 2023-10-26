
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
	for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
    {
		(*it).startServers();
    }
    listenClientRequest();
}

int Server::startServers()
{
    std::string hostStr = this->getHost();
    std::string portStr = this->getPort()[0];

    const char *host = hostStr.c_str();
    const char *port = portStr.c_str();

    memset(&_hints, 0, sizeof(struct addrinfo));
    _hints.ai_family = AF_UNSPEC;
    _hints.ai_socktype = SOCK_STREAM;
    _hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(host, port, &_hints, &_result) != 0) {
        std::cerr << "Error retrieving host information" << std::endl;
        return 0;
    }

    _serverSocket = -1;

    for (_rp = _result; _rp != NULL; _rp = _rp->ai_next) {
        _serverSocket = socket(_rp->ai_family, _rp->ai_socktype, _rp->ai_protocol);
        if (_serverSocket == -1)
            continue;

        int iSetOption = 1;
        setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

        if (bind(_serverSocket, _rp->ai_addr, _rp->ai_addrlen) == 0)
            break;

        close(_serverSocket);
    }

    freeaddrinfo(_result);

    if (_rp == NULL) {
        std::cerr << "Socket binding error" << std::endl;
        return 0;
    }

    if (listen(_serverSocket, 5) == -1) {
        std::cerr << "Socket listening error" << std::endl;
        close(_serverSocket);
        return 0;
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    int flags = fcntl(_serverSocket, F_GETFL, 0);
    fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK);

    return 1;
}

int StartServers::listenClientRequest()
{
    int _epollFd = epoll_create1(0);
    std::vector<Server>::iterator it;
	for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
        (*it).addSocketToEpoll(_epollFd);


    while (true)
    {
        int numEvents = epoll_wait(_epollFd, _events, 10, -1);
        for (int i = 0; i < numEvents; i++)
        {
	        for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
            {
                if (_events[i].data.fd == (*it).getServerSocket())
                {
                    (*it).acceptNewClient(_epollFd);
                }
            }
            if (it == _serversVec.end())
            {
                for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
                {
                    int clientIndex = -1;
	                for (int i = 0; i < (*it).getClientsVecSize(); i++)
	                	if ((*it).getClient(i) == _events[i].data.fd)
	                		clientIndex = i;

                    if (clientIndex != -1)
                        (*it).newResponseAndRequest(_epollFd, _events[i], clientIndex);
                }
            }
        }
    }

    close(_epollFd);
    for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
        close((*it).getServerSocket());

    return 1;
}

