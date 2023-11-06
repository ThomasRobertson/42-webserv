#include "Server.hpp"
#include "ConfigFile.hpp"
#include "StartServers.hpp"
#include "ClientResponse.hpp"
#include "GenerateMethod.hpp"

bool DEBUG_VERBOSE = false;

std::string StartServers::getUserResponse(Client client)
{
	std::string response;

	Server currentServer = this->_serversVec[client.serverIndex];

	if (client.request.method == "POST")
	{
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    	httpResponse += "Content-Type: application/json\r\n\r\n";
    	httpResponse += "{\"status\": \"success\", \"message\": \"The POST request was processed successfully.\", \"data\": {\"key1\": \"value10\", \"key2\": \"value2\"}}";
		return httpResponse;
	}
	else if (client.request.method == "DELETE")
	{
		std::string httpResponse = "HTTP/1.1 200 OK\r\n";
    	httpResponse += "Content-Type: application/json\r\n\r\n";
    	httpResponse += "{\"status\": \"success\", \"message\": \"The DELETE request was processed successfully.\"}";
		return httpResponse;
	}
	else if (client.request.method == "GET")
		return GenerateMethod::GETMethod(client, currentServer);

	#ifdef DEBUG
	else
		throw std::invalid_argument("Unknown method.");
	#endif // DEBUG

	return response;
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

void createFile(UserRequest request) // check if file already exists with same name
{
	std::string fileName = getFileName(request);
	std::string body = getRequestBody(request.body);

	std::ofstream outputFile(("POST/" + fileName).c_str(), std::ios::binary);
    if (outputFile.is_open())
    {
        outputFile.write(body.c_str(), body.size());
        outputFile.close();
        std::cout << "Binary data has been written to " << std::endl;
    }
    else
        std::cerr << "Failed to open the file for writing." << std::endl;
}

int deleteFiles()
{
	const char* folderPath = "./POST"; // Specify the folder path you want to list files from

    DIR* dir;
    struct dirent* entry;

    // Open the directory
    dir = opendir(folderPath);

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

    std::cout << "----------------------- NEW REPONSE: " << currentEvent.data.fd << " -----------------------" << std::endl;
    if (currentClient.request.method == "POST")
	{
        std::cout << currentClient.request.body << std::endl;
		createFile(currentClient.request);
	}

    if (currentClient.request.method == "DELETE")
	{
		std::cout << "DELETE METH" << std::endl;
		deleteFiles();
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