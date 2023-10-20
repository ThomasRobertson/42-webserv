#include "Server.hpp"

int Server::testServer(std::string hostStr, std::string portStr)
{
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

    if (!testListenClientRequest(serverSocket, epollFd))
        return 0;
    
    close(epollFd);
    close(serverSocket);
    return 1;
}

int Server::testListenClientRequest(int serverSocket, int epollFd)
{
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = serverSocket;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        std::cerr << "Error adding server socket to epoll" << std::endl;
        close(serverSocket);
        close(epollFd);
        return 0;
    }

    UserRequest userRequest; 
    std::string response;
    while (true)
    {
        struct epoll_event events[10];
        int numEvents = epoll_wait(epollFd, events, 10, -1);

        for (int i = 0; i < numEvents; i++)
        {
            if (events[i].data.fd == serverSocket)
            {
                sockaddr_in clientAddr;
                socklen_t clientAddrSize = sizeof(clientAddr);

                int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
                if (clientSocket == -1)
                {
                    std::cerr << "Error accepting connection" << std::endl;
                    continue;
                }

                int flags = fcntl(clientSocket, F_GETFL, 0);
                fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

                event.events = EPOLLIN | EPOLLOUT | EPOLLET;
                event.data.fd = clientSocket;


                if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1)
                {
                    std::cerr << "Error adding client socket to epoll" << std::endl;
                    close(clientSocket);
                    continue;
                }
            }
            else
            {
                int clientSocket = events[i].data.fd;

                if (events[i].events & EPOLLIN)
                {
                    char buffer[1024];
                    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                    if (bytesRead <= 0)
                    {
                        close(clientSocket);
                    }
                    else
                    {
                        std::string requestData(buffer, bytesRead);
                        userRequest = getUserRequest(requestData);
                        std::cout << "Received HTTP request:\n" << requestData << std::endl;
                    }
                }

                if (events[i].events & EPOLLOUT)
                {
                    response = getUserResponse(userRequest, this->_configFile);
                    ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
                    if (bytesSent == -1)
                    {
                        std::cerr << "Error sending response" << std::endl;
                        close(clientSocket);
                    }
                    else
                    {
                        close(clientSocket);
                    }
                }
            }
        }
    }
    return 1;
}


void epollOut()
{
    
}