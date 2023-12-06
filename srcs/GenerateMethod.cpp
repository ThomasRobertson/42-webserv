#include "GenerateMethod.hpp"
#include "ClientResponse.hpp"
#include "Settings.hpp"
#include "cgi.hpp"
#include "utils.hpp"
#include <ostream>
#include <stdexcept>
#include <string>

std::string GenerateMethod::CGIMethod()
{
	std::string status, body;
	bool is_dir = false;

	std::string fileLocation = _server.getFileRoute(_client.request.route, status, _client.request.method, is_dir, true);

	std::string cgiBinLocation;

	std::map<std::string, std::string> CGIMap = _server.getCgiPages();


	// for (std::map<std::string, std::string>::iterator it = CGIMap.begin(); it != CGIMap.end(); it++)
	// 	std::cout << it->first << " ; " << it->second << std::endl;

	// std::cout << "cgi : " << std::string(parseFileExtension(fileLocation)) << std::endl;

	std::string fileExtension = parseFileExtension(fileLocation);

	std::map<std::string, std::string>::iterator it = CGIMap.find(fileExtension);
	(void)it;

	if (CGIMap.find(parseFileExtension(fileLocation)) != CGIMap.end())
	{
		cgiBinLocation = CGIMap.find(parseFileExtension(fileLocation))->second;
	}
	else
	{
		std::cout << RED << "CGI was called but no CGI Bin was find.\n" << DEFAULT;
		throw std::runtime_error("");
	}

	if (_client.request.transferEncoding == "default")
	{
		body = getRequestBody();
		// std::cout << YELLOW << "DEFAULT BODY:" << std::endl;
		// std::cout << body << DEFAULT << std::endl;
	}
	else
	{
		body = getChunkedRequestBody();
		// std::cout << YELLOW << "CHUNKED BODY:" << std::endl;
		// std::cout << body << DEFAULT << std::endl;
	}

	CgiHandler CGI(_client, _server, fileLocation, cgiBinLocation, body);
	
	std::string response;

	try
	{
		response = CGI.execute();
		//std::cout << GREEN << "reponse is:" << response << DEFAULT << std::endl;
	}
	catch (const std::exception&)
	{
		response = getErrorPageResponse("500");
		//std::cout << GREEN << "reponse is:" << response << DEFAULT << std::endl;
	}

	return response;
}

std::string GenerateMethod::GETMethod()
{
	std::string response, fileLocation, contentType, status, htmlContent;
	bool is_dir = false;
	fileLocation = _server.getFileRoute(_client.request.route, status, _client.request.method, is_dir);
	std::cout << "File location : " << fileLocation << std::endl;
	std::cout << "Status : " << status << std::endl;

	if (status != "200")
	{
		std::cout << "Error while looking for the file, error : " << status << std::endl;
		contentType = getContentType(".html");
		return getErrorPageResponse(status);
	}

	if (is_dir)
	{
		std::cout << "Listing Directory." << std::endl;
		return listingDirectory(fileLocation, _client.request.route);
	}

	contentType = getContentType(fileLocation);

	std::ifstream file(fileLocation.c_str());
	if (!file.is_open())
	{
		std::cout << "Could not open file." << std::endl;
		contentType = getContentType(".html");
		return getErrorPageResponse("500");
	}

	htmlContent = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::cout << "Content generated for " << fileLocation << "." << std::endl;
	// std::cout << "Body : \n" << htmlContent << std::endl;
	ClientResponse clientReponse(true, status, contentType, htmlContent);
	response = clientReponse.getReponse();

	file.close();
	return response;
}

std::string GenerateMethod::POSTMethod()
{
	if (_client.request.bodySize > _client.server->getMaxClientBodySize())
	{
		std::string response = "HTTP/1.1 413 Content Too Large\r\n";
		response += "Connection: close\r\n";
		response += "Server: Webserv/42.1\r\n";
		response += "Content-Type: text/plain\r\n";
		response += "Content-Length: 0\r\n";
		response += "\r\n";
		std::cout << "CONTENT TOO LARGE ERROR SENT" << std::endl;
		return response;
	}

	std::string fileName = getFileName();
	std::string status, contentType, body;

	if (_client.request.transferEncoding == "default")
	{
		body = getRequestBody();
		std::cout << YELLOW << "DEFAULT BODY:" << std::endl;
		std::cout << body << DEFAULT << std::endl;
	}
	else
	{
		body = getChunkedRequestBody();
		std::cout << YELLOW << "CHUNKED BODY:" << std::endl;
		std::cout << body << DEFAULT << std::endl;
	}

	bool is_dir = false; //TODO: Check value
	
	std::string fileLocation = _server.getFileRoute("/form", status, _client.request.method, is_dir); //TODO: replace "/form" with the correct URL

	if (status != "200")
	{
		contentType = getContentType(".html");
		return getErrorPageResponse(status);
	}

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

	ClientResponse response(true, status, "text/plain");

	return response.getReponse();
}

std::string GenerateMethod::getRequestBody()
{
	std::string request = _client.request.fullStr;
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

// POST 127.0.0.1 HTTP1.1\r\n
// \r\n
// 5\r\n
// Hello\r\n
// 2\r\n
// cv\r\n
// 0\r\n
// \r\n

std::string GenerateMethod::getChunkedRequestBody()
{
	std::string request = _client.request.fullStr;
	size_t startPos, endPos;
	std::string chunkSizeStr;
	int chunkSize = 0;
	std::string body;

	startPos = request.find("\r\n\r\n") + std::strlen("\r\n\r\n");
	if (startPos == std::string::npos) // throw internal server error
		return body;

	while (true)
	{
		endPos = request.find("\r\n", startPos);
		if (endPos == std::string::npos) // throw internal server error
			break ;
		chunkSizeStr = request.substr(startPos, endPos - startPos);
		chunkSize = hexStringToInt(chunkSizeStr);
		if (chunkSize == 0)
			break ;
		startPos = endPos + std::strlen("\r\n");
		body += request.substr(startPos, chunkSize);
		startPos += chunkSize + std::strlen("\r\n");
	}
	return body;
}

std::string GenerateMethod::getFileName()
{
	size_t nameStartPos = _client.request.fullStr.find("filename=") + std::strlen("filename=\"");
	size_t nameEndPos = _client.request.fullStr.find("\"", nameStartPos);

	return _client.request.fullStr.substr(nameStartPos, nameEndPos - nameStartPos);
}

std::string GenerateMethod::DELETEMethod()
{
	std::string status, contentType;
	bool is_dir = false; //TODO: check value

	std::string fileLocation = _server.getFileRoute(_client.request.route, status, _client.request.method, is_dir);

	if (status != "200")
	{
		contentType = getContentType(".html");
		return getErrorPageResponse(status);
	}

	DIR* dir;
	struct dirent* entry;

	// Open the directory
	dir = opendir(fileLocation.c_str());

	if (!dir)
	{
		std::cerr << "Error opening directory." << std::endl;
		return getErrorPageResponse("500");
	}

	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_type == DT_REG) // Check if it's a regular file
			std::cout << entry->d_name << std::endl; // Print the file name
	}
	closedir(dir);

	ClientResponse response("200", "application/json", "{\"status\": \"success\", \"message\": \"The DELETE request was processed successfully.\"}");

	return response.getReponse();
}

std::string GenerateMethod::getErrorPageResponse(std::string errorCode)
{
	std::string response, fileLocation, contentType, content;

	contentType = "text/html";

	try
	{
		fileLocation = _server.getErrorPageRoute(errorCode);
		std::ifstream file(fileLocation.c_str());
		if (!file.is_open())
		{
			throw std::runtime_error("Cannot open the error page, generating one.");
		}
		content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
	}
	catch (const std::exception& e)
	{
		std::cout << YELLOW << e.what() << DEFAULT << std::endl;
		content = generateErrorPage(errorCode);
	}

	ClientResponse clientReponse(true, errorCode, contentType, content);

	response = clientReponse.getReponse();
	return response;
}

std::string GenerateMethod::listingDirectory(const std::string &fileLocation, std::string &fileName)
{
	std::string contentType, status, content;

	std::stringstream directoryListing;
	directoryListing << "<html><head><title>Webserv Listing</title><link rel=\"stylesheet\" type=\"text/css\" href=\"/styles/styleListing.css\"></head><body><h1 class=\"listing-title\">" << fileLocation << "</h1>";

	DIR *dir = opendir(fileLocation.c_str());
	if (dir != NULL)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)))
		{
			std::string entryName = entry->d_name;
			if (entryName != "." && entryName != "..")
				directoryListing << "<div class=\"listing-buttons-container\"><button class=\"listing-buttons\" onclick='location.href=\"" << fileName << "/" << entryName << "\";'>" << entryName << "</button><br>";
		}
		closedir(dir);
	}
	else
	{
		status = "500";
		content = getErrorPageResponse(status);
		contentType = "text/html";
		ClientResponse clientReponse(true, status, contentType, content);
		return clientReponse.getReponse();
	}
	
	directoryListing << "</div></body></html>";

	status = "200";
	contentType = "text/html";
	content = directoryListing.str();
	ClientResponse clientReponse(true, status, contentType, content);

	return clientReponse.getReponse();
}
