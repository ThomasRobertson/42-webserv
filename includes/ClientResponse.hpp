#pragma once

#include <string>
#include <cassert>
#include <ctime>
#include <iostream>
#include <sstream>

#include "Settings.hpp"
#include "utils.hpp"

class ClientResponse
{
	public:
		ClientResponse(bool parseStatusAndContentType, std::string status, std::string contentType, std::string contentBody = "", std::string authenticateRealm = "", std::string cookieSet = "", std::vector<std::string> extraHeaders = std::vector<std::string>());
		~ClientResponse() {}

		std::string getReponse();

	private:
		std::string _status;
		std::string _contentType;
		std::string _contentBody;
		const std::string _serverProtocol; //ex: HTTP/1.1
		const std::string _serverSoftware; //ex: Webserv/42.0
		std::string _date; //in GMT
		std::string _contentLength;
		std::string _cookieSet;
		std::string _authenticateRealm;
		std::vector<std::string> _extraHeaders;

		std::string _reponse;

		static const std::string spaceDelimiter;
		static const std::string newLineDelimiter;

	void generateResponse();
};
