#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include "ClientResponse.hpp"
#include "GenerateMethod.hpp"
#include "utils.hpp"

bool DEBUG_VERBOSE = false;

std::string StartServers::getUserResponse(Client client)
{
	std::string response;

	Server currentServer = this->_serversVec[client.serverIndex];

	if (client.request.method == "GET")
		return GenerateMethod::GETMethod(client, currentServer);

	if (client.request.method == "POST")
	{
		ClientResponse client("200", "application/json", "{\"status\": \"success\", \"message\": \"The POST request was processed successfully.\", \"data\": {\"key1\": \"value10\", \"key2\": \"value2\"}}");

		return client.getReponse();
	}
	
	else if (client.request.method == "DELETE")
	{
		ClientResponse client("200", "application/json", "{\"status\": \"success\", \"message\": \"The DELETE request was processed successfully.\"}");
		return client.getReponse();
	}

	else
		return GenerateMethod::getErrorPageResponse(client, currentServer, "405");
}

std::string getRequestBody(std::string request)
{
	size_t boundaryStartPos, boundaryEndPos, bodyStartPos, bodyEndPos;
	std::string boundary, body;

    boundaryStartPos = request.find("boundary=") + std::strlen("boundary=");
	boundaryEndPos = request.find("\r", boundaryStartPos);
	boundary = "--" + request.substr(boundaryStartPos, boundaryEndPos - boundaryStartPos);

	bodyStartPos = request.find(boundary) + boundary.size();
	bodyStartPos = request.find("\r\n\r\n", bodyStartPos) + std::strlen("\r\n\r\n");
	bodyEndPos = request.find(boundary + "--") - std::strlen("\r\n");

	body = request.substr(bodyStartPos, bodyEndPos - bodyStartPos);

	return body;
}

std::string getFileName(UserRequest request)
{
	size_t nameStartPos = request.body.find("filename=") + std::strlen("filename=\"");
	size_t nameEndPos = request.body.find("\"", nameStartPos);
	
	return request.body.substr(nameStartPos, nameEndPos - nameStartPos);
}

void StartServers::createFile(UserRequest request, Server currentServer) // check if file already exists with same name
{
	std::string fileName = getFileName(request);
	std::string body = getRequestBody(request.body);
	std::string status;
	bool is_dir; //TODO: Check value
	


	std::string fileLocation = currentServer.getFileRoute("/form", status, request.method, is_dir); //TODO: replace "/form" with the correct URL
	fileLocation += "/";
	fileLocation += fileName;
	std::cout << "Trying to post to : " << fileLocation << "(from fileName: " << fileName << ")" << std::endl;

	std::ofstream outputFile(fileLocation.c_str(), std::ios::binary);
    if (outputFile.is_open())
    {
        outputFile.write(body.c_str(), body.size());
        outputFile.close();
        std::cout << "Binary data has been written to " << std::endl;
    }
    else
        std::cerr << "Failed to open the file for writing." << std::endl;
}

int StartServers::deleteFiles(UserRequest request, Server currentServer)
{
	std::string status;
	bool is_dir; //TODO: check value

	std::string fileLocation = currentServer.getFileRoute(request.root, status, request.method, is_dir);

    DIR* dir;
    struct dirent* entry;

    // Open the directory
    dir = opendir(fileLocation.c_str());

    if (!dir)
	{
        std::cerr << "Error opening directory." << std::endl;
        return 1;
    }

    while ((entry = readdir(dir)) != NULL)
	{
        if (entry->d_type == DT_REG) // Check if it's a regular file
            std::cout << entry->d_name << std::endl; // Print the file name
    }

    closedir(dir);
	return 0;
}

void StartServers::processResponse(epoll_event currentEvent)
{
    std::string response;
	Client currentClient = _clientList[currentEvent.data.fd];

	Server currentServer = this->_serversVec[currentClient.serverIndex];


    std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;
    if (currentClient.request.method == "POST")
	{
        std::cout << currentClient.request.body << std::endl;
		createFile(currentClient.request, currentServer);
	}

    if (currentClient.request.method == "DELETE")
	{
		std::cout << "DELETE METH" << std::endl;
		deleteFiles(currentClient.request, currentServer);
	}

	// if (isValidRequest(client.request))
		response = getUserResponse(currentClient);
    // else
        // response = GenerateMethod::getErrorPageResponse(client, server, "400");

    write(currentEvent.data.fd, response.c_str(), response.length());

    _clientList.erase(currentEvent.data.fd);
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, currentEvent.data.fd, NULL);
    close(currentEvent.data.fd);

    std::cout << RED << "[i] Client disconnected: " << currentEvent.data.fd << DEFAULT << std::endl;
}