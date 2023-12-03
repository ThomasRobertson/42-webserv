#include "StartServers.hpp"
#include "ClientRequest.hpp"
#include "utils.hpp"

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
                newClient.lastActionDate = getDate();
                newClient.server = &(*serverIt);
                newClient.request.isBodyComplete = false; // default value
                newClient.request.isHeaderComplete = false; // default value
				newClient.request.isBodyTooLarge = false; // default value
                newClient.request.contentLength = -1; // default value
                newClient.request.bodySize = 0; // default value
                _clientList[newClient.fd] = newClient;
                return true;
            }
        }
        serverIndex++;
    }
    return false;
}

bool StartServers::isValidRequest(UserRequest requestData, std::string &status)
{
    // std::cout << "requestData: fullRequest: " << requestData.fullRequest << std::endl << "END" << std::endl;

    std::string httpRequest = requestData.fullStr;

    std::size_t pos = 0;
    std::string line;

    int countPost = 0;
    int countOther = 0;
    std::string method, uri, version;

	if (requestData.isBodyTooLarge)
	{
		status = "413";
		return false;
	}
	if (httpRequest.empty())
	{
		status = "400";
		return false;
	}

	pos = httpRequest.find("\r\n");
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
    {   
		status = "405"; 
		return false;
	}
    if (version != "HTTP/1.1")
    {   
		status = "505"; 
		return false;
	}
    // if (uri.find('/') != 0) //!Ne marche pas
    // {   
	// 	status = "400"; 
	// 	return false;
	// }
    if (line.length() > 8000)
    {   
		status = "414"; 
		return false;
	}
    if (std::count(line.begin(), line.end(), ' ') > 2)
    {   
		status = "400"; 
		return false;
	}

	bool lenghtFind = false;
    while (httpRequest.find("\r\n") != std::string::npos)
    {
		pos = httpRequest.find("\r\n");
        if (pos != std::string::npos)
        {
            line = httpRequest.substr(0, pos);
            httpRequest.erase(0, pos + 2);
        }
        else
            line = httpRequest;

        if (line.empty())
            break;
        else if (line.find("Host:") != std::string::npos || line.find("host:") != std::string::npos)
        {
            countPost++;
            countOther++;

        }
        else if (line.find("Transfer-Encoding:") != std::string::npos || line.find("transfer-encoding:") != std::string::npos)
        {
			if (lenghtFind == true)
			{
				status = "400";
				return false;
			}

			lenghtFind = true;
			countPost++;
        }
        else if (line.find("Content-Length:") != std::string::npos || line.find("content-length:") != std::string::npos)
        {
			if (lenghtFind == true)
			{
				status = "400";
				return false;
			}

			lenghtFind = true;
			countPost++;
            std::string lengthValue = line.substr(16); // Extract the value after "Content-Length:" or "content-length:"
            std::istringstream lengthStream(lengthValue);
            int contentLength;

			lengthStream >> contentLength;

            if (contentLength <= 0)
			{
				status = "400";
				return false;
			}
                
        }
        else if (line.find("Content-Type:") != std::string::npos || line.find("content-type:") != std::string::npos)
            countPost++;

        if (line.length() > 8000)
		{
			status = "414";
			return false;
		}

        size_t colonPos = line.find(":");
        if (colonPos != std::string::npos) {
            std::string headerKey = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);

            if (headerKey.empty() || headerKey.find_first_not_of(' ') == std::string::npos)
			{
				status = "400";
				return false;
			}
        }
    }


    if (countOther == 1 && (method == "GET" || method == "DELETE"))
        return true;
    else if (countPost >= 3 && method == "POST")
        return true;
    else
	{
		status = "400";
		return false;
	}
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
            if (serverIt->getServerSocket(portIndex) != -1)
                close(serverIt->getServerSocket(portIndex));
            std::cout << YELLOW << "[i] Server shutdown: " << serverIt->getHost() << "::" << *portIt << DEFAULT << std::endl;
        }
    }

    std::map<int, Client>::iterator clientIt;

    for (clientIt = this->_clientList.begin() ; clientIt != this->_clientList.end() ; clientIt++)
    {
        close(clientIt->first);
    }
}

void StartServers::checkTimeout()
{
    std::map<int, Client>::iterator clientIter;
    int timeoutValue = 5;

    for (clientIter = this->_clientList.begin(); clientIter != this->_clientList.end();)
    {
        if (getDate() - clientIter->second.lastActionDate > timeoutValue)
        {
            int clientFd = clientIter->second.fd;
            std::cout << RED << "Client: " << clientFd << " disconnected from timeout." << DEFAULT << std::endl;
            clientIter++;
            _clientList.erase(clientFd);
            epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
            close(clientFd);
        }
        else
            clientIter++;
    }
}

void StartServers::listenClientRequest()
{
    epoll_event events[64];

    while (true)
    {
        int numEvents = epoll_wait(_epollFd, events, 64, 1000);
        checkTimeout();
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