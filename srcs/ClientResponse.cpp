#include "ClientResponse.hpp"
#include "Settings.hpp"

const std::string ClientResponse::spaceDelimiter = " ";
const std::string ClientResponse::newLineDelimiter = "\r\n";

// ClientResponse::ClientResponse(std::string status, std::string contentType, std::string contentBody) : 
// _status(status), _contentType(contentType), _contentBody(contentBody), _serverProtocol(SERVER_PROTOCOL), _serverSoftware(SERVER_SOFTWARE)
ClientResponse::ClientResponse(bool parseStatus, std::string status, std::string contentType, std::string contentBody, std::string authenticateRealm, std::string cookieSet, std::vector<std::string> extraHeaders) : _status(status), _contentType(contentType), _contentBody(contentBody), _serverProtocol(SERVER_PROTOCOL), _serverSoftware(SERVER_SOFTWARE), _cookieSet(cookieSet), _authenticateRealm(authenticateRealm), _extraHeaders(extraHeaders)
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

	if (parseStatus)
	{
		std::pair<std::string, std::string> statusCode = getStatus(_status);
		
		_status = statusCode.first;
		_status += spaceDelimiter;
		_status += statusCode.second;
		
	}
	std::cout << RED << _status << DEFAULT << std::endl;
}

void ClientResponse::generateResponse()
{
	_reponse = _serverProtocol;
	_reponse += spaceDelimiter;

	_reponse += _status;
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

	_reponse += newLineDelimiter;
	_reponse += _contentBody;
}

std::string ClientResponse::getReponse()
{
	generateResponse();
	// std::cout << YELLOW << "Response is :\n" << _reponse << DEFAULT;
	return (_reponse);
}
