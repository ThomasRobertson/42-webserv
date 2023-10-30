#include "StartServers.hpp"

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
        std::map<std::string, page> _htmlPageMap;
        std::map<std::string, std::string> _errorsMap;
        std::map<std::string, std::string> _cgiMap;
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

int Server::getServerSocket(int i)
{
    return this->_serverSocketVec[i];
}

int Server::getServerSocketSize()
{
    return this->_serverSocketVec.size();
}

int Server::addSocketToEpoll(int epollFd, int i)
{

    _event.events = EPOLLIN;
    _event.data.fd = _serverSocketVec[i];

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _serverSocketVec[i], &_event) == -1)
    {
        std::cerr << "Error adding server socket to epoll " << std::endl;
        close(_serverSocketVec[i]);
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

void Server::acceptNewClient(int epollFd, int y, Client &newClient)
{
    epoll_event event;

    _clientAddrLen = sizeof(_clientAddr);

    _clientSocket = accept(_serverSocketVec[y], (struct sockaddr *)&_clientAddr, &_clientAddrLen);
    setNonBlocking(_clientSocket);
    event.data.fd = _clientSocket;
    event.events = EPOLLIN;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, _clientSocket, &event);
    newClient.fd = _clientSocket;

    std::cout << "New client connected: " << _clientSocket << std::endl;
}