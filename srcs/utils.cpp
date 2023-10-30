#include "utils.hpp"

static std::map<std::string, std::string> constructMapMIME()
{
	static const std::pair<std::string, std::string> extensionToMIME_pair[] = {
		std::make_pair("aac","audio/aac"),
		std::make_pair("abw","application/x-abiword"),
		std::make_pair("arc","application/x-freearc"),
		std::make_pair("avif","image/avif"),
		std::make_pair("avi","video/x-msvideo"),
		std::make_pair("azw","application/vnd.amazon.ebook"),
		std::make_pair("bin","application/octet-stream"),
		std::make_pair("bmp","image/bmp"),
		std::make_pair("bz","application/x-bzip"),
		std::make_pair("bz2","application/x-bzip2"),
		std::make_pair("cda","application/x-cdf"),
		std::make_pair("csh","application/x-csh"),
		std::make_pair("css","text/css"),
		std::make_pair("csv","text/csv"),
		std::make_pair("doc","application/msword"),
		std::make_pair("docx","application/vnd.openxmlformats-officedocument.wordprocessingml.document"),
		std::make_pair("eot","application/vnd.ms-fontobject"),
		std::make_pair("epub","application/epub+zip"),
		std::make_pair("gz","application/gzip"),
		std::make_pair("gif","image/gif"),
		std::make_pair("htm","text/html"),
		std::make_pair("html","text/html"),
		std::make_pair("ico","image/vnd.microsoft.icon"),
		std::make_pair("ics","text/calendar"),
		std::make_pair("jar","application/java-archive"),
		std::make_pair("jpg","image/jpeg"),
		std::make_pair("jpeg","image/jpeg"),
		std::make_pair("js","text/javascript"),
		std::make_pair("json","application/json"),
		std::make_pair("jsonld","application/ld+json"),
		std::make_pair("midi","audio/x-midi"),
		std::make_pair("mid","audio/x-midi"),
		std::make_pair("mjs","text/javascript"),
		std::make_pair("mp3","audio/mpeg"),
		std::make_pair("mp4","video/mp4"),
		std::make_pair("mpeg","video/mpeg"),
		std::make_pair("mpkg","application/vnd.apple.installer+xml"),
		std::make_pair("odp","application/vnd.oasis.opendocument.presentation"),
		std::make_pair("ods","application/vnd.oasis.opendocument.spreadsheet"),
		std::make_pair("odt","application/vnd.oasis.opendocument.text"),
		std::make_pair("oga","audio/ogg"),
		std::make_pair("ogv","video/ogg"),
		std::make_pair("ogx","application/ogg"),
		std::make_pair("opus","audio/opus"),
		std::make_pair("otf","font/otf"),
		std::make_pair("png","image/png"),
		std::make_pair("pdf","application/pdf"),
		std::make_pair("php","application/x-httpd-php"),
		std::make_pair("ppt","application/vnd.ms-powerpoint"),
		std::make_pair("pptx","application/vnd.openxmlformats-officedocument.presentationml.presentation"),
		std::make_pair("rar","application/vnd.rar"),
		std::make_pair("rtf","application/rtf"),
		std::make_pair("sh","application/x-sh"),
		std::make_pair("svg","image/svg+xml"),
		std::make_pair("tar","application/x-tar"),
		std::make_pair("tiff","image/tiff"),
		std::make_pair("tif","image/tiff"),
		std::make_pair("ts","video/mp2t"),
		std::make_pair("ttf","font/ttf"),
		std::make_pair("txt","text/plain"),
		std::make_pair("vsd","application/vnd.visio"),
		std::make_pair("wav","audio/wav"),
		std::make_pair("weba","audio/webm"),
		std::make_pair("webm","video/webm"),
		std::make_pair("webp","image/webp"),
		std::make_pair("woff","font/woff"),
		std::make_pair("woff2","font/woff2"),
		std::make_pair("xhtml","application/xhtml+xml"),
		std::make_pair("xls","application/vnd.ms-excel"),
		std::make_pair("xlsx","application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"),
		std::make_pair("xml","application/xml"),
		std::make_pair("zip","application/zip"),
		std::make_pair("3gp","video/3gpp"),
		std::make_pair("3g2","video/3gpp2"),
		std::make_pair("7z","application/x-7z-compressed")
		};

	const static std::map<std::string, std::string> extensionToMIME(extensionToMIME_pair, extensionToMIME_pair + sizeof extensionToMIME_pair / sizeof extensionToMIME_pair[0]);

	return extensionToMIME;
}

std::string getContentType(std::string fileName)
{
	std::map<std::string, std::string> extensionToMIME = constructMapMIME();

	size_t dotPos = fileName.find_last_of(".");
    if (dotPos == std::string::npos)
		return ("text/plain");

	std::string extension = fileName.substr(dotPos + 1);
	try
	{
		std::string type = extensionToMIME.at(extension);
		return (type);
	}
	catch (const std::exception&)
	{
		return ("text/plain");
	}
}

static std::map<std::string, std::string> constructMapStatus()
{
	static const std::pair<std::string, std::string> statusCodePairs[] = {
		std::make_pair("200", "OK"), //!required
		std::make_pair("201", "Created"),
		std::make_pair("206", "Partial Content"),
		std::make_pair("301", "Moved Permanently"),
		std::make_pair("400", "Bad Request"), //!required
		std::make_pair("401", "Unauthorized"),
		std::make_pair("403", "Forbidden"),
		std::make_pair("404", "Not Found"),
		std::make_pair("405", "Method Not Allowed"),
		std::make_pair("406", "Not Acceptable"),
		std::make_pair("411", "Length Required"),
		std::make_pair("416", "Range Not Satisfiable"),
		std::make_pair("500", "Internal Server Error"), //!required
		std::make_pair("501", "Not Implemented"),
		std::make_pair("502", "Bad Gateway"),
		std::make_pair("503", "Service Unavailable"),
		std::make_pair("505", "HTTP Version Not Supported")
		};

	const static std::map<std::string, std::string> statusCode(statusCodePairs, statusCodePairs + sizeof statusCodePairs / sizeof statusCodePairs[0]);

	return statusCode;
}

std::pair<std::string, std::string> getStatus(std::string statusCode)
{
	//source : https://www.rfc-editor.org/rfc/rfc9110.html#name-status-codes

	std::map<std::string, std::string> statusConversionMap = constructMapStatus();

	#ifdef DEBUG
	assert(statusConversionMap.find("500") != statusConversionMap.end());
	#endif // DEBUG

	std::map<std::string, std::string>::iterator key_pos = statusConversionMap.find(statusCode);
	if (key_pos == statusConversionMap.end())
		return *(statusConversionMap.find("500"));
	else
		return *key_pos;
}

std::string sizeToString(size_t value) {
    std::ostringstream oss;

    oss << value;
    return oss.str();
}
