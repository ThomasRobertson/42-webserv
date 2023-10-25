#include "Server.hpp"
#include <set>

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

int isConnectedAddress(std::set<char *> connectedAddress, struct sockaddr clientAddr)
{
    std::set<char *>::iterator it = connectedAddress.find(clientAddr.sa_data);

    if (it != connectedAddress.end())
        return 1;
    return 0;
}

void convertt(sockaddr sa)
{
    // Assume sa contains a valid IPv4 address

    if (sa.sa_family == AF_INET) {
        // Check if it's an IPv4 address

        // Cast the sockaddr pointer to sockaddr_in
        sockaddr_in* ipv4Addr = (sockaddr_in*)&sa;

        // Extract the IPv4 address
        unsigned char* addrBytes = (unsigned char*)&(ipv4Addr->sin_addr);

        char ipAddress[INET_ADDRSTRLEN]; // Buffer for the IP address
        int index = 0;

        for (int i = 0; i < 4; ++i) {
            int num = static_cast<int>(addrBytes[i]);
            if (i > 0) {
                ipAddress[index++] = '.';
            }
            // Convert each byte to string and append it to the buffer
            do {
                ipAddress[index++] = '0' + num % 10;
                num /= 10;
            } while (num > 0);
        }
        ipAddress[index] = '\0';

        std::cout << "IPv4 Address: " << ipAddress << std::endl;
    } else {
        std::cout << "Not an IPv4 address." << std::endl;
    }
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
    std::string response;
    int clientSocket;
    struct epoll_event events[10];
    struct sockaddr clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    std::set<char *> connectedAddress;

    while (true)
    {
        int numEvents = epoll_wait(epollFd, events, 10, -1);
        for (int i = 0; i < numEvents; i++)
        {
            if (events[i].data.fd == serverSocket)
            {
                clientSocket = accept(serverSocket, &clientAddr, &clientAddrLen);
                setNonBlocking(clientSocket);

                event.data.fd = clientSocket;
                event.events = EPOLLIN;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);
                // if (isConnectedAddress(connectedAddress, clientAddr))
                // {
                //     std::cout << "\033[31m" << "Client already connected, disconnected: " << clientSocket << "\033[0m" << std::endl;
                //     epoll_ctl(epollFd, EPOLL_CTL_DEL, clientSocket, NULL);
                //     close(events[i].data.fd);
                // }
                // else
                // {
                    // connectedAddress.insert(clientAddr.sa_data);
                    if (clientAddr.sa_family == AF_INET) std::cout << "IPV4" << std::endl;
                    convertt(clientAddr);
                    if (clientAddr.sa_family == AF_INET6) std::cout << "IPV6" << std::endl;
                    std::cout << "\033[32m" << "New client connected: " << clientSocket << "\033[0m" << std::endl;
                // }
            }
            else
            {
                if (events[i].events & EPOLLOUT)
                {
                    std::cout << "----------------------- NEW REPONSE: " << events[i].data.fd << " -----------------------" << std::endl;
                    response = getUserResponse(userRequest, this->_configFile);

                    ssize_t bytesSent = write(events[i].data.fd, response.c_str(), response.length());
                    // std::cout << "response sent" << response.substr(0, 200) << std::endl;

                    std::cout << "\033[31m" << "Client disconnected: " << clientSocket << "\033[0m" << std::endl;
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
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
                        event.events = EPOLLOUT;
                        epoll_ctl(epollFd, EPOLL_CTL_MOD, events[i].data.fd, &event);
                    }
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

    struct addrinfo hints, *result, *rp;
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