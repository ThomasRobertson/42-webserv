#include "Server.hpp"

Server::Server(ConfigFile configFile, int serverIndex)
{
    this->_configFile = configFile;
    this->_serverIndex = serverIndex;
    setServerValues();
    return ;
}

Server::~Server()
{
    return ;
}

void Server::setServerValues()
{
    this->_host = this->_configFile.getHost(this->getServerIndex());
    this->_port = this->_configFile.getPort(this->getServerIndex());
    this->_maxClientBodySize = this->_configFile.getMaxClientBodySize(this->getServerIndex());
    this->_errorsMap = this->_configFile.getErrorPages(this->getServerIndex());
    this->_cgiMap = this->_configFile.getCgiPages(this->getServerIndex());;
    this->_htmlPageMap = this->_configFile.getFileRoutes(this->getServerIndex());
}

std::string Server::getHost()
{
    return this->_host;
}

std::vector<std::string> Server::getPort()
{
    return this->_port;
}

int Server::getMaxClientBodySize()
{
    return this->_maxClientBodySize;
}

int Server::getServerIndex()
{
    return this->_serverIndex;
}

std::string Server::getFileRoute(std::string location, std::string &status)
{
    std::map<std::string, page>::iterator it = this->_htmlPageMap.find(location);
    status = "200";

    if (it == this->_htmlPageMap.end())
    {
        status = "404";
        return this->_htmlPageMap["/404"].index;
    }

    return this->_htmlPageMap[location].index;
}

std::string Server::getErrorPage(std::string errorCode)
{
	return this->_errorsMap[errorCode];
}

std::string Server::getCgiPage(std::string cgiName)
{
	return this->_cgiMap[cgiName];
}

int Server::getServerSocket()
{
    return this->_serverSocket;
}

int Server::getClient(int index)
{
    return this->_clientsVec[index];
}

int Server::getClientsVecSize()
{
    return this->_clientsVec.size();
}

int Server::addSocketToEpoll(int epollFd)
{
    if (epollFd == -1)
    {
        std::cerr << "Error creating epoll" << std::endl;
        close(_serverSocket);
        return 0;
    }

    _event.events = EPOLLIN;
    _event.data.fd = _serverSocket;


    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _serverSocket, &_event) == -1)
    {
        std::cerr << "Error adding server socket to epoll " << std::endl;
        close(_serverSocket);
        close(epollFd);
        return 0;
    }
    return 1;
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

void Server::acceptNewClient(int epollFd)
{
    _clientAddrLen = sizeof(_clientAddr);

    _clientSocket = accept(_serverSocket, (struct sockaddr *)&_clientAddr, &_clientAddrLen);
    setNonBlocking(_clientSocket);
    _event.data.fd = _clientSocket;
    _event.events = EPOLLIN;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, _clientSocket, &_event);
    _clientsVec.push_back(_clientSocket);
    std::cout << "New client connected: " << _clientSocket << std::endl;
}

void Server::newResponseAndRequest(int epollFd, epoll_event &events, int clientIndex)
{
    if (events.events & EPOLLOUT)
    {
        std::cout << "----------------------- NEW REPONSE: " << _clientsVec[clientIndex] << " -----------------------" << std::endl;
        _response = getUserResponse(_userRequest);
    
        ssize_t bytesSent = write(_clientsVec[clientIndex], _response.c_str(), _response.length());
        std::cout << "response sent" << _response.substr(0, 200) << std::endl;
    
        epoll_ctl(epollFd, EPOLL_CTL_DEL, _clientsVec[clientIndex], NULL);
        close(_clientsVec[clientIndex]);
    }
    else if (events.events & EPOLLIN)
    {
        std::cout << "----------------------- NEW REQUEST: " << _clientsVec[clientIndex] << " -----------------------" << std::endl;
        char buffer[1024];
        ssize_t bytesRead = read(_clientsVec[clientIndex], buffer, 1024);
        if (bytesRead <= 0)
        {
            epoll_ctl(epollFd, EPOLL_CTL_DEL, _clientsVec[clientIndex], &_event);
            close(_clientsVec[clientIndex]);
		    _clientsVec.erase(_clientsVec.begin() + clientIndex);
            std::cout << "Client disconnected: " << _clientsVec[clientIndex] << std::endl;
        }
        else
        {
            std::string requestData(buffer, bytesRead);
            _userRequest = getUserRequest(requestData);
            std::cout << "Received HTTP request:\n" << requestData << std::endl;
    
            _event.data.fd = _clientsVec[clientIndex];
            _event.events = EPOLLOUT;
            epoll_ctl(epollFd, EPOLL_CTL_MOD, _clientsVec[clientIndex], &_event);
        }
    }
}