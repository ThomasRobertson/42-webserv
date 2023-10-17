#include "Server.hpp"

Server::Server(ConfigFile configFile) : _configFile(configFile)
{
    return ;
}

Server::~Server()
{
    return ;
}

void Server::listenClientRequest(int serverSocket)
{
    while (true)
    {
        sockaddr_in clientAddress;
        socklen_t clientAddressSize = sizeof(clientAddress);
    
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressSize);
        if (clientSocket == -1)
        {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }
        
        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == -1)
        {
            std::cerr << "Error receiving data from client" << std::endl;
            close(clientSocket);
            continue;
        }

        std::string requestData(buffer, bytesRead);

        UserRequest userRequest = getUserRequest(requestData);

        std::string response = manageUserResponse(userRequest, this->_configFile);

        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
    }
}

int Server::startServer()
{
    ConfigFile configFile = this->_configFile;
    std::string host = configFile.getHost();
    std::string port = configFile.getPort();

    struct addrinfo hints, *serverInfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &serverInfo) != 0)
    {
        std::cerr << "Error retrieving host information" << std::endl;
        return 0;
    }

    int serverSocket;

    for (p = serverInfo; p != NULL; p = p->ai_next)
    {
        serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (serverSocket == -1)
            continue;

        int iSetOption = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption, sizeof(iSetOption));

        if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(serverSocket);
            continue;
        }

        break;
    }

    freeaddrinfo(serverInfo);

    if (p == NULL)
    {
        std::cerr << "Socket binding error" << std::endl;
        return 0;
    }

    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "Socket listening error" << std::endl;
        close(serverSocket);
        return 0;
    }

    std::cout << "Server listening on port " << port << "..." << std::endl << std::endl;
    listenClientRequest(serverSocket);

    close(serverSocket);
    std::cout << "SERVER CLOSED\n";
    return 1;
}