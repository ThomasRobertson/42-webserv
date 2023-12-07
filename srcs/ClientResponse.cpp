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

std::string ClientResponse::getReponse()
{
	_response = _serverProtocol;
	_response += spaceDelimiter;

	_response += _status;
	_response += newLineDelimiter;

	_response += "Connection: close";
	_response += newLineDelimiter;

	_response += "Server: ";
	_response += _serverSoftware;
	_response += newLineDelimiter;

	// _response += "Date: ";
	// _response += _date;
	// _response += newLineDelimiter;

	_response += "Content-Type: ";
	_response += _contentType;
	_response += newLineDelimiter;

	_response += "Content-Length: ";
	_response += _contentLength;
	_response += newLineDelimiter;

	if (!_cookieSet.empty())
	{
		_response += "Set-Cookie: ";
		_response += _cookieSet;
		_response += newLineDelimiter;
	}

	if (!_authenticateRealm.empty())
	{
		_response += "WWW-Authenticate: ";
		_response += "Basic realm=";
		_response += _authenticateRealm;
		_response += newLineDelimiter;
	}

	if (!_extraHeaders.empty())
	{
		for (std::vector<std::string>::iterator it = _extraHeaders.begin(); it != _extraHeaders.end(); it++)
		{
			_response += *it;
			_response += newLineDelimiter;
		}
	}

	_response += newLineDelimiter;
	_response += _contentBody;

	return _response;
}