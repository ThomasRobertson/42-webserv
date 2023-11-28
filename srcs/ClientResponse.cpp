#include "ClientResponse.hpp"

// ClientResponse::ClientResponse(std::string status, std::string contentType, std::string contentBody) : 
// _status(status), _contentType(contentType), _contentBody(contentBody), _serverProtocol(SERVER_PROTOCOL), _serverSoftware(SERVER_SOFTWARE)
ClientResponse::ClientResponse(std::string status, std::string contentType, std::string contentBody, std::string authenticateRealm, std::string cookieSet, std::vector<std::string> extraHeaders) : _status(status), _contentType(contentType), _contentBody(contentBody), _serverProtocol(SERVER_PROTOCOL), _serverSoftware(SERVER_SOFTWARE), _cookieSet(cookieSet), _authenticateRealm(authenticateRealm), _extraHeaders(extraHeaders)
{
	#ifdef DEBUG

	assert(!_status.empty());
	assert(!_contentType.empty());
	
	#endif // DEBUG
	std::stringstream _dateStream;
	std::time_t result = std::time(NULL);
	_dateStream << std::gmtime(&result);
	_date = _dateStream.str();

	_contentLength = sizeToString(_contentBody.length());
}

void ClientResponse::generateResponse()
{
	static const std::string spaceDelimiter = " ";
	static const std::string newLineDelimiter = "\r\n";

	_reponse = _serverProtocol;
	_reponse += spaceDelimiter;

	std::pair<std::string, std::string> statusCode = getStatus(_status);
	_reponse += statusCode.first;
	_reponse += spaceDelimiter;
	_reponse += statusCode.second;
	_reponse += newLineDelimiter;

	_reponse += "Connection: close";
	_reponse += newLineDelimiter;

	_reponse += "Server: ";
	_reponse += _serverSoftware;
	_reponse += newLineDelimiter;

	// _reponse += "Date: ";
	// _reponse += _date;
	// _reponse += newLineDelimiter;

	_reponse += "Content-Type: ";
	_reponse += _contentType;
	_reponse += newLineDelimiter;

	_reponse += "Content-Length: ";
	_reponse += _contentLength;
	_reponse += newLineDelimiter;

	if (!_cookieSet.empty())
	{
		_reponse += "Set-Cookie: ";
		_reponse += _cookieSet;
		_reponse += newLineDelimiter;
	}
	
	if (!_authenticateRealm.empty())
	{
		_reponse += "WWW-Authenticate: ";
		_reponse += "Basic realm=";
		_reponse += _authenticateRealm;
		_reponse += newLineDelimiter;
	}

	// _reponse += "Accept-Ranges: bytes";
	// _reponse += newLineDelimiter;

	if (!_extraHeaders.empty())
	{
		for (std::vector<std::string>::iterator it = _extraHeaders.begin(); it != _extraHeaders.end(); it++)
		{
			_reponse += *it;
			_reponse += newLineDelimiter;
		}
	}

	if (!_contentBody.empty())
	{
		_reponse += newLineDelimiter;
		_reponse += _contentBody;
	}
}

std::string ClientResponse::getReponse()
{
	generateResponse();
	return (_reponse);
}
