#include "StartServers.hpp"
#include <ostream>
#include <stdexcept>

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
    this->_host = this->_configFile.getHost(_serverIndex);
    this->_port = this->_configFile.getPort(_serverIndex);
    this->_maxClientBodySize = this->_configFile.getMaxClientBodySize(_serverIndex);
    this->_errorsMap = this->_configFile.getErrorPages(_serverIndex);
    this->_cgiMap = this->_configFile.getCgiPages(_serverIndex);
    this->_htmlPageMap = this->_configFile.getFileRoutes(_serverIndex);
    this->_root = this->_configFile.getRoot(_serverIndex);
    this->_server_name = this->_configFile.getServerName(_serverIndex);

}

std::string Server::getHost()
{
    return this->_host;
}

std::vector<std::string> Server::getPort()
{
    return this->_port;
}

std::string Server::getRoot()
{
    return this->_root;
}

std::string Server::getServerName()
{
    return this->_server_name;
}

int Server::getMaxClientBodySize()
{
    return this->_maxClientBodySize;
}

std::string Server::getErrorPageRoute(std::string errorCode)
{
	std::string fileLocation;

	if (_errorsMap.find(errorCode) == _errorsMap.end())
	{
		throw std::runtime_error("No valid page found, cannot continue execution.");
	}
	fileLocation = getRoot() + _errorsMap[errorCode];
	if (access(fileLocation.c_str(), R_OK) != 0)
	{
		throw std::runtime_error("No valid error page found, cannot continue execution.");
	}

	return fileLocation;
}

std::string Server::testAccessPath(std::string location, std::string method)
{
	if (access(location.c_str(), F_OK) != 0)
		return "404";

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

	if (access(location.c_str(), accessMode) == 0)
		return "200";
	else
		return "403";
}

std::pair<std::string, page> Server::getRootDir(std::string url)
{
	while (_htmlPageMap.find(url) == _htmlPageMap.end())
	{
		if (url.length() == 0)
			throw std::invalid_argument("Root is not present in config.");
		std::size_t found = url.find_last_of("/\\");
		url = url.substr(0,found);
		if (url.empty())
			url = "/";
	}
	return *(_htmlPageMap.find(url));
}

std::string Server::getFileRoute(const std::string fileName, std::string &status, std::string method)
{
	std::string fileLocation;
	std::pair<std::string, page> location;

	try
	{
		location = getRootDir(fileName);
	}
	catch (const std::exception&)
	{
		status = "404";
		return "";
	}

	if (std::find(location.second.methods.begin(), location.second.methods.end(), method) == location.second.methods.end())
	{
		status = "405";
		return "";
	}

	if (location.second.rootDir.empty())
		location.second.rootDir = _root;

	std::cout << "root : " << _root << " filename: " << fileName << " loc: " << location.first << std::endl;

	if (fileName == location.first && !location.second.index.empty()) //if rootDir, check for index config file
	{
		std::string rootIndex = location.second.rootDir + location.second.index;
		std::cout << "Testing index file : " << rootIndex << std::endl;
		status = testAccessPath(rootIndex, method);
		return (rootIndex);
	}

	std::string locationAfterRoot = fileName.substr(location.first.size(), std::string::npos);
	if (*(locationAfterRoot.begin()) != '/')
		locationAfterRoot = "/" + locationAfterRoot;
	fileLocation = location.second.rootDir + locationAfterRoot;

	if (*(fileLocation.rbegin()) == '/') //check if index.html present
	{
		status = testAccessPath(fileLocation + "index.html", method);
		if (status != "404")
		{
			return std::string(fileLocation + "index.html");
		}
	}

	if (*(fileLocation.rbegin()) == '/') //check for listing directory
	{
		struct stat path_stat;
		if (stat(fileLocation.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
		{
			status = "200";
			return fileLocation;
		}
		else
		{
			status = "403";
			return "";
		}
	}

	status = testAccessPath(fileLocation, method);
	return fileLocation;
}

bool Server::getListing(std::string fileLocation)
{
    return _htmlPageMap[fileLocation].listing;
}

std::string Server::getFileName(std::string fileName)
{
    return _htmlPageMap[fileName].index;
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

void Server::addSocketToEpoll(int epollFd, int i)
{
    epoll_event event;

    event.events = EPOLLIN;
    event.data.fd = _serverSocketVec[i];

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, _serverSocketVec[i], &event) == -1)
    {
        std::cerr << "Error adding server socket to epoll " << std::endl;
        close(_serverSocketVec[i]);
        close(epollFd);
        throw (Problem());
    }
}

void setNonBlocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(F_GETFL)");
        exit(1); //Problem here
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL)");
        exit(1); //Problem here
    }
}

int Server::acceptNewClient(int epollFd, int serverIndex)
{
    epoll_event event;
    int clientAddr;
    int clientSocket;
    socklen_t clientAddrLen = sizeof(clientAddr);

    clientSocket = accept(_serverSocketVec[serverIndex], (struct sockaddr *)&clientAddr, &clientAddrLen);
    setNonBlocking(clientSocket);

    event.data.fd = clientSocket;
    event.events = EPOLLIN;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);

    std::cout << GREEN << "New client connected: " << clientSocket << DEFAULT << std::endl;
    return clientSocket;
}

void Server::startServers(int epollFd)
{
    std::vector<std::string> portsVec = this->getPort();
    std::vector<std::string>::iterator portIt;
    int i;

    for (portIt = portsVec.begin(), i = 0 ; portIt != portsVec.end() ; i++, portIt++)
    {
        struct addrinfo hints;
        struct addrinfo *result;
        struct addrinfo *rp;

        std::string hostStr = this->getHost();
        std::string portStr = *portIt;

        const char *host = hostStr.c_str();
        const char *port = portStr.c_str();

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if (getaddrinfo(host, port, &hints, &result) != 0)
        {
            std::cerr << "Error retrieving host information" << std::endl;
            return ;
        }

        _serverSocketVec.push_back(-1);

        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            _serverSocketVec[i] = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (_serverSocketVec[i] == -1)
                continue;

            int iSetOption = 1;
            setsockopt(_serverSocketVec[i], SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

            if (bind(_serverSocketVec[i], rp->ai_addr, rp->ai_addrlen) == 0)
                break;

            close(_serverSocketVec[i]);
        }

        freeaddrinfo(result);

        if (rp == NULL)
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

        std::cout << YELLOW << "[i] Server listening on port " << port << "..." << DEFAULT << std::endl;

        int flags = fcntl(_serverSocketVec[i], F_GETFL, 0);
        fcntl(_serverSocketVec[i], F_SETFL, flags | O_NONBLOCK);

        this->addSocketToEpoll(epollFd, i);
    }
}