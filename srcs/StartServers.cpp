#include "StartServers.hpp"
#include "ClientRequest.hpp"

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
    std::vector<Server>::iterator serverIt;
    struct Client newClient;
    int serverIndex = 0;
    
    for (serverIt = this->_serversVec.begin() ; serverIt != this->_serversVec.end() ; serverIt++)
    {
        for (int portIndex = 0; portIndex < serverIt->getServerSocketSize(); portIndex++)
        {
            if (currentEvent.data.fd == serverIt->getServerSocket(portIndex))
            {
                newClient.fd = serverIt->acceptNewClient(_epollFd, portIndex);
                newClient.server = &(*serverIt);
                newClient.request.isBodyComplete = false; // default value
                newClient.request.isHeaderComplete = false; // default value
                newClient.request.contentLength = 0; // default value
                newClient.request.length = 0; // default value
                _clientList[newClient.fd] = newClient;
                return true;
            }
        }
        serverIndex++;
    }
    return false;
}

bool StartServers::isValidRequest(UserRequest requestData)
{
    std::cout << "requestData: fullRequest: " << requestData.fullRequest << std::endl << "END" << std::endl;

    std::string httpRequest = requestData.fullRequest;

    std::size_t pos = 0;
    std::string line;

    bool validRequest = true;

    int countPost = 0;
    int countOther = 0;
    std::string method, uri, version;

    if ((pos = httpRequest.find("\r\n")) != std::string::npos || !httpRequest.empty())
    {
        if (pos != std::string::npos)
        {
            line = httpRequest.substr(0, pos);
            httpRequest.erase(0, pos + 2);
        }
        else
            line = httpRequest;
        std::istringstream lineStream(line);
        lineStream >> method >> uri >> version;

        if (method != "GET" && method != "POST" && method != "DELETE")
            return false;
        if (version != "HTTP/1.1")
            return false;
        if (uri.find('/') != 0)
            return false;
        if (line.length() > 8000)
            return false;

        int spaceCount = std::count(line.begin(), line.end(), ' ');
        if (spaceCount > 2)
            return false;
    }


    while ((pos = httpRequest.find("\r\n")) != std::string::npos || !httpRequest.empty())
    {
        if (pos != std::string::npos)
        {
            line = httpRequest.substr(0, pos);
            httpRequest.erase(0, pos + 2);
        }
        else
            line = httpRequest;

        if (line.empty())
            break;
        else if (line.find("Host:") == 0 || line.find("host:") == 0)
        {
            countPost++;
            countOther++;
        }
        else if (line.find("Content-Length:") == 0 || line.find("content-length:") == 0)
        {
            countPost++;
            std::string lengthValue = line.substr(16); // Extract the value after "Content-Length:" or "content-length:"
            std::istringstream lengthStream(lengthValue);
            int contentLength;

            if (!(lengthStream >> contentLength) || contentLength <= 0)
                return false;
        }
        else if (line.find("Content-Type:") == 0 || line.find("content-type:") == 0)
            countPost++;

        if (line.length() > 8000)
            return false;

        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            std::string headerKey = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            if (headerKey.find(' ') != std::string::npos || headerKey.empty())
                return false;
        }
    }


    if (countOther == 1 && (method == "GET" || method == "DELETE"))
        validRequest = true;
    else if (countPost == 3 && method == "POST")
        validRequest = true;
    else 
        validRequest = false;

    return validRequest;
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

    std::map<int, Client>::iterator clientIt;

    for (clientIt = this->_clientList.begin() ; clientIt != this->_clientList.end() ; clientIt++)
        close(clientIt->first);
}

void StartServers::listenClientRequest()
{
    epoll_event events[64];

    while (true)
    {
        int numEvents = epoll_wait(_epollFd, events, 64, -1);
        if (EXIT_G == true)
            break;
        for (int i = 0; i < numEvents; i++)
        {
            bool isNewClient = getNewConnexion(events[i]);

            if (!isNewClient)
            {
                // if (events[i].events & EPOLLERR) check epoll error
                if (events[i].events & EPOLLIN)
                    processRequest(events[i]);
                if (events[i].events & EPOLLOUT)
                    processResponse(events[i]);
            }

        }
    }
}