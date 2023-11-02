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

std::string generateErrorPage(std::string errorCode)
{
	if (errorCode.empty())
		errorCode = "500";
	
	std::pair<std::string, std::string> status = getStatus(errorCode);

	std::string bodyReponse;
	bodyReponse = "<!doctype html>\n";
	bodyReponse += "<html>\n";
	bodyReponse += "  <head>\n";
	bodyReponse += "    <title>";
	bodyReponse += status.first;
	bodyReponse += "</title>\n";
	bodyReponse += "  </head>\n";
	bodyReponse += "  <body>\n";
	bodyReponse += "    <h1>";
	bodyReponse += status.first;
	bodyReponse += "</h1>\n";
	bodyReponse += "    <p>";
	bodyReponse += status.second;
	bodyReponse += "</p>\n";
	bodyReponse += "  </body>\n";
	bodyReponse += "</html>\n";
}

std::string Server::getErrorPageRoute(std::string errorCode)
{
	std::string fileLocation;

	if (_errorsMap.find(errorCode) == _errorsMap.end())
	{
		throw std::runtime_error("No valid page found, cannot continue execution.");
	}
	fileLocation = HTML_DIR + _errorsMap[errorCode];
	if (access(fileLocation.c_str(), R_OK) != 0)
	{
		throw std::runtime_error("No valid error page found, cannot continue execution.");
	}

	return fileLocation;
}

std::string Server::getFileRoute(std::string fileName, std::string &status, std::string method)
{
	std::string fileLocation;

	if (_htmlPageMap.find(fileName) != _htmlPageMap.end()) // fileLocation is not a file but a parent directory
		fileName = _htmlPageMap[fileName].index;
	fileLocation = HTML_DIR; //* tmp, waiting to parse root
	// fileLocation += root; //! must by without trailing '/'
	fileLocation += fileName;
	// std::cout << "fileadress: " <<fileAddress << std::endl;
	if (access(fileLocation.c_str(), F_OK) != 0)
	{
		status = "404";
		return "";
	}
	int accessMode;
	if (method == "GET")
		accessMode = R_OK;
	else if (method == "POST" || method == "DELETE")
		accessMode = W_OK;
	else
	{
		accessMode = R_OK | W_OK;
		#ifdef DEBUG
		throw std::runtime_error("Unknow method !");
		#endif // DEBUG
	}
	if (access(fileLocation.c_str(), accessMode) == 0)
		status = "200";
	else
	{
		status = "403";
		return "";
	}
	return fileLocation;
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