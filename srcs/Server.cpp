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
    // this->_server_name = this->_configFile.getServerName(_serverIndex);
    //this->_postRoot = this->_configFile.getPostRoot(_serverIndex);
    

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

// std::string Server::getPostRoot()
// {
//     return this->_postRoot;
// }

// std::string Server::getServerName()
// {
//     return this->_server_name;
// }

int Server::getMaxClientBodySize()
{
    return this->_maxClientBodySize;
}

std::string Server::getErrorPageRoute(std::string errorCode)
{
	std::string fileLocation;

	if (_errorsMap.find(errorCode) == _errorsMap.end())
	{
		throw std::runtime_error("No error page configured for this error code, generating one.");
	}
	fileLocation = getRoot() + _errorsMap[errorCode];
	if (access(fileLocation.c_str(), R_OK) != 0)
	{
		throw std::runtime_error("Cannot open the error page, generating one.");
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

std::pair<std::string, Location> Server::getRootDir(std::string url)
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

std::string Server::getFileRoute(const std::string fileName, std::string &status, std::string method, bool &is_dir, bool isCGI)
{
	std::string fileLocation, rootDir;
	std::pair<std::string, Location> location;

	try
	{
		location = getRootDir(fileName);
	}
	catch (const std::exception&)
	{
		status = "404";
		return fileName;
	}

	if (!isCGI && std::find(location.second.methods.begin(), location.second.methods.end(), method) == location.second.methods.end())
	{
		status = "405";
		return fileName;
	}

	if (method == "GET")
	{
		if (location.second.rootDir.empty())
			rootDir = _root + location.first;
		else
			rootDir = location.second.rootDir;
	}
	else if (method == "POST" || method == "DETELE")
	{
		if (location.second.postRoot.empty())
		{
			status = "500";
			return fileName;
		}
		else
		{
			status = "200";
			return (location.second.postRoot);	
		}
			
	}
	else
	{
		std::cout << RED << "Invalid method: " << method << DEFAULT << std::endl;
		status = "500";
		return fileName;
	}

	// std::cout << "root : " << rootDir << " filename: " << fileName << " location: " << location.first << " index: " << location.second.index << std::endl << "method: " << method << std::endl;

	std::string locationAfterRoot = fileName.substr(location.first.size(), std::string::npos);
	
	fileLocation = rootDir + locationAfterRoot;
	// std::cout << "second file loc : " << fileLocation<< std::endl;

	if (!location.second.index.empty())
	{
		std::string rootIndex = fileLocation;
		if (*fileLocation.rbegin() != '/')
			rootIndex += "/";
		rootIndex += location.second.index;
		// std::cout << "Testing index file : " << rootIndex << std::endl;
		status = testAccessPath(rootIndex, method);
		if (status != "404")
			return (rootIndex);
	}

	DIR *dir = opendir(fileLocation.c_str());
	if (dir != NULL)
	{
		if (location.second.listing)
		{
			struct stat path_stat;
			if (stat(fileLocation.c_str(), &path_stat) == 0 && S_ISDIR(path_stat.st_mode))
			{
				is_dir = true;
				status = "200";
				closedir(dir);
				return fileLocation;
			}
		}
		else
		{
			status = "404";
			closedir(dir);
			return fileLocation;
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

std::string Server::getPostRoot(std::string fileName)
{
    return _htmlPageMap[fileName].postRoot;
}

std::map<std::string, std::string> Server::getCgiPages()
{
	return this->_cgiMap;
}

int Server::getServerSocket(int i)
{
    return this->_serverSocketVec[i];
}

int Server::getServerSocketSize()
{
    return this->_serverSocketVec.size();
}

void Server::addSocketToEpoll(int epollFd, int serverSocket)
{
    epoll_event event;

    event.events = EPOLLIN;
    event.data.fd = serverSocket;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1)
    {
        std::cerr << "Error adding server socket to epoll " << std::endl;
        close(serverSocket);
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
        throw(Problem());
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL)");
        throw(Problem());
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

    std::cout << GREEN << "[i] New client connected: " << clientSocket << DEFAULT << std::endl;
    return clientSocket;
}

void Server::startServers(int epollFd)
{
    std::vector<std::string> portsVec = this->getPort();
    std::vector<std::string>::iterator portIt;

    for (portIt = portsVec.begin() ; portIt != portsVec.end() ; portIt++)
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

        int serverSocket = -1;

        if (getaddrinfo(host, port, &hints, &result) != 0)
        {
            std::cerr << "Error retrieving host information" << std::endl;
            return ;
        }

        for (rp = result; rp != NULL; rp = rp->ai_next)
        {
            serverSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (serverSocket == -1)
                continue;

            int iSetOption = 1;
            setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

            if (bind(serverSocket, rp->ai_addr, rp->ai_addrlen) == 0)
                break;

            close(serverSocket);
        }

        freeaddrinfo(result);

        if (rp == NULL)
        {
            std::cerr << "Socket binding error" << std::endl;
        	this->_serverSocketVec.push_back(-1);
            continue ;
        }

        if (listen(serverSocket, 100) == -1)
        {
            std::cerr << "Socket listening error" << std::endl;
            close(serverSocket);
            continue ;
        }



        std::cout << YELLOW << "[i] Server listening on port " << port << "..." << DEFAULT << std::endl;

        int flags = fcntl(serverSocket, F_GETFL, 0);
        fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

        this->addSocketToEpoll(epollFd, serverSocket);

        this->_serverSocketVec.push_back(serverSocket);
    }
}