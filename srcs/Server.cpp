#include "Server.hpp"

Server::Server()
{
    return ;
}

Server::~Server()
{
    return ;
}

struct UserRequest {
    std::string method;
    std::string root;
};

UserRequest getUserRequest(std::string requestStr)
{
    UserRequest data;
    
    size_t spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.method = requestStr.substr(0, spaceSepPos);
    requestStr.erase(0, spaceSepPos + 1);
    spaceSepPos = requestStr.find(' '); // first space char after "GET /scripts/script.js HTTP/1.1"
    data.root = requestStr.substr(0, spaceSepPos);
    return data;
}

void listenClientRequest(int serverSocket)
{
    std::ifstream htmlFile("www/srcs/index.html"); 
    if (!htmlFile.is_open())
    {
        std::cerr << "Failed to open HTML file." << std::endl;
        return ;
    }

    std::string htmlContent((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());

    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + htmlContent + "\r\n";
    response += "\r\n";
    response += htmlContent;

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
        if (userRequest.root == "/exit")
        {
            send(clientSocket, response.c_str(), response.size(), 0);
            close(clientSocket);
            break;
        }
        send(clientSocket, response.c_str(), response.size(), 0);
        close(clientSocket);
    }
}

int Server::startServer(std::string hostStr, std::string portStr)
{
    const char *host = hostStr.c_str();
    const char *port = portStr.c_str();

    struct addrinfo hints, *serverInfo, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &serverInfo) != 0)
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

// #include <fcntl.h>
// #include <sys/epoll.h>

// int Server::testServer(std::string hostStr, std::string portStr)
// {

//     const char *host = hostStr.c_str();
//     const char *port = portStr.c_str();

//     struct addrinfo hints, *result, *rp;
//     memset(&hints, 0, sizeof(struct addrinfo));
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE;

//     if (getaddrinfo(host, port, &hints, &result) != 0) {
//         std::cerr << "Error retrieving host information" << std::endl;
//         return 1;
//     }

//     int serverSocket = -1;

//     for (rp = result; rp != NULL; rp = rp->ai_next) {
//         serverSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
//         if (serverSocket == -1)
//             continue;

//         if (bind(serverSocket, rp->ai_addr, rp->ai_addrlen) == 0)
//             break;

//         close(serverSocket);
//     }

//     freeaddrinfo(result);

//     if (rp == NULL) {
//         std::cerr << "Socket binding error" << std::endl;
//         return 1;
//     }

//     if (listen(serverSocket, 5) == -1) {
//         std::cerr << "Socket listening error" << std::endl;
//         close(serverSocket);
//         return 1;
//     }

//     std::cout << "Server listening on port " << port << "..." << std::endl;

//     int flags = fcntl(serverSocket, F_GETFL, 0);
//     fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

//     int epollFd = epoll_create1(0);
//     if (epollFd == -1) {
//         std::cerr << "Error creating epoll" << std::endl;
//         close(serverSocket);
//         return 1;
//     }

//     struct epoll_event event;
//     event.events = EPOLLIN | EPOLLET;
//     event.data.fd = serverSocket;
//     if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
//         std::cerr << "Error adding server socket to epoll" << std::endl;
//         close(serverSocket);
//         close(epollFd);
//         return 1;
//     }

//     std::ifstream htmlFile("www/index.html");
//     if (!htmlFile.is_open()) {
//         std::cerr << "Failed to open HTML file." << std::endl;
//         close(serverSocket);
//         close(epollFd);
//         return 1;
//     }

//     std::string htmlContent((std::istreambuf_iterator<char>(htmlFile)), std::istreambuf_iterator<char>());

//     std::string response = "HTTP/1.1 200 OK\r\n";
//     response += "Content-Type: text/html\r\n";
//     response += "Content-Length: " + htmlContent + "\r\n";
//     response += "\r\n";
//     response += htmlContent;

//     while (true) {
//         struct epoll_event events[10];
//         int numEvents = epoll_wait(epollFd, events, 10, -1);

//         for (int i = 0; i < numEvents; i++) {
//             if (events[i].data.fd == serverSocket) {

//                 sockaddr_in clientAddr;
//                 socklen_t clientAddrSize = sizeof(clientAddr);
//                 int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
//                 if (clientSocket == -1) {
//                     std::cerr << "Error accepting connection" << std::endl;
//                     continue;
//                 }

//                 int flags = fcntl(clientSocket, F_GETFL, 0);
//                 fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

//                 event.events = EPOLLIN | EPOLLET;
//                 event.data.fd = clientSocket;
//                 if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
//                     std::cerr << "Error adding client socket to epoll" << std::endl;
//                     close(clientSocket);
//                     continue;
//                 }
//             } else {

//                 int clientSocket = events[i].data.fd;
//                 char buffer[1024];
//                 ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
//                 if (bytesRead <= 0) {

//                     close(clientSocket);
//                 } else {
//                     std::string requestData(buffer, bytesRead);
//                     std::cout << "Received HTTP request:\n" << requestData << std::endl;
//                     send(clientSocket, response.c_str(), response.size(), 0);
//                     close(clientSocket);
//                 }
//             }
//         }
//     }

//     close(epollFd);
//     close(serverSocket);
//     return 0;
// }