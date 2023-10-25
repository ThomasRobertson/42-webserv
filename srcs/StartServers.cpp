
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
		(*it).startServers();
    listenClientRequest(_serverSocket, epollFd);
}

void setNonBlocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(1);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(1);
    }
}

int StartServers::listenClientRequest(int serverSocket, int epollFd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;

	std::vector<Server>::iterator it;
	for (it = this->_serversVec.begin(); it < this->_serversVec.end(); it++)
    {
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
            std::cerr << "Error adding server socket to epoll" << std::endl;
            close(serverSocket);
            close(epollFd);
            return 0;
        }
    }

    _clientAddrLen = sizeof(_clientAddr);

    while (true)
    {
        int numEvents = epoll_wait(epollFd, _events, 10, -1);
        for (int i = 0; i < numEvents; i++)
        {
            if (_events[i].data.fd == serverSocket)
            {
                _clientSocket = accept(serverSocket, (struct sockaddr *)&_clientAddr, &_clientAddrLen);
                setNonBlocking(_clientSocket);
                event.data.fd = _clientSocket;
                event.events = EPOLLIN;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, _clientSocket, &event);
                std::cout << "New client connected: " << _clientSocket << std::endl;
            }
            else
            {
                if (_events[i].events & EPOLLOUT)
                {
                    std::cout << "----------------------- NEW REPONSE: " << _events[i].data.fd << " -----------------------" << std::endl;
                    _response = getUserResponse(_userRequest, this->_configFile);

                    ssize_t bytesSent = write(_events[i].data.fd, _response.c_str(), _response.length());
                    std::cout << "response sent" << _response.substr(0, 200) << std::endl;

                    epoll_ctl(epollFd, EPOLL_CTL_DEL, _events[i].data.fd, NULL);
                    close(_events[i].data.fd);
                }
                else if (_events[i].events & EPOLLIN)
                {
                    std::cout << "----------------------- NEW REQUEST: " << _events[i].data.fd << " -----------------------" << std::endl;
                    char buffer[1024];
                    ssize_t bytesRead = read(_events[i].data.fd, buffer, 1024);
                    if (bytesRead <= 0)
                    {
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, _events[i].data.fd, &event);
                        close(_events[i].data.fd);
                        std::cout << "Client disconnected: " << _events[i].data.fd << std::endl;
                    }
                    else
                    {
                        std::string requestData(buffer, bytesRead);
                        _userRequest = getUserRequest(requestData);
                        std::cout << "Received HTTP request:\n" << requestData << std::endl;

                        event.data.fd = _events[i].data.fd;
                        event.events = EPOLLOUT;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, _events[i].data.fd, &event);
                    }
                }
            }
        }
    }

    close(epollFd);
    close(_serverSocket);
    return 1;
}

int StartServers::startServers()
{
    std::string hostStr = this->getHost();
    std::string portStr = this->getPort()[0];

    const char *host = hostStr.c_str();
    const char *port = portStr.c_str();

    // struct addrinfo hints, *result, *rp;
    memset(&_hints, 0, sizeof(struct addrinfo));
    _hints.ai_family = AF_UNSPEC;
    _hints.ai_socktype = SOCK_STREAM;
    _hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(host, port, &_hints, &_result) != 0) {
        std::cerr << "Error retrieving host information" << std::endl;
        return 0;
    }

    int _serverSocket = -1;

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

    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Error creating epoll" << std::endl;
        close(_serverSocket);
        return 0;
    }
}