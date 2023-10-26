#include "Server.hpp"
#include <vector>
#include "Client.hpp"

Server::Server(ConfigFile configFile) : _configFile(configFile)
{
    return ;
}

Server::~Server()
{
    return ;
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

int isConnectedAddress(std::vector<Client> connectedAddress, Client newClient)
{
    std::vector<Client>::iterator it = connectedAddress.begin();

    while (it != connectedAddress.end())
    {
        if (*it == newClient)
            return 1;
        it++;
    }
    return 0;
}

int Server::listenClientRequest(int serverSocket, int epollFd)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1)
    {
        std::cerr << "Error adding server socket to epoll" << std::endl;
        close(serverSocket);
        close(epollFd);
        return 0;
    }

    UserRequest userRequest; 
    epoll_event events[100];
    sockaddr clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    std::string response;
    int clientSocket;
    std::vector<Client> connectedAddress;

    while (true)
    {
        int numEvents = epoll_wait(epollFd, events, 100, -1);
        for (int i = 0; i < numEvents; i++)
        {
            std::cout << "EVENT NBR: " << i << std::endl;
            if (events[i].data.fd == serverSocket)
            {
                clientSocket = accept(serverSocket, &clientAddr, &clientAddrLen);
                setNonBlocking(clientSocket);

                Client currentClient(&clientAddr);
                // if (isConnectedAddress(connectedAddress, currentClient))
                // {
                    // std::cout << "\033[31m" << "Client already connected, disconnected: " << clientSocket << "\033[0m" << std::endl;
                    // epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, NULL);
                    // close(clientSocket);
                    // i--;
                // }
                // else
                // {

                    event.data.fd = clientSocket;
                    event.events = EPOLLIN;
                    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);
                    std::cout << "\033[32m" << "New client with ip: " << currentClient.getIpAddress() << "\033[0m" << std::endl;
                    connectedAddress.push_back(currentClient);
                // }
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    std::cout << "----------------------- NEW REQUEST: " << events[i].data.fd << " -----------------------" << std::endl;
                    char buffer[1024];
                    ssize_t bytesRead = read(events[i].data.fd, buffer, 1024);
                    if (bytesRead <= 0)
                    {
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, &event);
                        close(events[i].data.fd);
                        std::cout << "\033[31m" << "Client disconnected from error: " << clientSocket << "\033[0m" << std::endl;
                    }
                    else
                    {
                        std::string requestData(buffer, bytesRead);
                        userRequest = getUserRequest(requestData);
                        std::cout << "Received HTTP request:\n" << requestData << std::endl;

                        event.data.fd = events[i].data.fd;
                        event.events = EPOLLIN | EPOLLOUT;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &event);
                    }
                }
                if (events[i].events & EPOLLOUT)
                {
                    std::cout << "----------------------- NEW REPONSE: " << events[i].data.fd << " -----------------------" << std::endl;
                    response = getUserResponse(userRequest, this->_configFile);

                    ssize_t bytesSent = write(events[i].data.fd, response.c_str(), response.length());
                    std::cout << "response sent " << response.substr(0, 10) << std::endl;

                    event.data.fd = events[i].data.fd;
                    event.events = EPOLLIN;
                    epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &event);
                }
            }
        }
    }
}

int Server::startServer()
{
    std::string hostStr = this->_configFile.getHost();
    std::string portStr = this->_configFile.getPort();

    const char *host = hostStr.c_str();
    const char *port = portStr.c_str();

    addrinfo hints, *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(host, port, &hints, &result) != 0) {
        std::cerr << "Error retrieving host information" << std::endl;
        return 0;
    }

    int serverSocket = -1;

    for (rp = result; rp != NULL; rp = rp->ai_next) {
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

    if (rp == NULL) {
        std::cerr << "Socket binding error" << std::endl;
        return 0;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Socket listening error" << std::endl;
        close(serverSocket);
        return 0;
    }

    std::cout << "Server listening on port " << port << "..." << std::endl;

    int flags = fcntl(serverSocket, F_GETFL, 0);
    fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        std::cerr << "Error creating epoll" << std::endl;
        close(serverSocket);
        return 0;
    }

    if (!listenClientRequest(serverSocket, epollFd))
        return 0;
    
    close(epollFd);
    close(serverSocket);
    return 1;
}