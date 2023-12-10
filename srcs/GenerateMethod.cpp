#include "GenerateMethod.hpp"
#include "ClientResponse.hpp"
#include "ConfigFile.hpp"
#include "Settings.hpp"
#include "cgi.hpp"
#include "utils.hpp"
#include <ostream>
#include <stdexcept>
#include <string>

std::string GenerateMethod::generateRedirect(std::string url)
{
	std::string response;

	response = "HTTP/1.1 307 Temporary Redirect\n";
	response += "Location:";
	response += url;

	return response;	
}

std::string GenerateMethod::CGIMethod()
{
	std::string status, body, response, cgiBinLocation;
	bool is_dir = false;

	std::string fileLocation = _server.getFileRoute(_client.request.route, status, _client.request.method, is_dir, true);
	std::map<std::string, std::string> CGIMap = _server.getCgiPages();

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
		body = getBoundaryRequestBody();
	else
		body = getChunkedRequestBody();

	CgiHandler CGI(_client, _server, fileLocation, cgiBinLocation,body);
	response = CGI.execute();

	return response;
}

std::string GenerateMethod::GETMethod()
{
	Location location;

	std::string response, fileLocation, contentType, status, htmlContent;
	bool is_dir = false;
	fileLocation = _server.getFileRoute(_client.request.route, status, "GET", is_dir);
	// std::cout << "File location : " << fileLocation << std::endl;
	// std::cout << "Status : " << status << std::endl;

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

	ClientResponse clientReponse(true, status, contentType, htmlContent);
	response = clientReponse.getReponse();

	file.close();
	return response;
}

std::string getFileName(std::string body)
{
	size_t startPos = body.find("filename=");
	if (startPos == std::string::npos)
	{
		time_t date = getDate();
		std::stringstream ss;
    	ss << date;
		return ss.str();
	}
	startPos += std::strlen("filename=\"");
	size_t endPos = body.find("\"", startPos);

	return body.substr(startPos, endPos - startPos);
}

void createFile(std::string multipartHeader, std::string multipartBinary, std::string postRoot)
{
	std::string fileName = getFileName(multipartHeader);
	std::string fileLocation = "./" + postRoot + "/" + fileName; // change with upload dir of the server

	std::ofstream outputFile(fileLocation.c_str(), std::ios::binary);
	if (outputFile.is_open())
	{
		outputFile.write(multipartBinary.c_str(), multipartBinary.size());
		outputFile.close();
		std::cout << "Binary data has been written to " << std::endl;
	}
	else
		std::cerr << "Failed to open the file for writing." << std::endl;
}

std::string getMultipartBoundary(std::string body)
{
	std::string boundary;
	size_t startPos, endPos;

	startPos = body.find("boundary=");
	if (startPos == std::string::npos)
		return boundary;
	startPos += std::strlen("boundary=");

	endPos = body.find("\r\n", startPos);
	if (endPos == std::string::npos)
		return boundary;
	boundary = "--" + body.substr(startPos, endPos - startPos);
	return boundary;
}

void parseMultipartRequest(std::string body, std::string postRoot)
{
	std::string boundary, multipartHeader, multipartBinary;
	size_t endPos, startPos = 0;

	boundary = getMultipartBoundary(body);
	if (boundary.empty())
		return;

	while (1)
	{
		startPos = body.find(boundary, startPos);
		if (startPos == std::string::npos)
			return;
		startPos += boundary.size() + std::strlen("\r\n");

		endPos = body.find("\r\n\r\n", startPos);
		if (endPos != std::string::npos) // add multipartHeader only if it was found.
		{
			multipartHeader = body.substr(startPos, endPos - startPos);
			startPos = endPos + std::strlen("\r\n\r\n");
		}

		endPos = body.find(boundary, startPos);
		if (endPos == std::string::npos)
			return;
		endPos -= std::strlen("\r\n");

		multipartBinary = body.substr(startPos, endPos - startPos);
		createFile(multipartHeader, multipartBinary, postRoot);

		// std::cout << RED << multipartBinary << DEFAULT << std::endl;
		startPos = endPos;
	}
}

std::string GenerateMethod::POSTMethod(Location location)
{
	if (_client.request.bodySize > _client.server->getMaxClientBodySize())
		return getErrorPageResponse("413");

	std::string status, contentType, body;

	if (_client.request.transferEncoding == "chunked")
		body = getChunkedRequestBody();
	// else if (_client.request.contentType == "multipart/form-data")
	// 	body = getBoundaryRequestBody();
	else
		body = getDefaultRequestBody();

	bool is_dir = false; //TODO: Check value
	
	std::string fileLocation = _server.getFileRoute(_client.request.route, status, "POST", is_dir);

	if (status != "200")
	{
		contentType = getContentType(".html");
		return getErrorPageResponse(status);
	}

	if (_client.request.contentType == "multipart/form-data")
		parseMultipartRequest(_client.request.fullStr, location.postRoot);

	ClientResponse response(true, status, "text/plain");

	return response.getReponse();
}

std::string GenerateMethod::getDefaultRequestBody()
{
	std::string body;
	size_t startPos;
	std::string request = _client.request.fullStr;
	int contentLength = _client.request.contentLength;

	startPos = request.find("\r\n\r\n");
	if (startPos == std::string::npos)
		return "";
	startPos += std::strlen("\r\n\r\n");

	if (startPos + contentLength > request.size())
		body = request.substr(startPos);
	else
	{
		body = request.substr(startPos, contentLength);
		std::cout << "IN CONTENT LENGTH RANGE" << std::endl;
	}
	return body;
}

std::string GenerateMethod::getBoundaryRequestBody()
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
		{
			std::string filename = "./" + fileLocation + "/" + entry->d_name;

			if (std::remove(filename.c_str()) == 0)
				std::cout << "File '" << filename << "' successfully deleted.\n";
			else
				std::cout << "Error deleting the file.\n";
		}
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
