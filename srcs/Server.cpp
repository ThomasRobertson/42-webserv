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

