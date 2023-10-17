#include "Server.hpp"

Server::Server(ConfigFile configFile) : _configFile(configFile)
{
    return ;
}

Server::~Server()
{
    return ;
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
    
    close(serverSocket);
    return 0;
}